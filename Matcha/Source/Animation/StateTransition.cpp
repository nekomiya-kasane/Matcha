#include "StateTransition.h"

#include "Matcha/Tree/WidgetNode.h"
#include "Matcha/Animation/IAnimationService.h"
#include "Matcha/Theming/IThemeService.h"
#include "Matcha/Theming/WidgetStyleSheet.h"

#include <QWidget>

#include <utility>

namespace matcha::gui {

namespace {

auto QColorToRgba(const QColor& c) -> fw::AnimatableValue
{
    return fw::AnimatableValue::FromRgba(
        static_cast<uint8_t>(c.red()),
        static_cast<uint8_t>(c.green()),
        static_cast<uint8_t>(c.blue()),
        static_cast<uint8_t>(c.alpha()));
}

} // anonymous namespace

StateTransition::StateTransition(fw::WidgetNode* target, uint8_t variant)
    : _target(target)
    , _variant(variant)
{
}

void StateTransition::OnStateChanged(const WidgetStyleSheet& sheet,
                                     fw::InteractionState oldState,
                                     fw::InteractionState newState,
                                     fw::AnimationsToken duration,
                                     fw::EasingToken easing)
{
    if (_target == nullptr || _target->Widget() == nullptr) {
        return;
    }

    if (!HasThemeService()) {
        return;
    }

    auto& theme = GetThemeService();

    // Bounds check variant index
    if (_variant >= sheet.variants.size()) {
        return;
    }

    const auto& variant = sheet.variants[_variant];
    const auto oldIdx = std::to_underlying(oldState);
    const auto newIdx = std::to_underlying(newState);

    if (oldIdx >= variant.colors.size() || newIdx >= variant.colors.size()) {
        return;
    }

    const auto& oldColors = variant.colors[oldIdx];
    const auto& newColors = variant.colors[newIdx];

    // Resolve actual QColor values from tokens
    const QColor bgFrom = theme.Color(oldColors.background);
    const QColor bgTo   = theme.Color(newColors.background);
    const QColor fgFrom = theme.Color(oldColors.foreground);
    const QColor fgTo   = theme.Color(newColors.foreground);
    const QColor brFrom = theme.Color(oldColors.border);
    const QColor brTo   = theme.Color(newColors.border);

    auto* svc = GetAnimationService();

    // If no animation service available, snap to final values
    if (svc == nullptr) {
        QWidget* widget = _target->Widget();
        widget->setProperty("_matcha_bg", bgTo);
        widget->setProperty("_matcha_fg", fgTo);
        widget->setProperty("_matcha_border", brTo);
        widget->update();
        return;
    }

    // Animate each color property that actually changes via AnimationService
    if (bgFrom != bgTo) {
        svc->Animate(_target, fw::AnimationPropertyId::BackgroundColor,
                     QColorToRgba(bgFrom), QColorToRgba(bgTo),
                     duration, easing);
    }
    if (fgFrom != fgTo) {
        svc->Animate(_target, fw::AnimationPropertyId::ForegroundColor,
                     QColorToRgba(fgFrom), QColorToRgba(fgTo),
                     duration, easing);
    }
    if (brFrom != brTo) {
        svc->Animate(_target, fw::AnimationPropertyId::BorderColor,
                     QColorToRgba(brFrom), QColorToRgba(brTo),
                     duration, easing);
    }
}

} // namespace matcha::gui
