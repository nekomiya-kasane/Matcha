#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file CrossPhaseIntegrationTest.cpp
 * @brief Cross-phase integration tests verifying Foundation component interplay.
 *
 * Tests:
 * - WidgetFsmBridge: FSM state → InteractionState mapping
 * - WidgetFsmController: FSM + auto-mapping + callbacks
 * - FSM + ActionGuard: double-click prevention during async operations
 * - FSM + FeedbackPolicy: state-driven feedback selection
 * - FSM + ReentrancyGuard: notification handler safety
 * - FSM + InputModeTracker: focus ring management
 */

#include "doctest.h"

#include <Matcha/Feedback/DestructiveActionPolicy.h>
#include <Matcha/Feedback/EdgeCaseGuard.h>
#include <Matcha/Feedback/FeedbackPolicy.h>
#include <Matcha/Interaction/Input/KeyboardContract.h>
#include <Matcha/Tree/FSM/WidgetFsm.h>

using namespace matcha::fw;
namespace pb = matcha::fw::fsm::push_button;
namespace le = matcha::fw::fsm::line_edit;
namespace cb = matcha::fw::fsm::combo_box;

// ============================================================================
// WidgetFsmBridge: constexpr mapping tests
// ============================================================================

TEST_SUITE("WidgetFsmBridge") {

TEST_CASE("PushButton state mapping") {
    CHECK(pb::ToInteractionState(pb::State::Normal)   == InteractionState::Normal);
    CHECK(pb::ToInteractionState(pb::State::Hover)    == InteractionState::Hovered);
    CHECK(pb::ToInteractionState(pb::State::Pressed)  == InteractionState::Pressed);
    CHECK(pb::ToInteractionState(pb::State::Focused)  == InteractionState::Focused);
    CHECK(pb::ToInteractionState(pb::State::Disabled) == InteractionState::Disabled);
}

TEST_CASE("LineEdit state mapping") {
    CHECK(le::ToInteractionState(le::State::Normal)   == InteractionState::Normal);
    CHECK(le::ToInteractionState(le::State::Hover)    == InteractionState::Hovered);
    CHECK(le::ToInteractionState(le::State::Focused)  == InteractionState::Focused);
    CHECK(le::ToInteractionState(le::State::Error)    == InteractionState::Error);
    CHECK(le::ToInteractionState(le::State::Disabled) == InteractionState::Disabled);
    CHECK(le::ToInteractionState(le::State::ReadOnly) == InteractionState::Disabled);
}

TEST_CASE("ComboBox state mapping") {
    CHECK(cb::ToInteractionState(cb::State::Closed)   == InteractionState::Normal);
    CHECK(cb::ToInteractionState(cb::State::Hover)    == InteractionState::Hovered);
    CHECK(cb::ToInteractionState(cb::State::Open)     == InteractionState::Focused);
    CHECK(cb::ToInteractionState(cb::State::Disabled) == InteractionState::Disabled);
}

} // TEST_SUITE

// ============================================================================
// WidgetFsmController: full lifecycle tests
// ============================================================================

TEST_SUITE("WidgetFsmController") {

TEST_CASE("PushButton controller: basic lifecycle") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);

    CHECK(ctrl.FsmState() == pb::State::Normal);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);

    ctrl.Process(pb::Event::MouseEnter);
    CHECK(ctrl.FsmState() == pb::State::Hover);
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);

    ctrl.Process(pb::Event::MouseDown);
    CHECK(ctrl.FsmState() == pb::State::Pressed);
    CHECK(ctrl.GetInteractionState() == InteractionState::Pressed);

    ctrl.Process(pb::Event::MouseUpInside);
    CHECK(ctrl.FsmState() == pb::State::Hover);
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);
}

TEST_CASE("PushButton controller: InteractionState callback fires") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);

    int callCount = 0;
    InteractionState lastOld{};
    InteractionState lastNew{};

    ctrl.OnInteractionStateChanged([&](InteractionState o, InteractionState n) {
        ++callCount;
        lastOld = o;
        lastNew = n;
    });

    ctrl.Process(pb::Event::MouseEnter); // Normal → Hover
    CHECK(callCount == 1);
    CHECK(lastOld == InteractionState::Normal);
    CHECK(lastNew == InteractionState::Hovered);

    ctrl.Process(pb::Event::MouseDown); // Hover → Pressed
    CHECK(callCount == 2);
    CHECK(lastNew == InteractionState::Pressed);
}

TEST_CASE("PushButton controller: FSM transition callback fires") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);

    pb::State lastFrom{};
    pb::Event lastEvent{};
    pb::State lastTo{};

    ctrl.OnFsmTransition([&](pb::State from, pb::Event ev, pb::State to) {
        lastFrom = from;
        lastEvent = ev;
        lastTo = to;
    });

    ctrl.Process(pb::Event::TabFocus);
    CHECK(lastFrom == pb::State::Normal);
    CHECK(lastEvent == pb::Event::TabFocus);
    CHECK(lastTo == pb::State::Focused);
}

TEST_CASE("LineEdit controller: Error state maps to InteractionState::Error") {
    LineEditFsmController ctrl(le::State::Normal, le::kTransitions);

    ctrl.Process(le::Event::TabFocus);    // Normal → Focused
    CHECK(ctrl.GetInteractionState() == InteractionState::Focused);

    ctrl.Process(le::Event::ValidationFail); // Focused → Error
    CHECK(ctrl.FsmState() == le::State::Error);
    CHECK(ctrl.GetInteractionState() == InteractionState::Error);

    ctrl.Process(le::Event::TextChanged); // Error → Focused
    CHECK(ctrl.FsmState() == le::State::Focused);
    CHECK(ctrl.GetInteractionState() == InteractionState::Focused);
}

