#include "MockPlugin.h"

#ifdef _MSC_VER
#define MOCK_PLUGIN_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define MOCK_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define MOCK_PLUGIN_EXPORT
#endif

namespace matcha::test {

std::atomic<int> MockPlugin::startCount {0};  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<int> MockPlugin::stopCount {0};   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
matcha::fw::Shell* MockPlugin::lastShell = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

} // namespace matcha::test

extern "C" MOCK_PLUGIN_EXPORT auto CreateExpansionPlugin() -> matcha::fw::IExpansionPlugin* // NOLINT(readability-identifier-naming)
{
    // Reset counters each time a new plugin instance is created
    matcha::test::MockPlugin::startCount = 0;
    matcha::test::MockPlugin::stopCount  = 0;
    matcha::test::MockPlugin::lastShell  = nullptr;
    return new matcha::test::MockPlugin(); // NOLINT(cppcoreguidelines-owning-memory)
}
