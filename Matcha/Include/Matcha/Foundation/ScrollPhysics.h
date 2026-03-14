#pragma once

/**
 * @file ScrollPhysics.h
 * @brief Scroll physics engine: momentum, overscroll, snap points.
 *
 * Implements §7.3 Scroll & Virtualization from the Matcha Design System Spec.
 * - §7.3.1 Momentum scrolling with exponential friction decay
 * - §7.3.2 Overscroll behavior: Clamp, Bounce (spring), Glow
 * - §7.3.3 Snap points with nearest-snap alignment
 * - §7.3.5 Scroll-to-focus (smooth scroll to make widget visible)
 *
 * This is a Foundation-layer component with zero Qt dependency.
 * All computations use double-precision floating point in pixels.
 *
 * @see Matcha_Design_System_Specification.md §7.3
 */

#include <Matcha/Foundation/Macros.h>

#include <cstdint>
#include <vector>

namespace matcha::fw {

// ============================================================================
// OverscrollMode (§7.3.2)
// ============================================================================

/**
 * @enum OverscrollMode
 * @brief How the scroll view responds at content boundaries.
 *
 * | Mode   | Description                                   | Platform |
 * |--------|-----------------------------------------------|----------|
 * | Clamp  | Hard stop, no visual feedback                 | Windows  |
 * | Bounce | Spring-back, max 30px overshoot               | macOS    |
 * | Glow   | Edge glow indicator, no content displacement  | Android  |
 */
enum class OverscrollMode : uint8_t {
    Clamp  = 0,
    Bounce = 1,
    Glow   = 2,
};

// ============================================================================
// SnapAlignment (§7.3.3)
// ============================================================================

/**
 * @enum SnapAlignment
 * @brief How snap points align items within the viewport.
 */
enum class SnapAlignment : uint8_t {
    Start  = 0,   ///< Snap item top to viewport top
    Center = 1,   ///< Snap item center to viewport center
    End    = 2,   ///< Snap item bottom to viewport bottom
};

// ============================================================================
// ScrollConfig
// ============================================================================

/**
 * @struct ScrollConfig
 * @brief Configuration parameters for scroll physics.
 */
struct ScrollConfig {
    double friction         = 0.95;     ///< Per-frame friction (60fps). v *= friction each frame.
    double stopThreshold    = 0.5;      ///< Stop when |v| < this (px/frame)
    double maxInitialVelocity = 5000.0; ///< Max initial velocity (px/s)
    double bounceStiffness  = 300.0;    ///< Spring stiffness for bounce overscroll
    double bounceDamping    = 20.0;     ///< Spring damping for bounce overscroll
    double maxOverscroll    = 30.0;     ///< Max overscroll displacement (px)
    OverscrollMode overscrollMode = OverscrollMode::Clamp;
    SnapAlignment  snapAlignment  = SnapAlignment::Start;
    double snapAnimDuration = 200.0;    ///< Snap animation duration (ms)
};

// ============================================================================
// ScrollState
// ============================================================================

/**
 * @struct ScrollState
 * @brief Current state of the scroll physics simulation.
 */
struct ScrollState {
    double position    = 0.0;    ///< Current scroll position (px)
    double velocity    = 0.0;    ///< Current velocity (px/frame at 60fps)
    double overscroll  = 0.0;    ///< Current overscroll displacement (px, 0 if clamped)
    bool   isAnimating = false;  ///< True if momentum/bounce/snap is active
};

// ============================================================================
// ScrollPhysics (§7.3)
// ============================================================================

/**
 * @class ScrollPhysics
 * @brief Physics simulation for scroll behavior.
 *
 * **Thread safety**: Not thread-safe. All calls from GUI thread.
 *
 * Usage:
 * @code
 *   ScrollPhysics scroll;
 *   scroll.SetContentSize(5000.0);
 *   scroll.SetViewportSize(600.0);
 *   scroll.Fling(2000.0);  // px/s initial velocity
 *
 *   while (scroll.IsAnimating()) {
 *       scroll.Step(1.0 / 60.0);  // 16.67ms per frame
 *       double pos = scroll.Position();
 *       // update render
 *   }
 * @endcode
 */
class MATCHA_EXPORT ScrollPhysics {
public:
    ScrollPhysics() = default;
    explicit ScrollPhysics(ScrollConfig config);

    // ====================================================================
    // Configuration
    // ====================================================================

    void SetConfig(ScrollConfig config);
    [[nodiscard]] auto Config() const -> const ScrollConfig& { return _config; }

    void SetContentSize(double size);
    void SetViewportSize(double size);
    [[nodiscard]] auto ContentSize() const -> double { return _contentSize; }
    [[nodiscard]] auto ViewportSize() const -> double { return _viewportSize; }

    /**
     * @brief Set snap points (sorted, ascending).
     */
    void SetSnapPoints(std::vector<double> points);

    // ====================================================================
    // Interaction API
    // ====================================================================

    /**
     * @brief Start a fling (momentum scroll) with initial velocity.
     * @param velocityPxPerSec Initial velocity in px/s.
     */
    void Fling(double velocityPxPerSec);

    /**
     * @brief Direct scroll by a delta (e.g., mouse wheel).
     * @param deltaPx Scroll delta in pixels (positive = scroll down).
     */
    void ScrollBy(double deltaPx);

    /**
     * @brief Scroll to make a target position visible (§7.3.5).
     * @param targetTop Top of the target element.
     * @param targetHeight Height of the target element.
     */
    void ScrollToVisible(double targetTop, double targetHeight);

    /**
     * @brief Scroll to an exact position.
     */
    void ScrollTo(double position);

    /**
     * @brief Stop any ongoing animation.
     */
    void Stop();

    // ====================================================================
    // Simulation
    // ====================================================================

    /**
     * @brief Advance the physics simulation by dt seconds.
     * @param dt Time step in seconds (e.g., 1/60 for 60fps).
     */
    void Step(double dt);

    // ====================================================================
    // Query
    // ====================================================================

    [[nodiscard]] auto Position() const -> double { return _state.position; }
    [[nodiscard]] auto Velocity() const -> double { return _state.velocity; }
    [[nodiscard]] auto Overscroll() const -> double { return _state.overscroll; }
    [[nodiscard]] auto IsAnimating() const -> bool { return _state.isAnimating; }
    [[nodiscard]] auto State() const -> const ScrollState& { return _state; }

    /**
     * @brief Maximum scroll position (contentSize - viewportSize, clamped >= 0).
     */
    [[nodiscard]] auto MaxPosition() const -> double;

    /**
     * @brief Find the nearest snap point to a given position.
     * @return The snap position, or the input position if no snap points.
     */
    [[nodiscard]] auto NearestSnapPoint(double position) const -> double;

private:
    void ClampPosition();
    void ApplyBounce(double dt);
    void ApplySnap();

    ScrollConfig        _config;
    ScrollState         _state;
    double              _contentSize  = 0.0;
    double              _viewportSize = 0.0;
    std::vector<double> _snapPoints;
    bool                _snapping     = false;
    double              _snapTarget   = 0.0;
};

} // namespace matcha::fw
