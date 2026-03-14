#pragma once

/**
 * @file EdgeCaseGuard.h
 * @brief Edge case robustness patterns (Spec §7.18).
 *
 * Foundation-layer descriptors for:
 * - ActionGuard: double-submit / rapid-click prevention (§7.18.1)
 * - ReentrancyGuard: notification handler reentrancy prevention (§7.18.4)
 * - InputModeTracker: keyboard/mouse/touch mode switching (§7.18.3)
 *
 * Qt-free Foundation layer.
 *
 * @see Matcha_Design_System_Specification.md §7.18
 */

#include <Matcha/Foundation/Macros.h>

#include <cstdint>

namespace matcha::fw {

// ============================================================================
// ActionGuard (§7.18.1)
// ============================================================================

/**
 * @class ActionGuard
 * @brief Prevents double-submit / rapid-click on async action buttons.
 *
 * Usage:
 * @code
 *   ActionGuard guard;
 *   if (!guard.TryAcquire()) return; // already executing
 *   // ... perform async action ...
 *   guard.Release(); // or auto-release on timeout
 * @endcode
 */
class MATCHA_EXPORT ActionGuard {
public:
    ActionGuard() = default;

    /**
     * @brief Try to acquire the guard.
     * @return true if acquired (action can proceed), false if already active.
     */
    auto TryAcquire() -> bool;

    /**
     * @brief Release the guard (action completed).
     */
    void Release();

    /**
     * @brief Check if guard is currently active.
     */
    [[nodiscard]] auto IsActive() const -> bool { return _active; }

    /**
     * @brief Advance elapsed time. If timeout exceeded, auto-release.
     * @param dtMs Milliseconds elapsed since last tick.
     * @return true if auto-released due to timeout.
     */
    auto Tick(double dtMs) -> bool;

    /**
     * @brief Set the timeout duration (default 10000ms = 10s).
     */
    void SetTimeoutMs(double timeoutMs) { _timeoutMs = timeoutMs; }
    [[nodiscard]] auto TimeoutMs() const -> double { return _timeoutMs; }

private:
    bool   _active    = false;
    double _elapsedMs = 0.0;
    double _timeoutMs = 10000.0;
};

// ============================================================================
// ReentrancyGuard (§7.18.4)
// ============================================================================

/**
 * @class ReentrancyGuard
 * @brief Prevents reentrant notification handler execution.
 *
 * Usage:
 * @code
 *   ReentrancyGuard guard;
 *   if (!guard.TryEnter()) return; // already handling
 *   // ... handle notification ...
 *   guard.Leave();
 * @endcode
 */
class MATCHA_EXPORT ReentrancyGuard {
public:
    ReentrancyGuard() = default;

    auto TryEnter() -> bool;
    void Leave();
    [[nodiscard]] auto IsInside() const -> bool { return _depth > 0; }
    [[nodiscard]] auto Depth() const -> int { return _depth; }

    static constexpr int kMaxDepth = 8;

private:
    int _depth = 0;
};

// ============================================================================
// InputMode (§7.18.3)
// ============================================================================

/**
 * @enum InputMode
 * @brief Current input mode for focus ring visibility.
 */
enum class InputMode : uint8_t {
    Keyboard = 0,   ///< Show focus ring
    Mouse    = 1,   ///< Hide focus ring, show hover
    Touch    = 2,   ///< Hide focus ring, enlarge hit targets
};

/**
 * @class InputModeTracker
 * @brief Tracks current input mode per-window for focus ring management.
 */
class MATCHA_EXPORT InputModeTracker {
public:
    InputModeTracker() = default;

    void OnKeyboardEvent();
    void OnMouseEvent();
    void OnTouchEvent();

    [[nodiscard]] auto Mode() const -> InputMode { return _mode; }
    [[nodiscard]] auto ShouldShowFocusRing() const -> bool {
        return _mode == InputMode::Keyboard;
    }

private:
    InputMode _mode = InputMode::Mouse;
};

} // namespace matcha::fw
