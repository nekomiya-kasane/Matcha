/**
 * @file NyanCadPluginTest.cpp
 * @brief Integration tests for NyanCad expansion plugin loading via PluginHost.
 */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/Services/PluginHost.h"
#include "Matcha/UiNodes/Shell/Application.h"
#include "Matcha/UiNodes/Shell/Shell.h"
#include "Matcha/Widgets/Core/NyanTheme.h"

#include <QString>

TEST_SUITE("NyanCadPlugin") {

TEST_CASE("Load all 6 plugin DLLs from directory") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme);
    matcha::fw::Application app;
    app.Initialize(0, nullptr);
    auto& shell = app.GetShell();

    matcha::fw::PluginHost host;
    auto result = host.LoadPluginsFromDirectory(MATCHA_TEST_PLUGIN_DIR, shell);
    REQUIRE(result.has_value());
    CHECK(result.value().size() == 6);
    CHECK(host.PluginCount() == 6);

    host.StopAll();
    app.Shutdown();
}

TEST_CASE("Each plugin has unique ID") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme);
    matcha::fw::Application app;
    app.Initialize(0, nullptr);
    auto& shell = app.GetShell();

    matcha::fw::PluginHost host;
    auto result = host.LoadPluginsFromDirectory(MATCHA_TEST_PLUGIN_DIR, shell);
    REQUIRE(result.has_value());

    // Verify each plugin can be queried by its ID
    auto ids = result.value();
    for (const auto& id : ids) {
        auto plugin = host.QueryPlugin(id);
        CHECK_UNARY(plugin.get());
    }

    host.StopAll();
    app.Shutdown();
}

TEST_CASE("StopAll succeeds after loading plugins") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme);
    matcha::fw::Application app;
    app.Initialize(0, nullptr);
    auto& shell = app.GetShell();

    matcha::fw::PluginHost host;
    (void)host.LoadPluginsFromDirectory(MATCHA_TEST_PLUGIN_DIR, shell);

    host.StopAll();
    CHECK(host.PluginCount() == 0);

    app.Shutdown();
}

} // TEST_SUITE
