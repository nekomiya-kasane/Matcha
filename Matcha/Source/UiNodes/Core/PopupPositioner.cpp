/**
 * @file PopupPositioner.cpp
 * @brief Implementation of the popup positioning engine (§7.4).
 */

#include <Matcha/UiNodes/Core/PopupPositioner.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Helper: classify placement axis
// ============================================================================

namespace {

enum class Axis : uint8_t { Vertical, Horizontal };

[[nodiscard]] constexpr auto PrimaryAxis(PopupPlacement p) -> Axis
{
    switch (p) {
    case PopupPlacement::TopStart:
    case PopupPlacement::TopCenter:
    case PopupPlacement::TopEnd:
    case PopupPlacement::BottomStart:
    case PopupPlacement::BottomCenter:
    case PopupPlacement::BottomEnd:
        return Axis::Vertical;
    default:
        return Axis::Horizontal;
    }
}

[[nodiscard]] constexpr auto IsBottomPlacement(PopupPlacement p) -> bool
{
    return p == PopupPlacement::BottomStart
        || p == PopupPlacement::BottomCenter
        || p == PopupPlacement::BottomEnd;
}

[[nodiscard]] constexpr auto IsRightPlacement(PopupPlacement p) -> bool
{
    return p == PopupPlacement::RightStart
        || p == PopupPlacement::RightCenter
        || p == PopupPlacement::RightEnd;
}

/// Check if popup rect overflows the viewport (with margin).
[[nodiscard]] constexpr auto Overflows(
    Point pos, Size sz, const Rect& vp, int margin) -> bool
{
    return pos.x < vp.x + margin
        || pos.y < vp.y + margin
        || pos.x + sz.w > vp.Right() - margin
        || pos.y + sz.h > vp.Bottom() - margin;
}

/// Check if popup overflows on the primary axis only.
[[nodiscard]] constexpr auto OverflowsPrimary(
    Point pos, Size sz, const Rect& vp, int margin, Axis axis) -> bool
{
    if (axis == Axis::Vertical) {
        return pos.y < vp.y + margin || pos.y + sz.h > vp.Bottom() - margin;
    }
    return pos.x < vp.x + margin || pos.x + sz.w > vp.Right() - margin;
}

/// Compute available space on primary axis for a given placement.
[[nodiscard]] constexpr auto AvailableSpace(
    PopupPlacement placement, const Rect& anchor, const Rect& vp, int margin, Axis axis) -> int
{
    if (axis == Axis::Vertical) {
        return IsBottomPlacement(placement)
            ? vp.Bottom() - margin - anchor.Bottom()
            : anchor.Top() - vp.Top() - margin;
    }
    return IsRightPlacement(placement)
        ? vp.Right() - margin - anchor.Right()
        : anchor.Left() - vp.Left() - margin;
}

/// Apply flip logic. Returns true if flip was applied.
auto TryFlip(const PopupRequest& req, PopupResult& result, Point& pos, Axis axis) -> bool
{
    if ((req.strategy & OverflowStrategy::Flip) == OverflowStrategy::None) { return false; }
    if (!OverflowsPrimary(pos, result.size, req.viewport, req.margin, axis)) { return false; }

    const auto flipped = PopupPositioner::Flip(result.placement);
    const auto flippedPos = PopupPositioner::RawPosition(
        req.anchorRect, result.size, flipped, {-req.offset.x, -req.offset.y});

    // Flipped side has no overflow — accept immediately
    if (!OverflowsPrimary(flippedPos, result.size, req.viewport, req.margin, axis)) {
        pos = flippedPos;
        result.placement = flipped;
        result.flipped = true;
        return true;
    }

    // Both sides overflow — pick the one with more space
    const int spaceOrig    = AvailableSpace(result.placement, req.anchorRect, req.viewport, req.margin, axis);
    const int spaceFlipped = AvailableSpace(flipped,          req.anchorRect, req.viewport, req.margin, axis);

    const bool flipBetter = (axis == Axis::Vertical)
        ? (spaceFlipped > spaceOrig && spaceFlipped >= req.minHeight)
        : (spaceFlipped > spaceOrig);

    if (flipBetter) {
        pos = flippedPos;
        result.placement = flipped;
        result.flipped = true;
        return true;
    }
    return false;
}

/// Apply shift on the cross axis.
auto TryShift(const PopupRequest& req, const PopupResult& result, Point& pos, Axis axis) -> bool
{
    if ((req.strategy & OverflowStrategy::Shift) == OverflowStrategy::None) { return false; }

    const int minX = req.viewport.Left()  + req.margin;
    const int maxX = req.viewport.Right() - req.margin - result.size.w;
    const int minY = req.viewport.Top()   + req.margin;
    const int maxY = req.viewport.Bottom() - req.margin - result.size.h;

    bool shifted = false;
    if (axis == Axis::Vertical) {
        // Guard: if popup wider than viewport, pin to minX (resize will handle the rest)
        const int clampHi = std::max(minX, maxX);
        if (pos.x < minX || pos.x > clampHi) {
            pos.x = std::clamp(pos.x, minX, clampHi);
            shifted = true;
        }
    } else {
        const int clampHi = std::max(minY, maxY);
        if (pos.y < minY || pos.y > clampHi) {
            pos.y = std::clamp(pos.y, minY, clampHi);
            shifted = true;
        }
    }
    return shifted;
}

/// Apply resize if popup still overflows.
auto TryResize(const PopupRequest& req, PopupResult& result, Point& pos) -> bool
{
    if ((req.strategy & OverflowStrategy::Resize) == OverflowStrategy::None) { return false; }
    if (!Overflows(pos, result.size, req.viewport, req.margin)) { return false; }

    bool resized = false;

    // Clamp right/bottom edges
    const int maxW = req.viewport.Right()  - req.margin - pos.x;
    const int maxH = req.viewport.Bottom() - req.margin - pos.y;
    if (result.size.w > maxW && maxW > 0) { result.size.w = maxW; resized = true; }
    if (result.size.h > maxH && maxH > 0) { result.size.h = maxH; resized = true; }

    // Clamp left/top edges (negative direction overflow)
    const int minX = req.viewport.Left() + req.margin;
    const int minY = req.viewport.Top()  + req.margin;
    if (pos.x < minX) { result.size.w -= (minX - pos.x); pos.x = minX; resized = true; }
    if (pos.y < minY) { result.size.h -= (minY - pos.y); pos.y = minY; resized = true; }

    result.size.w = std::max(result.size.w, 0);
    result.size.h = std::max(result.size.h, 0);
    return resized;
}

} // anonymous namespace

