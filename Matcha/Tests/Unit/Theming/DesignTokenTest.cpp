#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Theming/DesignTokens.h>

#include <utility>

using namespace matcha::gui;

// ============================================================================
// Compile-time enum consistency checks
// ============================================================================

// ColorToken: 75 values + Count_ (added Overlay)
static_assert(kColorTokenCount == 75);
static_assert(std::to_underlying(ColorToken::Surface) == 0);
static_assert(std::to_underlying(ColorToken::Separator) == 74);

// InteractionState: 8 values + Count_ (added DragOver)
static_assert(kInteractionStateCount == 8);
static_assert(std::to_underlying(InteractionState::Normal) == 0);
static_assert(std::to_underlying(InteractionState::DragOver) == 7);

// FontRole: 7 values + Count_
static_assert(kFontRoleCount == 7);
static_assert(std::to_underlying(FontRole::Body) == 0);
static_assert(std::to_underlying(FontRole::ToolTip) == 6);

// AnimationToken: 4 values + Count_
static_assert(kAnimationsTokenCount == 4);
static_assert(std::to_underlying(AnimationsToken::Instant) == 0);
static_assert(std::to_underlying(AnimationsToken::Slow) == 3);

// ElevationToken: 5 values + Count_
static_assert(kShadowTokenCount == 5);

// kDefaultAnimationMs consistency
static_assert(kDefaultAnimationMs[0] == 0);   // Instant
static_assert(kDefaultAnimationMs[1] == 160); // Quick
static_assert(kDefaultAnimationMs[2] == 200); // Normal
static_assert(kDefaultAnimationMs[3] == 350); // Slow

// ============================================================================
// Runtime tests
// ============================================================================

TEST_SUITE("DesignTokens") {

TEST_CASE("Spacing underlying value equals pixel value") {
    CHECK(ToPixels(SpaceToken::None) == 0);
    CHECK(ToPixels(SpaceToken::Px1) == 1);
    CHECK(ToPixels(SpaceToken::Px4) == 4);
    CHECK(ToPixels(SpaceToken::Px8) == 8);
    CHECK(ToPixels(SpaceToken::Px16) == 16);
    CHECK(ToPixels(SpaceToken::Px32) == 32);
}

TEST_CASE("RadiusToken underlying value equals pixel value") {
    CHECK(ToPixels(RadiusToken::None) == 0);
    CHECK(ToPixels(RadiusToken::Small) == 2);
    CHECK(ToPixels(RadiusToken::Default) == 3);
    CHECK(ToPixels(RadiusToken::Medium) == 4);
    CHECK(ToPixels(RadiusToken::Large) == 8);
    CHECK(ToPixels(RadiusToken::Round) == 255);
}

TEST_CASE("FontSpec default values") {
    FontSpec fs;
    CHECK(fs.sizeInPt == 9);
    CHECK(fs.weight == 400);
    CHECK(fs.italic == false);
    CHECK(fs.lineHeightMultiplier == doctest::Approx(1.4));
    CHECK(fs.letterSpacing == doctest::Approx(0.0));
}

TEST_CASE("ShadowSpec default is zero") {
    ShadowSpec ss;
    CHECK(ss.offsetX == 0);
    CHECK(ss.offsetY == 0);
    CHECK(ss.blurRadius == 0);
    CHECK(ss.opacity == doctest::Approx(0.0));
}

TEST_CASE("StateStyle default tokens") {
    StateStyle sc;
    CHECK(sc.background == ColorToken::Surface);
    CHECK(sc.foreground == ColorToken::colorText);
    CHECK(sc.border == ColorToken::BorderSubtle);
}

TEST_CASE("VariantStyle default has zeroed colors array") {
    VariantStyle vs {};
    CHECK(vs.colors[0].background == ColorToken::Surface);
    CHECK(vs.colors[0].foreground == ColorToken::colorText);
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
