#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file ChoreographyEngineTest.cpp
 * @brief Unit tests for ChoreographyEngine (Stagger, Cascade, Sequence).
 */

#include "doctest.h"

#include <Matcha/Foundation/ChoreographyEngine.h>

using namespace matcha::fw;

TEST_SUITE("ChoreographyEngine") {

// ============================================================================
// Stagger (§8.8.1)
// ============================================================================

TEST_CASE("Stagger: default config, 8 items") {
    auto schedule = ChoreographyEngine::ComputeStagger(8);
    REQUIRE(schedule.size() == 8);

    // delay_i = 0 + i * 30
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
    CHECK(schedule[1].delayMs == doctest::Approx(30.0));
    CHECK(schedule[2].delayMs == doctest::Approx(60.0));
    CHECK(schedule[7].delayMs == doctest::Approx(210.0));

    // All within maxStagger (300ms)
    for (const auto& e : schedule) {
        CHECK(e.delayMs <= 300.0);
        CHECK(e.durationMs == doctest::Approx(150.0));
    }
}

TEST_CASE("Stagger: delay capped at maxStagger") {
    StaggerConfig cfg;
    cfg.staggerIntervalMs = 50.0;
    cfg.maxStaggerMs = 100.0;

    auto schedule = ChoreographyEngine::ComputeStagger(5, cfg);
    REQUIRE(schedule.size() == 5);

    // item 0: 0, item 1: 50, item 2: 100, item 3: capped 100, item 4: capped 100
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
    CHECK(schedule[1].delayMs == doctest::Approx(50.0));
    CHECK(schedule[2].delayMs == doctest::Approx(100.0));
    CHECK(schedule[3].delayMs == doctest::Approx(100.0)); // capped
    CHECK(schedule[4].delayMs == doctest::Approx(100.0)); // capped
}

TEST_CASE("Stagger: zero items returns empty") {
    auto schedule = ChoreographyEngine::ComputeStagger(0);
    CHECK(schedule.empty());
}

TEST_CASE("Stagger: single item has zero delay") {
    auto schedule = ChoreographyEngine::ComputeStagger(1);
    REQUIRE(schedule.size() == 1);
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
}

TEST_CASE("Stagger: custom baseDelay") {
    StaggerConfig cfg;
    cfg.baseDelayMs = 50.0;
    cfg.staggerIntervalMs = 20.0;

    auto schedule = ChoreographyEngine::ComputeStagger(3, cfg);
    CHECK(schedule[0].delayMs == doctest::Approx(50.0));
    CHECK(schedule[1].delayMs == doctest::Approx(70.0));
    CHECK(schedule[2].delayMs == doctest::Approx(90.0));
}

TEST_CASE("Stagger: TotalDuration") {
    auto schedule = ChoreographyEngine::ComputeStagger(4);
    // last item: delay=90, duration=150 -> total=240
    double total = ChoreographyEngine::TotalDuration(schedule);
    CHECK(total == doctest::Approx(240.0));
}

// ============================================================================
// Cascade (§8.8.2)
// ============================================================================

TEST_CASE("Cascade: default config, 3 levels") {
    auto schedule = ChoreographyEngine::ComputeCascade(3);
    REQUIRE(schedule.size() == 3);

    // Level 0: delay=0, duration=200
    CHECK(schedule[0].depth == 0);
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
    CHECK(schedule[0].durationMs == doctest::Approx(200.0));
    CHECK_FALSE(schedule[0].instant);

    // Level 1: delay = 0.6 * 200 = 120
    CHECK(schedule[1].depth == 1);
    CHECK(schedule[1].delayMs == doctest::Approx(120.0));
    CHECK(schedule[1].durationMs == doctest::Approx(200.0));
    CHECK_FALSE(schedule[1].instant);

    // Level 2: delay = 120 + 0.6*200 = 240
    CHECK(schedule[2].depth == 2);
    CHECK(schedule[2].delayMs == doctest::Approx(240.0));
    CHECK(schedule[2].durationMs == doctest::Approx(200.0));
    CHECK_FALSE(schedule[2].instant);
}

TEST_CASE("Cascade: beyond maxDepth is instant") {
    auto schedule = ChoreographyEngine::ComputeCascade(5);
    REQUIRE(schedule.size() == 5);

    // Levels 0,1,2: normal. Levels 3,4: instant.
    CHECK_FALSE(schedule[0].instant);
    CHECK_FALSE(schedule[1].instant);
    CHECK_FALSE(schedule[2].instant);
    CHECK(schedule[3].instant);
    CHECK(schedule[3].durationMs == doctest::Approx(0.0));
    CHECK(schedule[4].instant);
}

TEST_CASE("Cascade: zero depth returns empty") {
    auto schedule = ChoreographyEngine::ComputeCascade(0);
    CHECK(schedule.empty());
}

TEST_CASE("Cascade: single level") {
    auto schedule = ChoreographyEngine::ComputeCascade(1);
    REQUIRE(schedule.size() == 1);
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
    CHECK(schedule[0].durationMs == doctest::Approx(200.0));
}

TEST_CASE("Cascade: custom trigger fraction") {
    CascadeConfig cfg;
    cfg.triggerFraction = 0.5;
    cfg.levelDurationMs = 100.0;

    auto schedule = ChoreographyEngine::ComputeCascade(3, cfg);
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
    CHECK(schedule[1].delayMs == doctest::Approx(50.0));   // 0.5 * 100
    CHECK(schedule[2].delayMs == doctest::Approx(100.0));  // 50 + 0.5*100
}

TEST_CASE("Cascade: TotalDuration") {
    auto schedule = ChoreographyEngine::ComputeCascade(3);
    // Level 2: delay=240, duration=200 -> end=440
    double total = ChoreographyEngine::TotalDuration(schedule);
    CHECK(total == doctest::Approx(440.0));
}

// ============================================================================
// Sequence (§8.8.3)
// ============================================================================

TEST_CASE("Sequence: fade out -> resize -> fade in") {
    std::vector<SequenceStep> steps = {
        {.id = "fadeOut", .durationMs = 150.0},
        {.id = "resize",  .durationMs = 200.0},
        {.id = "fadeIn",  .durationMs = 150.0},
    };

    auto schedule = ChoreographyEngine::ComputeSequence(steps);
    REQUIRE(schedule.size() == 3);

    CHECK(schedule[0].id == "fadeOut");
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
    CHECK(schedule[0].durationMs == doctest::Approx(150.0));

    CHECK(schedule[1].id == "resize");
    CHECK(schedule[1].delayMs == doctest::Approx(150.0));
    CHECK(schedule[1].durationMs == doctest::Approx(200.0));

    CHECK(schedule[2].id == "fadeIn");
    CHECK(schedule[2].delayMs == doctest::Approx(350.0));
    CHECK(schedule[2].durationMs == doctest::Approx(150.0));
}

TEST_CASE("Sequence: empty steps returns empty") {
    auto schedule = ChoreographyEngine::ComputeSequence({});
    CHECK(schedule.empty());
}

TEST_CASE("Sequence: single step") {
    std::vector<SequenceStep> steps = {
        {.id = "only", .durationMs = 100.0},
    };
    auto schedule = ChoreographyEngine::ComputeSequence(steps);
    REQUIRE(schedule.size() == 1);
    CHECK(schedule[0].delayMs == doctest::Approx(0.0));
}

TEST_CASE("Sequence: TotalDuration") {
    std::vector<SequenceStep> steps = {
        {.id = "a", .durationMs = 100.0},
        {.id = "b", .durationMs = 200.0},
        {.id = "c", .durationMs = 50.0},
    };
    auto schedule = ChoreographyEngine::ComputeSequence(steps);
    double total = ChoreographyEngine::TotalDuration(schedule);
    CHECK(total == doctest::Approx(350.0));
}

TEST_CASE("TotalDuration: empty schedule returns 0") {
    CHECK(ChoreographyEngine::TotalDuration(
        std::vector<StaggerScheduleEntry>{}) == doctest::Approx(0.0));
    CHECK(ChoreographyEngine::TotalDuration(
        std::vector<CascadeScheduleEntry>{}) == doctest::Approx(0.0));
    CHECK(ChoreographyEngine::TotalDuration(
        std::vector<SequenceScheduleEntry>{}) == doctest::Approx(0.0));
}

} // TEST_SUITE
