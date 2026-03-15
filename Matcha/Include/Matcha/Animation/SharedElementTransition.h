#pragma once

/**
 * @file SharedElementTransition.h
 * @brief Shared Element Transition descriptor and interpolation (Spec §8.8.4).
 *
 * When an element conceptually moves between two locations, a proxy animates
 * from source rect to target rect. This component computes the interpolated
 * rect at any progress t ∈ [0, 1].
 *
 * Qt-free Foundation layer. Widget layer creates the actual proxy widget.
 */

#include <Matcha/Core/Macros.h>

namespace matcha::fw {

// ============================================================================
// Rect2D
// ============================================================================

/**
 * @struct Rect2D
 * @brief Simple 2D rectangle for position/size interpolation.
 */
struct Rect2D {
    double x      = 0.0;
    double y      = 0.0;
    double width  = 0.0;
    double height = 0.0;
};

// ============================================================================
// SharedElementSpec
// ============================================================================

/**
 * @struct SharedElementSpec
 * @brief Describes a shared element transition.
 */
struct SharedElementSpec {
    Rect2D source;
    Rect2D target;
    double durationMs  = 200.0;
    double opacity     = 1.0;     ///< Proxy opacity during animation
    bool   crossFade   = false;   ///< If true, source fades out while target fades in
};

// ============================================================================
// SharedElementTransition
// ============================================================================

/**
 * @class SharedElementTransition
 * @brief Computes interpolated state for a shared element transition.
 *
 * Usage:
 * @code
 *   SharedElementTransition tx({
 *       .source = {10, 20, 100, 30},
 *       .target = {200, 50, 150, 30},
 *       .durationMs = 200,
 *   });
 *
 *   auto mid = tx.Interpolate(0.5);
 *   // mid.rect = {105, 35, 125, 30}, mid.opacity = 1.0
 * @endcode
 */
class MATCHA_EXPORT SharedElementTransition {
public:
    struct State {
        Rect2D rect;
        double sourceOpacity = 1.0;  ///< For cross-fade: source visibility
        double targetOpacity = 0.0;  ///< For cross-fade: target visibility
        double proxyOpacity  = 1.0;  ///< Non-crossfade: proxy visibility
    };

    explicit SharedElementTransition(SharedElementSpec spec);

    /**
     * @brief Interpolate the transition state at progress t ∈ [0, 1].
     * @param t Progress fraction. Clamped to [0, 1].
     */
    [[nodiscard]] auto Interpolate(double t) const -> State;

    [[nodiscard]] auto Spec() const -> const SharedElementSpec& { return _spec; }
    [[nodiscard]] auto DurationMs() const -> double { return _spec.durationMs; }

private:
    SharedElementSpec _spec;
};

} // namespace matcha::fw
