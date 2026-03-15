/**
 * @file NyanDoubleSpinBox.cpp
 * @brief Implementation of NyanDoubleSpinBox themed double spinner.
 */

#include <Matcha/Widgets/Controls/NyanDoubleSpinBox.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanDoubleSpinBox::NyanDoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent)
    , ThemeAware(WidgetKind::SpinBox)
{
    setFixedHeight(kFixedHeight);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setDecimals(_precision);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanDoubleSpinBox::~NyanDoubleSpinBox() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanDoubleSpinBox::SetRange(double minimum, double maximum)
{
    setRange(minimum, maximum);
}

void NyanDoubleSpinBox::SetStep(double step)
{
    setSingleStep(step);
}

void NyanDoubleSpinBox::SetPrecision(int decimals)
{
    _precision = std::clamp(decimals, 1, 15);
    setDecimals(_precision);
}

auto NyanDoubleSpinBox::Precision() const -> int
{
    return _precision;
}

auto NyanDoubleSpinBox::sizeHint() const -> QSize
{
    QSize s = QDoubleSpinBox::sizeHint();
    s.setHeight(kFixedHeight);
    s.setWidth(s.width() + kButtonWidth);
    return s;
}

auto NyanDoubleSpinBox::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanDoubleSpinBox::paintEvent(QPaintEvent* event)
{
    // Let QDoubleSpinBox paint text content first
    QDoubleSpinBox::paintEvent(event);

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

void NyanDoubleSpinBox::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
