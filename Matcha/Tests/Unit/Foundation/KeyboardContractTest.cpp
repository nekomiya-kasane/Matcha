#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file KeyboardContractTest.cpp
 * @brief Unit tests for KeyboardContract registry and built-in contracts (E4).
 */

#include "doctest.h"

#include <Matcha/Foundation/KeyboardContract.h>

using namespace matcha::fw;

TEST_SUITE("KeyboardContract") {

// ============================================================================
// Registry basics
// ============================================================================

TEST_CASE("Register and Get") {
    KeyboardContractRegistry reg;
    reg.Register(BuildPushButtonContract());

    const auto* c = reg.Get("PushButton");
    REQUIRE(c != nullptr);
    CHECK(c->widgetKind == "PushButton");
    CHECK(!c->bindings.empty());
}

TEST_CASE("Get returns nullptr for unknown") {
    KeyboardContractRegistry reg;
    CHECK(reg.Get("Unknown") == nullptr);
}

TEST_CASE("Has") {
    KeyboardContractRegistry reg;
    reg.Register(BuildPushButtonContract());
    CHECK(reg.Has("PushButton"));
    CHECK_FALSE(reg.Has("Unknown"));
}

TEST_CASE("RegisteredKinds") {
    KeyboardContractRegistry reg;
    reg.Register(BuildPushButtonContract());
    reg.Register(BuildLineEditContract());

    auto kinds = reg.RegisteredKinds();
    CHECK(kinds.size() == 2);
}

TEST_CASE("TotalBindingCount") {
    KeyboardContractRegistry reg;
    reg.Register(BuildPushButtonContract());
    reg.Register(BuildLineEditContract());
    reg.Register(BuildComboBoxContract());
    reg.Register(BuildSpinBoxContract());

    CHECK(reg.TotalBindingCount() > 20);
}

TEST_CASE("Clear") {
    KeyboardContractRegistry reg;
    reg.Register(BuildPushButtonContract());
    reg.Clear();
    CHECK_FALSE(reg.Has("PushButton"));
    CHECK(reg.TotalBindingCount() == 0);
}

// ============================================================================
// Built-in contract content validation
// ============================================================================

TEST_CASE("PushButton contract has Space and Enter") {
    auto c = BuildPushButtonContract();
    CHECK(c.widgetKind == "PushButton");
    REQUIRE(c.bindings.size() == 5);

    bool hasSpace = false;
    bool hasEnter = false;
    for (const auto& b : c.bindings) {
        if (b.key == "Space") { hasSpace = true; }
        if (b.key == "Enter") { hasEnter = true; }
    }
    CHECK(hasSpace);
    CHECK(hasEnter);
}

TEST_CASE("LineEdit contract has Ctrl+A, Ctrl+C, Ctrl+V") {
    auto c = BuildLineEditContract();
    CHECK(c.widgetKind == "LineEdit");
    REQUIRE(c.bindings.size() == 17);

    bool hasCtrlA = false;
    bool hasCtrlC = false;
    bool hasCtrlV = false;
    for (const auto& b : c.bindings) {
        if (b.key == "Ctrl+A") { hasCtrlA = true; }
        if (b.key == "Ctrl+C") { hasCtrlC = true; }
        if (b.key == "Ctrl+V") { hasCtrlV = true; }
    }
    CHECK(hasCtrlA);
    CHECK(hasCtrlC);
    CHECK(hasCtrlV);
}

TEST_CASE("ComboBox contract has Escape and Alt+Down") {
    auto c = BuildComboBoxContract();
    CHECK(c.widgetKind == "ComboBox");
    REQUIRE(c.bindings.size() == 10);

    bool hasEscape = false;
    bool hasAltDown = false;
    for (const auto& b : c.bindings) {
        if (b.key == "Escape") { hasEscape = true; }
        if (b.key == "Alt+Down") { hasAltDown = true; }
    }
    CHECK(hasEscape);
    CHECK(hasAltDown);
}

TEST_CASE("SpinBox contract has Up/Down/PageUp/PageDown") {
    auto c = BuildSpinBoxContract();
    CHECK(c.widgetKind == "SpinBox");
    REQUIRE(c.bindings.size() == 8);

    bool hasUp = false;
    bool hasPageUp = false;
    for (const auto& b : c.bindings) {
        if (b.key == "Up") { hasUp = true; }
        if (b.key == "Page Up") { hasPageUp = true; }
    }
    CHECK(hasUp);
    CHECK(hasPageUp);
}

} // TEST_SUITE
