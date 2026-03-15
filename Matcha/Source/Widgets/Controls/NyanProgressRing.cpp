/**
 * @file NyanProgressRing.cpp
 * @brief Implementation of NyanProgressRing themed circular progress indicator.
 */

#include <Matcha/Widgets/Controls/NyanProgressRing.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>

#include <cmath>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanProgressRing::NyanProgressRing(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::ProgressBar)
{
    setFixedSize(kDefaultSize, kDefaultSize);

    connect(&_spinTimer, &QTimer::timeout, this, [this] {
        _spinAngle = (_spinAngle + kSpinStep) % 360;
        update();
    });
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanProgressRing::~NyanProgressRing() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanProgressRing::SetValue(int value)
{
    _value = std::clamp(value, _minimum, _maximum);
    update();
}

auto NyanProgressRing::Value() const -> int { return _value; }

void NyanProgressRing::SetRange(int minimum, int maximum)
{
    _minimum = minimum;
    _maximum = std::max(minimum, maximum);
    _value = std::clamp(_value, _minimum, _maximum);
    update();
}

auto NyanProgressRing::Minimum() const -> int { return _minimum; }
auto NyanProgressRing::Maximum() const -> int { return _maximum; }

void NyanProgressRing::SetIndeterminate(bool indeterminate)
{
    _indeterminate = indeterminate;
    if (_indeterminate) {
        const int intervalMs = Theme().AnimationMs(AnimationToken::Quick);
        _spinTimer.start(intervalMs > 0 ? (intervalMs / 10) : 16);
    } else {
        _spinTimer.stop();
        _spinAngle = 0;
    }
    update();
}

auto NyanProgressRing::IsIndeterminate() const -> bool { return _indeterminate; }

void NyanProgressRing::SetThickness(int thickness)
{
    _thickness = std::max(1, thickness);
    update();
}

auto NyanProgressRing::Thickness() const -> int { return _thickness; }

void NyanProgressRing::SetTextVisible(bool visible)
{
    _textVisible = visible;
    update();
}

auto NyanProgressRing::IsTextVisible() const -> bool { return _textVisible; }

auto NyanProgressRing::sizeHint() const -> QSize
{
    return {kDefaultSize, kDefaultSize};
}

// ============================================================================
// Painting
// ============================================================================

void NyanProgressRing::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                                     : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::ProgressBar, 0, istate);
    const auto activeStyle = Theme().Resolve(WidgetKind::ProgressBar, 0, InteractionState::Selected);
    p.setOpacity(style.opacity);

    // Draw rect with inset (matches old 4px inset)
    QRect drawRect = rect().adjusted(kInset, kInset, -kInset, -kInset);

    const int penWidth = _thickness;
    const QPen trackPen(style.background, penWidth, Qt::SolidLine, Qt::RoundCap);
    const QPen arcPen(activeStyle.background, penWidth, Qt::SolidLine, Qt::RoundCap);

    // -- Track ring (full circle) --
    p.setPen(trackPen);
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(drawRect);

    // -- Progress arc or indeterminate spinner --
    p.setPen(arcPen);

    if (_indeterminate) {
        // Spinning arc segment
        const int startAngle = _spinAngle * 16; // QPainter uses 1/16th degrees
        const int spanAngle = kSpinArcSpan * 16;
        p.drawArc(drawRect, startAngle, spanAngle);
    } else {
        // Determinate arc
        const int range = _maximum - _minimum;
        if (range > 0 && _value > _minimum) {
            const double ratio = static_cast<double>(_value - _minimum) / range;
            const int spanDegrees = static_cast<int>(std::floor(360.0 * ratio));
            // Qt arc: start at 12 o'clock (90 degrees), sweep clockwise (negative)
            const int startAngle = 90 * 16;
            const int spanAngle = -spanDegrees * 16;
            p.drawArc(drawRect, startAngle, spanAngle);
        }
    }

    // -- Text overlay --
    if (_textVisible && !_indeterminate) {
        p.setFont(style.font);
        p.setPen(style.foreground);

        const int range = _maximum - _minimum;
        const int pct = (range > 0) ? static_cast<int>(100.0 * (_value - _minimum) / range) : 0;
        p.drawText(rect(), Qt::AlignCenter, QString::number(pct) + QStringLiteral("%"));
    }
}

void NyanProgressRing::OnThemeChanged()
{
    if (_indeterminate) {
        const int intervalMs = Theme().AnimationMs(AnimationToken::Quick);
        _spinTimer.setInterval(intervalMs > 0 ? (intervalMs / 10) : 16);
    }
    update();
}

} // namespace matcha::gui