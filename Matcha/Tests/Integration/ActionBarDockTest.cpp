#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <ostream>

#include "doctest.h"

#include "Matcha/UiNodes/ActionBar/ActionBarNode.h"
#include "Matcha/UiNodes/Shell/Shell.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Shell/WindowNode.h"
#include "Matcha/Widgets/ActionBar/NyanActionBar.h"
#include "Matcha/Widgets/Core/NyanTheme.h"

using matcha::fw::ActionBarNode;
using matcha::fw::Shell;
using matcha::fw::WindowId;
using matcha::fw::WindowKind;
using matcha::fw::WindowNode;
using matcha::gui::DockSide;

TEST_SUITE("ActionBarDock") {

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

TEST_CASE("ActionBarNode dock side switch via Shell") {
    Shell shell;
    ensureTheme();
    auto win = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    win->BuildWindow(nullptr);
    shell.AddNode(std::move(win));

    auto ab = shell.GetActionBar();
    REQUIRE(ab.get() != nullptr);

    ab->SetDockSide(DockSide::Right);
    CHECK(ab->GetDockSide() == DockSide::Right);

    ab->SetDockSide(DockSide::Left);
    CHECK(ab->GetDockSide() == DockSide::Left);
}

TEST_CASE("ActionBarNode collapse/expand round-trip") {
    Shell shell;
    ensureTheme();
    auto win = std::make_unique<WindowNode>("main", WindowId::From(1), WindowKind::Main);
    win->BuildWindow(nullptr);
    shell.AddNode(std::move(win));

    auto ab = shell.GetActionBar();
    REQUIRE(ab.get() != nullptr);

    CHECK(!ab->IsCollapsed());
    ab->SetCollapsed(true);
    CHECK(ab->IsCollapsed());
    ab->SetCollapsed(false);
    CHECK(!ab->IsCollapsed());
}

TEST_CASE("ActionBarNode IsDocked defaults to false") {
    ensureTheme();
    matcha::fw::ActionBarNode ab(nullptr);
    CHECK(!ab.IsDocked());
}

TEST_CASE("ActionBarNode SetDocked round-trip") {
    ensureTheme();
    matcha::fw::ActionBarNode ab(nullptr);
    ab.SetDocked(true);
    CHECK(ab.IsDocked());
    ab.SetDocked(false);
    CHECK(!ab.IsDocked());
}

TEST_CASE("ActionBarNode docked collapse hides widget") {
    ensureTheme();
    matcha::fw::ActionBarNode ab(nullptr);
    ab.SetDocked(true);
    ab.SetCollapsed(true);
    CHECK(ab.IsCollapsed());
    CHECK(!ab.ActionBar()->isVisible());
}

TEST_CASE("ActionBarNode undocked collapse shows mini-button") {
    ensureTheme();
    matcha::fw::ActionBarNode ab(nullptr);
    ab.SetDocked(false);
    ab.SetCollapsed(true);
    CHECK(ab.IsCollapsed());
    CHECK(!ab.ActionBar()->isVisible());
    // Mini-button should exist
    CHECK(ab.ActionBar()->MiniButton() != nullptr);
}

TEST_CASE("ActionBarNode expand after collapse restores visibility") {
    ensureTheme();
    matcha::fw::ActionBarNode ab(nullptr);
    ab.SetDocked(true);
    ab.SetCollapsed(true);
    CHECK(!ab.ActionBar()->isVisible());
    ab.SetCollapsed(false);
    CHECK(ab.ActionBar()->isVisible());
}

TEST_CASE("ActionBarNode CollapsedChanged notification fires") {
    ensureTheme();

    // Parent node that catches notifications via AnalyseNotification
    struct Catcher : matcha::fw::UiNode {
        bool fired = false;
        bool receivedValue = false;
        Catcher() : UiNode("catcher", matcha::fw::NodeType::Container) {}
        auto AnalyseNotification(matcha::CommandNode* /*sender*/,
                                 matcha::Notification& notif) -> matcha::PropagationMode override {
            if (notif.ClassName() == "CollapsedChanged") {
                fired = true;
                receivedValue = dynamic_cast<matcha::fw::CollapsedChanged&>(notif).IsCollapsed();
                return matcha::PropagationMode::DontTransmitToParent;
            }
            return matcha::PropagationMode::TransmitToParent;
        }
    };

    Catcher parent;
    auto abPtr = std::make_unique<ActionBarNode>(nullptr);
    auto* ab = abPtr.get();
    parent.AddNode(std::move(abPtr));

    ab->SetCollapsed(true);
    CHECK(parent.fired);
    CHECK(parent.receivedValue);
}

} // TEST_SUITE
