/**
 * @file NyanToolButton.cpp
 * @brief Implementation of NyanToolButton compact toolbar button with flyout.
 */

#include <Matcha/Widgets/Controls/NyanToolButton.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanToolButton::NyanToolButton(QWidget* parent)
    : QToolButton(parent)
    , ThemeAware(WidgetKind::ToolButton)
{
    setFixedSize(kDefaultSize, kDefaultSize);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanToolButton::~NyanToolButton() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanToolButton::SetFlyoutMenu(QMenu* menu)
{
    _flyoutMenu = menu;
    if (_flyoutMenu != nullptr) {
        setMenu(_flyoutMenu);
        setPopupMode(QToolButton::MenuButtonPopup);
    } else {
        setMenu(nullptr);
        setPopupMode(QToolButton::DelayedPopup);
    }
    update();
}

auto NyanToolButton::FlyoutMenu() const -> QMenu*
{
    return _flyoutMenu;
}

void NyanToolButton::SetFlyoutPolicy(FlyoutPolicy policy)
{
    _flyoutPolicy = policy;
}

auto NyanToolButton::GetFlyoutPolicy() const -> FlyoutPolicy
{
    return _flyoutPolicy;
}

auto NyanToolButton::sizeHint() const -> QSize
{
    int w = kDefaultSize;
    if (_flyoutMenu != nullptr) {
        w += kArrowWidth;
    }
    return {w, kDefaultSize};
}

auto NyanToolButton::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanToolButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Variant: Default=0, Active=1 (checked state)
    const bool checked = isCheckable() && isChecked();
    const std::size_t variantIdx = checked ? 1 : 0;

    auto istate = !isEnabled() ? InteractionState::Disabled
                : isDown()     ? InteractionState::Pressed
                : underMouse() ? InteractionState::Hovered
                               : InteractionState::Normal;

    const auto style = Theme().Resolve(WidgetKind::ToolButton, variantIdx, istate);
    p.setOpacity(style.opacity);

    const QRect r = rect().adjusted(1, 1, -1, -1);

    // -- Background + border --
    p.setPen(QPen(style.border, style.borderWidthPx));
    p.setBrush(style.background);
    p.drawRoundedRect(r, style.radiusPx, style.radiusPx);

    // -- Icon or text (centered in main button area) --
    const int mainW = (_flyoutMenu != nullptr) ? (r.width() - kArrowWidth) : r.width();
    if (!icon().isNull()) {
        const int iconX = ((mainW - kIconSize) / 2) + r.x();
        const int iconY = ((r.height() - kIconSize) / 2) + r.y();
        const QPixmap pix = icon().pixmap(kIconSize, kIconSize);
        p.drawPixmap(iconX, iconY, pix);
    } else if (!text().isEmpty()) {
        p.setPen(style.foreground);
        p.setFont(style.font);
        QRect textRect(r.x(), r.y(), mainW, r.height());
        p.drawText(textRect, Qt::AlignCenter, text());
    }

    // -- Dropdown arrow (if flyout menu attached) --
    if (_flyoutMenu != nullptr) {
        const int arrowX = r.x() + mainW + ((kArrowWidth - kArrowSize) / 2);
        const int arrowY = ((r.height() - kArrowSize) / 2) + r.y();

        p.setPen(Qt::NoPen);
        p.setBrush(style.foreground);
        QPainterPath arrow;
        arrow.moveTo(arrowX, arrowY);
        arrow.lineTo(arrowX + kArrowSize, arrowY);
        arrow.lineTo(arrowX + (kArrowSize / 2.0), arrowY + kArrowSize);
        arrow.closeSubpath();
        p.drawPath(arrow);
    }
}

void NyanToolButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        emit RightClicked();
        event->accept();
        return;
    }
    QToolButton::mousePressEvent(event);
}

void NyanToolButton::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
