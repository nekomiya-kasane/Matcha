/**
 * @file NyanRangeSlider.cpp
 * @brief Implementation of NyanRangeSlider dual-handle range slider.
 */

#include <Matcha/Widgets/Controls/NyanRangeSlider.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

#include <algorithm>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanRangeSlider::NyanRangeSlider(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Slider)
{
    setFixedHeight(kCrossSize);
    setMouseTracking(true);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanRangeSlider::~NyanRangeSlider() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanRangeSlider::SetRange(int minimum, int maximum)
{
    _minimum = minimum;
    _maximum = std::max(minimum, maximum);  // NOLINT: std::max is intentional here
    _low  = std::clamp(_low, _minimum, _maximum);
    _high = std::clamp(_high, _minimum, _maximum);
    if (_low > _high) {
        _low = _high;
    }
    update();
}

void NyanRangeSlider::SetLow(int value)
{
    _low = std::clamp(value, _minimum, _high);
    update();
    emit RangeChanged(_low, _high);
}

auto NyanRangeSlider::Low() const -> int { return _low; }

void NyanRangeSlider::SetHigh(int value)
{
    _high = std::clamp(value, _low, _maximum);
    update();
    emit RangeChanged(_low, _high);
}

auto NyanRangeSlider::High() const -> int { return _high; }

void NyanRangeSlider::SetStep(int step)
{
    _step = std::max(1, step);
}

auto NyanRangeSlider::Step() const -> int { return _step; }
auto NyanRangeSlider::Minimum() const -> int { return _minimum; }
auto NyanRangeSlider::Maximum() const -> int { return _maximum; }

auto NyanRangeSlider::sizeHint() const -> QSize
{
    return {kDefaultLength, kCrossSize};
}

// ============================================================================
// Coordinate conversion
// ============================================================================

auto NyanRangeSlider::ValueToX(int val) const -> int
{
    const int halfH = kHandleSize / 2;
    const int trackLeft = halfH;
    const int trackRight = width() - halfH;
    const int range = _maximum - _minimum;
    if (range <= 0) {
        return trackLeft;
    }
    return trackLeft + static_cast<int>(
        static_cast<double>(val - _minimum) / range * (trackRight - trackLeft));
}

auto NyanRangeSlider::XToValue(int x) const -> int
{
    const int halfH = kHandleSize / 2;
    const int trackLeft = halfH;
    const int trackRight = width() - halfH;
    const int range = _maximum - _minimum;
    if (range <= 0 || trackRight <= trackLeft) {
        return _minimum;
    }
    const double ratio = static_cast<double>(x - trackLeft) / (trackRight - trackLeft);
    int val = _minimum + static_cast<int>(ratio * range);
    // Snap to step
    val = _minimum + ((((val - _minimum) + (_step / 2)) / _step) * _step);
    return std::clamp(val, _minimum, _maximum);
}

// ============================================================================
// Painting
// ============================================================================

void NyanRangeSlider::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                                     : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::Slider, 0, istate);
    p.setOpacity(style.opacity);

    const int halfH = kHandleSize / 2;
    const int trackY = (height() - kTrackHeight) / 2;
    const int trackLeft = halfH;
    const int trackRight = width() - halfH;
    const int trackW = trackRight - trackLeft;

    // -- Full track background --
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    p.drawRoundedRect(QRect(trackLeft, trackY, trackW, kTrackHeight), 2, 2);

    // -- Highlighted region between handles --
    const int lowX = ValueToX(_low);
    const int highX = ValueToX(_high);
    if (highX > lowX && isEnabled()) {
        p.setBrush(style.foreground);
        p.drawRoundedRect(QRect(lowX, trackY, highX - lowX, kTrackHeight), 2, 2);
    }

    // -- Draw handles --
    auto drawHandle = [&](int cx) {
        const int hx = cx - halfH;
        const int hy = (height() - kHandleSize) / 2;
        p.setPen(QPen(style.border, 1));
        p.setBrush(Theme().Color(ColorToken::SurfaceElevated));
        p.drawEllipse(hx, hy, kHandleSize, kHandleSize);
    };

    drawHandle(lowX);
    drawHandle(highX);
}

// ============================================================================
// Mouse interaction
// ============================================================================

void NyanRangeSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !isEnabled()) {
        event->ignore();
        return;
    }

    const int mx = event->position().toPoint().x();
    const int lowX = ValueToX(_low);
    const int highX = ValueToX(_high);
    const int distLow = std::abs(mx - lowX);
    const int distHigh = std::abs(mx - highX);

    // Drag whichever handle is closer
    if (distLow <= distHigh) {
        _dragging = DragHandle::Low;
    } else {
        _dragging = DragHandle::High;
    }
    event->accept();
}

void NyanRangeSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (_dragging == DragHandle::None) {
        return;
    }

    const int mx = event->position().toPoint().x();
    const int val = XToValue(mx);

    if (_dragging == DragHandle::Low) {
        const int newLow = std::min(val, _high);
        if (newLow != _low) {
            _low = newLow;
            update();
            emit RangeChanged(_low, _high);
        }
    } else {
        const int newHigh = std::max(val, _low);
        if (newHigh != _high) {
            _high = newHigh;
            update();
            emit RangeChanged(_low, _high);
        }
    }
    event->accept();
}

void NyanRangeSlider::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = DragHandle::None;
        event->accept();
    }
}

void NyanRangeSlider::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
