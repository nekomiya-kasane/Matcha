#pragma once

/**
 * @file GestureMotionTracker.h
 * @brief Gesture-driven motion: velocity tracking, settling, rubber-banding.
 *
 * Implements the Spec §8.9 gesture-animation handoff:
 * - Tracking phase: element follows input 1:1 (no easing)
 * - Settling phase: after release, element animates to rest with velocity inheritance
 * - Rubber-banding: resistance when dragging beyond bounds (§8.9.4)
 *
 * Qt-free Foundation layer. Widget layer reads state to update positions.
 *
 * @see Matcha_Design_System_Specification.md §8.9
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <vector>

namespace matcha::fw {

// ============================================================================
// GesturePhase
// ============================================================================

/**
 * @enum GesturePhase
 * @brief Current phase of gesture-driven motion.
 */
enum class GesturePhase : uint8_t {
    Idle      = 0,   ///< No active gesture
    Tracking  = 1,   ///< Input is active, element follows 1:1
    Settling  = 2,   ///< Released, animating to rest position
};

// ============================================================================
// RubberBandConfig
// ============================================================================

/**
 * @struct RubberBandConfig
 * @brief Configuration for rubber-band resistance.
 */
struct RubberBandConfig {
    double resistance   = 0.55;   ///< Resistance factor [0,1]. Higher = stiffer.
    double maxStretch   = 100.0;  ///< Maximum overscroll distance (px)
    double snapBackMs   = 300.0;  ///< Duration to snap back after release
};

// ============================================================================
// VelocitySample
// ============================================================================

/**
 * @struct VelocitySample
 * @brief A single position+time sample for velocity estimation.
 */
struct VelocitySample {
    double position = 0.0;   ///< 1D position (px)
    double timeMs   = 0.0;   ///< Timestamp (ms, monotonic)
};

// ============================================================================
// GestureMotionTracker
// ============================================================================

/**
 * @class GestureMotionTracker
 * @brief Tracks gesture input and computes velocity for settling animation.
 *
 * Usage:
 * @code
 *   GestureMotionTracker tracker;
 *   tracker.SetBounds(0.0, 500.0);
 *
 *   // User starts dragging
 *   tracker.BeginTracking(100.0, 0.0);
 *   tracker.Track(120.0, 16.0);
 *   tracker.Track(150.0, 32.0);
 *
 *   // User releases
 *   tracker.EndTracking(160.0, 48.0);
 *   // tracker.Phase() == GesturePhase::Settling
 *   // tracker.ReleaseVelocity() ≈ 1.875 px/ms
 *
 *   // Tick settling (called from animation frame)
 *   tracker.TickSettling(16.0); // advance 16ms
 * @endcode
 */
class MATCHA_EXPORT GestureMotionTracker {
public:
    GestureMotionTracker() = default;

    // ====================================================================
    // Configuration
    // ====================================================================

    void SetBounds(double minPos, double maxPos);
    void SetRubberBand(RubberBandConfig config);
    void SetSnapTargets(std::vector<double> targets);

    // ====================================================================
    // Gesture lifecycle
    // ====================================================================

    /**
     * @brief Begin tracking a gesture.
     * @param position Initial position (px).
     * @param timeMs Timestamp (ms).
     */
    void BeginTracking(double position, double timeMs);

    /**
     * @brief Update position during gesture.
     * Applies rubber-banding if position is outside bounds.
     */
    void Track(double position, double timeMs);

    /**
     * @brief End the gesture and transition to Settling.
     */
    void EndTracking(double position, double timeMs);

    /**
     * @brief Cancel the gesture without settling.
     */
    void Cancel();

    // ====================================================================
    // Settling
    // ====================================================================

    /**
     * @brief Advance settling by dt milliseconds.
     * @return true if still settling, false if settled.
     */
    auto TickSettling(double dtMs) -> bool;

    // ====================================================================
    // Query
    // ====================================================================

    [[nodiscard]] auto Phase() const -> GesturePhase { return _phase; }
    [[nodiscard]] auto Position() const -> double { return _position; }
    [[nodiscard]] auto ReleaseVelocity() const -> double { return _releaseVelocity; }
    [[nodiscard]] auto IsOverscrolled() const -> bool;

    /**
     * @brief Compute rubber-banded position for a raw input beyond bounds.
     */
    [[nodiscard]] auto RubberBand(double rawPosition) const -> double;

private:
    void EstimateVelocity();
    auto FindSnapTarget() const -> double;

    GesturePhase   _phase = GesturePhase::Idle;
    double         _position        = 0.0;
    double         _releaseVelocity = 0.0;
    double         _minBound        = 0.0;
    double         _maxBound        = 1e9;
    double         _settleTarget    = 0.0;
    double         _settleProgress  = 0.0;  ///< [0, 1]
    double         _settleDuration  = 0.0;
    double         _settleStart     = 0.0;
    RubberBandConfig _rubberBand;
    std::vector<VelocitySample> _samples;
    std::vector<double> _snapTargets;

    static constexpr int kMaxSamples = 10;
    static constexpr double kVelocityThreshold = 0.01; ///< px/ms below which settling ends
    static constexpr double kDefaultSettleMs = 300.0;
};

} // namespace matcha::fw
