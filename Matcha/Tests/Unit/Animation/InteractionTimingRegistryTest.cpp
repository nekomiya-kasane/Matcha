#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file InteractionTimingRegistryTest.cpp
 * @brief Unit tests for InteractionTimingRegistry (§8.7).
 */

#include "doctest.h"

#include <Matcha/Animation/InteractionTimingRegistry.h>

using namespace matcha::fw;
using Ms = std::chrono::milliseconds;

TEST_SUITE("InteractionTimingRegistry") {

// ============================================================================
// Default values (§8.7.1)
// ============================================================================

TEST_CASE("Default: HoverDelay is 200ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::HoverDelay) == Ms{200});
}

TEST_CASE("Default: TooltipDelay is 500ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::TooltipDelay) == Ms{500});
}

TEST_CASE("Default: TooltipDismissDelay is 100ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::TooltipDismissDelay) == Ms{100});
}

TEST_CASE("Default: LongPressThreshold is 500ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::LongPressThreshold) == Ms{500});
}

TEST_CASE("Default: DoubleClickWindow is 400ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::DoubleClickWindow) == Ms{400});
}

TEST_CASE("Default: DebounceSearch is 300ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::DebounceSearch) == Ms{300});
}

TEST_CASE("Default: DebounceResize is 100ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::DebounceResize) == Ms{100});
}

TEST_CASE("Default: AutoSaveInterval is 30000ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::AutoSaveInterval) == Ms{30000});
}

TEST_CASE("Default: IdleTimeout is 60000ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::IdleTimeout) == Ms{60000});
}

TEST_CASE("Default: RepeatKeyInitial is 500ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::RepeatKeyInitial) == Ms{500});
}

TEST_CASE("Default: RepeatKeyInterval is 33ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::RepeatKeyInterval) == Ms{33});
}

TEST_CASE("Default: DragInitDelay is 150ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::DragInitDelay) == Ms{150});
}

TEST_CASE("Default: ToastDismissTimeout is 5000ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::ToastDismissTimeout) == Ms{5000});
}

TEST_CASE("Default: MenuOpenDelay is 200ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::MenuOpenDelay) == Ms{200});
}

TEST_CASE("Default: MenuCloseDelay is 300ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::MenuCloseDelay) == Ms{300});
}

// ============================================================================
// GetMs convenience
// ============================================================================

TEST_CASE("GetMs returns integer milliseconds") {
    InteractionTimingRegistry reg;
    CHECK(reg.GetMs(TimingToken::TooltipDelay) == 500);
    CHECK(reg.GetMs(TimingToken::RepeatKeyInterval) == 33);
}

// ============================================================================
// DefaultValue static method
// ============================================================================

TEST_CASE("DefaultValue returns spec defaults") {
    CHECK(InteractionTimingRegistry::DefaultValue(TimingToken::HoverDelay) == Ms{200});
    CHECK(InteractionTimingRegistry::DefaultValue(TimingToken::AutoSaveInterval) == Ms{30000});
}

// ============================================================================
// Set / override
// ============================================================================

TEST_CASE("Set overrides a token value") {
    InteractionTimingRegistry reg;
    reg.Set(TimingToken::TooltipDelay, Ms{300});
    CHECK(reg.Get(TimingToken::TooltipDelay) == Ms{300});
    // Other tokens unaffected
    CHECK(reg.Get(TimingToken::HoverDelay) == Ms{200});
}

// ============================================================================
// ResetToDefault
// ============================================================================

TEST_CASE("ResetToDefault restores single token") {
    InteractionTimingRegistry reg;
    reg.Set(TimingToken::TooltipDelay, Ms{300});
    reg.ResetToDefault(TimingToken::TooltipDelay);
    CHECK(reg.Get(TimingToken::TooltipDelay) == Ms{500});
}

// ============================================================================
// ResetAll
// ============================================================================

TEST_CASE("ResetAll restores all tokens") {
    InteractionTimingRegistry reg;
    reg.Set(TimingToken::HoverDelay, Ms{999});
    reg.Set(TimingToken::IdleTimeout, Ms{1});
    reg.ResetAll();
    CHECK(reg.Get(TimingToken::HoverDelay) == Ms{200});
    CHECK(reg.Get(TimingToken::IdleTimeout) == Ms{60000});
}

// ============================================================================
// Boundary: invalid token
// ============================================================================

TEST_CASE("Get with invalid token returns 0ms") {
    InteractionTimingRegistry reg;
    CHECK(reg.Get(TimingToken::_Count) == Ms{0});
    CHECK(reg.Get(static_cast<TimingToken>(255)) == Ms{0});
}

TEST_CASE("DefaultValue with invalid token returns 0ms") {
    CHECK(InteractionTimingRegistry::DefaultValue(TimingToken::_Count) == Ms{0});
}

// ============================================================================
// ApplyPlatformOverrides
// ============================================================================

TEST_CASE("ApplyPlatformOverrides runs without crash") {
    InteractionTimingRegistry reg;
    const int updated = reg.ApplyPlatformOverrides();
    // On Windows, should update at least 1 token; on other platforms, 0
#ifdef _WIN32
    CHECK(updated >= 1);
#else
    CHECK(updated == 0);
#endif
}

TEST_CASE("ApplyPlatformOverrides: DoubleClickWindow is positive on Windows") {
    InteractionTimingRegistry reg;
    reg.ApplyPlatformOverrides();
    // Regardless of platform, the value should be positive
    CHECK(reg.GetMs(TimingToken::DoubleClickWindow) > 0);
}

// ============================================================================
// Token count
// ============================================================================

TEST_CASE("kTimingTokenCount is 15") {
    CHECK(kTimingTokenCount == 15);
}

} // TEST_SUITE
