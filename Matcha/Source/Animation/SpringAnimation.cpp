/**
 * @file SpringAnimation.cpp
 * @brief Damped harmonic oscillator spring animation implementation.
 *
 * Physics model:
 *   F_spring  = -stiffness * displacement
 *   F_damping = -damping * velocity
 *   a = (F_spring + F_damping) / mass
 *
 * Integration: semi-implicit Euler with fixed sub-steps for stability.
 * Convergence: |displacement| < threshold AND |velocity| < threshold.
 */

#include "SpringAnimation.h"

#include <cmath>

namespace matcha::gui {

namespace {
constexpr int    kMaxDurationMs = 5000;  // upper bound safety cap
constexpr qreal  kSubStepSec    = 0.001; // 1ms in seconds
} // namespace

SpringAnimation::SpringAnimation(QObject* parent)
    : QVariantAnimation(parent)
{
    setDuration(kMaxDurationMs);
}

void SpringAnimation::SetSpring(const fw::SpringSpec& spec)
{
    _spec = spec;
    ResetState();
}

void SpringAnimation::SetConvergenceThreshold(qreal threshold)
{
    _convergenceThreshold = threshold;
}

void SpringAnimation::ResetState()
{
    _position = 0.0;
    _velocity = 0.0;
    _lastTimeMs = 0;
    _converged = false;
    _initialized = false;
}

auto SpringAnimation::interpolated(const QVariant& from, const QVariant& to,
                                   qreal /*progress*/) const -> QVariant
{
    // Spring animation ignores the easing-curve-based progress.
    // Instead, _position holds the actual interpolated value in [0, 1] space.
    // We use it to lerp between from and to.

    if (_converged) {
        return to;
    }

    const qreal fromVal = from.toReal();
    const qreal toVal   = to.toReal();
    return fromVal + (_position * (toVal - fromVal));
}

void SpringAnimation::updateCurrentTime(int currentTime)
{
    if (!_initialized) {
        _position = 0.0; // start at 'from' value (normalized 0)
        _velocity = 0.0;
        _lastTimeMs = 0;
        _initialized = true;
    }

    if (_converged) {
        // Already done — snap to end
        QVariantAnimation::updateCurrentTime(duration());
        return;
    }

    const int deltaMs = currentTime - _lastTimeMs;
    _lastTimeMs = currentTime;

    if (deltaMs <= 0) {
        QVariantAnimation::updateCurrentTime(currentTime);
        return;
    }

    // Spring parameters
    const auto mass      = static_cast<qreal>(_spec.mass);
    const auto stiffness = static_cast<qreal>(_spec.stiffness);
    const auto damping   = static_cast<qreal>(_spec.damping);

    // Target is 1.0 in normalized space (the 'to' value)
    constexpr qreal target = 1.0;

    // Sub-step integration for stability
    const int subSteps = deltaMs; // 1 sub-step per ms
    for (int i = 0; i < subSteps && !_converged; ++i) {
        const qreal displacement = _position - target;
        const qreal springForce  = -stiffness * displacement;
        const qreal dampingForce = -damping * _velocity;
        const qreal acceleration = (springForce + dampingForce) / mass;

        // Semi-implicit Euler: update velocity first, then position
        _velocity += acceleration * kSubStepSec;
        _position += _velocity * kSubStepSec;

        // Convergence check
        if (std::abs(displacement) < _convergenceThreshold
            && std::abs(_velocity) < _convergenceThreshold) {
            _position = target;
            _velocity = 0.0;
            _converged = true;
        }
    }

    // Let QVariantAnimation call interpolated() with updated state
    QVariantAnimation::updateCurrentTime(currentTime);

    if (_converged) {
        stop(); // signal finished
    }
}

} // namespace matcha::gui