TEST_CASE("ComboBox controller: Open maps to Focused") {
    ComboBoxFsmController ctrl(cb::State::Closed, cb::kTransitions);

    ctrl.Process(cb::Event::MouseEnter);   // Closed → Hover
    ctrl.Process(cb::Event::ActivateOpen); // Hover → Open
    CHECK(ctrl.FsmState() == cb::State::Open);
    CHECK(ctrl.GetInteractionState() == InteractionState::Focused);

    ctrl.Process(cb::Event::SelectItem);   // Open → Closed
    CHECK(ctrl.FsmState() == cb::State::Closed);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("Controller Reset") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);
    ctrl.Process(pb::Event::Disable);
    CHECK(ctrl.GetInteractionState() == InteractionState::Disabled);

    ctrl.Reset(pb::State::Normal);
    CHECK(ctrl.FsmState() == pb::State::Normal);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("CanProcess") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);
    CHECK(ctrl.CanProcess(pb::Event::MouseEnter));
    CHECK_FALSE(ctrl.CanProcess(pb::Event::MouseUpInside));
}

} // TEST_SUITE

// ============================================================================
// Cross-phase: FSM + ActionGuard (double-click prevention)
// ============================================================================

TEST_SUITE("CrossPhase_FSM_ActionGuard") {

TEST_CASE("PushButton: ActionGuard blocks during async operation") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);
    ActionGuard guard;

    // First click
    ctrl.Process(pb::Event::MouseEnter);
    ctrl.Process(pb::Event::MouseDown);
    ctrl.Process(pb::Event::MouseUpInside);
    CHECK(guard.TryAcquire());
    CHECK(guard.IsActive());

    // Second click while guard active — should be blocked
    ctrl.Process(pb::Event::MouseDown);
    ctrl.Process(pb::Event::MouseUpInside);
    CHECK_FALSE(guard.TryAcquire());

    // After release
    guard.Release();
    CHECK(guard.TryAcquire());
}

TEST_CASE("PushButton: ActionGuard auto-release on timeout") {
    ActionGuard guard;
    guard.SetTimeoutMs(200.0);
    guard.TryAcquire();

    guard.Tick(100.0); // 100ms
    CHECK(guard.IsActive());

    guard.Tick(150.0); // 250ms > 200ms timeout
    CHECK_FALSE(guard.IsActive());
}

} // TEST_SUITE

// ============================================================================
// Cross-phase: FSM + FeedbackPolicy (state-driven feedback)
// ============================================================================

TEST_SUITE("CrossPhase_FSM_Feedback") {

TEST_CASE("Button click triggers appropriate feedback based on operation duration") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);

    // Simulate click
    ctrl.Process(pb::Event::MouseEnter);
    ctrl.Process(pb::Event::MouseDown);
    ctrl.Process(pb::Event::MouseUpInside);

    // Fast sync operation — no feedback needed
    auto fb = FeedbackPolicy::BuildFeedback(50.0, "Saving");
    CHECK(fb.primary == FeedbackChannel::None);

    // Slow async operation — spinner needed
    auto fb2 = FeedbackPolicy::BuildFeedback(3000.0, "Uploading");
    CHECK(fb2.primary == FeedbackChannel::Spinner);
}

} // TEST_SUITE

// ============================================================================
// Cross-phase: FSM + DestructiveActionPolicy
// ============================================================================

TEST_SUITE("CrossPhase_FSM_Destructive") {

TEST_CASE("Button pressed on destructive action checks severity") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);

    ctrl.Process(pb::Event::MouseEnter);
    ctrl.Process(pb::Event::MouseDown);
    ctrl.Process(pb::Event::MouseUpInside);
    CHECK(ctrl.FsmState() == pb::State::Hover);

    // Determine confirmation requirement
    auto desc = DestructiveActionPolicy::BuildConfirmation(
        ActionSeverity::High, "Delete", "all documents");
    CHECK(desc.mode == ConfirmationMode::TwoStep);
    CHECK(desc.description == "This cannot be undone.");
}

} // TEST_SUITE

// ============================================================================
// Cross-phase: FSM + ReentrancyGuard
// ============================================================================

TEST_SUITE("CrossPhase_FSM_Reentrancy") {

TEST_CASE("FSM transition callback protected by ReentrancyGuard") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);
    ReentrancyGuard guard;
    int handledCount = 0;

    ctrl.OnInteractionStateChanged([&](InteractionState, InteractionState) {
        if (!guard.TryEnter()) {
            return; // reentrant call blocked
        }
        ++handledCount;
        guard.Leave();
    });

    ctrl.Process(pb::Event::MouseEnter);
    ctrl.Process(pb::Event::MouseDown);
    ctrl.Process(pb::Event::MouseUpInside);
    CHECK(handledCount == 3);
}

} // TEST_SUITE

// ============================================================================
// Cross-phase: FSM + InputModeTracker
// ============================================================================

TEST_SUITE("CrossPhase_FSM_InputMode") {

TEST_CASE("Keyboard focus shows ring, mouse hover hides it") {
    PushButtonFsmController ctrl(pb::State::Normal, pb::kTransitions);
    InputModeTracker mode;

    // Tab focus → keyboard mode → show ring
    mode.OnKeyboardEvent();
    ctrl.Process(pb::Event::TabFocus);
    CHECK(ctrl.GetInteractionState() == InteractionState::Focused);
    CHECK(mode.ShouldShowFocusRing());

    // Mouse enters → mouse mode → hide ring
    mode.OnMouseEvent();
    ctrl.Process(pb::Event::MouseEnter);
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);
    CHECK_FALSE(mode.ShouldShowFocusRing());
}

} // TEST_SUITE
