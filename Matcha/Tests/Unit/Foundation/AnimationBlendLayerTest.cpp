#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file AnimationBlendLayerTest.cpp
 * @brief Unit tests for AnimationBlendLayer (D7).
 */

#include "doctest.h"

#include <Matcha/Foundation/AnimationBlendLayer.h>

using namespace matcha::fw;

TEST_SUITE("AnimationBlendLayer") {

// ============================================================================
// Entry management
// ============================================================================

TEST_CASE("SetEntry and GetEntry") {
    AnimationBlendLayer layer;
    layer.SetEntry("opacity", {.value = 0.5, .weight = 1.0, .progress = 0.3,
                                .animId = 42, .active = true});

    const auto* e = layer.GetEntry("opacity");
    REQUIRE(e != nullptr);
    CHECK(e->value == doctest::Approx(0.5));
    CHECK(e->weight == doctest::Approx(1.0));
    CHECK(e->progress == doctest::Approx(0.3));
    CHECK(e->animId == 42);
    CHECK(e->active);
}

TEST_CASE("GetEntry returns nullptr for unknown key") {
    AnimationBlendLayer layer;
    CHECK(layer.GetEntry("nonexistent") == nullptr);
}

TEST_CASE("UpdateValue updates existing entry") {
    AnimationBlendLayer layer;
    layer.SetEntry("opacity", {.value = 0.0, .weight = 1.0, .progress = 0.0,
                                .animId = 1, .active = true});

    CHECK(layer.UpdateValue("opacity", 0.75, 0.6));

    const auto* e = layer.GetEntry("opacity");
    CHECK(e->value == doctest::Approx(0.75));
    CHECK(e->progress == doctest::Approx(0.6));
}

TEST_CASE("UpdateValue returns false for unknown key") {
    AnimationBlendLayer layer;
    CHECK_FALSE(layer.UpdateValue("unknown", 1.0, 1.0));
}

TEST_CASE("Remove removes entry") {
    AnimationBlendLayer layer;
    layer.SetEntry("opacity", {.value = 0.5, .weight = 1.0, .progress = 0.5,
                                .animId = 1, .active = true});

    CHECK(layer.Remove("opacity"));
    CHECK(layer.GetEntry("opacity") == nullptr);
    CHECK(layer.EntryCount() == 0);
}

TEST_CASE("Remove returns false for unknown key") {
    AnimationBlendLayer layer;
    CHECK_FALSE(layer.Remove("nonexistent"));
}

TEST_CASE("HasActive checks active flag") {
    AnimationBlendLayer layer;
    layer.SetEntry("opacity", {.value = 0.5, .weight = 1.0, .progress = 0.5,
                                .animId = 1, .active = true});
    layer.SetEntry("color", {.value = 0.3, .weight = 1.0, .progress = 0.2,
                              .animId = 2, .active = false});

    CHECK(layer.HasActive("opacity"));
    CHECK_FALSE(layer.HasActive("color"));
    CHECK_FALSE(layer.HasActive("nonexistent"));
}

// ============================================================================
// Blending
// ============================================================================

TEST_CASE("Apply: active entry with weight=1 replaces base") {
    AnimationBlendLayer layer;
    layer.SetEntry("opacity", {.value = 0.3, .weight = 1.0, .progress = 0.5,
                                .animId = 1, .active = true});

    double result = layer.Apply("opacity", 1.0);
    CHECK(result == doctest::Approx(0.3));
}

TEST_CASE("Apply: active entry with weight=0.5 blends 50/50") {
    AnimationBlendLayer layer;
    layer.SetEntry("opacity", {.value = 0.0, .weight = 0.5, .progress = 0.5,
                                .animId = 1, .active = true});

    double result = layer.Apply("opacity", 1.0);
    // 1.0 * 0.5 + 0.0 * 0.5 = 0.5
    CHECK(result == doctest::Approx(0.5));
}

TEST_CASE("Apply: inactive entry returns base") {
    AnimationBlendLayer layer;
    layer.SetEntry("opacity", {.value = 0.0, .weight = 1.0, .progress = 1.0,
                                .animId = 1, .active = false});

    double result = layer.Apply("opacity", 0.8);
    CHECK(result == doctest::Approx(0.8));
}

TEST_CASE("Apply: missing entry returns base") {
    AnimationBlendLayer layer;
    double result = layer.Apply("nonexistent", 0.8);
    CHECK(result == doctest::Approx(0.8));
}

TEST_CASE("Apply: weight clamped to [0,1]") {
    AnimationBlendLayer layer;
    layer.SetEntry("x", {.value = 10.0, .weight = 2.0, .progress = 0.5,
                          .animId = 1, .active = true});

    double result = layer.Apply("x", 5.0);
    // weight clamped to 1.0 -> result = 10.0
    CHECK(result == doctest::Approx(10.0));
}

// ============================================================================
// Bulk operations
// ============================================================================

TEST_CASE("DeactivateAll marks all inactive") {
    AnimationBlendLayer layer;
    layer.SetEntry("a", {.value = 1.0, .weight = 1.0, .progress = 0.5,
                          .animId = 1, .active = true});
    layer.SetEntry("b", {.value = 2.0, .weight = 1.0, .progress = 0.5,
                          .animId = 2, .active = true});

    layer.DeactivateAll();
    CHECK(layer.ActiveCount() == 0);
    CHECK(layer.EntryCount() == 2); // still present
}

TEST_CASE("PurgeInactive removes inactive entries") {
    AnimationBlendLayer layer;
    layer.SetEntry("a", {.value = 1.0, .weight = 1.0, .progress = 1.0,
                          .animId = 1, .active = true});
    layer.SetEntry("b", {.value = 2.0, .weight = 1.0, .progress = 1.0,
                          .animId = 2, .active = false});

    layer.PurgeInactive();
    CHECK(layer.EntryCount() == 1);
    CHECK(layer.GetEntry("a") != nullptr);
    CHECK(layer.GetEntry("b") == nullptr);
}

TEST_CASE("Clear removes all entries") {
    AnimationBlendLayer layer;
    layer.SetEntry("a", {.value = 1.0, .weight = 1.0, .progress = 0.5,
                          .animId = 1, .active = true});
    layer.SetEntry("b", {.value = 2.0, .weight = 1.0, .progress = 0.5,
                          .animId = 2, .active = true});

    layer.Clear();
    CHECK(layer.EntryCount() == 0);
    CHECK(layer.ActiveCount() == 0);
}

TEST_CASE("ActiveCount counts only active entries") {
    AnimationBlendLayer layer;
    layer.SetEntry("a", {.value = 1.0, .weight = 1.0, .progress = 0.5,
                          .animId = 1, .active = true});
    layer.SetEntry("b", {.value = 2.0, .weight = 1.0, .progress = 0.5,
                          .animId = 2, .active = false});
    layer.SetEntry("c", {.value = 3.0, .weight = 1.0, .progress = 0.5,
                          .animId = 3, .active = true});

    CHECK(layer.ActiveCount() == 2);
    CHECK(layer.EntryCount() == 3);
}

} // TEST_SUITE
