/**
 * @file NyanPushButton.cpp
 * @brief Implementation of NyanPushButton multi-variant themed button.
 */

#include <Matcha/Widgets/Controls/NyanPushButton.h>

#include "../Core/InteractionEventFilter.h"

#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanPushButton::NyanPushButton(QWidget* parent)
    : QPushButton(parent)
    , ThemeAware(WidgetKind::PushButton)
{
    setFixedHeight(static_cast<int>(_size));
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanPushButton::NyanPushButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
    , ThemeAware(WidgetKind::PushButton)
{
    setFixedHeight(static_cast<int>(_size));
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanPushButton::NyanPushButton(const QIcon& icon, const QString& text,
                               QWidget* parent)
    : QPushButton(icon, text, parent)
    , ThemeAware(WidgetKind::PushButton)
{
    setFixedHeight(static_cast<int>(_size));
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanPushButton::~NyanPushButton() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanPushButton::SetVariant(ButtonVariant variant)
{
    _variant = variant;
    update();
}

auto NyanPushButton::Variant() const -> ButtonVariant
{
    return _variant;
}

void NyanPushButton::SetSize(ButtonSize size)
{
    _size = size;
    setFixedHeight(static_cast<int>(_size));
    updateGeometry();
    update();
}

auto NyanPushButton::Size() const -> ButtonSize
{
    return _size;
}

auto NyanPushButton::sizeHint() const -> QSize
{
    QSize s = QPushButton::sizeHint();
    const int h = static_cast<int>(_size);
    if (s.width() < kMinWidth) {
        s.setWidth(kMinWidth);
    }
    if (s.height() < h) {
        s.setHeight(h);
    }
    return s;
}

auto NyanPushButton::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanPushButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Variant index maps directly from ButtonVariant enum
    const auto variantIdx = static_cast<std::size_t>(std::to_underlying(_variant));

    // Interaction state from FSM, with checked mapped to Pressed
    auto istate = _interactionFilter->Fsm().CurrentState();
    if (isCheckable() && isChecked() && istate == fw::InteractionState::Normal) {
        istate = fw::InteractionState::Pressed;
    }

    const auto style = Theme().Resolve(WidgetKind::PushButton, variantIdx, istate);
    p.setOpacity(style.opacity);

    const QRect r = rect().adjusted(1, 1, -1, -1);

    // -- Background + border --
    p.setPen(QPen(style.border, style.borderWidthPx));
    p.setBrush(style.background);
    p.drawRoundedRect(r, style.radiusPx, style.radiusPx);

    // -- Content area --
    const bool hasIcon = !icon().isNull();
    const bool hasText = !text().isEmpty();
    p.setFont(style.font);
    p.setPen(style.foreground);

    if (hasIcon && hasText) {
        // Icon + text layout
        const QFontMetrics fm(style.font);
        const int textW = fm.horizontalAdvance(text());
        const int totalW = kIconSize + kIconGap + textW;
        const int startX = ((r.width() - totalW) / 2) + r.x();
        const int iconY = ((r.height() - kIconSize) / 2) + r.y();

        const QPixmap pix = icon().pixmap(kIconSize, kIconSize);
        p.drawPixmap(startX, iconY, pix);
        const QRect textRect(startX + kIconSize + kIconGap, r.y(), textW, r.height());
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
    } else if (hasIcon) {
        // Icon-only (centered)
        const int iconX = ((r.width() - kIconSize) / 2) + r.x();
        const int iconY = ((r.height() - kIconSize) / 2) + r.y();
        const QPixmap pix = icon().pixmap(kIconSize, kIconSize);
        p.drawPixmap(iconX, iconY, pix);
    } else {
        // Text-only (centered)
        p.drawText(r, Qt::AlignCenter, text());
    }
}

void NyanPushButton::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
