/**
 * @file ContextualHelpService.cpp
 * @brief Implementation of ContextualHelpService.
 */

#include <Matcha/Interaction/ContextualHelpService.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Registration
// ============================================================================

auto ContextualHelpService::RegisterWalkthrough(Walkthrough wt) -> bool
{
    if (FindWalkthrough(wt.id) != nullptr) {
        return false;
    }
    _walkthroughs.push_back(std::move(wt));
    return true;
}

auto ContextualHelpService::UnregisterWalkthrough(std::string_view id) -> bool
{
    const auto it = std::ranges::find_if(_walkthroughs, [id](const Walkthrough& w) {
        return w.id == id;
    });
    if (it == _walkthroughs.end()) {
        return false;
    }
    // If this is the active walkthrough, dismiss it first
    if (_activeId == id && _state == WalkthroughState::Active) {
        Dismiss();
    }
    _walkthroughs.erase(it);
    return true;
}

auto ContextualHelpService::FindWalkthrough(std::string_view id) const -> const Walkthrough*
{
    for (const auto& w : _walkthroughs) {
        if (w.id == id) {
            return &w;
        }
    }
    return nullptr;
}

// ============================================================================
// Tour execution
// ============================================================================

auto ContextualHelpService::Start(std::string_view walkthroughId) -> bool
{
    if (_state == WalkthroughState::Active || _state == WalkthroughState::Paused) {
        return false; // already running
    }
    const auto* wt = FindWalkthrough(walkthroughId);
    if (wt == nullptr || wt->steps.empty()) {
        return false;
    }
    _activeId = std::string(walkthroughId);
    _currentStep = 0;
    SetState(WalkthroughState::Active);
    if (_stepCallback) {
        _stepCallback(_currentStep, wt->steps[0]);
    }
    return true;
}

void ContextualHelpService::Next()
{
    if (_state != WalkthroughState::Active) {
        return;
    }
    const auto* wt = ActiveWalkthrough();
    if (wt == nullptr) {
        return;
    }
    const int total = static_cast<int>(wt->steps.size());
    if (_currentStep + 1 >= total) {
        // Completed
        _completedIds.push_back(_activeId);
        _currentStep = total - 1;
        SetState(WalkthroughState::Completed);
        _activeId.clear();
        _currentStep = 0;
    } else {
        ++_currentStep;
        if (_stepCallback) {
            _stepCallback(_currentStep, wt->steps[static_cast<std::size_t>(_currentStep)]);
        }
    }
}

void ContextualHelpService::Previous()
{
    if (_state != WalkthroughState::Active) {
        return;
    }
    if (_currentStep <= 0) {
        return;
    }
    --_currentStep;
    const auto* wt = ActiveWalkthrough();
    if (wt != nullptr && _stepCallback) {
        _stepCallback(_currentStep, wt->steps[static_cast<std::size_t>(_currentStep)]);
    }
}

auto ContextualHelpService::GoToStep(int index) -> bool
{
    if (_state != WalkthroughState::Active) {
        return false;
    }
    const auto* wt = ActiveWalkthrough();
    if (wt == nullptr) {
        return false;
    }
    if (index < 0 || index >= static_cast<int>(wt->steps.size())) {
        return false;
    }
    _currentStep = index;
    if (_stepCallback) {
        _stepCallback(_currentStep, wt->steps[static_cast<std::size_t>(_currentStep)]);
    }
    return true;
}

void ContextualHelpService::Pause()
{
    if (_state == WalkthroughState::Active) {
        SetState(WalkthroughState::Paused);
    }
}

void ContextualHelpService::Resume()
{
    if (_state == WalkthroughState::Paused) {
        SetState(WalkthroughState::Active);
        const auto* wt = ActiveWalkthrough();
        if (wt != nullptr && _stepCallback) {
            _stepCallback(_currentStep, wt->steps[static_cast<std::size_t>(_currentStep)]);
        }
    }
}

void ContextualHelpService::Dismiss()
{
    if (_state == WalkthroughState::Active || _state == WalkthroughState::Paused) {
        SetState(WalkthroughState::Dismissed);
        _activeId.clear();
        _currentStep = 0;
    }
}

// ============================================================================
// Query
// ============================================================================

auto ContextualHelpService::TotalSteps() const -> int
{
    const auto* wt = ActiveWalkthrough();
    if (wt == nullptr) {
        return 0;
    }
    return static_cast<int>(wt->steps.size());
}

auto ContextualHelpService::CurrentStep() const -> const WalkthroughStep*
{
    if (_state != WalkthroughState::Active && _state != WalkthroughState::Paused) {
        return nullptr;
    }
    const auto* wt = ActiveWalkthrough();
    if (wt == nullptr || _currentStep < 0 ||
        _currentStep >= static_cast<int>(wt->steps.size())) {
        return nullptr;
    }
    return &wt->steps[static_cast<std::size_t>(_currentStep)];
}

auto ContextualHelpService::Progress() const -> double
{
    const auto* wt = ActiveWalkthrough();
    if (wt == nullptr || wt->steps.empty()) {
        return 0.0;
    }
    return static_cast<double>(_currentStep + 1) / static_cast<double>(wt->steps.size());
}

auto ContextualHelpService::IsCompleted(std::string_view walkthroughId) const -> bool
{
    return std::ranges::any_of(_completedIds, [walkthroughId](const std::string& id) {
        return id == walkthroughId;
    });
}

// ============================================================================
// Observers
// ============================================================================

void ContextualHelpService::OnStepChanged(StepChangedCallback cb)
{
    _stepCallback = std::move(cb);
}

void ContextualHelpService::OnStateChanged(StateChangedCallback cb)
{
    _stateCallback = std::move(cb);
}

// ============================================================================
// Private
// ============================================================================

void ContextualHelpService::SetState(WalkthroughState s)
{
    if (_state == s) {
        return;
    }
    _state = s;
    if (_stateCallback) {
        _stateCallback(_state);
    }
}

auto ContextualHelpService::ActiveWalkthrough() const -> const Walkthrough*
{
    return FindWalkthrough(_activeId);
}

} // namespace matcha::fw
