#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Theming/WidgetStyleSheet.h>

#include <utility>

using namespace matcha::gui;

// ============================================================================
// Compile-time checks
// ============================================================================

// WidgetKind: 57 entries (added DocumentToolBar + LogoButton)
static_assert(kWidgetKindCount == 57);
static_assert(std::to_underlying(WidgetKind::PushButton) == 0);
static_assert(std::to_underlying(WidgetKind::MenuCheckItem) == 41);

// ============================================================================
// Runtime tests
// ============================================================================

TEST_SUITE("WidgetStyleSheet") {

TEST_CASE("WidgetKind has exactly 57 entries") {
    CHECK(kWidgetKindCount == 57);
}

TEST_CASE("WidgetStyleSheet default construction") {
    WidgetStyleSheet sheet;
    CHECK(sheet.radius == RadiusToken::Default);
    CHECK(sheet.paddingH == SpacingToken::Px4);
    CHECK(sheet.paddingV == SpacingToken::Px4);
    CHECK(sheet.gap == SpacingToken::Px4);
    CHECK(sheet.minHeight == SizeToken::Md);
    CHECK(sheet.borderWidth == SpacingToken::Px1);
    CHECK(sheet.font == FontRole::Body);
    CHECK(sheet.elevation == ElevationToken::Flat);
    CHECK(sheet.layer == LayerToken::Base);
    CHECK(sheet.transition.duration == AnimationToken::Normal);
    CHECK(sheet.transition.easing == EasingToken::OutCubic);
}

TEST_CASE("WidgetStyleSheet default variant span is empty") {
    WidgetStyleSheet sheet;
    CHECK(sheet.variants.empty());
    CHECK(sheet.variants.size() == 0);
}

TEST_CASE("WidgetStyleSheet with variant span") {
    VariantStyle vs {};
    vs.colors[std::to_underlying(InteractionState::Normal)] = {
        ColorToken::Primary,
        ColorToken::Surface,
        ColorToken::Primary,
    };

    WidgetStyleSheet sheet {
        .radius = RadiusToken::Large,
        .paddingH = SpacingToken::Px8,
        .paddingV = SpacingToken::Px8,
        .font = FontRole::Heading,
        .elevation = ElevationToken::High,
        .transition = {AnimationToken::Slow, EasingToken::InOutCubic},
        .variants = std::span<const VariantStyle>(&vs, 1),
    };

    CHECK(sheet.radius == RadiusToken::Large);
    CHECK(sheet.variants.size() == 1);
    CHECK(sheet.variants[0].colors[0].background == ColorToken::Primary);
}

TEST_CASE("WidgetKind enum covers all tiers") {
    // Tier 1: Core Input (11: PushButton..Tag)
    CHECK(std::to_underlying(WidgetKind::Label) == 9);
    CHECK(std::to_underlying(WidgetKind::Tag) == 10);
    // Tier 2: Container & Layout (16, Drawer/DrawerBox removed)
    CHECK(std::to_underlying(WidgetKind::ScrollBar) == 12);
    CHECK(std::to_underlying(WidgetKind::CollapsibleSection) == 15);
    CHECK(std::to_underlying(WidgetKind::ListWidget) == 21);
    CHECK(std::to_underlying(WidgetKind::TableWidget) == 22);
    CHECK(std::to_underlying(WidgetKind::Splitter) == 26);
    CHECK(std::to_underlying(WidgetKind::PropertyGrid) == 27);
    CHECK(std::to_underlying(WidgetKind::ColorPicker) == 28);
    // Tier 3: Application-Level (8)
    CHECK(std::to_underlying(WidgetKind::Tooltip) == 36);
    // Menu system (5)
    CHECK(std::to_underlying(WidgetKind::MenuBar) == 37);
    CHECK(std::to_underlying(WidgetKind::MenuItem) == 39);
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
