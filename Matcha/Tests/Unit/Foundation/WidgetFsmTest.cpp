#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file WidgetFsmTest.cpp
 * @brief Unit tests for canonical widget FSM transition tables (E3).
 */

#include "doctest.h"

#include <Matcha/Foundation/WidgetFsm.h>

TEST_SUITE("WidgetFsm") {

// ============================================================================
// PushButton FSM (§5.1)
// ============================================================================

TEST_CASE("PushButton: Normal -> Hover -> Pressed -> Hover cycle") {
    using namespace matcha::fw::fsm::push_button;
    FSM fsm(State::Normal, kTransitions);

    CHECK(fsm.Process(Event::MouseEnter));
    CHECK(fsm.State() == State::Hover);

    CHECK(fsm.Process(Event::MouseDown));
    CHECK(fsm.State() == State::Pressed);

    CHECK(fsm.Process(Event::MouseUpInside));
    CHECK(fsm.State() == State::Hover);

    CHECK(fsm.Process(Event::MouseLeave));
    CHECK(fsm.State() == State::Normal);
}

TEST_CASE("PushButton: Pressed -> MouseUpOutside -> Normal") {
    using namespace matcha::fw::fsm::push_button;
    FSM fsm(State::Normal, kTransitions);

    fsm.Process(Event::MouseEnter);
    fsm.Process(Event::MouseDown);
    CHECK(fsm.State() == State::Pressed);

    CHECK(fsm.Process(Event::MouseUpOutside));
    CHECK(fsm.State() == State::Normal);
}

TEST_CASE("PushButton: Tab focus cycle") {
    using namespace matcha::fw::fsm::push_button;
    FSM fsm(State::Normal, kTransitions);

    CHECK(fsm.Process(Event::TabFocus));
    CHECK(fsm.State() == State::Focused);

    CHECK(fsm.Process(Event::SpaceDown));
    CHECK(fsm.State() == State::Pressed);
}

TEST_CASE("PushButton: Disable from any active state") {
    using namespace matcha::fw::fsm::push_button;

    // From Normal
    {
        FSM fsm(State::Normal, kTransitions);
        CHECK(fsm.Process(Event::Disable));
        CHECK(fsm.State() == State::Disabled);
    }
    // From Hover
    {
        FSM fsm(State::Normal, kTransitions);
        fsm.Process(Event::MouseEnter);
        CHECK(fsm.Process(Event::Disable));
        CHECK(fsm.State() == State::Disabled);
    }
    // From Focused
    {
        FSM fsm(State::Normal, kTransitions);
        fsm.Process(Event::TabFocus);
        CHECK(fsm.Process(Event::Disable));
        CHECK(fsm.State() == State::Disabled);
    }
}

TEST_CASE("PushButton: Enable from Disabled") {
    using namespace matcha::fw::fsm::push_button;
    FSM fsm(State::Disabled, kTransitions);

    CHECK(fsm.Process(Event::Enable));
    CHECK(fsm.State() == State::Normal);
}

TEST_CASE("PushButton: Disabled ignores input events") {
    using namespace matcha::fw::fsm::push_button;
    FSM fsm(State::Disabled, kTransitions);

    CHECK_FALSE(fsm.Process(Event::MouseEnter));
    CHECK_FALSE(fsm.Process(Event::MouseDown));
    CHECK_FALSE(fsm.Process(Event::TabFocus));
    CHECK(fsm.State() == State::Disabled);
}

// ============================================================================
// LineEdit FSM (§5.3)
// ============================================================================

TEST_CASE("LineEdit: Normal -> Hover -> Click -> Focused") {
    using namespace matcha::fw::fsm::line_edit;
    FSM fsm(State::Normal, kTransitions);

    CHECK(fsm.Process(Event::MouseEnter));
    CHECK(fsm.State() == State::Hover);

    CHECK(fsm.Process(Event::Click));
    CHECK(fsm.State() == State::Focused);
}

TEST_CASE("LineEdit: Tab focus and blur") {
    using namespace matcha::fw::fsm::line_edit;
    FSM fsm(State::Normal, kTransitions);

    CHECK(fsm.Process(Event::TabFocus));
    CHECK(fsm.State() == State::Focused);

    CHECK(fsm.Process(Event::FocusLost));
    CHECK(fsm.State() == State::Normal);
}

TEST_CASE("LineEdit: Focused -> Error -> edit back to Focused") {
    using namespace matcha::fw::fsm::line_edit;
    FSM fsm(State::Normal, kTransitions);

    fsm.Process(Event::TabFocus);
    CHECK(fsm.State() == State::Focused);

    CHECK(fsm.Process(Event::ValidationFail));
    CHECK(fsm.State() == State::Error);

    CHECK(fsm.Process(Event::TextChanged));
    CHECK(fsm.State() == State::Focused);
}

TEST_CASE("LineEdit: ReadOnly transition") {
    using namespace matcha::fw::fsm::line_edit;
    FSM fsm(State::Normal, kTransitions);

    CHECK(fsm.Process(Event::SetReadOnly));
    CHECK(fsm.State() == State::ReadOnly);

    CHECK(fsm.Process(Event::ClearReadOnly));
    CHECK(fsm.State() == State::Normal);
}

TEST_CASE("LineEdit: Disable/Enable") {
    using namespace matcha::fw::fsm::line_edit;
    FSM fsm(State::Normal, kTransitions);

    fsm.Process(Event::TabFocus);
    CHECK(fsm.Process(Event::Disable));
    CHECK(fsm.State() == State::Disabled);

    CHECK(fsm.Process(Event::Enable));
    CHECK(fsm.State() == State::Normal);
}

// ============================================================================
// ComboBox FSM (§5.6)
// ============================================================================

TEST_CASE("ComboBox: Closed -> Hover -> Open -> Select -> Closed") {
    using namespace matcha::fw::fsm::combo_box;
    FSM fsm(State::Closed, kTransitions);

    CHECK(fsm.Process(Event::MouseEnter));
    CHECK(fsm.State() == State::Hover);

    CHECK(fsm.Process(Event::ActivateOpen));
    CHECK(fsm.State() == State::Open);

    CHECK(fsm.Process(Event::SelectItem));
    CHECK(fsm.State() == State::Closed);
}

TEST_CASE("ComboBox: Open -> Escape -> Closed") {
    using namespace matcha::fw::fsm::combo_box;
    FSM fsm(State::Closed, kTransitions);

    fsm.Process(Event::ActivateOpen);
    CHECK(fsm.State() == State::Open);

    CHECK(fsm.Process(Event::Escape));
    CHECK(fsm.State() == State::Closed);
}

TEST_CASE("ComboBox: Open -> ArrowNavigate stays Open") {
    using namespace matcha::fw::fsm::combo_box;
    FSM fsm(State::Closed, kTransitions);

    fsm.Process(Event::ActivateOpen);
    CHECK(fsm.State() == State::Open);

    CHECK(fsm.Process(Event::ArrowNavigate));
    CHECK(fsm.State() == State::Open);
}

TEST_CASE("ComboBox: ClickOutside closes") {
    using namespace matcha::fw::fsm::combo_box;
    FSM fsm(State::Closed, kTransitions);

    fsm.Process(Event::ActivateOpen);
    CHECK(fsm.Process(Event::ClickOutside));
    CHECK(fsm.State() == State::Closed);
}

TEST_CASE("ComboBox: Disable/Enable") {
    using namespace matcha::fw::fsm::combo_box;
    FSM fsm(State::Closed, kTransitions);

    CHECK(fsm.Process(Event::Disable));
    CHECK(fsm.State() == State::Disabled);

    CHECK_FALSE(fsm.Process(Event::ActivateOpen)); // disabled
    CHECK(fsm.State() == State::Disabled);

    CHECK(fsm.Process(Event::Enable));
    CHECK(fsm.State() == State::Closed);
}

} // TEST_SUITE
