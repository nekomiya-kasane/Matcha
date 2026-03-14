#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file PhaseFTest.cpp
 * @brief Unit tests for Phase F: DestructiveActionPolicy, EdgeCaseGuard, FeedbackPolicy.
 */

#include "doctest.h"

#include <Matcha/Foundation/DestructiveActionPolicy.h>
#include <Matcha/Foundation/EdgeCaseGuard.h>
#include <Matcha/Foundation/FeedbackPolicy.h>

using namespace matcha::fw;

// ============================================================================
// F4: DestructiveActionPolicy (§7.17)
// ============================================================================

TEST_SUITE("DestructiveActionPolicy") {

TEST_CASE("RequiredConfirmation maps severity correctly") {
    CHECK(DestructiveActionPolicy::RequiredConfirmation(ActionSeverity::Low)
          == ConfirmationMode::None);
    CHECK(DestructiveActionPolicy::RequiredConfirmation(ActionSeverity::Medium)
          == ConfirmationMode::SingleConfirm);
    CHECK(DestructiveActionPolicy::RequiredConfirmation(ActionSeverity::High)
          == ConfirmationMode::TwoStep);
    CHECK(DestructiveActionPolicy::RequiredConfirmation(ActionSeverity::Critical)
          == ConfirmationMode::TwoStepTyping);
}

TEST_CASE("BuildConfirmation: Low severity returns None mode") {
    auto desc = DestructiveActionPolicy::BuildConfirmation(
        ActionSeverity::Low, "Delete", "a node", true);
    CHECK(desc.mode == ConfirmationMode::None);
}

TEST_CASE("BuildConfirmation: Medium severity single confirm") {
    auto desc = DestructiveActionPolicy::BuildConfirmation(
        ActionSeverity::Medium, "Close", "unsaved document");
    CHECK(desc.mode == ConfirmationMode::SingleConfirm);
    CHECK(desc.title == "Close unsaved document?");
    CHECK(desc.confirmLabel == "Close");
    CHECK(desc.confirmWord.empty());
}

TEST_CASE("BuildConfirmation: High severity two-step") {
    auto desc = DestructiveActionPolicy::BuildConfirmation(
        ActionSeverity::High, "Reset", "all settings");
    CHECK(desc.mode == ConfirmationMode::TwoStep);
    CHECK(desc.description == "This cannot be undone.");
    CHECK(desc.confirmWord.empty());
}

TEST_CASE("BuildConfirmation: Critical severity two-step + typing") {
    auto desc = DestructiveActionPolicy::BuildConfirmation(
        ActionSeverity::Critical, "Delete", "entire database");
    CHECK(desc.mode == ConfirmationMode::TwoStepTyping);
    CHECK(desc.confirmWord == "DELETE");
    CHECK(desc.description == "This cannot be undone.");
}

TEST_CASE("BuildConfirmation: undo available changes description") {
    auto desc = DestructiveActionPolicy::BuildConfirmation(
        ActionSeverity::Medium, "Delete", "3 selected nodes", true);
    CHECK(desc.undoAvailable);
    CHECK(desc.description.find("Ctrl+Z") != std::string::npos);
}

TEST_CASE("MakeConfirmWord uppercases") {
    CHECK(DestructiveActionPolicy::MakeConfirmWord("Delete") == "DELETE");
    CHECK(DestructiveActionPolicy::MakeConfirmWord("reset") == "RESET");
    CHECK(DestructiveActionPolicy::MakeConfirmWord("Overwrite") == "OVERWRITE");
}

} // TEST_SUITE

// ============================================================================
// F5: EdgeCaseGuard (§7.18)
// ============================================================================

