#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file SharedElementTransitionTest.cpp
 * @brief Unit tests for SharedElementTransition.
 */

#include "doctest.h"

#include <Matcha/Animation/SharedElementTransition.h>

using namespace matcha::fw;

TEST_SUITE("SharedElementTransition") {

TEST_CASE("Interpolate t=0 returns source rect") {
    SharedElementTransition tx({
        .source = {10, 20, 100, 30},
        .target = {200, 50, 150, 40},
        .durationMs = 200,
        .opacity = 1.0,
        .crossFade = false,
    });

    auto s = tx.Interpolate(0.0);
    CHECK(s.rect.x == doctest::Approx(10.0));
    CHECK(s.rect.y == doctest::Approx(20.0));
    CHECK(s.rect.width == doctest::Approx(100.0));
    CHECK(s.rect.height == doctest::Approx(30.0));
}

TEST_CASE("Interpolate t=1 returns target rect") {
    SharedElementTransition tx({
        .source = {10, 20, 100, 30},
        .target = {200, 50, 150, 40},
        .durationMs = 200,
        .opacity = 1.0,
        .crossFade = false,
    });

    auto s = tx.Interpolate(1.0);
    CHECK(s.rect.x == doctest::Approx(200.0));
    CHECK(s.rect.y == doctest::Approx(50.0));
    CHECK(s.rect.width == doctest::Approx(150.0));
    CHECK(s.rect.height == doctest::Approx(40.0));
}

TEST_CASE("Interpolate t=0.5 returns midpoint") {
    SharedElementTransition tx({
        .source = {0, 0, 100, 100},
        .target = {100, 200, 200, 200},
        .durationMs = 200,
        .opacity = 1.0,
        .crossFade = false,
    });

    auto s = tx.Interpolate(0.5);
    CHECK(s.rect.x == doctest::Approx(50.0));
    CHECK(s.rect.y == doctest::Approx(100.0));
    CHECK(s.rect.width == doctest::Approx(150.0));
    CHECK(s.rect.height == doctest::Approx(150.0));
}

TEST_CASE("Interpolate clamps t to [0,1]") {
    SharedElementTransition tx({
        .source = {0, 0, 100, 100},
        .target = {100, 100, 200, 200},
        .durationMs = 200,
        .opacity = 1.0,
        .crossFade = false,
    });

    auto sNeg = tx.Interpolate(-0.5);
    CHECK(sNeg.rect.x == doctest::Approx(0.0));

    auto sOver = tx.Interpolate(1.5);
    CHECK(sOver.rect.x == doctest::Approx(100.0));
}

TEST_CASE("CrossFade: opacity transitions") {
    SharedElementTransition tx({
        .source = {0, 0, 100, 100},
        .target = {100, 0, 100, 100},
        .durationMs = 200,
        .opacity = 0.8,
        .crossFade = true,
    });

    auto s0 = tx.Interpolate(0.0);
    CHECK(s0.sourceOpacity == doctest::Approx(1.0));
    CHECK(s0.targetOpacity == doctest::Approx(0.0));

    auto s05 = tx.Interpolate(0.5);
    CHECK(s05.sourceOpacity == doctest::Approx(0.5));
    CHECK(s05.targetOpacity == doctest::Approx(0.5));

    auto s1 = tx.Interpolate(1.0);
    CHECK(s1.sourceOpacity == doctest::Approx(0.0));
    CHECK(s1.targetOpacity == doctest::Approx(1.0));
}

TEST_CASE("Non-crossFade: source/target opacity are 0") {
    SharedElementTransition tx({
        .source = {0, 0, 100, 100},
        .target = {100, 0, 100, 100},
        .durationMs = 200,
        .opacity = 0.7,
        .crossFade = false,
    });

    auto s = tx.Interpolate(0.5);
    CHECK(s.sourceOpacity == doctest::Approx(0.0));
    CHECK(s.targetOpacity == doctest::Approx(0.0));
    CHECK(s.proxyOpacity == doctest::Approx(0.7));
}

TEST_CASE("DurationMs accessor") {
    SharedElementTransition tx({
        .source = {}, .target = {},
        .durationMs = 300,
        .opacity = 1.0,
        .crossFade = false,
    });
    CHECK(tx.DurationMs() == doctest::Approx(300.0));
}

} // TEST_SUITE
