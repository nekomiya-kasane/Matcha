/**
 * @file NyanPanelProgress.cpp
 * @brief Implementation of NyanPanelProgress panel with progress bar.
 */

#include <Matcha/Widgets/ActionBar/NyanPanelProgress.h>

#include <QPaintEvent>
#include <QPainter>

#include <algorithm>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanPanelProgress::NyanPanelProgress(QWidget* parent)
    : NyanPanel(parent)
{
}

NyanPanelProgress::~NyanPanelProgress() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanPanelProgress::SetProgress(int minimum, int maximum, int value)
{
    _minimum = minimum;
    _maximum = maximum;
    _value   = std::clamp(value, minimum, maximum);
    update();
}

void NyanPanelProgress::SetMinimum(int minimum)
{
    _minimum = minimum;
    _value   = std::clamp(_value, _minimum, _maximum);
    update();
}

void NyanPanelProgress::SetMaximum(int maximum)
{
    _maximum = maximum;
    _value   = std::clamp(_value, _minimum, _maximum);
    update();
}

void NyanPanelProgress::SetValue(int value)
{
    _value = std::clamp(value, _minimum, _maximum);
    update();
}

auto NyanPanelProgress::Value() const -> int { return _value; }
auto NyanPanelProgress::Maximum() const -> int { return _maximum; }
auto NyanPanelProgress::Minimum() const -> int { return _minimum; }

// ============================================================================
// Painting
// ============================================================================

void NyanPanelProgress::paintEvent(QPaintEvent* event)
{
    // Paint the panel base first.
    NyanPanel::paintEvent(event);

    // Paint progress bar at top edge.
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    const auto style = Theme().Resolve(WidgetKind::ProgressBar, 0, InteractionState::Normal);
    const auto activeStyle = Theme().Resolve(WidgetKind::ProgressBar, 0, InteractionState::Selected);
    const int range = _maximum - _minimum;

    // Track.
    const QRect trackRect(0, 0, width(), kBarHeight);
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    p.drawRoundedRect(trackRect, style.radiusPx, style.radiusPx);

    // Fill.
    if (range > 0) {
        const double ratio = static_cast<double>(_value - _minimum) / range;
        const int fillW = static_cast<int>(width() * ratio);
        if (fillW > 0) {
            const QRect fillRect(0, 0, fillW, kBarHeight);
            p.setBrush(activeStyle.background);
            p.drawRoundedRect(fillRect, style.radiusPx, style.radiusPx);
        }
    }
}

} // namespace matcha::gui
