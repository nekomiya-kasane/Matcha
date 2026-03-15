/**
 * @file NyanRadioButton.cpp
 * @brief Implementation of NyanRadioButton custom-painted radio button.
 */

#include <Matcha/Widgets/Controls/NyanRadioButton.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

NyanRadioButton::NyanRadioButton(QWidget* parent)
    : QRadioButton(parent)
    , ThemeAware(WidgetKind::RadioButton)
{
    setFixedHeight(kFixedHeight);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanRadioButton::NyanRadioButton(const QString& text, QWidget* parent)
    : QRadioButton(text, parent)
    , ThemeAware(WidgetKind::RadioButton)
{
    setFixedHeight(kFixedHeight);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanRadioButton::~NyanRadioButton() = default;

void NyanRadioButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRect widgetRect = rect();
    const int xPos = 0;
    const int yPos = (widgetRect.height() / 2) - (kIndicatorSize / 2);

    // Outer indicator rect (full size)
    const QRect indicatorOuter(xPos, yPos, kIndicatorSize, kIndicatorSize);
    // Inner indicator rect (inset for border drawing)
    const QRect indicatorInner = indicatorOuter.adjusted(
        kIndicatorInset, kIndicatorInset, -kIndicatorInset, -kIndicatorInset);

    // Resolve variant: Unchecked=0, Checked=1
    const std::size_t variantIdx = isChecked() ? 1 : 0;

    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : underMouse() ? InteractionState::Hovered
                                     : InteractionState::Normal;

    const auto style = Theme().Resolve(WidgetKind::RadioButton, variantIdx, istate);
    p.setOpacity(style.opacity);

    // -- Draw circular indicator --
    if (isChecked()) {
        // Selected: filled circle + foreground inner dot
        p.setPen(Qt::NoPen);
        p.setBrush(style.background);
        p.drawEllipse(indicatorInner);

        // Inner dot
        const QRect dotRect = indicatorInner.adjusted(
            kInnerDotInset, kInnerDotInset, -kInnerDotInset, -kInnerDotInset);
        p.setBrush(style.foreground);
        p.drawEllipse(dotRect);
    } else {
        // Unselected: outline circle
        p.setBrush(style.background);
        p.setPen(style.border);
        p.drawEllipse(indicatorInner);
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

void NyanRadioButton::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
