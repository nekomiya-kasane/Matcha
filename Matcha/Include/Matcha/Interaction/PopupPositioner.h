#pragma once

/**
 * @file PopupPositioner.h
 * @brief Qt-free popup positioning engine with flip/shift/resize strategies.
 *
 * Implements §7.4 Popup Positioning from the Matcha Design System Specification.
 * Supports 12 anchor placements, automatic flip on primary axis overflow,
 * shift on cross axis overflow, and resize when space is still insufficient.
 *
 * This is a fw-layer component with zero Qt dependency.
 * All geometry is expressed as plain int-based structs (Rect, Point, Size).
 *
 * @see Matcha_Design_System_Specification.md §7.4
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>

namespace matcha::fw {

// ============================================================================
// Geometry primitives (Qt-free)
// ============================================================================

struct Point {
    int x = 0;
    int y = 0;

    constexpr auto operator==(const Point&) const -> bool = default;
};

struct Size {
    int w = 0;
    int h = 0;

    constexpr auto operator==(const Size&) const -> bool = default;
};

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] constexpr auto Left()   const -> int { return x; }
    [[nodiscard]] constexpr auto Top()    const -> int { return y; }
    [[nodiscard]] constexpr auto Right()  const -> int { return x + w; }
    [[nodiscard]] constexpr auto Bottom() const -> int { return y + h; }
    [[nodiscard]] constexpr auto Width()  const -> int { return w; }
    [[nodiscard]] constexpr auto Height() const -> int { return h; }

    [[nodiscard]] constexpr auto Contains(Point p) const -> bool {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }

    [[nodiscard]] constexpr auto CenterX() const -> int { return x + (w / 2); }
    [[nodiscard]] constexpr auto CenterY() const -> int { return y + (h / 2); }

    constexpr auto operator==(const Rect&) const -> bool = default;
};

// ============================================================================
// PopupPlacement — 12 anchor positions (§7.4.1)
// ============================================================================

/**
 * @enum PopupPlacement
 * @brief 12 positions around an anchor rectangle.
 *
 * Naming convention: <Side><Alignment>
 * - Side: Top, Bottom, Left, Right
 * - Alignment: Start (toward top-left), Center, End (toward bottom-right)
 *
 * Example: BottomStart = popup below anchor, left-aligned.
 */
enum class PopupPlacement : uint8_t {
    TopStart,    TopCenter,    TopEnd,
    BottomStart, BottomCenter, BottomEnd,
    LeftStart,   LeftCenter,   LeftEnd,
    RightStart,  RightCenter,  RightEnd,
};

/// Total number of placement values.
inline constexpr int kPopupPlacementCount = 12;

// ============================================================================
// PopupConstraint — overflow strategy flags
// ============================================================================

/**
 * @enum OverflowStrategy
 * @brief Flags controlling which overflow corrections to apply.
 */
enum class OverflowStrategy : uint8_t {
    None    = 0,
    Flip    = uint8_t{1} << 0,   ///< Flip to opposite side on primary axis
    Shift   = uint8_t{1} << 1,   ///< Shift along cross axis to stay on screen
    Resize  = uint8_t{1} << 2,   ///< Reduce popup size if still overflowing
    All     = Flip | Shift | Resize,
};

[[nodiscard]] constexpr auto operator|(OverflowStrategy a, OverflowStrategy b) -> OverflowStrategy {
    return static_cast<OverflowStrategy>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
[[nodiscard]] constexpr auto operator&(OverflowStrategy a, OverflowStrategy b) -> OverflowStrategy {
    return static_cast<OverflowStrategy>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

// ============================================================================
// PopupRequest / PopupResult
// ============================================================================

/**
 * @struct PopupRequest
 * @brief Input parameters for popup positioning.
 */
struct PopupRequest {
    Rect           anchorRect  {};          ///< Anchor widget geometry (screen coords)
    Size           popupSize   {};          ///< Desired popup size
    PopupPlacement placement   {PopupPlacement::BottomStart};
    Point          offset      {0, 0};     ///< Additional offset from computed position
    Rect           viewport    {};          ///< Available screen geometry
    int            margin      {8};        ///< Safety margin from screen edges (px)
    OverflowStrategy strategy  {OverflowStrategy::All};
    int            minHeight   {120};      ///< Minimum popup height before flipping (§7.4.3)
};

/**
 * @struct PopupResult
 * @brief Output of popup positioning computation.
 */
struct PopupResult {
    Point          position    {};          ///< Final popup top-left (screen coords)
    Size           size        {};          ///< Final popup size (may be resized)
    PopupPlacement placement   {PopupPlacement::BottomStart}; ///< Actual placement after flip
    bool           flipped     {false};     ///< True if primary axis was flipped
    bool           shifted     {false};     ///< True if cross axis was shifted
    bool           resized     {false};     ///< True if popup was resized
};

// ============================================================================
// PopupPositioner — stateless positioning engine (§7.4)
// ============================================================================

/**
 * @class PopupPositioner
 * @brief Computes popup position with flip/shift/resize overflow handling.
 *
 * Usage:
 * @code
 *   PopupRequest req;
 *   req.anchorRect = {100, 200, 80, 30};
 *   req.popupSize  = {200, 300};
 *   req.placement  = PopupPlacement::BottomStart;
 *   req.viewport   = {0, 0, 1920, 1080};
 *
 *   auto result = PopupPositioner::Compute(req);
 *   // result.position, result.size, result.flipped, etc.
 * @endcode
 */
class MATCHA_EXPORT PopupPositioner {
public:
    PopupPositioner() = delete; // stateless, all static

    /**
     * @brief Compute the final popup position and size.
     * @param req Positioning request parameters.
     * @return Positioning result with final geometry and applied corrections.
     */
    [[nodiscard]] static auto Compute(const PopupRequest& req) -> PopupResult;

    /**
     * @brief Get the opposite placement (for flip).
     */
    [[nodiscard]] static constexpr auto Flip(PopupPlacement p) -> PopupPlacement;

    /**
     * @brief Raw position before overflow correction.
     * @param anchor Anchor rectangle.
     * @param popup  Popup size.
     * @param placement Desired placement.
     * @param offset Additional offset.
     * @return Top-left point of popup before any correction.
     */
    [[nodiscard]] static auto RawPosition(
        const Rect& anchor, const Size& popup,
        PopupPlacement placement, Point offset) -> Point;
};

// ============================================================================
// Inline implementation — Flip
// ============================================================================

constexpr auto PopupPositioner::Flip(PopupPlacement p) -> PopupPlacement
{
    switch (p) {
    case PopupPlacement::TopStart:      return PopupPlacement::BottomStart;
    case PopupPlacement::TopCenter:     return PopupPlacement::BottomCenter;
    case PopupPlacement::TopEnd:        return PopupPlacement::BottomEnd;
    case PopupPlacement::BottomStart:   return PopupPlacement::TopStart;
    case PopupPlacement::BottomCenter:  return PopupPlacement::TopCenter;
    case PopupPlacement::BottomEnd:     return PopupPlacement::TopEnd;
    case PopupPlacement::LeftStart:     return PopupPlacement::RightStart;
    case PopupPlacement::LeftCenter:    return PopupPlacement::RightCenter;
    case PopupPlacement::LeftEnd:       return PopupPlacement::RightEnd;
    case PopupPlacement::RightStart:    return PopupPlacement::LeftStart;
    case PopupPlacement::RightCenter:   return PopupPlacement::LeftCenter;
    case PopupPlacement::RightEnd:      return PopupPlacement::LeftEnd;
    }
    return p; // unreachable
}

} // namespace matcha::fw
