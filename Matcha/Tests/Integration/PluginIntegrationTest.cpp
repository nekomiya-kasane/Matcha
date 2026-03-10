#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Services/PluginHost.h>
#include <Matcha/UiNodes/ActionBar/ActionBarNode.h>
#include <Matcha/UiNodes/Document/DocumentArea.h>
#include <Matcha/Services/IExpansionPlugin.h>
#include <Matcha/UiNodes/Shell/Shell.h>
#include <Matcha/UiNodes/Shell/StatusBarNode.h>
#include <Matcha/UiNodes/Shell/WindowNode.h>
#include <Matcha/UiNodes/Shell/WorkspaceFrame.h>
#include <Matcha/Widgets/Core/NyanTheme.h>

#include <string>

using namespace matcha::fw;

#ifndef MATCHA_TEST_MOCK_PLUGIN_PATH
#define MATCHA_TEST_MOCK_PLUGIN_PATH ""
#endif

static const std::string kMockPluginPath = MATCHA_TEST_MOCK_PLUGIN_PATH; // NOLINT

// ============================================================================
// Helper: build a Shell with a WindowNode (like a real application)
// ============================================================================

namespace {

void ensureTheme()
{
    static matcha::gui::NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    static bool init = false;
    if (!init) {
        theme.SetTheme(matcha::gui::kThemeLight);
        theme.SetAnimationOverride(0);
        matcha::gui::SetThemeService(&theme);
        init = true;
    }
}

void BuildShellWithWindow(Shell& shell)
{
    ensureTheme();
    auto win = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    win->BuildWindow(nullptr);
    shell.AddNode(std::move(win));
}

} // anonymous namespace

// ============================================================================
// Plugin receives Shell& and can access UiNode tree
// ============================================================================

TEST_CASE("Plugin: Start receives Shell reference") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    auto plugin = host.QueryPlugin("mock-plugin");
    REQUIRE(plugin);
    CHECK(plugin->Id() == "mock-plugin");

    host.StopAll();
}

TEST_CASE("Plugin: can access ActionBar via Shell") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    // Verify Shell tree is accessible -- ActionBar exists via MainWindow
    auto actionBar = shell.GetActionBar();
    CHECK(actionBar);

    host.StopAll();
}

TEST_CASE("Plugin: can access StatusBar via Shell") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    auto statusBar = shell.GetStatusBar();
    CHECK(statusBar);

    host.StopAll();
}

TEST_CASE("Plugin: can access DocumentArea via Shell") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    auto docArea = shell.GetDocumentArea();
    CHECK(docArea);

    host.StopAll();
}

TEST_CASE("Plugin: can access MainWindow via Shell") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    auto* mainWin = shell.MainWindow();
    CHECK(mainWin != nullptr);

    host.StopAll();
}

// ============================================================================
// StopAll lifecycle
// ============================================================================

TEST_CASE("Plugin: StopAll calls Stop on loaded plugin") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());
    CHECK(host.PluginCount() == 1);

    host.StopAll();
    CHECK(host.PluginCount() == 0);
}

TEST_CASE("Plugin: StopAll on empty host is no-op") {
    PluginHost host;
    host.StopAll(); // Should not crash
    CHECK(host.PluginCount() == 0);
}

// ============================================================================
// Multiple operations
// ============================================================================

TEST_CASE("Plugin: load then stop then reload") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto r1 = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(r1.has_value());

    auto stopResult = host.StopPlugin("mock-plugin");
    CHECK(stopResult.has_value());
    CHECK(host.PluginCount() == 0);

    // Reload same plugin after stop
    auto r2 = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(r2.has_value());
    CHECK(r2.value() == "mock-plugin");
    CHECK(host.PluginCount() == 1);

    host.StopAll();
}

TEST_CASE("Plugin: QueryPlugin after StopAll returns nullptr") {
    Shell shell;
    BuildShellWithWindow(shell);
    PluginHost host;

    auto result = host.LoadPlugin(kMockPluginPath, shell);
    REQUIRE(result.has_value());

    host.StopAll();
    auto plugin = host.QueryPlugin("mock-plugin");
    CHECK_FALSE(plugin);
}
