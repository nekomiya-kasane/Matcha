/**
 * @file NyanBadge.cpp
 * @brief Implementation of NyanBadge themed status pill badge.
 */

#include <Matcha/Widgets/Controls/NyanBadge.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanBadge::NyanBadge(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Label)
{
    setFixedHeight(kFixedHeight);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanBadge::~NyanBadge() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanBadge::SetText(const QString& text)
{
    _text = text;
    updateGeometry();
    update();
}

auto NyanBadge::Text() const -> QString { return _text; }

void NyanBadge::SetVariant(BadgeVariant variant)
{
    _variant = variant;
    update();
}

auto NyanBadge::Variant() const -> BadgeVariant { return _variant; }

void NyanBadge::SetCustomColor(const QColor& color)
{
    _customColor = color;
    if (_variant == BadgeVariant::Custom) {
        update();
    }
}

void NyanBadge::SetClosable(bool closable)
{
    _closable = closable;
    updateGeometry();
    update();
}

auto NyanBadge::IsClosable() const -> bool { return _closable; }

auto NyanBadge::sizeHint() const -> QSize
{
    const auto style = Theme().Resolve(WidgetKind::Label, 0, InteractionState::Normal);
    QFontMetrics fm(style.font);
    int w = (kHPadding * 2) + fm.horizontalAdvance(_text);
    if (_closable) {
        w += kCloseGap + kCloseSize;
    }
    return {w, kFixedHeight};
}

// ============================================================================
// Color resolution
// ============================================================================

auto NyanBadge::ResolveColors() const -> ResolvedColors
{
    // Semantic badge variant colors — not in the variant matrix
    switch (_variant) {
    case BadgeVariant::Success:
        return {Theme().Color(ColorToken::colorSuccess), Theme().Color(ColorToken::OnAccent)};
    case BadgeVariant::Warning:
        return {Theme().Color(ColorToken::colorWarning), Theme().Color(ColorToken::OnAccent)};
    case BadgeVariant::Error:
        return {Theme().Color(ColorToken::colorError), Theme().Color(ColorToken::OnAccent)};
    case BadgeVariant::Info:
        return {Theme().Color(ColorToken::colorPrimary), Theme().Color(ColorToken::OnAccent)};
    case BadgeVariant::Custom:
        return {_customColor.isValid() ? _customColor : Theme().Color(ColorToken::colorFill),
                Theme().Color(ColorToken::OnAccent)};
    case BadgeVariant::Neutral:
    default:
        return {Theme().Color(ColorToken::colorFill), Theme().Color(ColorToken::colorText)};
    }
}

// ============================================================================
// Painting
// ============================================================================

void NyanBadge::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto colors = ResolveColors();
    const QRect r = rect();
    const int radius = r.height() / 2; // Pill shape

    // -- Pill background --
    p.setPen(Qt::NoPen);
    p.setBrush(colors.background);
    p.drawRoundedRect(r, radius, radius);

    // -- Text --
    const auto style = Theme().Resolve(WidgetKind::Label, 0, InteractionState::Normal);
    p.setFont(style.font);
    p.setPen(colors.foreground);

    int textRight = r.right() - kHPadding;
    if (_closable) {
        textRight -= (kCloseGap + kCloseSize);
    }
    const QRect textRect(r.x() + kHPadding, r.y(), textRight - r.x() - kHPadding, r.height());
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, _text);

    // -- Close button (x) --
    if (_closable) {
        const int cx = textRight + kCloseGap + (kCloseSize / 2);
        const int cy = r.height() / 2;
        const int arm = 3; // Half-size of X arms
        p.setPen(QPen(colors.foreground, 1.5));
        p.drawLine(cx - arm, cy - arm, cx + arm, cy + arm);
        p.drawLine(cx - arm, cy + arm, cx + arm, cy - arm);
    }
}

void NyanBadge::mousePressEvent(QMouseEvent* event)
{
    if (_closable && event->button() == Qt::LeftButton) {
        // Check if click is in close button area
        const int closeX = width() - kHPadding - kCloseSize;
        if (event->position().toPoint().x() >= closeX) {
            emit Closed();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void NyanBadge::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui