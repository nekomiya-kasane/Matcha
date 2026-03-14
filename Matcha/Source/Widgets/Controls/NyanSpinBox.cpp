/**
 * @file NyanSpinBox.cpp
 * @brief Implementation of NyanSpinBox themed integer spinner.
 */

#include <Matcha/Widgets/Controls/NyanSpinBox.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanSpinBox::NyanSpinBox(QWidget* parent)
    : QSpinBox(parent)
    , ThemeAware(WidgetKind::SpinBox)
{
    setFixedHeight(kFixedHeight);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanSpinBox::~NyanSpinBox() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanSpinBox::SetRange(int minimum, int maximum)
{
    setRange(minimum, maximum);
}

void NyanSpinBox::SetStep(int step)
{
    setSingleStep(step);
}

auto NyanSpinBox::sizeHint() const -> QSize
{
    QSize s = QSpinBox::sizeHint();
    s.setHeight(kFixedHeight);
    s.setWidth(s.width() + kButtonWidth);
    return s;
}

auto NyanSpinBox::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanSpinBox::paintEvent(QPaintEvent* event)
{
    // Let QSpinBox paint text content first
    QSpinBox::paintEvent(event);

    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : hasFocus()   ? InteractionState::Focused
                      : underMouse() ? InteractionState::Hovered
                                     : InteractionState::Normal;

    const auto style = Theme().Resolve(WidgetKind::SpinBox, 0, istate);
    const QRect r = rect().adjusted(0, 0, -1, -1);

    // -- Border overlay --
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(style.border, style.borderWidthPx));
    p.drawRoundedRect(r, style.radiusPx, style.radiusPx);

    // -- Up/down button area (right side) --
    const int btnX = r.right() - kButtonWidth;
    const int halfH = r.height() / 2;

    p.setPen(Qt::NoPen);
    p.setBrush(style.foreground);

    // Up arrow (top half)
    {
        const double cx = btnX + (kButtonWidth / 2.0);
        const double cy = (halfH / 2.0) + r.y();
        QPainterPath arrow;
        arrow.moveTo(cx, cy - kArrowSize);
        arrow.lineTo(cx + kArrowSize, cy + 1);
        arrow.lineTo(cx - kArrowSize, cy + 1);
        arrow.closeSubpath();
        p.drawPath(arrow);
    }

    // Down arrow (bottom half)
    {
        const double cx = btnX + (kButtonWidth / 2.0);
        const double cy = halfH + (halfH / 2.0) + r.y();
        QPainterPath arrow;
        arrow.moveTo(cx, cy + kArrowSize);
        arrow.lineTo(cx + kArrowSize, cy - 1);
        arrow.lineTo(cx - kArrowSize, cy - 1);
        arrow.closeSubpath();
        p.drawPath(arrow);
    }

    // -- Separator line between up/down --
    p.setPen(QPen(style.border, 1));
    p.drawLine(btnX, halfH + r.y(), r.right(), halfH + r.y());
}

void NyanSpinBox::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