// ============================================================================
// RawPosition — initial placement without overflow correction
// ============================================================================

auto PopupPositioner::RawPosition(
    const Rect& anchor, const Size& popup,
    PopupPlacement placement, Point offset) -> Point
{
    int px = 0;
    int py = 0;

    switch (placement) {
    case PopupPlacement::TopStart:
        px = anchor.Left();
        py = anchor.Top() - popup.h;
        break;
    case PopupPlacement::TopCenter:
        px = anchor.CenterX() - (popup.w / 2);
        py = anchor.Top() - popup.h;
        break;
    case PopupPlacement::TopEnd:
        px = anchor.Right() - popup.w;
        py = anchor.Top() - popup.h;
        break;
    case PopupPlacement::BottomStart:
        px = anchor.Left();
        py = anchor.Bottom();
        break;
    case PopupPlacement::BottomCenter:
        px = anchor.CenterX() - (popup.w / 2);
        py = anchor.Bottom();
        break;
    case PopupPlacement::BottomEnd:
        px = anchor.Right() - popup.w;
        py = anchor.Bottom();
        break;
    case PopupPlacement::LeftStart:
        px = anchor.Left() - popup.w;
        py = anchor.Top();
        break;
    case PopupPlacement::LeftCenter:
        px = anchor.Left() - popup.w;
        py = anchor.CenterY() - (popup.h / 2);
        break;
    case PopupPlacement::LeftEnd:
        px = anchor.Left() - popup.w;
        py = anchor.Bottom() - popup.h;
        break;
    case PopupPlacement::RightStart:
        px = anchor.Right();
        py = anchor.Top();
        break;
    case PopupPlacement::RightCenter:
        px = anchor.Right();
        py = anchor.CenterY() - (popup.h / 2);
        break;
    case PopupPlacement::RightEnd:
        px = anchor.Right();
        py = anchor.Bottom() - popup.h;
        break;
    }

    px += offset.x;
    py += offset.y;
    return {px, py};
}

// ============================================================================
// Compute — main entry point
// ============================================================================

auto PopupPositioner::Compute(const PopupRequest& req) -> PopupResult
{
    PopupResult result;
    result.placement = req.placement;
    result.size      = req.popupSize;

    const auto axis = PrimaryAxis(req.placement);
    auto pos = RawPosition(req.anchorRect, result.size, result.placement, req.offset);

    TryFlip(req, result, pos, axis);
    result.shifted = TryShift(req, result, pos, axis);
    result.resized = TryResize(req, result, pos);

    result.position = pos;
    return result;
}

} // namespace matcha::fw
