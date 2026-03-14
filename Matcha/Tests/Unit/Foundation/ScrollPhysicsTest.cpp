#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file ScrollPhysicsTest.cpp
 * @brief Unit tests for ScrollPhysics (§7.3).
 */

#include "doctest.h"

#include <Matcha/Foundation/ScrollPhysics.h>

#include <cmath>

using namespace matcha::fw;

TEST_SUITE("ScrollPhysics") {

// ============================================================================
// Basic setup
// ============================================================================

TEST_CASE("Default construction: position=0, not animating") {
    ScrollPhysics sp;
    CHECK(sp.Position() == doctest::Approx(0.0));
    CHECK(sp.Velocity() == doctest::Approx(0.0));
    CHECK_FALSE(sp.IsAnimating());
}

TEST_CASE("MaxPosition = contentSize - viewportSize, clamped >= 0") {
    ScrollPhysics sp;
    sp.SetContentSize(1000.0);
    sp.SetViewportSize(300.0);
    CHECK(sp.MaxPosition() == doctest::Approx(700.0));

    sp.SetContentSize(100.0);
    sp.SetViewportSize(300.0);
    CHECK(sp.MaxPosition() == doctest::Approx(0.0)); // content < viewport
}

// ============================================================================
// ScrollBy
// ============================================================================

TEST_CASE("ScrollBy moves position within bounds") {
    ScrollPhysics sp;
    sp.SetContentSize(1000.0);
    sp.SetViewportSize(300.0);

    sp.ScrollBy(100.0);
    CHECK(sp.Position() == doctest::Approx(100.0));

    sp.ScrollBy(800.0); // would overshoot 700 max
    CHECK(sp.Position() == doctest::Approx(700.0)); // clamped
}

TEST_CASE("ScrollBy clamps at 0") {
    ScrollPhysics sp;
    sp.SetContentSize(1000.0);
    sp.SetViewportSize(300.0);

    sp.ScrollBy(-100.0);
    CHECK(sp.Position() == doctest::Approx(0.0));
}

// ============================================================================
// ScrollTo
// ============================================================================

TEST_CASE("ScrollTo sets exact position") {
    ScrollPhysics sp;
    sp.SetContentSize(1000.0);
    sp.SetViewportSize(300.0);

    sp.ScrollTo(500.0);
    CHECK(sp.Position() == doctest::Approx(500.0));
    CHECK_FALSE(sp.IsAnimating());

    sp.ScrollTo(9999.0);
    CHECK(sp.Position() == doctest::Approx(700.0)); // clamped
}

// ============================================================================
// Fling + momentum (§7.3.1)
// ============================================================================

TEST_CASE("Fling starts animation with velocity") {
    ScrollPhysics sp;
    sp.SetContentSize(5000.0);
    sp.SetViewportSize(600.0);

    sp.Fling(2000.0);
    CHECK(sp.IsAnimating());
    CHECK(sp.Velocity() != doctest::Approx(0.0));
}

TEST_CASE("Fling: velocity decays and eventually stops") {
    ScrollPhysics sp;
    sp.SetContentSize(50000.0);
    sp.SetViewportSize(600.0);

    sp.Fling(1000.0);

    int steps = 0;
    while (sp.IsAnimating() && steps < 600) { // max 10s at 60fps
        sp.Step(1.0 / 60.0);
        ++steps;
    }

    CHECK_FALSE(sp.IsAnimating());
    CHECK(sp.Position() > 0.0); // moved forward
    CHECK(steps > 5); // took multiple frames
    CHECK(steps < 600); // but didn't run forever
}

TEST_CASE("Fling: velocity is clamped to maxInitialVelocity") {
    ScrollPhysics sp;
    sp.SetContentSize(100000.0);
    sp.SetViewportSize(600.0);

    sp.Fling(99999.0); // exceeds default 5000
    // Velocity should be clamped: 5000/60 ≈ 83.33 px/frame
    CHECK(std::abs(sp.Velocity()) <= doctest::Approx(5000.0 / 60.0).epsilon(0.01));
}

TEST_CASE("Fling: stops at boundary (Clamp mode)") {
    ScrollPhysics sp;
    sp.SetContentSize(500.0);
    sp.SetViewportSize(400.0);
    // maxPos = 100

    sp.Fling(3000.0);
    for (int i = 0; i < 300; ++i) {
        sp.Step(1.0 / 60.0);
    }

    CHECK(sp.Position() <= doctest::Approx(100.0));
    CHECK(sp.Position() >= doctest::Approx(0.0));
}

// ============================================================================
// Overscroll: Bounce (§7.3.2)
// ============================================================================

TEST_CASE("Bounce mode: overscroll displacement then spring back") {
    ScrollConfig cfg;
    cfg.overscrollMode = OverscrollMode::Bounce;
    ScrollPhysics sp(cfg);
    sp.SetContentSize(1000.0);
    sp.SetViewportSize(600.0);
    // maxPos = 400

    sp.ScrollTo(400.0);
    sp.Fling(3000.0); // will overshoot

    bool sawOverscroll = false;
    for (int i = 0; i < 600; ++i) {
        sp.Step(1.0 / 60.0);
        if (std::abs(sp.Overscroll()) > 0.1) {
            sawOverscroll = true;
        }
        if (!sp.IsAnimating()) {
            break;
        }
    }

    CHECK(sawOverscroll);
    // After settling, position should be back within bounds
    CHECK(sp.Position() <= doctest::Approx(400.0).epsilon(1.0));
}

// ============================================================================
// Stop
// ============================================================================

TEST_CASE("Stop halts animation immediately") {
    ScrollPhysics sp;
    sp.SetContentSize(5000.0);
    sp.SetViewportSize(600.0);
    sp.Fling(2000.0);
    CHECK(sp.IsAnimating());

    sp.Stop();
    CHECK_FALSE(sp.IsAnimating());
    CHECK(sp.Velocity() == doctest::Approx(0.0));
}

// ============================================================================
// ScrollToVisible (§7.3.5)
// ============================================================================

TEST_CASE("ScrollToVisible: already visible -> no change") {
    ScrollPhysics sp;
    sp.SetContentSize(2000.0);
    sp.SetViewportSize(500.0);
    sp.ScrollTo(100.0);

    sp.ScrollToVisible(200.0, 50.0); // fully within [100, 600]
    CHECK(sp.Position() == doctest::Approx(100.0));
}

TEST_CASE("ScrollToVisible: target above viewport -> scroll up") {
    ScrollPhysics sp;
    sp.SetContentSize(2000.0);
    sp.SetViewportSize(500.0);
    sp.ScrollTo(300.0);

    sp.ScrollToVisible(100.0, 50.0); // above viewport [300, 800]
    CHECK(sp.Position() == doctest::Approx(100.0));
}

TEST_CASE("ScrollToVisible: target below viewport -> scroll down") {
    ScrollPhysics sp;
    sp.SetContentSize(2000.0);
    sp.SetViewportSize(500.0);
    sp.ScrollTo(0.0);

    sp.ScrollToVisible(600.0, 50.0); // below viewport [0, 500]
    CHECK(sp.Position() == doctest::Approx(150.0)); // 650 - 500
}

// ============================================================================
// Snap points (§7.3.3)
// ============================================================================

TEST_CASE("NearestSnapPoint: no snap points returns input") {
    ScrollPhysics sp;
    CHECK(sp.NearestSnapPoint(123.0) == doctest::Approx(123.0));
}

TEST_CASE("NearestSnapPoint: finds closest") {
    ScrollPhysics sp;
    sp.SetSnapPoints({0.0, 100.0, 200.0, 300.0});

    CHECK(sp.NearestSnapPoint(45.0) == doctest::Approx(0.0));    // closer to 0
    CHECK(sp.NearestSnapPoint(55.0) == doctest::Approx(100.0));  // closer to 100
    CHECK(sp.NearestSnapPoint(250.0) == doctest::Approx(200.0)); // equidistant, prefer before
    CHECK(sp.NearestSnapPoint(280.0) == doctest::Approx(300.0));
}

TEST_CASE("NearestSnapPoint: beyond range snaps to edge") {
    ScrollPhysics sp;
    sp.SetSnapPoints({100.0, 200.0});

    CHECK(sp.NearestSnapPoint(50.0) == doctest::Approx(100.0));
    CHECK(sp.NearestSnapPoint(999.0) == doctest::Approx(200.0));
}

TEST_CASE("Fling with snap points: settles at snap") {
    ScrollConfig cfg;
    cfg.overscrollMode = OverscrollMode::Clamp;
    ScrollPhysics sp(cfg);
    sp.SetContentSize(2000.0);
    sp.SetViewportSize(500.0);
    sp.SetSnapPoints({0.0, 200.0, 400.0, 600.0, 800.0});

    sp.Fling(500.0); // gentle fling
    for (int i = 0; i < 600; ++i) {
        sp.Step(1.0 / 60.0);
        if (!sp.IsAnimating()) {
            break;
        }
    }

    // Should have settled at one of the snap points
    const double pos = sp.Position();
    bool atSnap = (std::abs(pos - 0.0) < 1.0 ||
                   std::abs(pos - 200.0) < 1.0 ||
                   std::abs(pos - 400.0) < 1.0 ||
                   std::abs(pos - 600.0) < 1.0 ||
                   std::abs(pos - 800.0) < 1.0);
    CHECK(atSnap);
}

// ============================================================================
// Config
// ============================================================================

TEST_CASE("Config: custom friction changes deceleration rate") {
    ScrollConfig cfg;
    cfg.friction = 0.80; // much higher friction
    ScrollPhysics sp(cfg);
    sp.SetContentSize(50000.0);
    sp.SetViewportSize(600.0);
    sp.Fling(2000.0);

    int steps = 0;
    while (sp.IsAnimating() && steps < 600) {
        sp.Step(1.0 / 60.0);
        ++steps;
    }

    // Higher friction -> fewer steps to stop
    CHECK(steps < 100);
}

} // TEST_SUITE
