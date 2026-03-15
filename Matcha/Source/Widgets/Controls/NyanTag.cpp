/**
 * @file NyanTag.cpp
 * @brief Implementation of NyanTag themed closable tag/chip widget.
 */

#include <Matcha/Widgets/Controls/NyanTag.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <algorithm>

#include <QEnterEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanTag::NyanTag(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Tag)
{
    setFixedHeight(kFixedHeight);
    setMaximumWidth(kMaxWidth);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setAttribute(Qt::WA_Hover, true);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanTag::~NyanTag() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanTag::SetText(const QString& text)
{
    _text = text;
    updateGeometry();
    update();
}

auto NyanTag::Text() const -> QString { return _text; }

void NyanTag::SetColor(const QColor& color)
{
    _color = color;
    update();
}

auto NyanTag::Color() const -> QColor { return _color; }

void NyanTag::SetIcon(const QIcon& icon)
{
    _icon = icon;
    updateGeometry();
    update();
}

auto NyanTag::Icon() const -> QIcon { return _icon; }

void NyanTag::SetClosable(bool closable)
{
    _closable = closable;
    updateGeometry();
    update();
}

auto NyanTag::IsClosable() const -> bool { return _closable; }

void NyanTag::SetSelected(bool selected)
{
    _selected = selected;
    update();
}

auto NyanTag::IsSelected() const -> bool { return _selected; }

auto NyanTag::sizeHint() const -> QSize
{
    const auto& fontSpec = Theme().Font(StyleSheet().font);
    QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
    QFontMetrics fm(f);

    int w = kHPadding * 2;
    if (!_icon.isNull()) {
        w += kIconSize + kIconGap;
    }
    w += fm.horizontalAdvance(_text);
    if (_closable && IsCloseVisible()) {
        w += kCloseGap + kCloseSize;
    }
    w = std::min(w, kMaxWidth);
    return {w, kFixedHeight};
}

// ============================================================================
// Close button helpers
// ============================================================================

auto NyanTag::IsCloseVisible() const -> bool
{
    if (!_closable) {
        return false;
    }
    // Selected: always show close. Unselected: show on hover only.
    return _selected || _hovered;
}

auto NyanTag::CloseButtonRect() const -> QRect
{
    const int x = width() - kHPadding - kCloseSize;
    const int y = (height() - kCloseSize) / 2;
    return {x, y, kCloseSize, kCloseSize};
}

// ============================================================================
// Painting
// ============================================================================

void NyanTag::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = _selected ? InteractionState::Selected : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::Tag, 0, istate);
    const QRect r = rect();

    // -- Background (custom color overrides when not selected) --
    QColor bg = style.background;
    if (!_selected && _color.isValid()) {
        bg = _color;
    }
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(r, style.radiusPx, style.radiusPx);

    // -- Text color --
    const QColor fg = style.foreground;

    // -- Font --
    p.setFont(style.font);
    QFontMetrics fm(style.font);

    int contentLeft = r.x() + kHPadding;

    // -- Prefix icon --
    if (!_icon.isNull()) {
        const int iconY = (r.height() - kIconSize) / 2;
        _icon.paint(&p, contentLeft, iconY, kIconSize, kIconSize);
        contentLeft += kIconSize + kIconGap;
    }

    // -- Text (with ellipsis) --
    int textRight = r.right() - kHPadding;
    if (IsCloseVisible()) {
        textRight -= (kCloseGap + kCloseSize);
    }
    const int textWidth = textRight - contentLeft;
    if (textWidth > 0) {
        const QString elided = fm.elidedText(_text, Qt::ElideRight, textWidth);
        p.setPen(fg);
        const QRect textRect(contentLeft, r.y(), textWidth, r.height());
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elided);
    }

    // -- Close button (x) --
    if (IsCloseVisible()) {
        const QRect cr = CloseButtonRect();
        const int cx = cr.center().x();
        const int cy = cr.center().y();
        const int arm = 3;
        p.setPen(QPen(fg, 1.5));
        p.drawLine(cx - arm, cy - arm, cx + arm, cy + arm);
        p.drawLine(cx - arm, cy + arm, cx + arm, cy - arm);
    }
}

// ============================================================================
// Events
// ============================================================================

void NyanTag::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (_closable && IsCloseVisible()
            && CloseButtonRect().contains(event->position().toPoint())) {
            emit Closed();
            event->accept();
            return;
        }
        emit Clicked();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void NyanTag::enterEvent(QEnterEvent* /*event*/)
{
    _hovered = true;
    if (_closable && !_selected) {
        updateGeometry();
        update();
    }
}

void NyanTag::leaveEvent(QEvent* /*event*/)
{
    _hovered = false;
    if (_closable && !_selected) {
        updateGeometry();
        update();
    }
}

void NyanTag::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui