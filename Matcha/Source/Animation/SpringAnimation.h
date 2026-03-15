#pragma once

/**
 * @file SpringAnimation.h
 * @brief QVariantAnimation subclass implementing a critically/under-damped
 *        harmonic oscillator for spring-based animations.
 *
 * Uses semi-implicit Euler integration of the equation:
 *   a = (-stiffness * (x - target) - damping * v) / mass
 *
 * The animation runs until the displacement and velocity are both
 * below a convergence threshold, then snaps to the target value.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/Token/TokenEnums.h>

#include <QVariantAnimation>

namespace matcha::gui {

/**
 * @brief Spring-physics QVariantAnimation for qreal properties.
 *
 * Unlike QEasingCurve-based animations which have a fixed duration,
 * SpringAnimation runs until convergence. The duration() returned
 * is an upper bound; the animation may finish earlier.
 */
class MATCHA_EXPORT SpringAnimation : public QVariantAnimation {
    Q_OBJECT

public:
    explicit SpringAnimation(QObject* parent = nullptr);

    void SetSpring(const fw::SpringSpec& spec);
    [[nodiscard]] auto Spring() const -> const fw::SpringSpec& { return _spec; }

    void SetConvergenceThreshold(qreal threshold);

protected:
    auto interpolated(const QVariant& from, const QVariant& to,
                      qreal progress) const -> QVariant override;

    void updateCurrentTime(int currentTime) override;

private:
    void ResetState();

    fw::SpringSpec _spec;
    qreal _convergenceThreshold = 0.001;

    // Integration state
    mutable qreal _position = 0.0;
    mutable qreal _velocity = 0.0;
    mutable int   _lastTimeMs = 0;
    mutable bool  _converged = false;
    mutable bool  _initialized = false;
};

} // namespace matcha::gui
