/**
 * @file ThemeAware.cpp
 * @brief ThemeAware mixin implementation.
 */

#include <Matcha/Widgets/Core/ThemeAware.h>

#include <Matcha/UiNodes/Core/WidgetNode.h>
#include <Matcha/Widgets/Core/IAnimationService.h>

#include <QPainter>
#include <QWidget>

namespace matcha::gui {

ThemeAware::ThemeAware(WidgetKind kind)
    : _theme(GetThemeService())
    , _kind(kind)
    , _sheet(_theme.ResolveStyleSheet(kind))
    , _conn(QObject::connect(&_theme, &IThemeService::ThemeChanged,
                             [this](const QString& /*newTheme*/) {
                                 _sheet = _theme.ResolveStyleSheet(_kind);
                                 OnThemeChanged();
                             }))
{
}

ThemeAware::~ThemeAware()
{
    QObject::disconnect(_conn);
}

void ThemeAware::PaintFocusRing(QWidget* widget, const IThemeService& theme,
                                int radius)
{
    if (widget == nullptr || !widget->hasFocus()) {
        return;
    }

    QPainter p(widget);
    p.setRenderHint(QPainter::Antialiasing);

    QColor ringColor = theme.Color(ColorToken::Focus);
    ringColor.setAlpha(200);

    constexpr int kRingWidth  = 2;
    constexpr int kRingOffset = 1;

    QPen pen(ringColor, kRingWidth);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    const QRect r = widget->rect().adjusted(
        kRingOffset, kRingOffset, -kRingOffset, -kRingOffset);
    p.drawRoundedRect(r, radius, radius);
}

// -------------------------------------------------------------------------- //
//  Animation helpers (RFC-08 Phase F)
// -------------------------------------------------------------------------- //

auto ThemeAware::AnimateTransition(fw::WidgetNode* target,
                                   fw::AnimationPropertyId property,
                                   fw::AnimatableValue from,
                                   fw::AnimatableValue to) -> fw::TransitionHandle
{
    return AnimateTransition(target, property, from, to,
                             _sheet.transition.duration,
                             _sheet.transition.easing);
}

auto ThemeAware::AnimateTransition(fw::WidgetNode* target,
                                   fw::AnimationPropertyId property,
                                   fw::AnimatableValue from,
                                   fw::AnimatableValue to,
                                   fw::AnimationToken duration,
                                   fw::EasingToken easing) -> fw::TransitionHandle
{
    auto* svc = GetAnimationService();
    if (svc == nullptr || target == nullptr) {
        return fw::TransitionHandle::Invalid;
    }
    return svc->Animate(target, property, from, to, duration, easing);
}

} // namespace matcha::gui
