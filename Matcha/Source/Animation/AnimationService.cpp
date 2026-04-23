#include "AnimationService.h"
#include "SpringAnimation.h"

#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Tree/WidgetNode.h"
#include "Matcha/Theming/IThemeService.h"

#include <QColor>
#include <QEasingCurve>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPoint>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QWidget>

#include <algorithm>
#include <cmath>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace matcha::gui {

// ============================================================================
// OS reduced-motion detection
// ============================================================================

namespace {

auto DetectOsReducedMotion() -> bool
{
#ifdef _WIN32
    BOOL animationsEnabled = TRUE;
    if (SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0,
                               &animationsEnabled, 0)) {
        return animationsEnabled == FALSE;
    }
#endif
    // Linux/macOS: no standard API; default to false.
    // GTK: org.gnome.desktop.interface.enable-animations
    // macOS: NSWorkspace.accessibilityDisplayShouldReduceMotion
    return false;
}

} // anonymous namespace

// ============================================================================
// Construction
// ============================================================================

AnimationService::AnimationService(IThemeService& theme, QObject* parent)
    : QObject(parent)
    , _theme(theme)
    , _reducedMotion(DetectOsReducedMotion())
{
}

AnimationService::~AnimationService() = default;

// ============================================================================
// Property / Value mapping (internal)
// ============================================================================

auto AnimationService::MapPropertyId(fw::AnimationPropertyId id) -> QByteArray
{
    switch (id) {
    case fw::AnimationPropertyId::Opacity:         return "opacity";
    case fw::AnimationPropertyId::Position:         return "pos";
    case fw::AnimationPropertyId::Size:             return "size";
    case fw::AnimationPropertyId::MinimumHeight:    return "minimumHeight";
    case fw::AnimationPropertyId::MaximumHeight:    return "maximumHeight";
    case fw::AnimationPropertyId::Geometry:         return "geometry";
    case fw::AnimationPropertyId::BackgroundColor:  return "_matcha_bg";
    case fw::AnimationPropertyId::ForegroundColor:  return "_matcha_fg";
    case fw::AnimationPropertyId::BorderColor:      return "_matcha_border";
    case fw::AnimationPropertyId::ArrowRotation:    return "arrowRotation";
    case fw::AnimationPropertyId::SlideOffset:      return "animationOffset";
    case fw::AnimationPropertyId::ContentHeight:    return "maximumHeight";
    default:                                        return "windowOpacity";
    }
}

auto AnimationService::MapValue(fw::AnimatableValue val) -> QVariant
{
    switch (val.type) {
    case fw::AnimatableValue::Type::Double:
        return QVariant(val.asDouble);
    case fw::AnimatableValue::Type::Int:
        return QVariant(val.asInt);
    case fw::AnimatableValue::Type::Rgba: {
        const auto a = static_cast<int>((val.asRgba >> 24U) & 0xFFU);
        const auto r = static_cast<int>((val.asRgba >> 16U) & 0xFFU);
        const auto g = static_cast<int>((val.asRgba >> 8U) & 0xFFU);
        const auto b = static_cast<int>(val.asRgba & 0xFFU);
        return QVariant::fromValue(QColor(r, g, b, a));
    }
    case fw::AnimatableValue::Type::Point2D:
        return QVariant(QPoint(val.asPoint.x, val.asPoint.y));
    }
    return QVariant(val.asDouble);
}

// ============================================================================
// IAnimationService (RFC-08)
// ============================================================================

auto AnimationService::Animate(fw::WidgetNode* target,
                               fw::AnimationPropertyId property,
                               fw::AnimatableValue from,
                               fw::AnimatableValue to,
                               fw::AnimationsToken duration,
                               fw::EasingToken easing) -> fw::TransitionHandle
{
    if (target == nullptr || target->Widget() == nullptr) {
        return fw::TransitionHandle::Invalid;
    }

    const auto handle = NextHandle();
    const auto qtProp = MapPropertyId(property);
    const auto qTo    = MapValue(to);

    // Interrupt existing animation on same target+property.
    // If one was running, use its current interpolated value as the new 'from';
    // otherwise use the caller-supplied 'from'.
    const QVariant captured = InterruptExisting(target, property);
    const QVariant qFrom = captured.isValid() ? captured : MapValue(from);

    // Dispatch AnimationStarted
    fw::AnimationStarted startNotif(property, handle);
    target->SendNotification(target, startNotif);

    // Calculate effective duration
    int durationMs = ResolveDurationMs(duration);
    if (_speedMultiplier > 0.0F && _speedMultiplier != 1.0F) {
        durationMs = static_cast<int>(
            std::lroundf(static_cast<float>(durationMs) / _speedMultiplier));
    }

    // Determine animation target object and property.
    // For Opacity on child widgets, use QGraphicsOpacityEffect instead of
    // the top-level-only "windowOpacity" property.
    QObject* animTarget = target->Widget();
    QByteArray animProp = qtProp;
    if (property == fw::AnimationPropertyId::Opacity) {
        auto* w = target->Widget();
        auto* effect = qobject_cast<QGraphicsOpacityEffect*>(w->graphicsEffect());
        if (effect == nullptr) {
            effect = new QGraphicsOpacityEffect(w);
            effect->setOpacity(qFrom.toDouble());
            w->setGraphicsEffect(effect);
        }
        animTarget = effect;
        animProp = "opacity";
    }

    // Reduced motion or instant: snap to target, dispatch completed immediately
    if (_reducedMotion || durationMs <= 0) {
        animTarget->setProperty(animProp.constData(), qTo);
        fw::AnimationCompleted completeNotif(property, handle);
        target->SendNotification(target, completeNotif);
        return handle;
    }

    // Create Qt animation
    QAbstractAnimation* anim = nullptr;

    if (easing == fw::EasingToken::Spring) {
        auto* spring = new SpringAnimation(this);
        spring->SetSpring(_theme.Spring());
        spring->setStartValue(qFrom);
        spring->setEndValue(qTo);
        // SpringAnimation is QVariantAnimation, not QPropertyAnimation,
        // so we must manually forward interpolated values to the target.
        connect(spring, &QVariantAnimation::valueChanged,
                animTarget, [animTarget, animProp](const QVariant& value) {
            animTarget->setProperty(animProp.constData(), value);
        });
        anim = spring;
    } else {
        auto* propAnim = new QPropertyAnimation(animTarget, animProp, this);
        propAnim->setDuration(durationMs);
        propAnim->setStartValue(qFrom);
        propAnim->setEndValue(qTo);
        propAnim->setEasingCurve(
            static_cast<QEasingCurve::Type>(ResolveEasing(easing)));
        anim = propAnim;
    }

    TransitionEntry entry;
    entry.handle     = handle;
    entry.target     = target;
    entry.propertyId = property;
    entry.qtProperty = qtProp;
    entry.animation  = anim;
    _active[static_cast<uint64_t>(handle)] = entry;

    connect(anim, &QAbstractAnimation::finished, this, [this, handle]() {
        OnFinished(handle);
    });

    anim->start(QAbstractAnimation::KeepWhenStopped);

    return handle;
}

auto AnimationService::AnimateSpring(fw::WidgetNode* target,
                                     fw::AnimationPropertyId property,
                                     fw::AnimatableValue from,
                                     fw::AnimatableValue to,
                                     fw::SpringSpec spring) -> fw::TransitionHandle
{
    if (target == nullptr || target->Widget() == nullptr) {
        return fw::TransitionHandle::Invalid;
    }

    const auto handle = NextHandle();
    const auto qtProp = MapPropertyId(property);
    const auto qTo    = MapValue(to);

    // Interrupt existing — re-target from current interpolated value.
    const QVariant captured = InterruptExisting(target, property);
    const QVariant qFrom = captured.isValid() ? captured : MapValue(from);

    // Dispatch AnimationStarted
    fw::AnimationStarted startNotif(property, handle);
    target->SendNotification(target, startNotif);

    // Determine animation target object and property (Opacity special case).
    QObject* animTarget = target->Widget();
    QByteArray animProp = qtProp;
    if (property == fw::AnimationPropertyId::Opacity) {
        auto* w = target->Widget();
        auto* effect = qobject_cast<QGraphicsOpacityEffect*>(w->graphicsEffect());
        if (effect == nullptr) {
            effect = new QGraphicsOpacityEffect(w);
            effect->setOpacity(qFrom.toDouble());
            w->setGraphicsEffect(effect);
        }
        animTarget = effect;
        animProp = "opacity";
    }

    // Reduced motion: snap immediately
    if (_reducedMotion) {
        animTarget->setProperty(animProp.constData(), qTo);
        fw::AnimationCompleted completeNotif(property, handle);
        target->SendNotification(target, completeNotif);
        return handle;
    }

    // Create spring animation with custom parameters
    auto* springAnim = new SpringAnimation(this);
    springAnim->SetSpring(spring);
    springAnim->setStartValue(qFrom);
    springAnim->setEndValue(qTo);
    // SpringAnimation is QVariantAnimation — manually forward values to target.
    connect(springAnim, &QVariantAnimation::valueChanged,
            animTarget, [animTarget, animProp](const QVariant& value) {
        animTarget->setProperty(animProp.constData(), value);
    });

    TransitionEntry entry;
    entry.handle     = handle;
    entry.target     = target;
    entry.propertyId = property;
    entry.qtProperty = qtProp;
    entry.animation  = springAnim;
    _active[static_cast<uint64_t>(handle)] = entry;

    connect(springAnim, &QAbstractAnimation::finished, this, [this, handle]() {
        OnFinished(handle);
    });

    springAnim->start(QAbstractAnimation::KeepWhenStopped);

    return handle;
}

auto AnimationService::AnimateGroup(std::span<const GroupAnimationSpec> specs,
                                    GroupMode mode) -> GroupId
{
    if (specs.empty()) {
        return GroupId::Invalid;
    }

    const auto gid = static_cast<GroupId>(_nextId++);

    QAnimationGroup* group = nullptr;
    if (mode == GroupMode::Parallel) {
        group = new QParallelAnimationGroup(this);
    } else {
        group = new QSequentialAnimationGroup(this);
    }

    GroupEntry groupEntry;
    groupEntry.id    = gid;
    groupEntry.group = group;

    for (const auto& spec : specs) {
        if (spec.target == nullptr || spec.target->Widget() == nullptr) {
            continue;
        }

        const auto qtProp = MapPropertyId(spec.property);
        const auto qTo    = MapValue(spec.to);

        // Interrupt existing animation on same (target, property) — re-target.
        const QVariant captured = InterruptExisting(spec.target, spec.property);
        const QVariant qFrom = captured.isValid() ? captured : MapValue(spec.from);

        int durationMs = ResolveDurationMs(spec.duration);
        if (_speedMultiplier > 0.0F && _speedMultiplier != 1.0F) {
            durationMs = static_cast<int>(
                std::lroundf(static_cast<float>(durationMs) / _speedMultiplier));
        }

        // Allocate a handle for this sub-animation
        const auto subHandle = NextHandle();

        // Dispatch AnimationStarted for each sub-animation
        fw::AnimationStarted startNotif(spec.property, subHandle);
        spec.target->SendNotification(spec.target, startNotif);

        if (_reducedMotion || durationMs <= 0) {
            spec.target->Widget()->setProperty(qtProp.constData(), qTo);
            fw::AnimationCompleted completeNotif(spec.property, subHandle);
            spec.target->SendNotification(spec.target, completeNotif);
            continue;
        }

        QAbstractAnimation* anim = nullptr;
        if (spec.easing == fw::EasingToken::Spring) {
            auto* spring = new SpringAnimation(group);
            spring->SetSpring(_theme.Spring());
            spring->setStartValue(qFrom);
            spring->setEndValue(qTo);
            group->addAnimation(spring);
            anim = spring;
        } else {
            auto* propAnim = new QPropertyAnimation(spec.target->Widget(),
                                                    qtProp, group);
            propAnim->setDuration(durationMs);
            propAnim->setStartValue(qFrom);
            propAnim->setEndValue(qTo);
            propAnim->setEasingCurve(
                static_cast<QEasingCurve::Type>(ResolveEasing(spec.easing)));
            group->addAnimation(propAnim);
            anim = propAnim;
        }

        // Register sub-animation in _active for interruption isolation
        TransitionEntry entry;
        entry.handle      = subHandle;
        entry.target      = spec.target;
        entry.propertyId  = spec.property;
        entry.qtProperty  = qtProp;
        entry.animation   = anim;
        entry.owningGroup = gid;
        _active[static_cast<uint64_t>(subHandle)] = entry;

        groupEntry.members.push_back(subHandle);
    }

    if (groupEntry.members.empty()) {
        // All specs were reduced-motion or invalid — nothing to run.
        delete group;
        return gid;
    }

    _activeGroups[static_cast<uint64_t>(gid)] = std::move(groupEntry);

    connect(group, &QAbstractAnimation::finished, this, [this, gid]() {
        OnGroupFinished(gid);
    });

    group->start(QAbstractAnimation::KeepWhenStopped);

    return gid;
}

