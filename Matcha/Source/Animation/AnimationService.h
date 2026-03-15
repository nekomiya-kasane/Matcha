#pragma once

/**
 * @file AnimationService.h
 * @brief Concrete IAnimationService implementation using QPropertyAnimation (RFC-08).
 *
 * Internal to Matcha library. Not part of the public API.
 * Owned by Application::Impl.
 */

#include "Matcha/Animation/IAnimationService.h"

#include <QByteArray>
#include <QObject>
#include <QVariant>

#include <unordered_map>

class QAbstractAnimation;

namespace matcha::gui {

class IThemeService;

/**
 * @brief Concrete animation engine backed by QPropertyAnimation (RFC-08).
 *
 * Maps AnimationPropertyId -> QByteArray and AnimatableValue -> QVariant
 * internally. Dispatches AnimationStarted/Completed/Cancelled notifications
 * via WidgetNode::SendNotification().
 *
 * Thread safety: GUI thread only (same as QPropertyAnimation).
 */
class AnimationService final : public QObject, public IAnimationService {
    Q_OBJECT

public:
    explicit AnimationService(IThemeService& theme, QObject* parent = nullptr);
    ~AnimationService() override;

    // -- IAnimationService (RFC-08) --
    auto Animate(fw::WidgetNode* target,
                 fw::AnimationPropertyId property,
                 fw::AnimatableValue from,
                 fw::AnimatableValue to,
                 fw::AnimationToken duration,
                 fw::EasingToken easing) -> fw::TransitionHandle override;

    auto AnimateSpring(fw::WidgetNode* target,
                       fw::AnimationPropertyId property,
                       fw::AnimatableValue from,
                       fw::AnimatableValue to,
                       fw::SpringSpec spring) -> fw::TransitionHandle override;

    auto AnimateGroup(std::span<const GroupAnimationSpec> specs,
                      GroupMode mode) -> GroupId override;

    void Cancel(fw::TransitionHandle handle) override;
    void CancelAll(fw::WidgetNode* target) override;
    void CancelGroup(GroupId gid) override;
    [[nodiscard]] auto IsRunning(fw::TransitionHandle handle) const -> bool override;
    [[nodiscard]] auto IsAnimatingProperty(fw::WidgetNode* target,
                                            fw::AnimationPropertyId property) const -> bool override;

    void SetReducedMotion(bool enabled) override;
    [[nodiscard]] auto IsReducedMotion() const -> bool override;

    void SetSpeedMultiplier(float factor) override;
    [[nodiscard]] auto SpeedMultiplier() const -> float override;

private:
    struct TransitionEntry {
        fw::TransitionHandle handle;
        fw::WidgetNode* target = nullptr;
        fw::AnimationPropertyId propertyId = fw::AnimationPropertyId::Opacity;
        QByteArray qtProperty;
        QAbstractAnimation* animation = nullptr;
        GroupId owningGroup = GroupId::Invalid;
    };

    struct GroupEntry {
        GroupId id = GroupId::Invalid;
        QAbstractAnimation* group = nullptr;
        std::vector<fw::TransitionHandle> members;
    };

    auto NextHandle() -> fw::TransitionHandle;
    auto ResolveEasing(fw::EasingToken token) const -> int;
    auto ResolveDurationMs(fw::AnimationToken token) const -> int;
    static auto MapPropertyId(fw::AnimationPropertyId id) -> QByteArray;
    static auto MapValue(fw::AnimatableValue val) -> QVariant;

    auto InterruptExisting(fw::WidgetNode* target,
                           fw::AnimationPropertyId property) -> QVariant;
    void OnFinished(fw::TransitionHandle handle);
    void OnGroupFinished(GroupId gid);

    IThemeService& _theme;
    uint64_t _nextId = 1;
    std::unordered_map<uint64_t, TransitionEntry> _active;
    std::unordered_map<uint64_t, GroupEntry> _activeGroups;
    bool  _reducedMotion   = false;
    float _speedMultiplier = 1.0F;
};

} // namespace matcha::gui
