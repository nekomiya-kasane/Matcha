#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file GestureMotionTrackerTest.cpp
 * @brief Unit tests for GestureMotionTracker (D5 + D6).
 */

#include "doctest.h"

#include <Matcha/Foundation/GestureMotionTracker.h>

#include <cmath>

using namespace matcha::fw;

TEST_SUITE("GestureMotionTracker") {

// ============================================================================
// Basic lifecycle
// ============================================================================

TEST_CASE("Initial state is Idle") {
    GestureMotionTracker t;
    CHECK(t.Phase() == GesturePhase::Idle);
    CHECK(t.Position() == doctest::Approx(0.0));
}

TEST_CASE("BeginTracking sets Tracking phase") {
    GestureMotionTracker t;
    t.BeginTracking(100.0, 0.0);
    CHECK(t.Phase() == GesturePhase::Tracking);
    CHECK(t.Position() == doctest::Approx(100.0));
}

TEST_CASE("Track updates position within bounds") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.BeginTracking(100.0, 0.0);
    t.Track(200.0, 16.0);
    CHECK(t.Position() == doctest::Approx(200.0));
}

TEST_CASE("EndTracking transitions to Settling") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.BeginTracking(100.0, 0.0);
    t.Track(150.0, 16.0);
    t.EndTracking(200.0, 32.0);
    CHECK(t.Phase() == GesturePhase::Settling);
}

TEST_CASE("Cancel returns to Idle") {
    GestureMotionTracker t;
    t.BeginTracking(100.0, 0.0);
    t.Cancel();
    CHECK(t.Phase() == GesturePhase::Idle);
}

// ============================================================================
// Velocity estimation
// ============================================================================

TEST_CASE("Velocity estimation from samples") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 1000.0);
    t.BeginTracking(100.0, 0.0);
    t.Track(120.0, 16.0);
    t.Track(150.0, 32.0);
    t.EndTracking(180.0, 48.0);

    // Velocity from last two samples: (180-150)/(48-32) = 30/16 = 1.875 px/ms
    CHECK(t.ReleaseVelocity() == doctest::Approx(1.875));
}

TEST_CASE("Zero velocity when stationary") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.BeginTracking(100.0, 0.0);
    t.Track(100.0, 16.0);
    t.EndTracking(100.0, 32.0);
    CHECK(t.ReleaseVelocity() == doctest::Approx(0.0));
}

// ============================================================================
// Settling
// ============================================================================

TEST_CASE("TickSettling progresses and completes") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.BeginTracking(100.0, 0.0);
    t.Track(100.0, 16.0);
    t.EndTracking(100.0, 32.0);

    // With zero velocity, settle target = position
    // Tick should complete quickly
    bool stillSettling = t.TickSettling(500.0); // way past duration
    CHECK_FALSE(stillSettling);
    CHECK(t.Phase() == GesturePhase::Idle);
}

TEST_CASE("TickSettling animates incrementally") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.BeginTracking(0.0, 0.0);
    t.Track(50.0, 16.0);
    t.EndTracking(100.0, 32.0);

    // Should be in Settling, not at final position yet after small tick
    CHECK(t.Phase() == GesturePhase::Settling);
    bool still = t.TickSettling(10.0);
    CHECK(still);
    CHECK(t.Phase() == GesturePhase::Settling);
}

// ============================================================================
// Rubber-banding (D6)
// ============================================================================

TEST_CASE("RubberBand: within bounds returns raw position") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    CHECK(t.RubberBand(250.0) == doctest::Approx(250.0));
}

TEST_CASE("RubberBand: beyond max bound is dampened") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);

    double rb = t.RubberBand(600.0);
    // Should be > 500 but < 600
    CHECK(rb > 500.0);
    CHECK(rb < 600.0);
}

TEST_CASE("RubberBand: beyond min bound is dampened") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);

    double rb = t.RubberBand(-100.0);
    // Should be < 0 but > -100
    CHECK(rb < 0.0);
    CHECK(rb > -100.0);
}

TEST_CASE("RubberBand: large overscroll approaches maxStretch") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.SetRubberBand({.resistance = 0.55, .maxStretch = 50.0, .snapBackMs = 300.0});

    double rb = t.RubberBand(10000.0); // very large overscroll
    // Should approach maxBound + maxStretch = 550
    CHECK(rb < 550.1);
    CHECK(rb > 540.0);
}

TEST_CASE("IsOverscrolled during tracking beyond bounds") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.BeginTracking(490.0, 0.0);
    t.Track(550.0, 16.0); // beyond max
    CHECK(t.IsOverscrolled());
}

TEST_CASE("Overscroll snap-back on EndTracking") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.BeginTracking(490.0, 0.0);
    t.Track(550.0, 16.0);
    t.EndTracking(560.0, 32.0);

    CHECK(t.Phase() == GesturePhase::Settling);

    // Tick until settled
    for (int i = 0; i < 100; ++i) {
        if (!t.TickSettling(10.0)) {
            break;
        }
    }

    // Should snap back to maxBound
    CHECK(t.Position() == doctest::Approx(500.0));
    CHECK(t.Phase() == GesturePhase::Idle);
}

// ============================================================================
// Snap targets
// ============================================================================

TEST_CASE("Snap targets: settles to nearest target") {
    GestureMotionTracker t;
    t.SetBounds(0.0, 500.0);
    t.SetSnapTargets({0.0, 100.0, 200.0, 300.0, 400.0, 500.0});

    t.BeginTracking(145.0, 0.0);
    t.Track(155.0, 16.0);
    t.EndTracking(160.0, 32.0);

    // With positive velocity and position ~160, should snap toward 200
    // Tick until settled
    for (int i = 0; i < 100; ++i) {
        if (!t.TickSettling(10.0)) {
            break;
        }
    }

    // Should be at a snap target
    double pos = t.Position();
    bool atSnap = false;
    for (double s : {0.0, 100.0, 200.0, 300.0, 400.0, 500.0}) {
        if (std::abs(pos - s) < 1.0) {
            atSnap = true;
            break;
        }
    }
    CHECK(atSnap);
}

} // TEST_SUITE