TEST_SUITE("EdgeCaseGuard") {

// -- ActionGuard --

TEST_CASE("ActionGuard: initial state is inactive") {
    ActionGuard guard;
    CHECK_FALSE(guard.IsActive());
}

TEST_CASE("ActionGuard: TryAcquire succeeds once") {
    ActionGuard guard;
    CHECK(guard.TryAcquire());
    CHECK(guard.IsActive());
    CHECK_FALSE(guard.TryAcquire()); // second attempt fails
}

TEST_CASE("ActionGuard: Release allows re-acquire") {
    ActionGuard guard;
    guard.TryAcquire();
    guard.Release();
    CHECK_FALSE(guard.IsActive());
    CHECK(guard.TryAcquire());
}

TEST_CASE("ActionGuard: Tick auto-releases on timeout") {
    ActionGuard guard;
    guard.SetTimeoutMs(100.0);
    guard.TryAcquire();

    CHECK_FALSE(guard.Tick(50.0));   // not yet
    CHECK(guard.IsActive());

    CHECK(guard.Tick(60.0));         // total 110ms > 100ms
    CHECK_FALSE(guard.IsActive());   // auto-released
}

TEST_CASE("ActionGuard: Tick does nothing when inactive") {
    ActionGuard guard;
    CHECK_FALSE(guard.Tick(1000.0));
}

// -- ReentrancyGuard --

TEST_CASE("ReentrancyGuard: initial state") {
    ReentrancyGuard guard;
    CHECK_FALSE(guard.IsInside());
    CHECK(guard.Depth() == 0);
}

TEST_CASE("ReentrancyGuard: TryEnter increments depth") {
    ReentrancyGuard guard;
    CHECK(guard.TryEnter());
    CHECK(guard.Depth() == 1);
    CHECK(guard.TryEnter());
    CHECK(guard.Depth() == 2);
}

TEST_CASE("ReentrancyGuard: max depth prevents entry") {
    ReentrancyGuard guard;
    for (int i = 0; i < ReentrancyGuard::kMaxDepth; ++i) {
        CHECK(guard.TryEnter());
    }
    CHECK_FALSE(guard.TryEnter()); // depth 8, max reached
    CHECK(guard.Depth() == ReentrancyGuard::kMaxDepth);
}

TEST_CASE("ReentrancyGuard: Leave decrements depth") {
    ReentrancyGuard guard;
    guard.TryEnter();
    guard.TryEnter();
    guard.Leave();
    CHECK(guard.Depth() == 1);
    guard.Leave();
    CHECK(guard.Depth() == 0);
    CHECK_FALSE(guard.IsInside());
}

TEST_CASE("ReentrancyGuard: Leave at zero is safe") {
    ReentrancyGuard guard;
    guard.Leave(); // should not crash
    CHECK(guard.Depth() == 0);
}

// -- InputModeTracker --

TEST_CASE("InputModeTracker: default is Mouse") {
    InputModeTracker tracker;
    CHECK(tracker.Mode() == InputMode::Mouse);
    CHECK_FALSE(tracker.ShouldShowFocusRing());
}

TEST_CASE("InputModeTracker: keyboard shows focus ring") {
    InputModeTracker tracker;
    tracker.OnKeyboardEvent();
    CHECK(tracker.Mode() == InputMode::Keyboard);
    CHECK(tracker.ShouldShowFocusRing());
}

TEST_CASE("InputModeTracker: mouse hides focus ring") {
    InputModeTracker tracker;
    tracker.OnKeyboardEvent();
    tracker.OnMouseEvent();
    CHECK(tracker.Mode() == InputMode::Mouse);
    CHECK_FALSE(tracker.ShouldShowFocusRing());
}

TEST_CASE("InputModeTracker: touch mode") {
    InputModeTracker tracker;
    tracker.OnTouchEvent();
    CHECK(tracker.Mode() == InputMode::Touch);
    CHECK_FALSE(tracker.ShouldShowFocusRing());
}

} // TEST_SUITE

// ============================================================================
// F6: FeedbackPolicy (§7.11)
// ============================================================================

TEST_SUITE("FeedbackPolicy") {

TEST_CASE("Classify thresholds") {
    CHECK(FeedbackPolicy::Classify(50.0) == ResponseTimeClass::Instant);
    CHECK(FeedbackPolicy::Classify(99.0) == ResponseTimeClass::Instant);
    CHECK(FeedbackPolicy::Classify(100.0) == ResponseTimeClass::Brief);
    CHECK(FeedbackPolicy::Classify(500.0) == ResponseTimeClass::Brief);
    CHECK(FeedbackPolicy::Classify(1000.0) == ResponseTimeClass::Noticeable);
    CHECK(FeedbackPolicy::Classify(5000.0) == ResponseTimeClass::Noticeable);
    CHECK(FeedbackPolicy::Classify(10000.0) == ResponseTimeClass::Long);
    CHECK(FeedbackPolicy::Classify(30000.0) == ResponseTimeClass::Long);
}

TEST_CASE("BuildFeedback: Instant needs no feedback") {
    auto fb = FeedbackPolicy::BuildFeedback(50.0, "Saving");
    CHECK(fb.timeClass == ResponseTimeClass::Instant);
    CHECK(fb.primary == FeedbackChannel::None);
    CHECK_FALSE(fb.showCancel);
}

TEST_CASE("BuildFeedback: Brief uses cursor change") {
    auto fb = FeedbackPolicy::BuildFeedback(500.0, "Loading");
    CHECK(fb.timeClass == ResponseTimeClass::Brief);
    CHECK(fb.primary == FeedbackChannel::CursorChange);
    CHECK(fb.secondary == FeedbackChannel::StatusBar);
}

TEST_CASE("BuildFeedback: Noticeable indeterminate uses spinner") {
    auto fb = FeedbackPolicy::BuildFeedback(3000.0, "Processing", false);
    CHECK(fb.timeClass == ResponseTimeClass::Noticeable);
    CHECK(fb.primary == FeedbackChannel::Spinner);
    CHECK_FALSE(fb.showProgress);
}

TEST_CASE("BuildFeedback: Noticeable determinate uses progress bar") {
    auto fb = FeedbackPolicy::BuildFeedback(3000.0, "Exporting", true);
    CHECK(fb.primary == FeedbackChannel::ProgressBar);
    CHECK(fb.showProgress);
}

TEST_CASE("BuildFeedback: Long uses progress bar + cancel + ETA") {
    auto fb = FeedbackPolicy::BuildFeedback(15000.0, "Importing");
    CHECK(fb.timeClass == ResponseTimeClass::Long);
    CHECK(fb.primary == FeedbackChannel::ProgressBar);
    CHECK(fb.showCancel);
    CHECK(fb.showProgress);
    CHECK(fb.showEta);
}

} // TEST_SUITE
