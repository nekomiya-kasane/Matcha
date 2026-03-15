#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file ContextualHelpServiceTest.cpp
 * @brief Unit tests for ContextualHelpService.
 */

#include "doctest.h"

#include <Matcha/Interaction/ContextualHelpService.h>

using namespace matcha::fw;

namespace {

auto MakeWalkthrough(std::string id, int stepCount) -> Walkthrough
{
    Walkthrough wt;
    wt.id = std::move(id);
    wt.name = "Test Walkthrough";
    wt.description = "A test walkthrough";
    for (int i = 0; i < stepCount; ++i) {
        wt.steps.push_back({
            .targetId = "target_" + std::to_string(i),
            .title = "Step " + std::to_string(i),
            .body = "Body " + std::to_string(i),
            .anchor = CoachMarkAnchor::Auto,
            .optional = false,
        });
    }
    return wt;
}

} // namespace

TEST_SUITE("ContextualHelpService") {

// ============================================================================
// Registration
// ============================================================================

TEST_CASE("RegisterWalkthrough and FindWalkthrough") {
    ContextualHelpService svc;
    CHECK(svc.RegisterWalkthrough(MakeWalkthrough("tour1", 3)));
    CHECK(svc.WalkthroughCount() == 1);

    const auto* wt = svc.FindWalkthrough("tour1");
    REQUIRE(wt != nullptr);
    CHECK(wt->steps.size() == 3);
}

TEST_CASE("RegisterWalkthrough rejects duplicate ID") {
    ContextualHelpService svc;
    CHECK(svc.RegisterWalkthrough(MakeWalkthrough("tour1", 2)));
    CHECK_FALSE(svc.RegisterWalkthrough(MakeWalkthrough("tour1", 5)));
    CHECK(svc.WalkthroughCount() == 1);
}

TEST_CASE("UnregisterWalkthrough removes entry") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour1", 2));
    CHECK(svc.UnregisterWalkthrough("tour1"));
    CHECK(svc.WalkthroughCount() == 0);
    CHECK(svc.FindWalkthrough("tour1") == nullptr);
}

TEST_CASE("UnregisterWalkthrough returns false for unknown") {
    ContextualHelpService svc;
    CHECK_FALSE(svc.UnregisterWalkthrough("nonexistent"));
}

// ============================================================================
// Start / state
// ============================================================================

TEST_CASE("Initial state is Idle") {
    ContextualHelpService svc;
    CHECK(svc.State() == WalkthroughState::Idle);
    CHECK(svc.CurrentStep() == nullptr);
    CHECK(svc.TotalSteps() == 0);
}

TEST_CASE("Start transitions to Active") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour1", 3));
    CHECK(svc.Start("tour1"));
    CHECK(svc.State() == WalkthroughState::Active);
    CHECK(svc.ActiveWalkthroughId() == "tour1");
    CHECK(svc.CurrentStepIndex() == 0);
    CHECK(svc.TotalSteps() == 3);
}

TEST_CASE("Start returns false for unknown walkthrough") {
    ContextualHelpService svc;
    CHECK_FALSE(svc.Start("nonexistent"));
    CHECK(svc.State() == WalkthroughState::Idle);
}

TEST_CASE("Start returns false if already active") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour1", 2));
    svc.RegisterWalkthrough(MakeWalkthrough("tour2", 2));
    svc.Start("tour1");
    CHECK_FALSE(svc.Start("tour2"));
}

TEST_CASE("Start returns false for empty walkthrough") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("empty", 0));
    CHECK_FALSE(svc.Start("empty"));
}

// ============================================================================
// Navigation: Next / Previous / GoToStep
// ============================================================================

TEST_CASE("Next advances step") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 3));
    svc.Start("tour");

    svc.Next();
    CHECK(svc.CurrentStepIndex() == 1);
    CHECK(svc.CurrentStep()->title == "Step 1");
}

TEST_CASE("Next at last step completes walkthrough") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 2));
    svc.Start("tour");

    svc.Next(); // step 1
    svc.Next(); // completes
    CHECK(svc.State() == WalkthroughState::Completed);
    CHECK(svc.IsCompleted("tour"));
}

TEST_CASE("Previous goes back") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 3));
    svc.Start("tour");

    svc.Next(); // step 1
    svc.Previous(); // back to step 0
    CHECK(svc.CurrentStepIndex() == 0);
}

TEST_CASE("Previous at first step is no-op") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 3));
    svc.Start("tour");

    svc.Previous();
    CHECK(svc.CurrentStepIndex() == 0);
}

TEST_CASE("GoToStep jumps to specific step") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 5));
    svc.Start("tour");

    CHECK(svc.GoToStep(3));
    CHECK(svc.CurrentStepIndex() == 3);
    CHECK(svc.CurrentStep()->title == "Step 3");
}

TEST_CASE("GoToStep out of range returns false") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 3));
    svc.Start("tour");

    CHECK_FALSE(svc.GoToStep(-1));
    CHECK_FALSE(svc.GoToStep(3));
    CHECK(svc.CurrentStepIndex() == 0); // unchanged
}

// ============================================================================
// Pause / Resume / Dismiss
// ============================================================================

TEST_CASE("Pause and Resume") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 3));
    svc.Start("tour");
    svc.Next(); // step 1

    svc.Pause();
    CHECK(svc.State() == WalkthroughState::Paused);
    CHECK(svc.CurrentStepIndex() == 1); // preserved

    svc.Resume();
    CHECK(svc.State() == WalkthroughState::Active);
    CHECK(svc.CurrentStepIndex() == 1); // still there
}

TEST_CASE("Dismiss cancels walkthrough") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 3));
    svc.Start("tour");

    svc.Dismiss();
    CHECK(svc.State() == WalkthroughState::Dismissed);
    CHECK(svc.ActiveWalkthroughId().empty());
    CHECK_FALSE(svc.IsCompleted("tour")); // dismissed, not completed
}

// ============================================================================
// Progress
// ============================================================================

TEST_CASE("Progress reports fraction") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 4));
    svc.Start("tour");

    CHECK(svc.Progress() == doctest::Approx(0.25)); // 1/4
    svc.Next();
    CHECK(svc.Progress() == doctest::Approx(0.5)); // 2/4
    svc.Next();
    CHECK(svc.Progress() == doctest::Approx(0.75)); // 3/4
}

// ============================================================================
// Callbacks
// ============================================================================

TEST_CASE("StepChanged callback fires") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 3));

    int lastStep = -1;
    svc.OnStepChanged([&](int idx, const WalkthroughStep&) { lastStep = idx; });

    svc.Start("tour");
    CHECK(lastStep == 0);
    svc.Next();
    CHECK(lastStep == 1);
}

TEST_CASE("StateChanged callback fires") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 2));

    std::vector<WalkthroughState> states;
    svc.OnStateChanged([&](WalkthroughState s) { states.push_back(s); });

    svc.Start("tour");
    svc.Pause();
    svc.Resume();
    svc.Dismiss();

    REQUIRE(states.size() == 4);
    CHECK(states[0] == WalkthroughState::Active);
    CHECK(states[1] == WalkthroughState::Paused);
    CHECK(states[2] == WalkthroughState::Active);
    CHECK(states[3] == WalkthroughState::Dismissed);
}

// ============================================================================
// IsCompleted tracking
// ============================================================================

TEST_CASE("IsCompleted tracks completed walkthroughs") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour", 1));

    CHECK_FALSE(svc.IsCompleted("tour"));
    svc.Start("tour");
    svc.Next(); // completes 1-step tour
    CHECK(svc.IsCompleted("tour"));
}

// ============================================================================
// Can restart after completion
// ============================================================================

TEST_CASE("Can start a new tour after previous completes") {
    ContextualHelpService svc;
    svc.RegisterWalkthrough(MakeWalkthrough("tour1", 1));
    svc.RegisterWalkthrough(MakeWalkthrough("tour2", 2));

    svc.Start("tour1");
    svc.Next(); // completes

    CHECK(svc.Start("tour2"));
    CHECK(svc.State() == WalkthroughState::Active);
    CHECK(svc.ActiveWalkthroughId() == "tour2");
}

} // TEST_SUITE
