#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Foundation/BreakpointObserver.h>

#include <vector>

using namespace matcha::fw;

TEST_SUITE("fw::BreakpointObserver") {

// ============================================================================
// Basic
// ============================================================================

TEST_CASE("Default state: no rules, no states") {
    BreakpointObserver obs;
    CHECK(obs.RuleCount() == 0);
    CHECK(obs.States().empty());
}

TEST_CASE("Add rule increases count") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 1000, .action = CollapseAction::AutoHide});
    CHECK(obs.RuleCount() == 1);
}

TEST_CASE("Evaluate above threshold -> not collapsed") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 1000, .action = CollapseAction::AutoHide});
    const auto states = obs.Evaluate(1200);
    REQUIRE(states.size() == 1);
    CHECK_FALSE(states[0].collapsed);
    CHECK(states[0].elementId == "panel");
}

TEST_CASE("Evaluate below threshold -> collapsed") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 1000, .action = CollapseAction::AutoHide});
    const auto states = obs.Evaluate(900);
    REQUIRE(states.size() == 1);
    CHECK(states[0].collapsed);
}

TEST_CASE("Evaluate at exact threshold -> not collapsed") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 1000, .action = CollapseAction::AutoHide});
    const auto states = obs.Evaluate(1000);
    REQUIRE(states.size() == 1);
    CHECK_FALSE(states[0].collapsed);
}

// ============================================================================
// §6.3.1 Priority matrix
// ============================================================================

TEST_CASE("Priority matrix from §6.3.1") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "propertyPanel", .threshold = 1000, .action = CollapseAction::AutoHide});
    obs.AddRule({.priority = 2, .elementId = "actionBarLabels", .threshold = 600, .action = CollapseAction::IconOnly});
    obs.AddRule({.priority = 3, .elementId = "statusBarCenter", .threshold = 800, .action = CollapseAction::HideSection});
    obs.AddRule({.priority = 4, .elementId = "menuBarOverflow", .threshold = 500, .action = CollapseAction::OverflowMenu});

    SUBCASE("Wide window (1200px) -> nothing collapsed") {
        obs.Evaluate(1200);
        CHECK_FALSE(obs.IsCollapsed("propertyPanel"));
        CHECK_FALSE(obs.IsCollapsed("actionBarLabels"));
        CHECK_FALSE(obs.IsCollapsed("statusBarCenter"));
        CHECK_FALSE(obs.IsCollapsed("menuBarOverflow"));
    }

    SUBCASE("900px -> only propertyPanel collapsed") {
        obs.Evaluate(900);
        CHECK(obs.IsCollapsed("propertyPanel"));
        CHECK_FALSE(obs.IsCollapsed("actionBarLabels"));
        CHECK_FALSE(obs.IsCollapsed("statusBarCenter"));
        CHECK_FALSE(obs.IsCollapsed("menuBarOverflow"));
    }

    SUBCASE("750px -> propertyPanel + statusBarCenter collapsed") {
        obs.Evaluate(750);
        CHECK(obs.IsCollapsed("propertyPanel"));
        CHECK_FALSE(obs.IsCollapsed("actionBarLabels"));
        CHECK(obs.IsCollapsed("statusBarCenter"));
        CHECK_FALSE(obs.IsCollapsed("menuBarOverflow"));
    }

    SUBCASE("550px -> propertyPanel + actionBarLabels + statusBarCenter") {
        obs.Evaluate(550);
        CHECK(obs.IsCollapsed("propertyPanel"));
        CHECK(obs.IsCollapsed("actionBarLabels"));
        CHECK(obs.IsCollapsed("statusBarCenter"));
        CHECK_FALSE(obs.IsCollapsed("menuBarOverflow"));
    }

    SUBCASE("400px -> all collapsed") {
        obs.Evaluate(400);
        CHECK(obs.IsCollapsed("propertyPanel"));
        CHECK(obs.IsCollapsed("actionBarLabels"));
        CHECK(obs.IsCollapsed("statusBarCenter"));
        CHECK(obs.IsCollapsed("menuBarOverflow"));
    }
}

// ============================================================================
// Callback
// ============================================================================

