/**
 * @file NyanCheckBox.cpp
 * @brief Implementation of NyanCheckBox custom-painted checkbox.
 */

#include <Matcha/Widgets/Controls/NyanCheckBox.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace matcha::gui {

NyanCheckBox::NyanCheckBox(QWidget* parent)
    : QCheckBox(parent)
    , ThemeAware(WidgetKind::CheckBox)
{
    setFixedHeight(kFixedHeight);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanCheckBox::NyanCheckBox(const QString& text, QWidget* parent)
    : QCheckBox(text, parent)
    , ThemeAware(WidgetKind::CheckBox)
{
    setFixedHeight(kFixedHeight);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanCheckBox::~NyanCheckBox() = default;

void NyanCheckBox::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRect widgetRect = rect();
    const int xPos = 0;
    const int yPos = (widgetRect.height() / 2) - (kIndicatorSize / 2);

    // Outer indicator rect (full size for pixmap-like placement)
    const QRect indicatorOuter(xPos, yPos, kIndicatorSize, kIndicatorSize);
    // Inner indicator rect (inset for border drawing, matching old style)
    QRect indicatorInner = indicatorOuter.adjusted(
        kIndicatorInset, kIndicatorInset, -kIndicatorInset, -kIndicatorInset);

    // Resolve variant: Unchecked=0, Checked=1, Partial=2
    const auto qtState = checkState();
    const std::size_t variantIdx = (qtState == Qt::Checked)        ? 1
                                 : (qtState == Qt::PartiallyChecked) ? 2
                                                                     : 0;

    // Resolve interaction state
    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : underMouse() ? InteractionState::Hovered
                                     : InteractionState::Normal;

    const auto style = Theme().Resolve(WidgetKind::CheckBox, variantIdx, istate);
    p.setOpacity(style.opacity);

    // -- Draw indicator --
    if (qtState == Qt::Checked) {
        // Checked: filled rounded rect + white check mark
        p.setPen(Qt::NoPen);
        p.setBrush(style.background);
        p.drawRoundedRect(indicatorInner, 3, 3);

        // Draw check mark (foreground color polyline)
        p.setPen(QPen(style.foreground, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        const qreal cx = indicatorInner.center().x();
        const qreal cy = indicatorInner.center().y();
        const qreal s = indicatorInner.width() * 0.3;
        QPainterPath checkPath;
        checkPath.moveTo(cx - s, cy);
        checkPath.lineTo(cx - (s * 0.3), cy + (s * 0.7));
        checkPath.lineTo(cx + s, cy - (s * 0.5));
        p.drawPath(checkPath);
    } else if (qtState == Qt::PartiallyChecked) {
        // Partial: outline rect + foreground-colored inner rect
        p.setBrush(style.background);
        p.setPen(style.border);
        p.drawRoundedRect(indicatorInner, 3, 3);

        QRect innerFill = indicatorInner.adjusted(3, 3, -3, -3);
        p.setBrush(style.foreground);
        p.setPen(Qt::NoPen);
        p.drawRect(innerFill);
    } else {
        // Unchecked: outline rect
        p.setBrush(style.background);
        p.setPen(style.border);
        p.drawRoundedRect(indicatorInner, 3, 3);
    }

    // -- Draw text --
    const QRect textRect(
        indicatorOuter.right() + style.gapPx,
        0,
        widgetRect.width() - indicatorOuter.width() - (style.gapPx * 2),
        widgetRect.height()
    );

    p.setPen(style.foreground);
    p.setFont(style.font);
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
}

void NyanCheckBox::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
