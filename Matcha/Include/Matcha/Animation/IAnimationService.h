#pragma once

/**
 * @file IAnimationService.h
 * @brief Animation Engine interface (RFC-08).
 *
 * Owned by Application. Provides token-driven animation for WidgetNode
 * properties. Internally wraps QPropertyAnimation / QParallelAnimationGroup.
 *
 * Key design decisions (RFC-08 redesign):
 * - Public API uses Qt-free types: AnimationPropertyId, AnimatableValue, TransitionHandle.
 * - Property-to-QByteArray and Value-to-QVariant mapping is internal.
 * - Duration/easing resolved from IThemeService tokens.
 * - Interruption: re-targeting mid-flight starts from current interpolated value.
 * - SetAnimationOverride(0) snaps all animations instantly (test mode).
 * - SetReducedMotion(true) snaps all animations (accessibility).
 * - Completion notifications dispatched via WidgetNode::SendNotification().
 */

#include "Matcha/Core/Macros.h"
#include "Matcha/Theming/Token/TokenEnums.h"

#include <cstdint>
#include <span>

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

/**
 * @brief Grouping mode for grouped animations.
 */
enum class GroupMode : uint8_t {
    Parallel,   ///< All children run simultaneously
    Sequential, ///< Children run one after another
};

/**
 * @brief Specification for a single transition within a group (Qt-free).
 */
struct GroupAnimationSpec {
    fw::WidgetNode*        target   = nullptr;
    fw::AnimationPropertyId property = fw::AnimationPropertyId::Opacity;
    fw::AnimatableValue    from;
    fw::AnimatableValue    to;
    fw::AnimationsToken     duration = fw::AnimationsToken::motionBase;
    fw::EasingToken        easing   = fw::EasingToken::OutCubic;
};

/**
 * @brief Opaque group handle returned by AnimateGroup().
 */
enum class GroupId : uint64_t { Invalid = 0 };

/**
 * @brief Abstract animation engine interface (RFC-08).
 *
 * Concrete implementation: AnimationService (internal to Matcha library).
 *
 * All public methods use Qt-free types. Internal implementation maps
 * AnimationPropertyId -> QByteArray and AnimatableValue -> QVariant.
 */
class MATCHA_EXPORT IAnimationService {
public:
    virtual ~IAnimationService() = default;

    IAnimationService(const IAnimationService&)             = delete;
    IAnimationService& operator=(const IAnimationService&)  = delete;
    IAnimationService(IAnimationService&&)                  = delete;
    IAnimationService& operator=(IAnimationService&&)       = delete;

    /**
     * @brief Animate a property on a WidgetNode using Qt-free types.
     *
     * If the widget already has an active animation on the same property,
     * the existing animation is interrupted. Dispatches AnimationStarted
     * notification on start, AnimationCompleted on finish.
     *
     * @param target   Widget to animate (resolved to QWidget* internally).
     * @param property Identifies which property to animate.
     * @param from     Start value (Qt-free).
     * @param to       End value (Qt-free).
     * @param duration Animation speed token (resolved via IThemeService).
     * @param easing   Easing curve token.
     * @return Handle for the running animation.
     */
    virtual auto Animate(fw::WidgetNode* target,
                         fw::AnimationPropertyId property,
                         fw::AnimatableValue from,
                         fw::AnimatableValue to,
                         fw::AnimationsToken duration,
                         fw::EasingToken easing) -> fw::TransitionHandle = 0;

    /**
     * @brief Animate a property with spring dynamics and custom spring parameters.
     *
     * Forces easing to EasingToken::Spring internally. The SpringSpec
     * controls mass, stiffness, and damping of the harmonic oscillator.
     *
     * @param target   Widget to animate.
     * @param property Property to animate.
     * @param from     Start value.
     * @param to       End value.
     * @param spring   Spring dynamics parameters.
     * @return Handle for the running animation.
     */
    virtual auto AnimateSpring(fw::WidgetNode* target,
                               fw::AnimationPropertyId property,
                               fw::AnimatableValue from,
                               fw::AnimatableValue to,
                               fw::SpringSpec spring) -> fw::TransitionHandle = 0;

    /**
     * @brief Run a group of transitions together.
     *
     * @param specs Array of transition specifications (Qt-free).
     * @param mode  Parallel or Sequential execution.
     * @return Handle for the group.
     */
    virtual auto AnimateGroup(std::span<const GroupAnimationSpec> specs,
                              GroupMode mode) -> GroupId = 0;

    /**
     * @brief Cancel a running animation.
     * Dispatches AnimationCancelled notification.
     * @param handle Animation handle. No-op if already finished or invalid.
     */
    virtual void Cancel(fw::TransitionHandle handle) = 0;

    /**
     * @brief Cancel all running animations on a widget.
     * @param target Widget whose animations should be stopped.
     */
    virtual void CancelAll(fw::WidgetNode* target) = 0;

    /**
     * @brief Cancel an entire animation group and all its member transitions.
     * Dispatches AnimationCancelled for each member. No-op if invalid or finished.
     * @param gid Group handle returned by AnimateGroup().
     */
    virtual void CancelGroup(GroupId gid) = 0;

    /**
     * @brief Check if an animation is still running.
     */
    [[nodiscard]] virtual auto IsRunning(fw::TransitionHandle handle) const -> bool = 0;

    /**
     * @brief Check if a specific property on a target is currently animating.
     */
    [[nodiscard]] virtual auto IsAnimatingProperty(fw::WidgetNode* target,
                                                    fw::AnimationPropertyId property) const -> bool = 0;

    /**
     * @brief Enable reduced-motion mode (WCAG 2.1 SC 2.3.3).
     * When enabled, all Animate() calls snap to target value immediately.
     * AnimationStarted and AnimationCompleted are still dispatched.
     */
    virtual void SetReducedMotion(bool enabled) = 0;

    /**
     * @brief Query reduced-motion state.
     */
    [[nodiscard]] virtual auto IsReducedMotion() const -> bool = 0;

    /**
     * @brief Set global animation speed multiplier.
     * @param factor 1.0 = normal, 0.5 = half speed, 2.0 = double speed.
     */
    virtual void SetSpeedMultiplier(float factor) = 0;

    /**
     * @brief Get current speed multiplier.
     */
    [[nodiscard]] virtual auto SpeedMultiplier() const -> float = 0;

protected:
    IAnimationService() = default;
};

// ============================================================================
// Global accessor (same pattern as GetThemeService)
// ============================================================================

MATCHA_EXPORT void SetAnimationService(IAnimationService* svc);

[[nodiscard]] MATCHA_EXPORT auto GetAnimationService() -> IAnimationService*;

[[nodiscard]] MATCHA_EXPORT auto HasAnimationService() -> bool;

} // namespace matcha::gui
