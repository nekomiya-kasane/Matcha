#pragma once

#include <Matcha/Services/IExpansionPlugin.h>

#include <atomic>

namespace matcha::test {

class MockPlugin : public matcha::fw::IExpansionPlugin {
public:
    static std::atomic<int> startCount;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    static std::atomic<int> stopCount;   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    static matcha::fw::Shell* lastShell; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    [[nodiscard]] auto Id() const -> std::string_view override { return "mock-plugin"; }

    [[nodiscard]] auto Start(matcha::fw::Shell& shell) -> matcha::fw::Expected<void> override
    {
        ++startCount;
        lastShell = &shell;
        return {};
    }

    void Stop() override
    {
        ++stopCount;
    }
};

} // namespace matcha::test
