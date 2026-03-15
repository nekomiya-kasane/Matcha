#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Interaction/Focus/MnemonicManager.h>

#include <memory>

using namespace matcha::fw;

// ============================================================================
// Basic registration and dispatch
// ============================================================================

TEST_SUITE("MnemonicManager") {

TEST_CASE("Default active scope is Global") {
    MnemonicManager mgr;
    CHECK(mgr.ActiveScope() == MnemonicScope::Global);
}

TEST_CASE("Register and dispatch in Global scope") {
    MnemonicManager mgr;
    int callCount = 0;

    mgr.Register({MnemonicScope::Global, u'F', [&]() { ++callCount; }, {}});

    CHECK(mgr.Dispatch(u'F'));
    CHECK(callCount == 1);
}

TEST_CASE("Dispatch is case-insensitive") {
    MnemonicManager mgr;
    int callCount = 0;

    mgr.Register({MnemonicScope::Global, u'f', [&]() { ++callCount; }, {}});

    CHECK(mgr.Dispatch(u'F'));
    CHECK(callCount == 1);

    CHECK(mgr.Dispatch(u'f'));
    CHECK(callCount == 2);
}

TEST_CASE("Dispatch returns false for unmatched character") {
    MnemonicManager mgr;
    int callCount = 0;

    mgr.Register({MnemonicScope::Global, u'F', [&]() { ++callCount; }, {}});

    CHECK_FALSE(mgr.Dispatch(u'X'));
    CHECK(callCount == 0);
}

TEST_CASE("Registration in wrong scope is not dispatched") {
    MnemonicManager mgr;
    int callCount = 0;

    mgr.Register({MnemonicScope::Menu, u'S', [&]() { ++callCount; }, {}});

    // Active scope is Global, registration is Menu -> should not dispatch
    CHECK_FALSE(mgr.Dispatch(u'S'));
    CHECK(callCount == 0);
}

// ============================================================================
// Scope stack
// ============================================================================

TEST_CASE("PushScope changes active scope") {
    MnemonicManager mgr;
    mgr.PushScope(MnemonicScope::Menu);
    CHECK(mgr.ActiveScope() == MnemonicScope::Menu);

    mgr.PushScope(MnemonicScope::Dialog);
    CHECK(mgr.ActiveScope() == MnemonicScope::Dialog);
}

TEST_CASE("PopScope reverts to previous scope") {
    MnemonicManager mgr;
    mgr.PushScope(MnemonicScope::Menu);
    mgr.PushScope(MnemonicScope::Dialog);

    mgr.PopScope();
    CHECK(mgr.ActiveScope() == MnemonicScope::Menu);

    mgr.PopScope();
    CHECK(mgr.ActiveScope() == MnemonicScope::Global);
}

TEST_CASE("PopScope on empty stack is no-op") {
    MnemonicManager mgr;
    mgr.PopScope(); // should not crash
    CHECK(mgr.ActiveScope() == MnemonicScope::Global);
}

TEST_CASE("Menu-scope registration dispatches after PushScope(Menu)") {
    MnemonicManager mgr;
    int menuCall = 0;
    int globalCall = 0;

    mgr.Register({MnemonicScope::Global, u'F', [&]() { ++globalCall; }, {}});
    mgr.Register({MnemonicScope::Menu, u'S', [&]() { ++menuCall; }, {}});

    // Global scope: only 'F' dispatches
    CHECK(mgr.Dispatch(u'F'));
    CHECK(globalCall == 1);
    CHECK_FALSE(mgr.Dispatch(u'S'));
    CHECK(menuCall == 0);

    // Push Menu scope: only 'S' dispatches
    mgr.PushScope(MnemonicScope::Menu);
    CHECK(mgr.Dispatch(u'S'));
    CHECK(menuCall == 1);
    CHECK_FALSE(mgr.Dispatch(u'F'));
    CHECK(globalCall == 1); // unchanged

    // Pop back to Global
    mgr.PopScope();
    CHECK(mgr.Dispatch(u'F'));
    CHECK(globalCall == 2);
}

// ============================================================================
// Unregister
// ============================================================================

TEST_CASE("Unregister removes registration") {
    MnemonicManager mgr;
    int callCount = 0;

    auto id = mgr.Register({MnemonicScope::Global, u'F', [&]() { ++callCount; }, {}});
    CHECK(mgr.Dispatch(u'F'));
    CHECK(callCount == 1);

    mgr.Unregister(id);
    CHECK_FALSE(mgr.Dispatch(u'F'));
    CHECK(callCount == 1);
}

// ============================================================================
// ClearScope
// ============================================================================

TEST_CASE("ClearScope removes all registrations in scope") {
    MnemonicManager mgr;
    int menuA = 0;
    int menuB = 0;
    int globalF = 0;

    mgr.Register({MnemonicScope::Global, u'F', [&]() { ++globalF; }, {}});
    mgr.Register({MnemonicScope::Menu, u'A', [&]() { ++menuA; }, {}});
    mgr.Register({MnemonicScope::Menu, u'B', [&]() { ++menuB; }, {}});

    mgr.ClearScope(MnemonicScope::Menu);

    // Menu registrations gone
    mgr.PushScope(MnemonicScope::Menu);
    CHECK_FALSE(mgr.Dispatch(u'A'));
    CHECK_FALSE(mgr.Dispatch(u'B'));

    // Global registration still exists
    mgr.PopScope();
    CHECK(mgr.Dispatch(u'F'));
    CHECK(globalF == 1);
}

// ============================================================================
// Reset
// ============================================================================

TEST_CASE("Reset clears everything") {
    MnemonicManager mgr;
    mgr.Register({MnemonicScope::Global, u'F', []() {}, {}});
    mgr.PushScope(MnemonicScope::Dialog);

    mgr.Reset();

    CHECK(mgr.ActiveScope() == MnemonicScope::Global);
    CHECK_FALSE(mgr.Dispatch(u'F'));
}

// ============================================================================
// Lifetime guard (aliveToken)
// ============================================================================

TEST_CASE("Expired aliveToken skips registration") {
    MnemonicManager mgr;
    int callCount = 0;

    auto token = std::make_shared<int>(42);
    std::weak_ptr<void> weak = token;

    mgr.Register({MnemonicScope::Global, u'F', [&]() { ++callCount; }, weak});

    // While alive: dispatch works
    CHECK(mgr.Dispatch(u'F'));
    CHECK(callCount == 1);

    // Destroy the owner
    token.reset();

    // Token expired: dispatch should skip
    CHECK_FALSE(mgr.Dispatch(u'F'));
    CHECK(callCount == 1);
}

TEST_CASE("Empty aliveToken (no guard) always dispatches") {
    MnemonicManager mgr;
    int callCount = 0;

    mgr.Register({MnemonicScope::Global, u'F', [&]() { ++callCount; }, {}});

    CHECK(mgr.Dispatch(u'F'));
    CHECK(mgr.Dispatch(u'F'));
    CHECK(callCount == 2);
}

// ============================================================================
// RegistrationCount
// ============================================================================

TEST_CASE("RegistrationCount counts live entries") {
    MnemonicManager mgr;

    auto token = std::make_shared<int>(1);
    std::weak_ptr<void> weak = token;

    mgr.Register({MnemonicScope::Global, u'F', []() {}, {}});
    mgr.Register({MnemonicScope::Global, u'E', []() {}, weak});
    mgr.Register({MnemonicScope::Menu, u'S', []() {}, {}});

    CHECK(mgr.RegistrationCount(MnemonicScope::Global) == 2);
    CHECK(mgr.RegistrationCount(MnemonicScope::Menu) == 1);
    CHECK(mgr.RegistrationCount(MnemonicScope::Dialog) == 0);

    // Expire one
    token.reset();
    CHECK(mgr.RegistrationCount(MnemonicScope::Global) == 1);
}

// ============================================================================
// Global accessor
// ============================================================================

TEST_CASE("Global accessor default is nullptr") {
    auto* saved = GetMnemonicManager();
    SetMnemonicManager(nullptr);

    CHECK(GetMnemonicManager() == nullptr);
    CHECK_FALSE(HasMnemonicManager());

    SetMnemonicManager(saved);
}

TEST_CASE("Global accessor set and get") {
    auto* saved = GetMnemonicManager();

    MnemonicManager mgr;
    SetMnemonicManager(&mgr);
    CHECK(GetMnemonicManager() == &mgr);
    CHECK(HasMnemonicManager());

    SetMnemonicManager(saved);
}

} // TEST_SUITE
