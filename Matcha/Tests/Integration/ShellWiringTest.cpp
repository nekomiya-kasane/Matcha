#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <ostream>

#include "doctest.h"

#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
#include "Matcha/Tree/Composition/Document/DocumentArea.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"
#include "Matcha/Tree/Composition/Shell/StatusBarNode.h"
#include "Matcha/Tree/Composition/Shell/WindowNode.h"
#include "Matcha/Tree/Composition/Shell/WorkspaceFrame.h"
#include "Matcha/Theming/NyanTheme.h"

using matcha::fw::Shell;
using matcha::fw::WindowKind;
using matcha::fw::WindowNode;
using matcha::fw::WindowId;

TEST_SUITE("ShellWiring") {

static void ensureTheme() {
    static matcha::gui::NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    static bool init = false;
    if (!init) {
        theme.SetTheme(matcha::gui::kThemeLight);
        theme.SetAnimationOverride(0);
        matcha::gui::SetThemeService(&theme);
        init = true;
    }
}

TEST_CASE("Shell with built WindowNode has ActionBar via WorkspaceFrame") {
    ensureTheme();
    Shell shell;
    auto win = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    win->BuildWindow(nullptr);
    shell.AddNode(std::move(win));

    auto ab = shell.GetActionBar();
    CHECK(ab.get() != nullptr);
}

TEST_CASE("Shell with built WindowNode has StatusBar") {
    ensureTheme();
    Shell shell;
    auto win = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    win->BuildWindow(nullptr);
    shell.AddNode(std::move(win));

    auto sb = shell.GetStatusBar();
    CHECK(sb.get() != nullptr);
}

TEST_CASE("Shell with built WindowNode has DocumentArea") {
    ensureTheme();
    Shell shell;
    auto win = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    win->BuildWindow(nullptr);
    shell.AddNode(std::move(win));

    auto da = shell.GetDocumentArea();
    CHECK(da.get() != nullptr);
}

TEST_CASE("Shell MainWindow returns Main kind only") {
    ensureTheme();
    Shell shell;
    auto main = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    main->BuildWindow(nullptr);
    auto floating = std::make_unique<WindowNode>("float", WindowId::From(2), WindowKind::Floating);
    floating->BuildWindow(nullptr);

    shell.AddNode(std::move(main));
    shell.AddNode(std::move(floating));

    auto* mainWin = shell.MainWindow();
    REQUIRE(mainWin != nullptr);
    CHECK(mainWin->Kind() == WindowKind::Main);
    CHECK(mainWin->Id() == WindowId::From(1));
}

TEST_CASE("Shell Windows returns all windows") {
    ensureTheme();
    Shell shell;
    auto main = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    main->BuildWindow(nullptr);
    auto floating = std::make_unique<WindowNode>("float", WindowId::From(2), WindowKind::Floating);
    floating->BuildWindow(nullptr);

    shell.AddNode(std::move(main));
    shell.AddNode(std::move(floating));

    auto windows = shell.Windows();
    CHECK(windows.size() == 2);
}

} // TEST_SUITE
