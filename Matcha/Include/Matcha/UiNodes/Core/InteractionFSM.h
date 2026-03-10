#pragma once

/**
 * @file InteractionFSM.h
 * @brief Finite State Machine for widget interaction states.
 *
 * InteractionFSM tracks the current InteractionState of a widget and
 * transitions between states based on input events (hover, press, focus,
 * enable/disable). When a transition occurs, it emits an
 * InteractionStateChanged notification via the owning WidgetNode.
 *
 * The FSM is embedded as a member in WidgetNode subclasses that need
 * state-dependent rendering (PushButton, CheckBox, ToggleSwitch, etc.).
 *
 * Transition table:
 *   Normal  + Enter     -> Hovered
 *   Normal  + Focus     -> Focused
 *   Hovered + Leave     -> Normal (or Focused if has focus)
 *   Hovered + Press     -> Pressed
 *   Hovered + Focus     -> Hovered (stays; focus is secondary)
 *   Pressed + Release   -> Hovered (if still inside) or Normal
 *   Pressed + Leave     -> Normal (cancel press)
 *   Focused + Enter     -> Hovered
 *   Focused + Blur      -> Normal
 *   Any     + Disable   -> Disabled
 *   Disabled+ Enable    -> Normal
 */

#include "Matcha/UiNodes/Core/TokenEnums.h"

namespace matcha::fw {

/**
 * @brief Input events that drive FSM transitions.
 */
enum class InteractionInput : uint8_t {
    Enter,    ///< Mouse entered widget bounds
    Leave,    ///< Mouse left widget bounds
    Press,    ///< Mouse button pressed
    Release,  ///< Mouse button released (still inside)
    Focus,    ///< Keyboard focus gained
    Blur,     ///< Keyboard focus lost
    Disable,  ///< Widget became disabled
    Enable,   ///< Widget became enabled
};

/**
 * @brief Lightweight FSM for widget interaction state tracking.
 *
 * Does NOT own the WidgetNode or emit notifications itself.
 * The caller (WidgetNode subclass) checks the return value of
 * HandleInput() and emits InteractionStateChanged if state changed.
 */
class InteractionFSM {
public:
    InteractionFSM() = default;

    [[nodiscard]] auto CurrentState() const -> InteractionState { return _state; }

    /**
     * @brief Process an input event and return the new state.
     *
     * If the state changed, the caller should emit InteractionStateChanged.
     *
     * @param input The input event.
     * @return The new state after the transition.
     */
    auto HandleInput(InteractionInput input) -> InteractionState
    {
        switch (_state) {
        case InteractionState::Normal:
            if (input == InteractionInput::Enter)   { _state = InteractionState::Hovered; }
            else if (input == InteractionInput::Focus) { _state = InteractionState::Focused; }
            else if (input == InteractionInput::Disable) { _state = InteractionState::Disabled; }
            break;

        case InteractionState::Hovered:
            if (input == InteractionInput::Leave)   { _state = _hasFocus ? InteractionState::Focused : InteractionState::Normal; }
            else if (input == InteractionInput::Press) { _state = InteractionState::Pressed; }
            else if (input == InteractionInput::Disable) { _state = InteractionState::Disabled; }
            // Focus/Blur while hovered: stay hovered, just track focus flag
            else if (input == InteractionInput::Focus) { _hasFocus = true; }
            else if (input == InteractionInput::Blur)  { _hasFocus = false; }
            break;

        case InteractionState::Pressed:
            if (input == InteractionInput::Release) { _state = InteractionState::Hovered; }
            else if (input == InteractionInput::Leave) { _state = InteractionState::Normal; _hasFocus = false; }
            else if (input == InteractionInput::Disable) { _state = InteractionState::Disabled; }
            break;

        case InteractionState::Focused:
            if (input == InteractionInput::Enter)   { _state = InteractionState::Hovered; _hasFocus = true; }
            else if (input == InteractionInput::Blur) { _state = InteractionState::Normal; _hasFocus = false; }
            else if (input == InteractionInput::Disable) { _state = InteractionState::Disabled; }
            break;

        case InteractionState::Disabled:
            if (input == InteractionInput::Enable) { _state = InteractionState::Normal; _hasFocus = false; }
            break;

        case InteractionState::Selected:
        case InteractionState::Error:
        case InteractionState::DragOver:
        case InteractionState::Count_:
            // These states are managed externally, not via FSM inputs.
            if (input == InteractionInput::Disable) { _state = InteractionState::Disabled; }
            break;
        }

        // Track focus flag
        if (input == InteractionInput::Focus) { _hasFocus = true; }
        else if (input == InteractionInput::Blur) { _hasFocus = false; }

        return _state;
    }

    /**
     * @brief Force-set the state (e.g., for ReadOnly or initial state).
     */
    void ForceState(InteractionState state) { _state = state; }

    /**
     * @brief Check if state changed after HandleInput.
     * Convenience: compare prev with current.
     */
    [[nodiscard]] auto HasFocus() const -> bool { return _hasFocus; }

private:
    InteractionState _state = InteractionState::Normal;
    bool _hasFocus = false;
};

} // namespace matcha::fw
