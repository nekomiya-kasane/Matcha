#pragma once

/**
 * @file WidgetFsm.h
 * @brief Canonical FSM transition tables for core widgets (Spec §5.x).
 *
 * Each widget kind has:
 * - A State enum (uint8_t)
 * - An Event enum (uint8_t)
 * - A constexpr transition table (std::array)
 * - A type alias for StateMachine<State, Event>
 *
 * These transition tables are the single source of truth for widget
 * interaction behavior. The Widget layer's Qt event handlers should
 * dispatch events into these FSMs.
 *
 * Qt-free Foundation layer.
 *
 * @see Matcha_Design_System_Specification.md §5.1, §5.3, §5.6
 * @see StateMachine.h
 */

#include <Matcha/Foundation/StateMachine.h>

#include <array>
#include <cstdint>

namespace matcha::fw::fsm {

// ============================================================================
// PushButton FSM (§5.1)
// ============================================================================

namespace push_button {

enum class State : uint8_t {
    Normal,
    Hover,
    Pressed,
    Focused,
    Disabled,
};

enum class Event : uint8_t {
    MouseEnter,
    MouseLeave,
    MouseDown,
    MouseUpInside,
    MouseUpOutside,
    TabFocus,
    TabAway,
    SpaceDown,
    EnterDown,
    Disable,
    Enable,
};

using FSM = StateMachine<State, Event>;

inline constexpr std::array kTransitions = {
    // Normal
    FSM::Transition{State::Normal,  Event::MouseEnter,    State::Hover},
    FSM::Transition{State::Normal,  Event::TabFocus,      State::Focused},
    FSM::Transition{State::Normal,  Event::Disable,       State::Disabled},

    // Hover
    FSM::Transition{State::Hover,   Event::MouseLeave,    State::Normal},
    FSM::Transition{State::Hover,   Event::MouseDown,     State::Pressed},
    FSM::Transition{State::Hover,   Event::Disable,       State::Disabled},

    // Pressed
    FSM::Transition{State::Pressed, Event::MouseUpInside, State::Hover},
    FSM::Transition{State::Pressed, Event::MouseUpOutside,State::Normal},

    // Focused
    FSM::Transition{State::Focused, Event::TabAway,       State::Normal},
    FSM::Transition{State::Focused, Event::SpaceDown,     State::Pressed},
    FSM::Transition{State::Focused, Event::EnterDown,     State::Pressed},
    FSM::Transition{State::Focused, Event::MouseEnter,    State::Hover},
    FSM::Transition{State::Focused, Event::Disable,       State::Disabled},

    // Disabled
    FSM::Transition{State::Disabled, Event::Enable,       State::Normal},
};

} // namespace push_button

// ============================================================================
// LineEdit FSM (§5.3)
// ============================================================================

namespace line_edit {

enum class State : uint8_t {
    Normal,
    Hover,
    Focused,
    Error,
    Disabled,
    ReadOnly,
};

enum class Event : uint8_t {
    MouseEnter,
    MouseLeave,
    Click,
    TabFocus,
    FocusLost,
    TextChanged,
    ValidationFail,
    ValidationPass,
    Disable,
    Enable,
    SetReadOnly,
    ClearReadOnly,
};

using FSM = StateMachine<State, Event>;

inline constexpr std::array kTransitions = {
    // Normal
    FSM::Transition{State::Normal,   Event::MouseEnter,     State::Hover},
    FSM::Transition{State::Normal,   Event::TabFocus,       State::Focused},
    FSM::Transition{State::Normal,   Event::Disable,        State::Disabled},
    FSM::Transition{State::Normal,   Event::SetReadOnly,    State::ReadOnly},

    // Hover
    FSM::Transition{State::Hover,    Event::MouseLeave,     State::Normal},
    FSM::Transition{State::Hover,    Event::Click,          State::Focused},
    FSM::Transition{State::Hover,    Event::Disable,        State::Disabled},

    // Focused
    FSM::Transition{State::Focused,  Event::FocusLost,      State::Normal},
    FSM::Transition{State::Focused,  Event::ValidationFail, State::Error},
    FSM::Transition{State::Focused,  Event::Disable,        State::Disabled},

    // Error
    FSM::Transition{State::Error,    Event::TextChanged,    State::Focused},
    FSM::Transition{State::Error,    Event::ValidationPass, State::Focused},
    FSM::Transition{State::Error,    Event::FocusLost,      State::Normal},
    FSM::Transition{State::Error,    Event::Disable,        State::Disabled},

    // Disabled
    FSM::Transition{State::Disabled, Event::Enable,         State::Normal},

    // ReadOnly
    FSM::Transition{State::ReadOnly, Event::ClearReadOnly,  State::Normal},
};

} // namespace line_edit

// ============================================================================
// ComboBox FSM (§5.6)
// ============================================================================

namespace combo_box {

enum class State : uint8_t {
    Closed,
    Hover,
    Open,
    Disabled,
};

enum class Event : uint8_t {
    MouseEnter,
    MouseLeave,
    ActivateOpen,      ///< Click / Space / Enter / Alt+Down
    ClickOutside,      ///< Click outside / Tab
    Escape,
    SelectItem,        ///< Enter or Click on item
    ArrowNavigate,     ///< Arrow keys while open (stays Open)
    Disable,
    Enable,
};

using FSM = StateMachine<State, Event>;

inline constexpr std::array kTransitions = {
    // Closed
    FSM::Transition{State::Closed,   Event::MouseEnter,    State::Hover},
    FSM::Transition{State::Closed,   Event::ActivateOpen,  State::Open},
    FSM::Transition{State::Closed,   Event::Disable,       State::Disabled},

    // Hover
    FSM::Transition{State::Hover,    Event::MouseLeave,    State::Closed},
    FSM::Transition{State::Hover,    Event::ActivateOpen,  State::Open},
    FSM::Transition{State::Hover,    Event::Disable,       State::Disabled},

    // Open
    FSM::Transition{State::Open,     Event::ClickOutside,  State::Closed},
    FSM::Transition{State::Open,     Event::Escape,        State::Closed},
    FSM::Transition{State::Open,     Event::SelectItem,    State::Closed},
    FSM::Transition{State::Open,     Event::ArrowNavigate, State::Open},

    // Disabled
    FSM::Transition{State::Disabled, Event::Enable,        State::Closed},
};

} // namespace combo_box

} // namespace matcha::fw::fsm
