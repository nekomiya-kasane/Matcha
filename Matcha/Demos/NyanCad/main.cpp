/**
 * @file main.cpp
 * @brief NyanCad demo entry point.
 *
 * Wraps the application in crash diagnostics:
 *   - Windows SEH (__try/__except) around Run()
 *   - SetUnhandledExceptionFilter for out-of-scope crashes
 *   - C++ std::set_terminate for uncaught exceptions
 *   - POSIX signal handlers (SIGSEGV, SIGABRT, SIGFPE, SIGILL)
 */

#include "NyanCadApp.h"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <print>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

// ---- Windows stack trace helper ----

static void PrintStackTrace(CONTEXT* ctx)
{
    auto process = GetCurrentProcess();
    auto thread  = GetCurrentThread();
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    SymInitialize(process, nullptr, TRUE);

    STACKFRAME64 frame{};
#ifdef _M_X64
    frame.AddrPC.Offset    = ctx->Rip;
    frame.AddrFrame.Offset = ctx->Rbp;
    frame.AddrStack.Offset = ctx->Rsp;
    constexpr DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_ARM64)
    frame.AddrPC.Offset    = ctx->Pc;
    frame.AddrFrame.Offset = ctx->Fp;
    frame.AddrStack.Offset = ctx->Sp;
    constexpr DWORD machineType = IMAGE_FILE_MACHINE_ARM64;
#else
    frame.AddrPC.Offset    = ctx->Eip;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrStack.Offset = ctx->Esp;
    constexpr DWORD machineType = IMAGE_FILE_MACHINE_I386;
#endif
    frame.AddrPC.Mode    = AddrModeFlat;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

    constexpr int kMaxFrames = 64;
    alignas(SYMBOL_INFO) char symbolBuf[sizeof(SYMBOL_INFO) + 256]{};
    auto* symbol     = reinterpret_cast<SYMBOL_INFO*>(symbolBuf);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen   = 255;

    IMAGEHLP_LINE64 line{};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    std::println(stderr, "--- Stack Trace ---");
    for (int i = 0; i < kMaxFrames; ++i) {
        if (!StackWalk64(machineType, process, thread, &frame,
                         ctx, nullptr, SymFunctionTableAccess64,
                         SymGetModuleBase64, nullptr)) {
            break;
        }
        if (frame.AddrPC.Offset == 0) break;

        DWORD64 displacement64 = 0;
        DWORD   displacement32 = 0;
        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement64, symbol)) {
            if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &displacement32, &line)) {
                std::println(stderr, "  #{:2d}  {} + 0x{:x}  ({}:{})",
                    i, symbol->Name, displacement64, line.FileName, line.LineNumber);
            } else {
                std::println(stderr, "  #{:2d}  {} + 0x{:x}  [0x{:016x}]",
                    i, symbol->Name, displacement64, frame.AddrPC.Offset);
            }
        } else {
            std::println(stderr, "  #{:2d}  [0x{:016x}]", i, frame.AddrPC.Offset);
        }
    }
    std::println(stderr, "--- End Stack Trace ---");
    SymCleanup(process);
}

static auto ExceptionCodeToString(DWORD code) -> const char*
{
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:      return "EXCEPTION_ACCESS_VIOLATION";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_STACK_OVERFLOW:        return "EXCEPTION_STACK_OVERFLOW";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:   return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:    return "EXCEPTION_INT_DIVIDE_BY_ZERO";
    case EXCEPTION_ILLEGAL_INSTRUCTION:   return "EXCEPTION_ILLEGAL_INSTRUCTION";
    case EXCEPTION_IN_PAGE_ERROR:         return "EXCEPTION_IN_PAGE_ERROR";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
    case 0xE06D7363:                      return "C++ Exception (msvc)";
    default:                              return "UNKNOWN";
    }
}

static LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* info)
{
    std::println(stderr, "\n[CRASH] Unhandled exception: {} (0x{:08x})",
        ExceptionCodeToString(info->ExceptionRecord->ExceptionCode),
        info->ExceptionRecord->ExceptionCode);

    if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
        info->ExceptionRecord->NumberParameters >= 2) {
        auto rw   = info->ExceptionRecord->ExceptionInformation[0];
        auto addr = info->ExceptionRecord->ExceptionInformation[1];
        std::println(stderr, "  {} address: 0x{:016x}",
            (rw == 0 ? "Reading" : rw == 1 ? "Writing" : "DEP violation at"), addr);
    }

    PrintStackTrace(info->ContextRecord);
    return EXCEPTION_CONTINUE_SEARCH; // let debugger catch if attached
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"

static auto RunWithSEH(nyancad::NyanCadApp& app, int argc, char** argv) -> int
{
    __try {
        return app.Run(argc, argv);
    }
    __except (
        [](EXCEPTION_POINTERS* info) -> DWORD {
            std::println(stderr, "\n[CRASH] SEH caught exception: {} (0x{:08x})",
                ExceptionCodeToString(info->ExceptionRecord->ExceptionCode),
                info->ExceptionRecord->ExceptionCode);
            PrintStackTrace(info->ContextRecord);
            return EXCEPTION_EXECUTE_HANDLER;
        }(GetExceptionInformation())
    ) {
        std::println(stderr, "[CRASH] Application terminated by SEH handler.");
        return 1;
    }
}

#pragma clang diagnostic pop

#endif // _WIN32

// ---- Signal handler (cross-platform fallback) ----

static void CrashSignalHandler(int sig)
{
    const char* name = "UNKNOWN";
    switch (sig) {
    case SIGSEGV: name = "SIGSEGV"; break;
    case SIGABRT: name = "SIGABRT"; break;
    case SIGFPE:  name = "SIGFPE";  break;
    case SIGILL:  name = "SIGILL";  break;
    default: break;
    }
    std::println(stderr, "\n[CRASH] Signal {} ({})", sig, name);

#ifdef _WIN32
    // Capture current context for stack trace
    CONTEXT ctx{};
    RtlCaptureContext(&ctx);
    PrintStackTrace(&ctx);
#endif

    // Re-raise with default handler to produce core dump / crash dialog
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

// ---- C++ terminate handler ----

[[noreturn]] static void TerminateHandler()
{
    std::println(stderr, "\n[CRASH] std::terminate called (uncaught exception or noexcept violation)");

    if (auto eptr = std::current_exception()) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            std::println(stderr, "  what(): {}", e.what());
        } catch (...) {
            std::println(stderr, "  (non-std::exception type)");
        }
    }

#ifdef _WIN32
    CONTEXT ctx{};
    RtlCaptureContext(&ctx);
    PrintStackTrace(&ctx);
#endif

    std::abort();
}

// ---- Entry point ----

auto main(int argc, char** argv) -> int
{
    // Install crash handlers
    std::signal(SIGSEGV, CrashSignalHandler);
    std::signal(SIGABRT, CrashSignalHandler);
    std::signal(SIGFPE,  CrashSignalHandler);
    std::signal(SIGILL,  CrashSignalHandler);
    std::set_terminate(TerminateHandler);

#ifdef _WIN32
    SetUnhandledExceptionFilter(UnhandledExceptionHandler);
#endif

    nyancad::NyanCadApp app;

#ifdef _WIN32
    return RunWithSEH(app, argc, argv);
#else
    return app.Run(argc, argv);
#endif
}
