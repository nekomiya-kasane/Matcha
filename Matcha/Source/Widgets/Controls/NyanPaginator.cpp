/**
 * @file NyanPaginator.cpp
 * @brief Implementation of NyanPaginator themed page navigation widget.
 */

#include <Matcha/Widgets/Controls/NyanPaginator.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QMouseEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanPaginator::NyanPaginator(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Paginator)
{
    setFixedHeight(kHeight);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanPaginator::~NyanPaginator() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanPaginator::SetCount(int count)
{
    _count = count;
    if (_current >= _count) {
        _current = _count - 1;
    }
    if (_current < 0 && _count > 0) {
        _current = 0;
    }
    update();
}

auto NyanPaginator::Count() const -> int
{
    return _count;
}

void NyanPaginator::SetCurrent(int page)
{
    if (page < -1 || page >= _count) {
        return;
    }
    if (_current != page) {
        _current = page;
        emit PageChanged(_current);
        update();
    }
}

auto NyanPaginator::Current() const -> int
{
    return _current;
}

void NyanPaginator::SetResetButtonVisible(bool visible)
{
    _resetButtonVisible = visible;
    updateGeometry();
    update();
}

auto NyanPaginator::IsResetButtonVisible() const -> bool
{
    return _resetButtonVisible;
}

auto NyanPaginator::sizeHint() const -> QSize
{
    int w = kButtonSize + kSpacing + 60 + kSpacing + kButtonSize;  // prev + indicator + next
    if (_resetButtonVisible) {
        w += kSpacing + kButtonSize;
    }
    return {w, kHeight};
}

auto NyanPaginator::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanPaginator::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto style = Theme().Resolve(WidgetKind::Paginator, 0, InteractionState::Normal);
    const auto disStyle = Theme().Resolve(WidgetKind::Paginator, 0, InteractionState::Disabled);
    p.setFont(style.font);

    // Prev button.
    const QRect prevRect = PrevButtonRect();
    const bool canPrev = _current > 0;
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    p.drawRoundedRect(prevRect, style.radiusPx, style.radiusPx);
    p.setPen(canPrev ? style.foreground : disStyle.foreground);
    p.drawText(prevRect, Qt::AlignCenter, QStringLiteral("<"));

    // Next button.
    const QRect nextRect = NextButtonRect();
    const bool canNext = _current < _count - 1;
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    p.drawRoundedRect(nextRect, style.radiusPx, style.radiusPx);
    p.setPen(canNext ? style.foreground : disStyle.foreground);
    p.drawText(nextRect, Qt::AlignCenter, QStringLiteral(">"));

    // Page indicator.
    const QRect indRect = IndicatorRect();
    p.setPen(style.foreground);
    const QString text = _count > 0
        ? QStringLiteral("%1/%2").arg(_current + 1).arg(_count)
        : QStringLiteral("0/0");
    p.drawText(indRect, Qt::AlignCenter, text);

    // Reset button.
    if (_resetButtonVisible) {
        const QRect resetRect = ResetButtonRect();
        p.setPen(Qt::NoPen);
        p.setBrush(style.background);
        p.drawRoundedRect(resetRect, style.radiusPx, style.radiusPx);
        p.setPen(style.foreground);
        p.drawText(resetRect, Qt::AlignCenter, QStringLiteral("R"));
    }
}

// ============================================================================
// Mouse Events
// ============================================================================

void NyanPaginator::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint pos = event->pos();

    // Prev button.
    if (PrevButtonRect().contains(pos) && _current > 0) {
        SetCurrent(_current - 1);
        return;
    }

    // Next button.
    if (NextButtonRect().contains(pos) && _current < _count - 1) {
        SetCurrent(_current + 1);
        return;
    }

    // Reset button.
    if (_resetButtonVisible && ResetButtonRect().contains(pos)) {
        emit ResetClicked();
        return;
    }

    QWidget::mousePressEvent(event);
}

void NyanPaginator::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private Helpers
// ============================================================================

auto NyanPaginator::PrevButtonRect() const -> QRect
{
    return {0, (height() - kButtonSize) / 2, kButtonSize, kButtonSize};
}

auto NyanPaginator::NextButtonRect() const -> QRect
{
    const int x = kButtonSize + kSpacing + 60 + kSpacing;
    return {x, (height() - kButtonSize) / 2, kButtonSize, kButtonSize};
}

auto NyanPaginator::ResetButtonRect() const -> QRect
{
    if (!_resetButtonVisible) {
        return {};
    }
    const int x = kButtonSize + kSpacing + 60 + kSpacing + kButtonSize + kSpacing;
    return {x, (height() - kButtonSize) / 2, kButtonSize, kButtonSize};
}

auto NyanPaginator::IndicatorRect() const -> QRect
{
    const int x = kButtonSize + kSpacing;
    return {x, 0, 60, height()};
}

} // namespace matcha::gui