void AnimationService::Cancel(fw::TransitionHandle handle)
{
    const auto key = static_cast<uint64_t>(handle);
    auto it = _active.find(key);
    if (it == _active.end()) {
        return;
    }

    // Copy fields, erase BEFORE stop() — see comment in Animate().
    auto* anim = it->second.animation;
    auto prop = it->second.propertyId;
    auto hndl = it->second.handle;
    auto* tgt = it->second.target;
    _active.erase(it);

    if (anim != nullptr) {
        anim->stop();
        anim->deleteLater();
    }

    if (tgt != nullptr) {
        fw::AnimationCancelled notif(prop, hndl);
        tgt->SendNotification(tgt, notif);
    }
}

void AnimationService::CancelAll(fw::WidgetNode* target)
{
    // Collect entries to cancel, then erase+stop outside iteration.
    struct PendingCancel {
        uint64_t key;
        QAbstractAnimation* anim;
        fw::AnimationPropertyId prop;
        fw::TransitionHandle handle;
        GroupId owningGroup;
    };
    std::vector<PendingCancel> pending;
    for (auto& [key, entry] : _active) {
        if (entry.target == target) {
            pending.push_back({key, entry.animation, entry.propertyId,
                               entry.handle, entry.owningGroup});
        }
    }
    for (auto& p : pending) {
        _active.erase(p.key);
        // For non-group animations, stop directly.
        // For group members, the group owns the QAbstractAnimation lifetime;
        // we only remove from _active and let the group finish naturally.
        if (p.owningGroup == GroupId::Invalid && p.anim != nullptr) {
            p.anim->stop();
            p.anim->deleteLater();
        }
        fw::AnimationCancelled notif(p.prop, p.handle);
        target->SendNotification(target, notif);
    }
}

void AnimationService::CancelGroup(GroupId gid)
{
    const auto gKey = static_cast<uint64_t>(gid);
    auto git = _activeGroups.find(gKey);
    if (git == _activeGroups.end()) {
        return;
    }

    auto groupEntry = std::move(git->second);
    _activeGroups.erase(git);

    // Cancel each member's entry from _active and dispatch notification.
    for (auto memberHandle : groupEntry.members) {
        const auto mKey = static_cast<uint64_t>(memberHandle);
        auto mit = _active.find(mKey);
        if (mit != _active.end()) {
            auto* tgt = mit->second.target;
            auto prop = mit->second.propertyId;
            auto hndl = mit->second.handle;
            _active.erase(mit);

            if (tgt != nullptr) {
                fw::AnimationCancelled notif(prop, hndl);
                tgt->SendNotification(tgt, notif);
            }
        }
    }

    // Stop and destroy the Qt group animation.
    if (groupEntry.group != nullptr) {
        groupEntry.group->stop();
        groupEntry.group->deleteLater();
    }
}

auto AnimationService::IsRunning(fw::TransitionHandle handle) const -> bool
{
    const auto key = static_cast<uint64_t>(handle);
    auto it = _active.find(key);
    if (it == _active.end()) {
        return false;
    }
    return it->second.animation != nullptr &&
           it->second.animation->state() == QAbstractAnimation::Running;
}

auto AnimationService::IsAnimatingProperty(fw::WidgetNode* target,
                                            fw::AnimationPropertyId property) const -> bool
{
    return std::any_of(_active.begin(), _active.end(),
        [target, property](const auto& pair) {
            const auto& e = pair.second;
            return e.target == target && e.propertyId == property
                   && e.animation != nullptr
                   && e.animation->state() == QAbstractAnimation::Running;
        });
}

void AnimationService::SetReducedMotion(bool enabled)
{
    _reducedMotion = enabled;
}

auto AnimationService::IsReducedMotion() const -> bool
{
    return _reducedMotion;
}

void AnimationService::SetSpeedMultiplier(float factor)
{
    _speedMultiplier = (factor > 0.0F) ? factor : 1.0F;
}

auto AnimationService::SpeedMultiplier() const -> float
{
    return _speedMultiplier;
}

// ============================================================================
// Private helpers
// ============================================================================

auto AnimationService::NextHandle() -> fw::TransitionHandle
{
    return static_cast<fw::TransitionHandle>(_nextId++);
}

auto AnimationService::ResolveEasing(fw::EasingToken token) const -> int
{
    return _theme.Easing(token);
}

auto AnimationService::ResolveDurationMs(fw::AnimationsToken token) const -> int
{
    return _theme.AnimationMs(token);
}

auto AnimationService::InterruptExisting(fw::WidgetNode* target,
                                         fw::AnimationPropertyId property) -> QVariant
{
    QVariant captured;
    for (auto it = _active.begin(); it != _active.end(); ++it) {
        auto& entry = it->second;
        if (entry.target == target && entry.propertyId == property
            && entry.animation != nullptr) {
            // Capture the current interpolated value for re-targeting.
            // QPropertyAnimation inherits QVariantAnimation, so this covers both
            // QPropertyAnimation and SpringAnimation (also a QVariantAnimation).
            if (auto* varAnim = qobject_cast<QVariantAnimation*>(entry.animation)) {
                captured = varAnim->currentValue();
            }

            auto* anim = entry.animation;
            auto oldProp = entry.propertyId;
            auto oldHandle = entry.handle;
            auto* oldTarget = entry.target;
            auto oldGroup = entry.owningGroup;
            _active.erase(it);

            // For non-group animations, stop and destroy.
            // For group members, the group manages lifetime.
            if (oldGroup == GroupId::Invalid) {
                anim->stop();
                anim->deleteLater();
            }

            fw::AnimationCancelled cancelNotif(oldProp, oldHandle);
            oldTarget->SendNotification(oldTarget, cancelNotif);
            break;
        }
    }
    return captured;
}

void AnimationService::OnFinished(fw::TransitionHandle handle)
{
    const auto key = static_cast<uint64_t>(handle);
    auto it = _active.find(key);
    if (it == _active.end()) {
        return;
    }

    auto& entry = it->second;
    auto* anim = entry.animation;
    auto* tgt = entry.target;
    auto prop = entry.propertyId;
    auto hndl = entry.handle;
    auto ownGroup = entry.owningGroup;
    _active.erase(it);

    // Non-group animations: clean up directly.
    // Group member animations: the group manages Qt object lifetime.
    if (anim != nullptr && ownGroup == GroupId::Invalid) {
        anim->deleteLater();
    }

    if (tgt != nullptr) {
        fw::AnimationCompleted notif(prop, hndl);
        tgt->SendNotification(tgt, notif);
    }
}

void AnimationService::OnGroupFinished(GroupId gid)
{
    const auto gKey = static_cast<uint64_t>(gid);
    auto git = _activeGroups.find(gKey);
    if (git == _activeGroups.end()) {
        return;
    }

    auto groupEntry = std::move(git->second);
    _activeGroups.erase(git);

    // Dispatch AnimationCompleted for each member still in _active.
    for (auto memberHandle : groupEntry.members) {
        const auto mKey = static_cast<uint64_t>(memberHandle);
        auto mit = _active.find(mKey);
        if (mit != _active.end()) {
            auto* tgt = mit->second.target;
            auto prop = mit->second.propertyId;
            auto hndl = mit->second.handle;
            _active.erase(mit);

            if (tgt != nullptr) {
                fw::AnimationCompleted notif(prop, hndl);
                tgt->SendNotification(tgt, notif);
            }
        }
    }

    // Clean up the Qt group object.
    if (groupEntry.group != nullptr) {
        groupEntry.group->deleteLater();
    }
}

} // namespace matcha::gui
