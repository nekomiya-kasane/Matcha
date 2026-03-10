#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/UiNodes/Core/TokenEnums.h>

#include <ostream>
#include <utility>

using namespace matcha::fw;

// ============================================================================
// Compile-time enum consistency checks (fw layer)
// ============================================================================

// SpacingToken
static_assert(std::to_underlying(SpacingToken::None) == 0);
static_assert(std::to_underlying(SpacingToken::Px8) == 8);
static_assert(std::to_underlying(SpacingToken::Px32) == 32);
static_assert(std::to_underlying(SpacingToken::Count_) == 16);

// RadiusToken
static_assert(std::to_underlying(RadiusToken::None) == 0);
static_assert(std::to_underlying(RadiusToken::Default) == 3);
static_assert(std::to_underlying(RadiusToken::Round) == 255);
static_assert(std::to_underlying(RadiusToken::Count_) == 6);

// ElevationToken
static_assert(kElevationTokenCount == 5);

// AnimationToken
static_assert(kAnimationTokenCount == 4);
static_assert(kDefaultAnimationMs[0] == 0);
static_assert(kDefaultAnimationMs[3] == 350);

// EasingToken
static_assert(kEasingTokenCount == 4);
static_assert(std::to_underlying(EasingToken::Linear) == 0);
static_assert(std::to_underlying(EasingToken::InOutCubic) == 2);

// InteractionState (8 including DragOver)
static_assert(kInteractionStateCount == 8);
static_assert(std::to_underlying(InteractionState::Normal) == 0);
static_assert(std::to_underlying(InteractionState::DragOver) == 7);

// DensityLevel
static_assert(kDensityLevelCount == 3);
static_assert(std::to_underlying(DensityLevel::Compact) == 0);
static_assert(std::to_underlying(DensityLevel::Default) == 1);
static_assert(std::to_underlying(DensityLevel::Comfortable) == 2);

// TextDirection
static_assert(std::to_underlying(TextDirection::LTR) == 0);
static_assert(std::to_underlying(TextDirection::RTL) == 1);

// IconSize
static_assert(std::to_underlying(IconSize::Xs) == 12);
static_assert(std::to_underlying(IconSize::Sm) == 16);
static_assert(std::to_underlying(IconSize::Xl) == 32);

// ============================================================================
// Runtime tests
// ============================================================================

TEST_SUITE("fw::TokenEnums") {

TEST_CASE("ToPixels(SpacingToken) returns underlying value") {
    CHECK(ToPixels(SpacingToken::None) == 0);
    CHECK(ToPixels(SpacingToken::Px1) == 1);
    CHECK(ToPixels(SpacingToken::Px4) == 4);
    CHECK(ToPixels(SpacingToken::Px8) == 8);
    CHECK(ToPixels(SpacingToken::Px16) == 16);
    CHECK(ToPixels(SpacingToken::Px32) == 32);
}

TEST_CASE("ToPixels(RadiusToken) returns underlying value") {
    CHECK(ToPixels(RadiusToken::None) == 0);
    CHECK(ToPixels(RadiusToken::Small) == 2);
    CHECK(ToPixels(RadiusToken::Default) == 3);
    CHECK(ToPixels(RadiusToken::Round) == 255);
}

TEST_CASE("DensityScale returns correct factors") {
    CHECK(DensityScale(DensityLevel::Compact) == doctest::Approx(0.875f));
    CHECK(DensityScale(DensityLevel::Default) == doctest::Approx(1.0f));
    CHECK(DensityScale(DensityLevel::Comfortable) == doctest::Approx(1.125f));
}

TEST_CASE("DensityScale out-of-range returns 1.0") {
    CHECK(DensityScale(static_cast<DensityLevel>(99)) == doctest::Approx(1.0f));
}

TEST_CASE("FontSizePreset scale factors") {
    CHECK(FontSizePresetScale(FontSizePreset::Small) == doctest::Approx(0.875f));
    CHECK(FontSizePresetScale(FontSizePreset::Medium) == doctest::Approx(1.0f));
    CHECK(FontSizePresetScale(FontSizePreset::Large) == doctest::Approx(1.25f));
}

TEST_CASE("FontSizePresetScale out-of-range returns 1.0") {
    CHECK(FontSizePresetScale(static_cast<FontSizePreset>(99)) == doctest::Approx(1.0f));
}

TEST_CASE("Font scale clamp constants") {
    CHECK(kFontScaleMin == doctest::Approx(0.5f));
    CHECK(kFontScaleMax == doctest::Approx(3.0f));
    CHECK(kFontScaleMin < kFontScaleMax);
}

TEST_CASE("IconId constants use asset:// URI prefix") {
    CHECK(icons::Close == "asset://matcha/icons/close");
    CHECK(icons::Save == "asset://matcha/icons/save");
    CHECK(std::string_view(kMatchaIconPrefix) == "asset://matcha/icons/");
    // Empty IconId represents "no icon"
    CHECK(IconId().empty());
}

TEST_CASE("CursorToken has expected values") {
    CHECK(std::to_underlying(CursorToken::Default) == 0);
    CHECK(std::to_underlying(CursorToken::Pointer) == 1);
    CHECK(std::to_underlying(CursorToken::Count_) > 10);
}

TEST_CASE("LayerToken z-index ordering") {
    CHECK(std::to_underlying(LayerToken::Base) < std::to_underlying(LayerToken::Elevated));
    CHECK(std::to_underlying(LayerToken::Elevated) < std::to_underlying(LayerToken::Dropdown));
    CHECK(std::to_underlying(LayerToken::Dropdown) < std::to_underlying(LayerToken::Modal));
    CHECK(std::to_underlying(LayerToken::Modal) < std::to_underlying(LayerToken::Popover));
    CHECK(std::to_underlying(LayerToken::Popover) < std::to_underlying(LayerToken::Overlay));
    CHECK(std::to_underlying(LayerToken::Overlay) < std::to_underlying(LayerToken::Maximum));
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
