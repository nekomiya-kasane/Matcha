/**
 * @file EdgeCaseGuard.cpp
 * @brief Implementation of EdgeCaseGuard components.
 */

#include <Matcha/Foundation/EdgeCaseGuard.h>

namespace matcha::fw {

// ============================================================================
// ActionGuard
// ============================================================================

auto ActionGuard::TryAcquire() -> bool
{
    if (_active) {
        return false;
    }
    _active = true;
    _elapsedMs = 0.0;
    return true;
}

void ActionGuard::Release()
{
    _active = false;
    _elapsedMs = 0.0;
}

auto ActionGuard::Tick(double dtMs) -> bool
{
    if (!_active) {
        return false;
    }
    _elapsedMs += dtMs;
    if (_elapsedMs >= _timeoutMs) {
        Release();
        return true; // auto-released
    }
    return false;
}

// ============================================================================
// ReentrancyGuard
// ============================================================================

auto ReentrancyGuard::TryEnter() -> bool
{
    if (_depth >= kMaxDepth) {
        return false;
    }
    ++_depth;
    return true;
}

void ReentrancyGuard::Leave()
{
    if (_depth > 0) {
        --_depth;
    }
}

// ============================================================================
// InputModeTracker
// ============================================================================

void InputModeTracker::OnKeyboardEvent()
{
    _mode = InputMode::Keyboard;
}

void InputModeTracker::OnMouseEvent()
{
    _mode = InputMode::Mouse;
}

void InputModeTracker::OnTouchEvent()
{
    _mode = InputMode::Touch;
}

} // namespace matcha::fw
