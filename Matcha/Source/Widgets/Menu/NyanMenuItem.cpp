#include <Matcha/Widgets/Menu/NyanMenuItem.h>

#include <QMouseEvent>
#include <QPainter>

namespace matcha::gui {

NyanMenuItem::NyanMenuItem(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::MenuItem)
{
    setFixedHeight(kHeight);
    setMinimumWidth(kMinWidth);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMouseTracking(true);
}

NyanMenuItem::~NyanMenuItem() = default;

// -- Properties --

void NyanMenuItem::SetText(const QString& text)
{
    if (_text != text) {
        _text = text;
        update();
    }
}

auto NyanMenuItem::Text() const -> QString
{
    return _text;
}

void NyanMenuItem::SetIcon(const QIcon& icon)
{
    _icon = icon;
    update();
}

auto NyanMenuItem::Icon() const -> QIcon
{
    return _icon;
}

void NyanMenuItem::SetShortcut(const QKeySequence& shortcut)
{
    if (_shortcut != shortcut) {
        _shortcut = shortcut;
        update();
    }
}

auto NyanMenuItem::Shortcut() const -> QKeySequence
{
    return _shortcut;
}

void NyanMenuItem::SetEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;
        update();
    }
}

auto NyanMenuItem::IsEnabled() const -> bool
{
    return _enabled;
}

void NyanMenuItem::SetSubmenuIndicator(bool show)
{
    if (_submenuIndicator != show) {
        _submenuIndicator = show;
        update();
    }
}

auto NyanMenuItem::HasSubmenuIndicator() const -> bool
{
    return _submenuIndicator;
}

// -- Size hints --

auto NyanMenuItem::sizeHint() const -> QSize
{
    return {kMinWidth, kHeight};
}

auto NyanMenuItem::minimumSizeHint() const -> QSize
{
    return {kMinWidth, kHeight};
}

// -- Paint --

void NyanMenuItem::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    DrawContent(p, rect(), _hovered, _pressed);
}

void NyanMenuItem::DrawContent(QPainter& painter, const QRect& rect, bool hovered, bool pressed) const
{
    const auto& theme = Theme();

    // Determine colors based on state
    QColor bgColor;
    QColor textColor;

    if (!_enabled) {
        bgColor = theme.Color(ColorToken::SurfaceElevated);
        textColor = theme.Color(ColorToken::TextDisabled);
    } else if (pressed) {
        bgColor = theme.Color(ColorToken::PrimaryBgHover);
        textColor = theme.Color(ColorToken::Primary);
    } else if (hovered) {
        bgColor = theme.Color(ColorToken::FillActive);
        textColor = theme.Color(ColorToken::TextPrimary);
    } else {
        bgColor = theme.Color(ColorToken::SurfaceElevated);
        textColor = theme.Color(ColorToken::TextPrimary);
    }

    // Background
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect, kRadius, kRadius);

    // Icon
    if (!_icon.isNull()) {
        QRect iconRect(rect.x() + kIconLeft, rect.center().y() - (kIconSize / 2), kIconSize, kIconSize);
        QPixmap pixmap = _icon.pixmap(kIconSize, kIconSize);
        painter.drawPixmap(iconRect, pixmap);
    }

    // Calculate text area
    int textLeft = rect.x() + kTextLeft;
    int textRight = rect.right() - 8;

    // Submenu arrow
    if (_submenuIndicator) {
        QRect arrowRect(rect.right() - kArrowRight, rect.center().y() - (kArrowSize / 2), kArrowSize, kArrowSize);
        textRight = arrowRect.left() - 8;

        // Draw arrow (simple triangle)
        painter.setPen(textColor);
        painter.setBrush(textColor);
        QPolygonF arrow;
        int cx = arrowRect.center().x();
        int cy = arrowRect.center().y();
        arrow << QPointF(cx - 3, cy - 5) << QPointF(cx + 4, cy) << QPointF(cx - 3, cy + 5);
        painter.drawPolygon(arrow);
    }

    // Text and shortcut
    painter.setPen(textColor);
    QRect textRect(textLeft, rect.y(), textRight - textLeft, rect.height());

    if (!_shortcut.isEmpty()) {
        QString shortcutStr = _shortcut.toString(QKeySequence::NativeText);
        QFontMetrics fm(painter.font());
        int shortcutWidth = fm.horizontalAdvance(shortcutStr);

        // Draw shortcut right-aligned
        QRect shortcutRect(textRight - shortcutWidth, rect.y(), shortcutWidth, rect.height());
        painter.drawText(shortcutRect, Qt::AlignVCenter | Qt::AlignRight, shortcutStr);

        // Adjust text rect
        textRect.setRight(shortcutRect.left() - 8);
    }

    // Draw text
    QFontMetrics fm(painter.font());
    QString elidedText = fm.elidedText(_text, Qt::ElideRight, textRect.width());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedText);
}

// -- Mouse Events --

void NyanMenuItem::enterEvent(QEnterEvent* /*event*/)
{
    _hovered = true;
    update();
}

void NyanMenuItem::leaveEvent(QEvent* /*event*/)
{
    _hovered = false;
    _pressed = false;
    update();
}

void NyanMenuItem::mousePressEvent(QMouseEvent* /*event*/)
{
    if (_enabled) {
        _pressed = true;
        update();
    }
}

void NyanMenuItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (_enabled && _pressed && rect().contains(event->pos())) {
        Q_EMIT Triggered();
    }
    _pressed = false;
    update();
}

void NyanMenuItem::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
