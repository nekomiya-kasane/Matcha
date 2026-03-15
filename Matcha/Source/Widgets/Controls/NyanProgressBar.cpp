/**
 * @file NyanProgressBar.cpp
 * @brief Implementation of NyanProgressBar themed linear progress bar.
 */

#include <Matcha/Widgets/Controls/NyanProgressBar.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanProgressBar::NyanProgressBar(QWidget* parent)
    : QProgressBar(parent)
    , ThemeAware(WidgetKind::ProgressBar)
{
    setFixedHeight(kBarHeight);
    setTextVisible(false);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanProgressBar::~NyanProgressBar() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanProgressBar::SetTextVisible(bool visible)
{
    _textVisible = visible;
    setFixedHeight(visible ? kBarHeightText : kBarHeight);
    QProgressBar::setTextVisible(visible);
    updateGeometry();
    update();
}

auto NyanProgressBar::IsTextVisible() const -> bool
{
    return _textVisible;
}

auto NyanProgressBar::sizeHint() const -> QSize
{
    const int h = _textVisible ? kBarHeightText : kBarHeight;
    return {kDefaultWidth, h};
}

auto NyanProgressBar::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanProgressBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                                     : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::ProgressBar, 0, istate);
    p.setOpacity(style.opacity);

    const QRect r = rect();
    const int radius = r.height() / 2; // Pill/round groove

    // -- Groove (background) --
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    p.drawRoundedRect(r, radius, radius);

    // -- Fill (progress) -- uses Selected state for active bar color
    const int range = maximum() - minimum();
    if (range > 0) {
        const double ratio = static_cast<double>(value() - minimum()) / range;
        const int fillW = static_cast<int>(r.width() * ratio);
        if (fillW > 0) {
            const QRect fillRect(r.x(), r.y(), fillW, r.height());
            const auto activeStyle = Theme().Resolve(WidgetKind::ProgressBar, 0, InteractionState::Selected);
            p.setBrush(activeStyle.background);
            p.drawRoundedRect(fillRect, radius, radius);
        }
    }

    // -- Text overlay --
    if (_textVisible) {
        p.setFont(style.font);
        p.setPen(style.foreground);
        const QString txt = QString::number(
            range > 0 ? static_cast<int>(100.0 * (value() - minimum()) / range) : 0)
            + QStringLiteral("%");
        p.drawText(r, Qt::AlignCenter, txt);
    }
}

void NyanProgressBar::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui