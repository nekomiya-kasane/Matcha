#include <Matcha/Services/PluginHost.h>

#include <Matcha/Services/IExpansionPlugin.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace matcha::fw {

namespace {

// ============================================================================
// Platform library handle abstraction
// ============================================================================

#ifdef _WIN32
using LibHandle = HMODULE;
#else
using LibHandle = void*;
#endif

auto PlatformLoadLibrary(const std::string& path) -> LibHandle
{
#ifdef _WIN32
    // Convert UTF-8 path to wide string
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
    if (wideLen <= 0) { return nullptr; }
    std::wstring widePath(static_cast<size_t>(wideLen), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, widePath.data(), wideLen);
    return LoadLibraryW(widePath.c_str());
#else
    return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL); // NOLINT(hicpp-signed-bitwise)
#endif
}

auto PlatformGetSymbol(LibHandle handle, const char* name) -> void*
{
#ifdef _WIN32
    return reinterpret_cast<void*>(GetProcAddress(handle, name)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
#else
    return dlsym(handle, name);
#endif
}

struct PluginRecord {
    LibHandle                         handle = nullptr;
    std::unique_ptr<IExpansionPlugin> plugin;
    bool                              started = false;
};

} // anonymous namespace

struct PluginHost::Impl {
    std::unordered_map<std::string, PluginRecord> plugins;
    std::vector<std::string>                      loadOrder;
};

// ============================================================================
// PluginHost lifecycle
// ============================================================================

PluginHost::PluginHost() : _impl(std::make_unique<Impl>()) {}

PluginHost::~PluginHost()
{
    StopAll();
}

// ============================================================================
// LoadPlugin
// ============================================================================

using FactoryFunc = IExpansionPlugin* (*)();

auto PluginHost::LoadPlugin(std::string_view libraryPath, Shell& shell)
    -> Expected<std::string>
{
    std::string pathStr(libraryPath);

    // Load shared library
    LibHandle handle = PlatformLoadLibrary(pathStr);
    if (handle == nullptr) {
        return std::unexpected(ErrorCode::PluginLoadFailed);
    }

    // Resolve factory symbol
    auto* factoryRaw = PlatformGetSymbol(handle, "CreateExpansionPlugin");
    if (factoryRaw == nullptr) {
        return std::unexpected(ErrorCode::PluginLoadFailed);
    }

    auto factory = reinterpret_cast<FactoryFunc>(factoryRaw); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    IExpansionPlugin* rawPlugin = factory();
    if (rawPlugin == nullptr) {
        return std::unexpected(ErrorCode::PluginLoadFailed);
    }

    auto plugin = std::unique_ptr<IExpansionPlugin>(rawPlugin);
    std::string pluginId(plugin->Id());

    // Check for duplicate
    if (_impl->plugins.contains(pluginId)) {
        return std::unexpected(ErrorCode::AlreadyExists);
    }

    // Start the plugin
    auto startResult = plugin->Start(shell);
    if (!startResult.has_value()) {
        return std::unexpected(startResult.error());
    }

    // Store
    PluginRecord record;
    record.handle  = handle;
    record.plugin  = std::move(plugin);
    record.started = true;

    _impl->plugins.emplace(pluginId, std::move(record));
    _impl->loadOrder.push_back(pluginId);

    return pluginId;
}

// ============================================================================
// LoadPluginsFromDirectory
// ============================================================================

auto PluginHost::LoadPluginsFromDirectory(
    std::string_view directoryPath, Shell& shell)
    -> Expected<std::vector<std::string>>
{
    namespace fs = std::filesystem;

    std::error_code ec;
    fs::path dirPath(directoryPath);
    if (!fs::is_directory(dirPath, ec)) {
        return std::unexpected(ErrorCode::NotFound);
    }

    // Platform-specific shared library extension
#ifdef _WIN32
    constexpr std::string_view kExtension = ".dll";
#elif defined(__APPLE__)
    constexpr std::string_view kExtension = ".dylib";
#else
    constexpr std::string_view kExtension = ".so";
#endif

    std::vector<std::string> loadedIds;

    for (const auto& entry : fs::directory_iterator(dirPath, ec)) {
        if (!entry.is_regular_file(ec)) { continue; }
        auto ext = entry.path().extension().string();
        if (ext != kExtension) { continue; }

        auto result = LoadPlugin(entry.path().string(), shell);
        if (result.has_value()) {
            loadedIds.push_back(std::move(result.value()));
        }
        // Individual plugin load failures are silently skipped
    }

    return loadedIds;
}

// ============================================================================
// StopPlugin / StopAll
// ============================================================================

auto PluginHost::StopPlugin(std::string_view pluginId) -> Expected<void>
{
    std::string idStr(pluginId);
    auto it = _impl->plugins.find(idStr);
    if (it == _impl->plugins.end()) {
        return std::unexpected(ErrorCode::NotFound);
    }

    auto& record = it->second;
    if (record.started) {
        record.plugin->Stop();
        record.started = false;
    }

    // Remove from load order
    std::erase(_impl->loadOrder, idStr);
    _impl->plugins.erase(it);

    return {};
}

void PluginHost::StopAll()
{
    // Stop in reverse load order
    auto order = _impl->loadOrder; // copy -- StopPlugin modifies loadOrder
    std::ranges::reverse(order);
    for (const auto& id : order) {
        auto result = StopPlugin(id);
        (void)result;
    }
}

// ============================================================================
// Query
// ============================================================================

auto PluginHost::QueryPlugin(std::string_view pluginId) const
    -> observer_ptr<IExpansionPlugin>
{
    std::string idStr(pluginId);
    auto it = _impl->plugins.find(idStr);
    if (it == _impl->plugins.end()) {
        return observer_ptr<IExpansionPlugin>(nullptr);
    }
    return observer_ptr<IExpansionPlugin>(it->second.plugin.get());
}

auto PluginHost::PluginCount() const -> size_t
{
    return _impl->plugins.size();
}

} // namespace matcha::fw
