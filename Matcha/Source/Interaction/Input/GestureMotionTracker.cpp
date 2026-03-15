/**
 * @file GestureMotionTracker.cpp
 * @brief Implementation of GestureMotionTracker.
 */

#include <Matcha/Interaction/Input/GestureMotionTracker.h>

#include <algorithm>
#include <cmath>

namespace matcha::fw {

// ============================================================================
// Configuration
// ============================================================================

void GestureMotionTracker::SetBounds(double minPos, double maxPos)
{
    _minBound = minPos;
    _maxBound = maxPos;
}

void GestureMotionTracker::SetRubberBand(RubberBandConfig config)
{
    _rubberBand = config;
}

void GestureMotionTracker::SetSnapTargets(std::vector<double> targets)
{
    _snapTargets = std::move(targets);
    std::ranges::sort(_snapTargets);
}

// ============================================================================
// Gesture lifecycle
// ============================================================================

void GestureMotionTracker::BeginTracking(double position, double timeMs)
{
    _phase = GesturePhase::Tracking;
    _position = position;
    _releaseVelocity = 0.0;
    _samples.clear();
    _samples.push_back({.position = position, .timeMs = timeMs});
}

void GestureMotionTracker::Track(double position, double timeMs)
{
    if (_phase != GesturePhase::Tracking) {
        return;
    }

    // Apply rubber-banding if beyond bounds
    _position = RubberBand(position);

    _samples.push_back({.position = position, .timeMs = timeMs});
    if (static_cast<int>(_samples.size()) > kMaxSamples) {
        _samples.erase(_samples.begin());
    }
}

void GestureMotionTracker::EndTracking(double position, double timeMs)
{
    if (_phase != GesturePhase::Tracking) {
        return;
    }

    _samples.push_back({.position = position, .timeMs = timeMs});
    _position = RubberBand(position);

    EstimateVelocity();

    // Determine settle target
    if (IsOverscrolled()) {
        // Snap back to nearest bound
        _settleTarget = (position < _minBound) ? _minBound : _maxBound;
        _settleDuration = _rubberBand.snapBackMs;
    } else if (!_snapTargets.empty()) {
        _settleTarget = FindSnapTarget();
        _settleDuration = kDefaultSettleMs;
    } else {
        // Decelerate based on velocity
        const double decelDist = _releaseVelocity * (kDefaultSettleMs * 0.5);
        _settleTarget = std::clamp(_position + decelDist, _minBound, _maxBound);
        _settleDuration = kDefaultSettleMs;
    }

    _settleStart = _position;
    _settleProgress = 0.0;
    _phase = GesturePhase::Settling;
}

void GestureMotionTracker::Cancel()
{
    _phase = GesturePhase::Idle;
    _releaseVelocity = 0.0;
    _samples.clear();
}

// ============================================================================
// Settling
// ============================================================================

auto GestureMotionTracker::TickSettling(double dtMs) -> bool
{
    if (_phase != GesturePhase::Settling) {
        return false;
    }

    if (_settleDuration <= 0.0) {
        _position = _settleTarget;
        _phase = GesturePhase::Idle;
        return false;
    }

    _settleProgress += dtMs / _settleDuration;
    if (_settleProgress >= 1.0) {
        _settleProgress = 1.0;
        _position = _settleTarget;
        _phase = GesturePhase::Idle;
        return false;
    }

    // OutCubic easing: t' = 1 - (1-t)^3
    const double t = _settleProgress;
    const double inv = 1.0 - t;
    const double eased = 1.0 - (inv * inv * inv);

    _position = _settleStart + ((_settleTarget - _settleStart) * eased);
    return true;
}

// ============================================================================
// Query
// ============================================================================

auto GestureMotionTracker::IsOverscrolled() const -> bool
{
    return _position < _minBound || _position > _maxBound;
}

auto GestureMotionTracker::RubberBand(double rawPosition) const -> double
{
    if (rawPosition >= _minBound && rawPosition <= _maxBound) {
        return rawPosition;
    }

    const double r = _rubberBand.resistance;
    const double maxS = _rubberBand.maxStretch;

    if (rawPosition < _minBound) {
        const double overscroll = _minBound - rawPosition;
        // Diminishing returns: offset = maxStretch * (1 - exp(-overscroll * r / maxStretch))
        const double dampened = maxS * (1.0 - std::exp((-overscroll * r) / maxS));
        return _minBound - dampened;
    }

    // rawPosition > _maxBound
    const double overscroll = rawPosition - _maxBound;
    const double dampened = maxS * (1.0 - std::exp((-overscroll * r) / maxS));
    return _maxBound + dampened;
}

// ============================================================================
// Private
// ============================================================================

void GestureMotionTracker::EstimateVelocity()
{
    if (_samples.size() < 2) {
        _releaseVelocity = 0.0;
        return;
    }

    // Use last two samples for simple velocity estimation
    const auto& s1 = _samples[_samples.size() - 2];
    const auto& s2 = _samples[_samples.size() - 1];
    const double dt = s2.timeMs - s1.timeMs;

    if (dt <= 0.0) {
        _releaseVelocity = 0.0;
        return;
    }

    _releaseVelocity = (s2.position - s1.position) / dt; // px/ms
}

auto GestureMotionTracker::FindSnapTarget() const -> double
{
    if (_snapTargets.empty()) {
        return _position;
    }

    // Project position forward by velocity
    const double projected = _position + (_releaseVelocity * 100.0); // ~100ms look-ahead

    // Find nearest snap target to projected position
    auto it = std::ranges::lower_bound(_snapTargets, projected);

    if (it == _snapTargets.end()) {
        return _snapTargets.back();
    }
    if (it == _snapTargets.begin()) {
        return *it;
    }

    const double after = *it;
    const double before = *std::prev(it);
    return (std::abs(projected - before) <= std::abs(projected - after)) ? before : after;
}

} // namespace matcha::fw
