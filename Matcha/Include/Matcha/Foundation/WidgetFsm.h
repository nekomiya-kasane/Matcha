#pragma once

/**
 * @file WidgetFsm.h
 * @brief Single source of truth for all widget FSMs.
 *
 * Each widget namespace contains:
 * - State enum + Event enum
 * - constexpr transition table
 * - constexpr ToInteractionState mapping
 * - Controller type alias (WidgetFsmController instantiation)
 *
 * This file is the ONLY place to look for a widget's interaction model.
 *
 * Qt-free Foundation layer.
 *
 * @see StateMachine.h
 * @see WidgetFsmBridge.h for WidgetFsmController template definition
 * @see Matcha_Design_System_Specification.md §5.x
 */

#include <Matcha/Foundation/WidgetFsmBridge.h>

#include <array>
#include <cstdint>

namespace matcha::fw::fsm {

// ============================================================================
// SimpleWidget FSM — for widgets with basic hover/press/focus/disabled
// (CheckBox, RadioButton, ToggleSwitch, Slider, SpinBox, ToolButton, etc.)
// ============================================================================

namespace simple_widget {

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
    MouseUp,
    FocusIn,
    FocusOut,
    Disable,
    Enable,
};

using FSM = StateMachine<State, Event>;

inline constexpr std::array kTransitions = {
    // Normal
    FSM::Transition{State::Normal,   Event::MouseEnter, State::Hover},
    FSM::Transition{State::Normal,   Event::FocusIn,    State::Focused},
    FSM::Transition{State::Normal,   Event::Disable,    State::Disabled},

    // Hover
    FSM::Transition{State::Hover,    Event::MouseLeave, State::Normal},
    FSM::Transition{State::Hover,    Event::MouseDown,  State::Pressed},
    FSM::Transition{State::Hover,    Event::Disable,    State::Disabled},

    // Pressed
    FSM::Transition{State::Pressed,  Event::MouseUp,    State::Hover},
    FSM::Transition{State::Pressed,  Event::MouseLeave, State::Normal},
    FSM::Transition{State::Pressed,  Event::Disable,    State::Disabled},

    // Focused
    FSM::Transition{State::Focused,  Event::FocusOut,   State::Normal},
    FSM::Transition{State::Focused,  Event::MouseEnter, State::Hover},
    FSM::Transition{State::Focused,  Event::Disable,    State::Disabled},

    // Disabled
    FSM::Transition{State::Disabled, Event::Enable,     State::Normal},
};

[[nodiscard]] constexpr auto ToInteractionState(State s) -> InteractionState
{
    switch (s) {
    case State::Normal:   return InteractionState::Normal;
    case State::Hover:    return InteractionState::Hovered;
    case State::Pressed:  return InteractionState::Pressed;
    case State::Focused:  return InteractionState::Focused;
    case State::Disabled: return InteractionState::Disabled;
    }
    return InteractionState::Normal;
}

using Controller = WidgetFsmController<State, Event, ToInteractionState>;

} // namespace simple_widget

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

[[nodiscard]] constexpr auto ToInteractionState(State s) -> InteractionState
{
    switch (s) {
    case State::Normal:   return InteractionState::Normal;
    case State::Hover:    return InteractionState::Hovered;
    case State::Pressed:  return InteractionState::Pressed;
    case State::Focused:  return InteractionState::Focused;
    case State::Disabled: return InteractionState::Disabled;
    }
    return InteractionState::Normal;
}

using Controller = WidgetFsmController<State, Event, ToInteractionState>;

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

[[nodiscard]] constexpr auto ToInteractionState(State s) -> InteractionState
{
    switch (s) {
    case State::Normal:   return InteractionState::Normal;
    case State::Hover:    return InteractionState::Hovered;
    case State::Focused:  return InteractionState::Focused;
    case State::Error:    return InteractionState::Error;
    case State::Disabled: return InteractionState::Disabled;
    case State::ReadOnly: return InteractionState::Disabled;
    }
    return InteractionState::Normal;
}

using Controller = WidgetFsmController<State, Event, ToInteractionState>;

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

[[nodiscard]] constexpr auto ToInteractionState(State s) -> InteractionState
{
    switch (s) {
    case State::Closed:   return InteractionState::Normal;
    case State::Hover:    return InteractionState::Hovered;
    case State::Open:     return InteractionState::Focused;
    case State::Disabled: return InteractionState::Disabled;
    }
    return InteractionState::Normal;
}

using Controller = WidgetFsmController<State, Event, ToInteractionState>;

} // namespace combo_box

} // namespace matcha::fw::fsm

// ============================================================================
// Backward-compatible aliases at matcha::fw scope
// ============================================================================

namespace matcha::fw {

using SimpleWidgetFsmController = fsm::simple_widget::Controller;
using PushButtonFsmController   = fsm::push_button::Controller;
using LineEditFsmController     = fsm::line_edit::Controller;
using ComboBoxFsmController     = fsm::combo_box::Controller;

} // namespace matcha::fw
