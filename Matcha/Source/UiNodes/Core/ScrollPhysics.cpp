/**
 * @file ScrollPhysics.cpp
 * @brief Implementation of ScrollPhysics (§7.3).
 */

#include <Matcha/Foundation/ScrollPhysics.h>

#include <algorithm>
#include <cmath>

namespace matcha::fw {

// ============================================================================
// Construction / Configuration
// ============================================================================

ScrollPhysics::ScrollPhysics(ScrollConfig config)
    : _config(config)
{
}

void ScrollPhysics::SetConfig(ScrollConfig config)
{
    _config = config;
}

void ScrollPhysics::SetContentSize(double size)
{
    _contentSize = std::max(0.0, size);
}

void ScrollPhysics::SetViewportSize(double size)
{
    _viewportSize = std::max(0.0, size);
}

void ScrollPhysics::SetSnapPoints(std::vector<double> points)
{
    _snapPoints = std::move(points);
    std::ranges::sort(_snapPoints);
}

// ============================================================================
// Interaction API
// ============================================================================

void ScrollPhysics::Fling(double velocityPxPerSec)
{
    // Clamp initial velocity
    const double clamped = std::clamp(velocityPxPerSec,
                                       -_config.maxInitialVelocity,
                                       _config.maxInitialVelocity);
    // Convert px/s to px/frame at 60fps
    _state.velocity = clamped / 60.0;
    _state.isAnimating = true;
    _snapping = false;
}

void ScrollPhysics::ScrollBy(double deltaPx)
{
    _state.position += deltaPx;
    ClampPosition();
}

void ScrollPhysics::ScrollToVisible(double targetTop, double targetHeight)
{
    const double viewTop = _state.position;
    const double viewBottom = viewTop + _viewportSize;
    const double targetBottom = targetTop + targetHeight;

    if (targetTop < viewTop) {
        // Target is above viewport — scroll up
        ScrollTo(targetTop);
    } else if (targetBottom > viewBottom) {
        // Target is below viewport — scroll down
        ScrollTo(targetBottom - _viewportSize);
    }
    // Otherwise already visible — no-op
}

void ScrollPhysics::ScrollTo(double position)
{
    _state.position = std::clamp(position, 0.0, MaxPosition());
    _state.velocity = 0.0;
    _state.overscroll = 0.0;
    _state.isAnimating = false;
    _snapping = false;
}

void ScrollPhysics::Stop()
{
    _state.velocity = 0.0;
    _state.overscroll = 0.0;
    _state.isAnimating = false;
    _snapping = false;
}

// ============================================================================
// Simulation
// ============================================================================

void ScrollPhysics::Step(double dt)
{
    if (!_state.isAnimating) {
        return;
    }

    // Number of sub-steps at 60fps equivalent
    const double frames = dt * 60.0;

    if (_snapping) {
        // Simple linear interpolation toward snap target
        const double diff = _snapTarget - _state.position;
        const double step = diff * std::min(1.0, frames * 0.15); // ease factor
        _state.position += step;
        if (std::abs(_snapTarget - _state.position) < 0.5) {
            _state.position = _snapTarget;
            _state.isAnimating = false;
            _snapping = false;
        }
        return;
    }

    const double maxPos = MaxPosition();

    // Apply friction: v *= friction^frames
    _state.velocity *= std::pow(_config.friction, frames);

    // Update position
    _state.position += _state.velocity * frames;

    // Handle overscroll
    if (_state.position < 0.0) {
        switch (_config.overscrollMode) {
        case OverscrollMode::Clamp:
            _state.position = 0.0;
            _state.velocity = 0.0;
            _state.overscroll = 0.0;
            break;
        case OverscrollMode::Bounce:
        case OverscrollMode::Glow:
            _state.overscroll = _state.position; // negative
            _state.overscroll = std::max(_state.overscroll, -_config.maxOverscroll);
            ApplyBounce(dt);
            break;
        }
    } else if (_state.position > maxPos) {
        switch (_config.overscrollMode) {
        case OverscrollMode::Clamp:
            _state.position = maxPos;
            _state.velocity = 0.0;
            _state.overscroll = 0.0;
            break;
        case OverscrollMode::Bounce:
        case OverscrollMode::Glow:
            _state.overscroll = _state.position - maxPos; // positive
            _state.overscroll = std::min(_state.overscroll, _config.maxOverscroll);
            ApplyBounce(dt);
            break;
        }
    } else {
        _state.overscroll = 0.0;
    }

    // Check stop condition
    if (std::abs(_state.velocity) < _config.stopThreshold / 60.0 &&
        std::abs(_state.overscroll) < 0.5) {
        _state.velocity = 0.0;
        _state.overscroll = 0.0;
        ClampPosition();
        ApplySnap();
        if (!_snapping) {
            _state.isAnimating = false;
        }
    }
}

// ============================================================================
// Query
// ============================================================================

auto ScrollPhysics::MaxPosition() const -> double
{
    return std::max(0.0, _contentSize - _viewportSize);
}

auto ScrollPhysics::NearestSnapPoint(double position) const -> double
{
    if (_snapPoints.empty()) {
        return position;
    }

    auto it = std::ranges::lower_bound(_snapPoints, position);

    if (it == _snapPoints.end()) {
        return _snapPoints.back();
    }
    if (it == _snapPoints.begin()) {
        return *it;
    }

    const double after = *it;
    const double before = *std::prev(it);

    // Adjust for snap alignment
    double adjustedPos = position;
    if (_config.snapAlignment == SnapAlignment::Center) {
        adjustedPos = position + (_viewportSize / 2.0);
    } else if (_config.snapAlignment == SnapAlignment::End) {
        adjustedPos = position + _viewportSize;
    }
    (void)adjustedPos; // alignment adjustment is for future use

    return (position - before <= after - position) ? before : after;
}

// ============================================================================
// Private
// ============================================================================

void ScrollPhysics::ClampPosition()
{
    _state.position = std::clamp(_state.position, 0.0, MaxPosition());
}

void ScrollPhysics::ApplyBounce(double dt)
{
    // Spring force: F = -stiffness * displacement - damping * velocity
    const double displacement = _state.overscroll;
    const double springForce = (-_config.bounceStiffness * displacement)
                               - (_config.bounceDamping * (_state.velocity * 60.0));

    // Apply acceleration (F = ma, assume m=1)
    _state.velocity += (springForce * dt) / 60.0;

    // If overscroll is small and velocity is near zero, snap back
    if (std::abs(displacement) < 0.5 && std::abs(_state.velocity) < _config.stopThreshold / 60.0) {
        _state.overscroll = 0.0;
        _state.velocity = 0.0;
        ClampPosition();
    }
}

void ScrollPhysics::ApplySnap()
{
    if (_snapPoints.empty()) {
        return;
    }
    const double target = NearestSnapPoint(_state.position);
    if (std::abs(target - _state.position) > 0.5) {
        _snapTarget = target;
        _snapping = true;
        _state.isAnimating = true;
    }
}

} // namespace matcha::fw
