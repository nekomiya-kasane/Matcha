/**
 * @file SharedElementTransition.cpp
 * @brief Implementation of SharedElementTransition.
 */

#include <Matcha/Foundation/SharedElementTransition.h>

#include <algorithm>

namespace matcha::fw {

namespace {

auto Lerp(double a, double b, double t) -> double
{
    return a + ((b - a) * t);
}

auto LerpRect(const Rect2D& a, const Rect2D& b, double t) -> Rect2D
{
    return {
        .x      = Lerp(a.x, b.x, t),
        .y      = Lerp(a.y, b.y, t),
        .width  = Lerp(a.width, b.width, t),
        .height = Lerp(a.height, b.height, t),
    };
}

} // namespace

SharedElementTransition::SharedElementTransition(SharedElementSpec spec)
    : _spec(spec)
{
}

auto SharedElementTransition::Interpolate(double t) const -> State
{
    const double tc = std::clamp(t, 0.0, 1.0);

    State s;
    s.rect = LerpRect(_spec.source, _spec.target, tc);

    if (_spec.crossFade) {
        s.sourceOpacity = 1.0 - tc;
        s.targetOpacity = tc;
        s.proxyOpacity = _spec.opacity;
    } else {
        s.sourceOpacity = 0.0;
        s.targetOpacity = 0.0;
        s.proxyOpacity = _spec.opacity;
    }

    return s;
}

} // namespace matcha::fw
