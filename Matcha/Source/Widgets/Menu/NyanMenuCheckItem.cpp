#include <Matcha/Widgets/Menu/NyanMenuCheckItem.h>

#include <QPainter>

namespace matcha::gui {

NyanMenuCheckItem::NyanMenuCheckItem(QWidget* parent)
    : NyanMenuItem(parent)
{
    // Connect to our own Triggered signal to toggle
    connect(this, &NyanMenuItem::Triggered, this, &NyanMenuCheckItem::Toggle);
}

NyanMenuCheckItem::~NyanMenuCheckItem() = default;

// -- Check State --

void NyanMenuCheckItem::SetChecked(bool checked)
{
    if (_checked != checked) {
        _checked = checked;
        update();
        Q_EMIT Toggled(_checked);
    }
}

auto NyanMenuCheckItem::IsChecked() const -> bool
{
    return _checked;
}

void NyanMenuCheckItem::Toggle()
{
    SetChecked(!_checked);
}

// -- Paint --

void NyanMenuCheckItem::DrawContent(QPainter& painter, const QRect& rect, bool hovered, bool pressed) const
{
    // Call base class to draw background and text
    NyanMenuItem::DrawContent(painter, rect, hovered, pressed);

    // Draw checkmark if checked
    if (_checked) {
        DrawCheckmark(painter, rect);
    }
}

void NyanMenuCheckItem::DrawCheckmark(QPainter& painter, const QRect& rect) const
{
    const auto& theme = Theme();

    // Checkmark in icon area (same position as icon)
    constexpr int kIconSize = 16;
    constexpr int kIconLeft = 4;

    QRect checkRect(rect.x() + kIconLeft, rect.center().y() - (kIconSize / 2), kIconSize, kIconSize);

    // Draw checkmark
    painter.setPen(QPen(theme.Color(ColorToken::colorPrimary), 2));

    // Checkmark path: small L shape
    int x = checkRect.center().x();
    int y = checkRect.center().y();
    painter.drawLine(x - 4, y, x - 1, y + 3);
    painter.drawLine(x - 1, y + 3, x + 5, y - 4);
}

} // namespace matcha::gui
