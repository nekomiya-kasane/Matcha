/**
 * @file NyanDateTimePicker.cpp
 * @brief Implementation of NyanDateTimePicker themed date/time input.
 */

#include <Matcha/Widgets/Controls/NyanDateTimePicker.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanDateTimePicker::NyanDateTimePicker(QWidget* parent)
    : QDateTimeEdit(parent)
    , ThemeAware(WidgetKind::SpinBox) // reuse SpinBox style tokens
{
    setFixedHeight(kFixedHeight);
    setCalendarPopup(true);
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    ApplyMode();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanDateTimePicker::~NyanDateTimePicker() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanDateTimePicker::SetMode(DateTimeMode mode)
{
    if (_mode == mode) {
        return;
    }
    _mode = mode;
    ApplyMode();
    update();
}

auto NyanDateTimePicker::Mode() const -> DateTimeMode { return _mode; }

auto NyanDateTimePicker::sizeHint() const -> QSize
{
    QSize s = QDateTimeEdit::sizeHint();
    s.setHeight(kFixedHeight);
    return s;
}

auto NyanDateTimePicker::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanDateTimePicker::paintEvent(QPaintEvent* event)
{
    // Let QDateTimeEdit paint its content first
    QDateTimeEdit::paintEvent(event);

    // Overlay themed border
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : hasFocus()   ? InteractionState::Focused
                                     : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::SpinBox, 0, istate);

    p.setPen(QPen(style.border, style.borderWidthPx));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), style.radiusPx, style.radiusPx);
}

void NyanDateTimePicker::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private
// ============================================================================

void NyanDateTimePicker::ApplyMode()
{
    switch (_mode) {
    case DateTimeMode::Date:
        setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
        break;
    case DateTimeMode::Time:
        setDisplayFormat(QStringLiteral("HH:mm:ss"));
        break;
    case DateTimeMode::DateTime:
    default:
        setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        break;
    }
}

} // namespace matcha::gui