TEST_CASE("Callback fires on state change") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 1000, .action = CollapseAction::AutoHide});

    int callCount = 0;
    obs.OnChanged([&](const std::vector<CollapseState>&) { ++callCount; });

    obs.Evaluate(1200);  // initial, panel not collapsed -> no change from default
    obs.Evaluate(1200);  // same width -> no fire
    obs.Evaluate(900);   // panel collapses -> fires
    obs.Evaluate(900);   // same -> no fire
    obs.Evaluate(1100);  // panel uncollapsed -> fires

    CHECK(callCount == 2);
}

TEST_CASE("Callback does not fire when width changes but states don't") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 500, .action = CollapseAction::AutoHide});

    int callCount = 0;
    obs.OnChanged([&](const std::vector<CollapseState>&) { ++callCount; });

    obs.Evaluate(1000);
    obs.Evaluate(800);   // both above 500
    obs.Evaluate(600);   // still above 500

    CHECK(callCount == 0);
}

// ============================================================================
// Remove / Clear
// ============================================================================

TEST_CASE("RemoveRules removes element") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "a", .threshold = 1000, .action = CollapseAction::AutoHide});
    obs.AddRule({.priority = 2, .elementId = "b", .threshold = 800, .action = CollapseAction::IconOnly});
    obs.RemoveRules("a");
    CHECK(obs.RuleCount() == 1);
    CHECK(obs.States().size() == 1);
    CHECK(obs.States()[0].elementId == "b");
}

TEST_CASE("ClearRules removes all") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "a", .threshold = 1000, .action = CollapseAction::AutoHide});
    obs.AddRule({.priority = 2, .elementId = "b", .threshold = 800, .action = CollapseAction::IconOnly});
    obs.ClearRules();
    CHECK(obs.RuleCount() == 0);
    CHECK(obs.States().empty());
}

// ============================================================================
// IsCollapsed for unknown element
// ============================================================================

TEST_CASE("IsCollapsed returns false for unknown element") {
    BreakpointObserver obs;
    CHECK_FALSE(obs.IsCollapsed("nonexistent"));
}

// ============================================================================
// Hysteresis: toggle back and forth
// ============================================================================

TEST_CASE("Collapse and restore cycle") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 1000, .action = CollapseAction::AutoHide});

    obs.Evaluate(1200);
    CHECK_FALSE(obs.IsCollapsed("panel"));

    obs.Evaluate(800);
    CHECK(obs.IsCollapsed("panel"));

    obs.Evaluate(1200);
    CHECK_FALSE(obs.IsCollapsed("panel"));

    obs.Evaluate(999);
    CHECK(obs.IsCollapsed("panel"));

    obs.Evaluate(1000);
    CHECK_FALSE(obs.IsCollapsed("panel"));
}

// ============================================================================
// Height-based breakpoints (Fix #7)
// ============================================================================

TEST_CASE("Height-based breakpoint rule") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "statusBar", .threshold = 600,
                 .action = CollapseAction::AutoHide, .axis = BreakpointAxis::Height});

    obs.Evaluate(1200, 700);
    CHECK_FALSE(obs.IsCollapsed("statusBar"));

    obs.Evaluate(1200, 500);
    CHECK(obs.IsCollapsed("statusBar"));
}

TEST_CASE("Mixed width + height rules") {
    BreakpointObserver obs;
    obs.AddRule({.priority = 1, .elementId = "panel", .threshold = 1000,
                 .action = CollapseAction::AutoHide, .axis = BreakpointAxis::Width});
    obs.AddRule({.priority = 2, .elementId = "statusBar", .threshold = 600,
                 .action = CollapseAction::HideSection, .axis = BreakpointAxis::Height});

    obs.Evaluate(1200, 700);
    CHECK_FALSE(obs.IsCollapsed("panel"));
    CHECK_FALSE(obs.IsCollapsed("statusBar"));

    obs.Evaluate(900, 700);
    CHECK(obs.IsCollapsed("panel"));
    CHECK_FALSE(obs.IsCollapsed("statusBar"));

    obs.Evaluate(900, 500);
    CHECK(obs.IsCollapsed("panel"));
    CHECK(obs.IsCollapsed("statusBar"));
}

} // TEST_SUITE
