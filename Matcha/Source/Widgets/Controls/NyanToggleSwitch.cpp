/**
 * @file NyanToggleSwitch.cpp
 * @brief Implementation of NyanToggleSwitch animated toggle control.
 */

#include <Matcha/Widgets/Controls/NyanToggleSwitch.h>

#include "../Core/InteractionEventFilter.h"

#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>

namespace matcha::gui {

NyanToggleSwitch::NyanToggleSwitch(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Toggle)
{
    setFixedSize(sizeHint());
    setCursor(Qt::PointingHandCursor);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanToggleSwitch::~NyanToggleSwitch() = default;

void NyanToggleSwitch::SetChecked(bool checked)
{
    if (_checked == checked) {
        return;
    }
    _checked = checked;

    // Animate knob position
    auto* anim = new QVariantAnimation(this);
    const int durationMs = Theme().AnimationMs(StyleSheet().transition.duration);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    anim->setStartValue(_knobPos);
    anim->setEndValue(_checked ? 1.0 : 0.0);
    connect(anim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v) {
                _knobPos = v.toDouble();
                update();
            });
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    emit Toggled(_checked);
}

auto NyanToggleSwitch::IsChecked() const -> bool
{
    return _checked;
}

void NyanToggleSwitch::SetOnText(const QString& text)
{
    _onText = text;
    updateGeometry();
    update();
}

void NyanToggleSwitch::SetOffText(const QString& text)
{
    _offText = text;
    updateGeometry();
    update();
}

void NyanToggleSwitch::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Resolve variant: Off=0, On=1
    const std::size_t variantIdx = _checked ? 1 : 0;
    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : underMouse() ? InteractionState::Hovered
                                     : InteractionState::Normal;

    const auto style = Theme().Resolve(WidgetKind::Toggle, variantIdx, istate);
    p.setOpacity(style.opacity);

    // -- Draw track (background = track color) --
    const int trackY = (height() - kTrackHeight) / 2;
    const QRectF trackRect(0, trackY, kTrackWidth, kTrackHeight);
    const qreal trackRadius = kTrackHeight / 2.0;

    p.setBrush(style.background);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(trackRect, trackRadius, trackRadius);

    // -- Draw knob (foreground = knob color, border = knob border) --
    const qreal knobTravel = kTrackWidth - kKnobSize - (kKnobMargin * 2);
    const qreal knobX = kKnobMargin + (_knobPos * knobTravel);
    const qreal knobY = trackY + kKnobMargin;
    const QRectF knobRect(knobX, knobY, kKnobSize, kKnobSize);

    p.setBrush(style.foreground);
    if (!_checked || !isEnabled()) {
        p.setPen(QPen(style.border, 1));
    } else {
        p.setPen(Qt::NoPen);
    }
    p.drawEllipse(knobRect);

    // -- Draw text label --
    const QString& label = _checked ? _onText : _offText;
    if (!label.isEmpty()) {
        const QRect textRect(
            kTrackWidth + style.gapPx, 0,
            width() - kTrackWidth - style.gapPx, height()
        );

        p.setPen(style.foreground);
        p.setFont(style.font);
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, label);
    }
}

void NyanToggleSwitch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled()) {
        SetChecked(!_checked);
    }
    QWidget::mousePressEvent(event);
}

auto NyanToggleSwitch::sizeHint() const -> QSize
{
    int w = kTrackWidth;
    if (!_onText.isEmpty() || !_offText.isEmpty()) {
        const auto& fontSpec = Theme().Font(StyleSheet().font);
        QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
        QFontMetrics fm(f);
        const int textW = std::max(fm.horizontalAdvance(_onText),
                                   fm.horizontalAdvance(_offText));
        w += kTextGap + textW;
    }
    return {w, kTrackHeight + 4}; // 4px vertical breathing room
}

void NyanToggleSwitch::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
