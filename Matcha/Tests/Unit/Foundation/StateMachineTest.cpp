#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Foundation/StateMachine.h>

#include <array>
#include <cstdint>
#include <vector>

using namespace matcha::fw;

// ============================================================================
// Test enums — models a PushButton interaction FSM
// ============================================================================

namespace {

enum class BtnState : uint8_t { Idle, Hovered, Pressed, Disabled };
enum class BtnEvent : uint8_t { Enter, Leave, Press, Release, Disable, Enable };

using BtnFSM = StateMachine<BtnState, BtnEvent>;

} // anonymous namespace

static constexpr std::array kBtnTransitions = {
    BtnFSM::Transition{BtnState::Idle,     BtnEvent::Enter,   BtnState::Hovered},
    BtnFSM::Transition{BtnState::Hovered,  BtnEvent::Leave,   BtnState::Idle},
    BtnFSM::Transition{BtnState::Hovered,  BtnEvent::Press,   BtnState::Pressed},
    BtnFSM::Transition{BtnState::Pressed,  BtnEvent::Release, BtnState::Hovered},
    BtnFSM::Transition{BtnState::Pressed,  BtnEvent::Leave,   BtnState::Idle},
    BtnFSM::Transition{BtnState::Idle,     BtnEvent::Disable, BtnState::Disabled},
    BtnFSM::Transition{BtnState::Hovered,  BtnEvent::Disable, BtnState::Disabled},
    BtnFSM::Transition{BtnState::Disabled, BtnEvent::Enable,  BtnState::Idle},
};

TEST_SUITE("fw::StateMachine") {

// ============================================================================
// Construction & Initial State
// ============================================================================

TEST_CASE("Initial state is set by constructor") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);
    CHECK(fsm.State() == BtnState::Idle);
}

// ============================================================================
// Basic Transitions
// ============================================================================

TEST_CASE("Process valid event transitions state") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);

    CHECK(fsm.Process(BtnEvent::Enter));
    CHECK(fsm.State() == BtnState::Hovered);

    CHECK(fsm.Process(BtnEvent::Press));
    CHECK(fsm.State() == BtnState::Pressed);

    CHECK(fsm.Process(BtnEvent::Release));
    CHECK(fsm.State() == BtnState::Hovered);

    CHECK(fsm.Process(BtnEvent::Leave));
    CHECK(fsm.State() == BtnState::Idle);
}

TEST_CASE("Process invalid event returns false, state unchanged") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);

    // No transition for Release from Idle
    CHECK_FALSE(fsm.Process(BtnEvent::Release));
    CHECK(fsm.State() == BtnState::Idle);

    // No transition for Press from Idle
    CHECK_FALSE(fsm.Process(BtnEvent::Press));
    CHECK(fsm.State() == BtnState::Idle);
}

// ============================================================================
// CanProcess
// ============================================================================

TEST_CASE("CanProcess returns true for valid transitions") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);
    CHECK(fsm.CanProcess(BtnEvent::Enter));
    CHECK(fsm.CanProcess(BtnEvent::Disable));
    CHECK_FALSE(fsm.CanProcess(BtnEvent::Press));
    CHECK_FALSE(fsm.CanProcess(BtnEvent::Release));
}

// ============================================================================
// OnTransition callback
// ============================================================================

TEST_CASE("OnTransition callback fires on valid transition") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);

    struct Record {
        BtnState from;
        BtnEvent event;
        BtnState to;
    };
    std::vector<Record> log;

    fsm.OnTransition([&log](BtnState from, BtnEvent ev, BtnState to) {
        log.push_back({from, ev, to});
    });

    fsm.Process(BtnEvent::Enter);
    fsm.Process(BtnEvent::Press);
    fsm.Process(BtnEvent::Release);

    REQUIRE(log.size() == 3);
    CHECK(log[0].from == BtnState::Idle);
    CHECK(log[0].event == BtnEvent::Enter);
    CHECK(log[0].to == BtnState::Hovered);
    CHECK(log[1].from == BtnState::Hovered);
    CHECK(log[1].to == BtnState::Pressed);
    CHECK(log[2].from == BtnState::Pressed);
    CHECK(log[2].to == BtnState::Hovered);
}

TEST_CASE("OnTransition does not fire on invalid transition") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);

    int callCount = 0;
    fsm.OnTransition([&callCount](BtnState, BtnEvent, BtnState) { ++callCount; });

    fsm.Process(BtnEvent::Release);  // invalid
    CHECK(callCount == 0);
}

// ============================================================================
// Guard
// ============================================================================

TEST_CASE("Guard blocks transition when returning false") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);

    // Block Enter transition
    fsm.SetGuard([](BtnState, BtnEvent ev, BtnState) {
        return ev != BtnEvent::Enter;
    });

    CHECK_FALSE(fsm.Process(BtnEvent::Enter));
    CHECK(fsm.State() == BtnState::Idle);

    // Disable still works
    CHECK(fsm.Process(BtnEvent::Disable));
    CHECK(fsm.State() == BtnState::Disabled);
}

// ============================================================================
// Reset
// ============================================================================

TEST_CASE("Reset sets state without transition") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);

    int callCount = 0;
    fsm.OnTransition([&callCount](BtnState, BtnEvent, BtnState) { ++callCount; });

    fsm.Process(BtnEvent::Enter);
    CHECK(callCount == 1);

    fsm.Reset(BtnState::Disabled);
    CHECK(fsm.State() == BtnState::Disabled);
    CHECK(callCount == 1);  // no callback for Reset
}

// ============================================================================
// Disabled state cycle
// ============================================================================

TEST_CASE("Full disable/enable cycle") {
    BtnFSM fsm(BtnState::Idle, kBtnTransitions);

    fsm.Process(BtnEvent::Enter);    // -> Hovered
    fsm.Process(BtnEvent::Disable);  // -> Disabled
    CHECK(fsm.State() == BtnState::Disabled);

    // While disabled, Enter/Press etc. are ignored
    CHECK_FALSE(fsm.Process(BtnEvent::Enter));
    CHECK_FALSE(fsm.Process(BtnEvent::Press));
    CHECK(fsm.State() == BtnState::Disabled);

    fsm.Process(BtnEvent::Enable);   // -> Idle
    CHECK(fsm.State() == BtnState::Idle);
}

} // TEST_SUITE
