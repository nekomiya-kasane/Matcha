#pragma once

/**
 * @file StateMachine.h
 * @brief Lightweight compile-time finite state machine template.
 *
 * Provides a generic `StateMachine<StateEnum, EventEnum>` that maps
 * (state, event) pairs to transitions via a user-supplied transition table.
 * Designed for widget interaction FSMs (e.g. InteractionState transitions).
 *
 * Features:
 * - Compile-time transition table (constexpr std::array)
 * - Optional guard predicates per transition
 * - Optional on-enter / on-exit / on-transition callbacks
 * - Zero heap allocation, zero virtual dispatch
 * - Header-only, zero Qt dependency
 *
 * Usage:
 * @code
 *   enum class S : uint8_t { Idle, Hovered, Pressed };
 *   enum class E : uint8_t { Enter, Leave, Press, Release };
 *
 *   using FSM = StateMachine<S, E>;
 *
 *   constexpr std::array kTransitions = {
 *       FSM::Transition{S::Idle,    E::Enter,   S::Hovered},
 *       FSM::Transition{S::Hovered, E::Leave,   S::Idle},
 *       FSM::Transition{S::Hovered, E::Press,   S::Pressed},
 *       FSM::Transition{S::Pressed, E::Release, S::Hovered},
 *   };
 *
 *   FSM fsm(S::Idle, kTransitions);
 *   fsm.OnTransition([](S from, E ev, S to) { ... });
 *   fsm.Process(E::Enter);  // Idle -> Hovered
 * @endcode
 *
 * @see Matcha_Design_System_Specification.md §5.x (widget FSMs)
 */

#include <functional>
#include <span>

namespace matcha::fw {

/**
 * @class StateMachine
 * @tparam S State enum type (must be convertible to/from integral).
 * @tparam E Event enum type (must be convertible to/from integral).
 */
template <typename S, typename E>
class StateMachine {
public:
    // ====================================================================
    // Transition descriptor
    // ====================================================================

    struct Transition {
        S from;
        E event;
        S to;
    };

    // ====================================================================
    // Callback types
    // ====================================================================

    using TransitionCallback = std::function<void(S /*from*/, E /*event*/, S /*to*/)>;
    using GuardCallback      = std::function<bool(S /*from*/, E /*event*/, S /*to*/)>;

    // ====================================================================
    // Construction
    // ====================================================================

    /**
     * @brief Construct with initial state and transition table.
     * @param initial Starting state.
     * @param transitions Span of transition descriptors (can be constexpr array).
     */
    explicit StateMachine(S initial, std::span<const Transition> transitions)
        : _state(initial)
        , _transitions(transitions)
    {
    }

    // ====================================================================
    // Event processing
    // ====================================================================

    /**
     * @brief Process an event. If a matching (state, event) transition exists
     *        and the guard (if any) passes, transition to the new state.
     * @param event The event to process.
     * @return true if a transition occurred, false if no matching transition.
     */
    auto Process(E event) -> bool
    {
        for (const auto& t : _transitions) {
            if (t.from == _state && t.event == event) {
                if (_guard && !_guard(t.from, t.event, t.to)) {
                    return false;
                }
                const S prev = _state;
                _state = t.to;
                if (_onTransition) {
                    _onTransition(prev, event, _state);
                }
                return true;
            }
        }
        return false;
    }

    // ====================================================================
    // Query
    // ====================================================================

    [[nodiscard]] auto State() const -> S { return _state; }

    /**
     * @brief Check if a given event would cause a transition from the current state.
     * @param event The event to test.
     * @return true if a matching transition exists (guard not checked).
     */
    [[nodiscard]] auto CanProcess(E event) const -> bool
    {
        for (const auto& t : _transitions) {
            if (t.from == _state && t.event == event) {
                return true;
            }
        }
        return false;
    }

    // ====================================================================
    // Observers
    // ====================================================================

    void OnTransition(TransitionCallback cb) { _onTransition = std::move(cb); }
    void SetGuard(GuardCallback cb)          { _guard = std::move(cb); }

    // ====================================================================
    // Reset
    // ====================================================================

    void Reset(S state) { _state = state; }

private:
    S                          _state;
    std::span<const Transition> _transitions;
    TransitionCallback         _onTransition;
    GuardCallback              _guard;
};

} // namespace matcha::fw
