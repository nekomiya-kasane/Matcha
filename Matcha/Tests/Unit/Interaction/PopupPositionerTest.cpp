#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Interaction/PopupPositioner.h>

using namespace matcha::fw;

// Viewport: 1920x1080 starting at (0,0)
static constexpr Rect kScreen{0, 0, 1920, 1080};

// A button at center-ish area
static constexpr Rect kAnchorCenter{500, 400, 120, 30};

// A button near bottom edge
static constexpr Rect kAnchorBottom{500, 1040, 120, 30};

// A button near right edge
static constexpr Rect kAnchorRight{1800, 400, 120, 30};

TEST_SUITE("fw::PopupPositioner") {

// ============================================================================
// Geometry primitives
// ============================================================================

TEST_CASE("Rect accessors") {
    constexpr Rect r{10, 20, 100, 50};
    CHECK(r.Left() == 10);
    CHECK(r.Top() == 20);
    CHECK(r.Right() == 110);
    CHECK(r.Bottom() == 70);
    CHECK(r.Width() == 100);
    CHECK(r.Height() == 50);
    CHECK(r.CenterX() == 60);
    CHECK(r.CenterY() == 45);
    CHECK(r.Contains({50, 40}));
    CHECK_FALSE(r.Contains({5, 40}));
}

// ============================================================================
// Flip
// ============================================================================

TEST_CASE("Flip returns opposite placement") {
    CHECK(PopupPositioner::Flip(PopupPlacement::BottomStart) == PopupPlacement::TopStart);
    CHECK(PopupPositioner::Flip(PopupPlacement::TopCenter) == PopupPlacement::BottomCenter);
    CHECK(PopupPositioner::Flip(PopupPlacement::LeftEnd) == PopupPlacement::RightEnd);
    CHECK(PopupPositioner::Flip(PopupPlacement::RightStart) == PopupPlacement::LeftStart);
    // Double flip = identity
    CHECK(PopupPositioner::Flip(PopupPositioner::Flip(PopupPlacement::BottomEnd)) == PopupPlacement::BottomEnd);
}

// ============================================================================
// RawPosition — 12 placements
// ============================================================================

TEST_CASE("RawPosition: BottomStart") {
    auto p = PopupPositioner::RawPosition(kAnchorCenter, {200, 300}, PopupPlacement::BottomStart, {0, 0});
    CHECK(p.x == kAnchorCenter.Left());
    CHECK(p.y == kAnchorCenter.Bottom());
}

TEST_CASE("RawPosition: BottomCenter") {
    auto p = PopupPositioner::RawPosition(kAnchorCenter, {200, 300}, PopupPlacement::BottomCenter, {0, 0});
    CHECK(p.x == kAnchorCenter.CenterX() - 100);
    CHECK(p.y == kAnchorCenter.Bottom());
}

TEST_CASE("RawPosition: TopStart") {
    auto p = PopupPositioner::RawPosition(kAnchorCenter, {200, 300}, PopupPlacement::TopStart, {0, 0});
    CHECK(p.x == kAnchorCenter.Left());
    CHECK(p.y == kAnchorCenter.Top() - 300);
}

TEST_CASE("RawPosition: RightStart") {
    auto p = PopupPositioner::RawPosition(kAnchorCenter, {200, 300}, PopupPlacement::RightStart, {0, 0});
    CHECK(p.x == kAnchorCenter.Right());
    CHECK(p.y == kAnchorCenter.Top());
}

TEST_CASE("RawPosition: LeftCenter") {
    auto p = PopupPositioner::RawPosition(kAnchorCenter, {200, 300}, PopupPlacement::LeftCenter, {0, 0});
    CHECK(p.x == kAnchorCenter.Left() - 200);
    CHECK(p.y == kAnchorCenter.CenterY() - 150);
}

TEST_CASE("RawPosition: with offset") {
    auto p = PopupPositioner::RawPosition(kAnchorCenter, {200, 300}, PopupPlacement::BottomStart, {0, 4});
    CHECK(p.x == kAnchorCenter.Left());
    CHECK(p.y == kAnchorCenter.Bottom() + 4);
}

// ============================================================================
// Compute — no overflow (center of screen)
// ============================================================================

TEST_CASE("Compute: no overflow, BottomStart") {
    PopupRequest req;
    req.anchorRect = kAnchorCenter;
    req.popupSize  = {200, 300};
    req.placement  = PopupPlacement::BottomStart;
    req.viewport   = kScreen;

    auto r = PopupPositioner::Compute(req);
    CHECK(r.position.x == kAnchorCenter.Left());
    CHECK(r.position.y == kAnchorCenter.Bottom());
    CHECK(r.size.w == 200);
    CHECK(r.size.h == 300);
    CHECK(r.placement == PopupPlacement::BottomStart);
    CHECK_FALSE(r.flipped);
    CHECK_FALSE(r.shifted);
    CHECK_FALSE(r.resized);
}

// ============================================================================
// Compute — FLIP
// ============================================================================

TEST_CASE("Compute: flip BottomStart to TopStart near bottom edge") {
    PopupRequest req;
    req.anchorRect = kAnchorBottom;
    req.popupSize  = {200, 300};
    req.placement  = PopupPlacement::BottomStart;
    req.viewport   = kScreen;

    auto r = PopupPositioner::Compute(req);
    CHECK(r.flipped);
    CHECK(r.placement == PopupPlacement::TopStart);
    // Flipped: popup above anchor, y = anchor.Top() - popupH = 1040 - 300 = 740
    CHECK(r.position.y == kAnchorBottom.Top() - 300);
}

TEST_CASE("Compute: flip RightStart to LeftStart near right edge") {
    PopupRequest req;
    req.anchorRect = kAnchorRight;
    req.popupSize  = {200, 300};
    req.placement  = PopupPlacement::RightStart;
    req.viewport   = kScreen;

    auto r = PopupPositioner::Compute(req);
    CHECK(r.flipped);
    CHECK(r.placement == PopupPlacement::LeftStart);
}

// ============================================================================
// Compute — SHIFT
// ============================================================================

TEST_CASE("Compute: shift on cross axis when popup too wide") {
    PopupRequest req;
    req.anchorRect = {1800, 400, 50, 30};  // near right, but popup goes Bottom
    req.popupSize  = {200, 100};
    req.placement  = PopupPlacement::BottomStart;  // primary=vertical, cross=horizontal
    req.viewport   = kScreen;

    auto r = PopupPositioner::Compute(req);
    CHECK(r.shifted);
    // Popup should be shifted left to fit within viewport - margin
    CHECK(r.position.x + r.size.w <= kScreen.Right() - req.margin);
}

// ============================================================================
// Compute — RESIZE
// ============================================================================

TEST_CASE("Compute: resize when popup exceeds viewport after flip+shift") {
    PopupRequest req;
    req.anchorRect = {10, 10, 50, 30};  // near top-left corner
    req.popupSize  = {2000, 2000};      // enormous popup
    req.placement  = PopupPlacement::BottomStart;
    req.viewport   = kScreen;

    auto r = PopupPositioner::Compute(req);
    CHECK(r.resized);
    // Must fit within viewport margins
    CHECK(r.position.x + r.size.w <= kScreen.Right() - req.margin);
    CHECK(r.position.y + r.size.h <= kScreen.Bottom() - req.margin);
    CHECK(r.size.w > 0);
    CHECK(r.size.h > 0);
}

// ============================================================================
// Compute — strategy flags
// ============================================================================

TEST_CASE("Compute: OverflowStrategy::None disables all corrections") {
    PopupRequest req;
    req.anchorRect = kAnchorBottom;
    req.popupSize  = {200, 300};
    req.placement  = PopupPlacement::BottomStart;
    req.viewport   = kScreen;
    req.strategy   = OverflowStrategy::None;

    auto r = PopupPositioner::Compute(req);
    CHECK_FALSE(r.flipped);
    CHECK_FALSE(r.shifted);
    CHECK_FALSE(r.resized);
    // Popup goes below viewport — no correction
    CHECK(r.position.y == kAnchorBottom.Bottom());
}

TEST_CASE("Compute: Flip-only strategy does not shift or resize") {
    PopupRequest req;
    req.anchorRect = {1800, 1040, 50, 30};
    req.popupSize  = {200, 300};
    req.placement  = PopupPlacement::BottomStart;
    req.viewport   = kScreen;
    req.strategy   = OverflowStrategy::Flip;

    auto r = PopupPositioner::Compute(req);
    CHECK(r.flipped);
    CHECK_FALSE(r.shifted);
    CHECK_FALSE(r.resized);
}

// ============================================================================
// Compute — with offset
// ============================================================================

TEST_CASE("Compute: offset is applied") {
    PopupRequest req;
    req.anchorRect = kAnchorCenter;
    req.popupSize  = {200, 100};
    req.placement  = PopupPlacement::BottomStart;
    req.offset     = {0, 4};
    req.viewport   = kScreen;

    auto r = PopupPositioner::Compute(req);
    CHECK(r.position.y == kAnchorCenter.Bottom() + 4);
}

// ============================================================================
// OverflowStrategy bitwise operators
// ============================================================================

TEST_CASE("OverflowStrategy bitwise ops") {
    auto fs = OverflowStrategy::Flip | OverflowStrategy::Shift;
    CHECK((fs & OverflowStrategy::Flip) != OverflowStrategy::None);
    CHECK((fs & OverflowStrategy::Shift) != OverflowStrategy::None);
    CHECK((fs & OverflowStrategy::Resize) == OverflowStrategy::None);
}

// ============================================================================
// PopupPlacement count
// ============================================================================

TEST_CASE("kPopupPlacementCount is 12") {
    CHECK(kPopupPlacementCount == 12);
}

} // TEST_SUITE
