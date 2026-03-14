#pragma once

/**
 * @file WidgetFsmBridge.h
 * @brief WidgetFsmController template — wraps StateMachine + auto-maps to InteractionState.
 *
 * This file provides the generic template only. Per-widget FSM definitions,
 * state mappings, and Controller aliases live in WidgetFsm.h (single source of truth).
 *
 * Qt-free Foundation layer.
 *
 * @see WidgetFsm.h for all widget FSM definitions
 */

#include <Matcha/Foundation/StateMachine.h>
#include <Matcha/UiNodes/Core/TokenEnums.h>

#include <functional>
#include <span>

namespace matcha::fw {

/**
 * @class WidgetFsmController
 * @tparam S Widget-specific State enum
 * @tparam E Widget-specific Event enum
 * @tparam MapFn constexpr function: S → InteractionState
 *
 * Owns a StateMachine<S,E> and provides:
 * - Process(E) → processes event, maps to InteractionState
 * - GetInteractionState() → current mapped state for Theme::Resolve()
 * - OnInteractionStateChanged callback
 * - OnFsmTransition callback for widget-specific logic
 *
 * Usage:
 * @code
 *   namespace pb = matcha::fw::fsm::push_button;
 *   pb::Controller ctrl(pb::State::Normal, pb::kTransitions);
 *   ctrl.OnInteractionStateChanged([](InteractionState o, InteractionState n) { ... });
 *   ctrl.Process(pb::Event::MouseEnter);
 * @endcode
 */
template <typename S, typename E, InteractionState (*MapFn)(S)>
class WidgetFsmController {
public:
    using StateChangedCallback = std::function<void(InteractionState /*old*/, InteractionState /*new*/)>;
    using FsmTransitionCallback = std::function<void(S /*from*/, E /*event*/, S /*to*/)>;

    explicit WidgetFsmController(S initial,
                                  std::span<const typename StateMachine<S, E>::Transition> transitions)
        : _fsm(initial, transitions)
        , _interactionState(MapFn(initial))
    {
        _fsm.OnTransition([this](S from, E event, S to) {
            const auto oldIS = _interactionState;
            _interactionState = MapFn(to);
            if (_fsmCallback) {
                _fsmCallback(from, event, to);
            }
            if (oldIS != _interactionState && _isCallback) {
                _isCallback(oldIS, _interactionState);
            }
        });
    }

    /**
     * @brief Process an event. Returns true if FSM transitioned.
     */
    auto Process(E event) -> bool { return _fsm.Process(event); }

    /**
     * @brief Current widget-specific FSM state.
     */
    [[nodiscard]] auto FsmState() const -> S { return _fsm.State(); }

    /**
     * @brief Current mapped InteractionState for Theme::Resolve().
     */
    [[nodiscard]] auto GetInteractionState() const -> InteractionState { return _interactionState; }

    /**
     * @brief Register callback for InteractionState changes (theme-relevant).
     */
    void OnInteractionStateChanged(StateChangedCallback cb) { _isCallback = std::move(cb); }

    /**
     * @brief Register callback for raw FSM transitions (widget-specific logic).
     */
    void OnFsmTransition(FsmTransitionCallback cb) { _fsmCallback = std::move(cb); }

    /**
     * @brief Check if event would cause a transition.
     */
    [[nodiscard]] auto CanProcess(E event) const -> bool { return _fsm.CanProcess(event); }

    /**
     * @brief Reset to a specific state.
     */
    void Reset(S state)
    {
        _fsm.Reset(state);
        _interactionState = MapFn(state);
    }

    /**
     * @brief Access underlying StateMachine for guard setup etc.
     */
    [[nodiscard]] auto Fsm() -> StateMachine<S, E>& { return _fsm; }
    [[nodiscard]] auto Fsm() const -> const StateMachine<S, E>& { return _fsm; }

private:
    StateMachine<S, E>    _fsm;
    InteractionState      _interactionState;
    StateChangedCallback  _isCallback;
    FsmTransitionCallback _fsmCallback;
};

} // namespace matcha::fw
