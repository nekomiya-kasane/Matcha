/**
 * @file NyanPanel.cpp
 * @brief Implementation of NyanPanel themed container.
 */

#include <Matcha/Widgets/ActionBar/NyanPanel.h>

#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanPanel::NyanPanel(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Panel)
{
}

NyanPanel::~NyanPanel() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanPanel::SetElevation(ShadowToken elevation)
{
    _elevation = elevation;
    update();
}

auto NyanPanel::Elevation() const -> ShadowToken
{
    return _elevation;
}

void NyanPanel::SetBorderVisible(bool visible)
{
    _borderVisible = visible;
    update();
}

auto NyanPanel::BorderVisible() const -> bool
{
    return _borderVisible;
}

auto NyanPanel::sizeHint() const -> QSize
{
    return {200, 150};
}

// ============================================================================
// Painting
// ============================================================================

void NyanPanel::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    const auto style = Theme().Resolve(WidgetKind::Panel, 0, InteractionState::Normal);

    // Background color depends on elevation.
    const QColor bg = (_elevation == ShadowToken::shadow)
        ? style.background
        : Theme().Color(ColorToken::colorFillHover);

    // Shadow for non-flat elevations.
    if (_elevation != ShadowToken::shadow) {
        const auto& shadow = Theme().Shadow(_elevation);
        QColor shadowColor(0, 0, 0, shadow.opacity);
        const QRect shadowRect = rect().adjusted(
            shadow.offsetX, shadow.offsetY,
            shadow.offsetX + shadow.blurRadius,
            shadow.offsetY + shadow.blurRadius);
        p.setPen(Qt::NoPen);
        p.setBrush(shadowColor);
        p.drawRoundedRect(shadowRect, style.radiusPx + 1, style.radiusPx + 1);
    }

    // Panel background.
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(rect(), style.radiusPx, style.radiusPx);

    // Border.
    if (_borderVisible) {
        p.setPen(QPen(style.border, style.borderWidthPx));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), style.radiusPx, style.radiusPx);
    }
}

void NyanPanel::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
