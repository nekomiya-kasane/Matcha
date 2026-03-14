/**
 * @file NyanLegendSlider.cpp
 * @brief Implementation of NyanLegendSlider themed slider with color gradient bar.
 */

#include <Matcha/Widgets/Controls/NyanLegendSlider.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QLinearGradient>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

#include <algorithm>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanLegendSlider::NyanLegendSlider(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Legend)
{
    setFixedHeight(kTotalHeight);
    setMouseTracking(true);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanLegendSlider::~NyanLegendSlider() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanLegendSlider::SetRange(double lower, double upper)
{
    _lower = lower;
    _upper = std::max(lower, upper);
    _value = std::clamp(_value, _lower, _upper);
    update();
}

auto NyanLegendSlider::Lower() const -> double { return _lower; }
auto NyanLegendSlider::Upper() const -> double { return _upper; }

void NyanLegendSlider::SetValue(double value)
{
    _value = std::clamp(value, _lower, _upper);
    update();
    emit ValueChanged(_value);
}

auto NyanLegendSlider::Value() const -> double { return _value; }

void NyanLegendSlider::SetColorMap(const std::vector<ColorStop>& stops)
{
    _colorStops = stops;
    update();
}

auto NyanLegendSlider::ColorMap() const -> const std::vector<ColorStop>&
{
    return _colorStops;
}

auto NyanLegendSlider::sizeHint() const -> QSize
{
    return {kDefaultWidth, kTotalHeight};
}

auto NyanLegendSlider::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Coordinate conversion
// ============================================================================

auto NyanLegendSlider::ValueToX(double val) const -> int
{
    const int halfH = kHandleSize / 2;
    const int trackLeft = halfH;
    const int trackRight = width() - halfH;
    const double range = _upper - _lower;
    if (range <= 0.0) {
        return trackLeft;
    }
    return trackLeft + static_cast<int>(
        ((val - _lower) / range) * (trackRight - trackLeft));
}

auto NyanLegendSlider::XToValue(int x) const -> double
{
    const int halfH = kHandleSize / 2;
    const int trackLeft = halfH;
    const int trackRight = width() - halfH;
    const double range = _upper - _lower;
    if (range <= 0.0 || trackRight <= trackLeft) {
        return _lower;
    }
    const double ratio = static_cast<double>(x - trackLeft) / (trackRight - trackLeft);
    return std::clamp(_lower + (ratio * range), _lower, _upper);
}

// ============================================================================
// Painting
// ============================================================================

void NyanLegendSlider::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : underMouse() ? InteractionState::Hovered
                                     : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::Slider, 0, istate);
    p.setOpacity(style.opacity);

    const int halfH = kHandleSize / 2;
    const int barY = (height() - kBarHeight) / 2;
    const int barLeft = halfH;
    const int barRight = width() - halfH;
    const int barW = barRight - barLeft;

    // -- Gradient bar --
    if (!_colorStops.empty() && barW > 0) {
        QLinearGradient grad(barLeft, 0, barRight, 0);
        for (const auto& stop : _colorStops) {
            grad.setColorAt(std::clamp(stop.position, 0.0, 1.0), stop.color);
        }
        p.setPen(Qt::NoPen);
        p.setBrush(grad);
        p.drawRoundedRect(QRect(barLeft, barY, barW, kBarHeight), 2, 2);
    } else {
        // Fallback: plain bar from style
        p.setPen(Qt::NoPen);
        p.setBrush(style.background);
        p.drawRoundedRect(QRect(barLeft, barY, barW, kBarHeight), 2, 2);
    }

    // -- Handle --
    const int handleX = ValueToX(_value);
    const int handleY = (height() - kHandleSize) / 2;
    p.setPen(QPen(style.border, 1));
    p.setBrush(Theme().Color(ColorToken::SurfaceElevated));
    p.drawEllipse(handleX - halfH, handleY, kHandleSize, kHandleSize);
}

// ============================================================================
// Mouse interaction
// ============================================================================

void NyanLegendSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled()) {
        _dragging = true;
        const double val = XToValue(event->position().toPoint().x());
        if (val != _value) {
            _value = val;
            update();
            emit ValueChanged(_value);
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void NyanLegendSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (!_dragging) {
        return;
    }
    const double val = XToValue(event->position().toPoint().x());
    if (val != _value) {
        _value = val;
        update();
        emit ValueChanged(_value);
    }
    event->accept();
}

void NyanLegendSlider::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void NyanLegendSlider::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui