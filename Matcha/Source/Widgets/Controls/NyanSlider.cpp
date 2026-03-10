/**
 * @file NyanSlider.cpp
 * @brief Implementation of NyanSlider themed single-handle slider.
 */

#include <Matcha/Widgets/Controls/NyanSlider.h>

#include "../Core/InteractionEventFilter.h"

#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanSlider::NyanSlider(QWidget* parent)
    : QSlider(Qt::Horizontal, parent)
    , ThemeAware(WidgetKind::Slider)
{
    setFixedHeight(kCrossSize);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanSlider::NyanSlider(Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent)
    , ThemeAware(WidgetKind::Slider)
{
    if (orientation == Qt::Horizontal) {
        setFixedHeight(kCrossSize);
    } else {
        setFixedWidth(kCrossSize);
    }
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanSlider::~NyanSlider() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanSlider::SetRange(int minimum, int maximum)
{
    setRange(minimum, maximum);
}

void NyanSlider::SetStep(int step)
{
    setSingleStep(step);
}

void NyanSlider::SetTicksVisible(bool visible)
{
    _ticksVisible = visible;
    update();
}

auto NyanSlider::TicksVisible() const -> bool
{
    return _ticksVisible;
}

auto NyanSlider::sizeHint() const -> QSize
{
    if (orientation() == Qt::Horizontal) {
        return {kDefaultLength, kCrossSize};
    }
    return {kCrossSize, kDefaultLength};
}

// ============================================================================
// Painting
// ============================================================================

void NyanSlider::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : underMouse() ? InteractionState::Hovered
                                     : InteractionState::Normal;

    // style.background = track, style.foreground = thumb accent, style.border = handle border
    const auto style = Theme().Resolve(WidgetKind::Slider, 0, istate);
    p.setOpacity(style.opacity);

    const bool horiz = (orientation() == Qt::Horizontal);
    const int range = maximum() - minimum();
    const double ratio = (range > 0)
        ? static_cast<double>(value() - minimum()) / range
        : 0.0;

    // -- Track --
    const int halfHandle = kHandleSize / 2;
    if (horiz) {
        const int trackY = (height() - kTrackHeight) / 2;
        const int trackLeft = halfHandle;
        const int trackRight = width() - halfHandle;
        const int trackW = trackRight - trackLeft;
        const int filledW = static_cast<int>(trackW * ratio);

        // Unfilled track
        p.setPen(Qt::NoPen);
        p.setBrush(style.background);
        p.drawRoundedRect(QRect(trackLeft, trackY, trackW, kTrackHeight), 2, 2);

        // Filled track
        if (filledW > 0 && isEnabled()) {
            p.setBrush(style.foreground);
            p.drawRoundedRect(QRect(trackLeft, trackY, filledW, kTrackHeight), 2, 2);
        }

        // -- Handle --
        const int handleX = trackLeft + filledW - halfHandle;
        const int handleY = (height() - kHandleSize) / 2;
        p.setPen(QPen(style.border, 1));
        p.setBrush(Theme().Color(ColorToken::SurfaceElevated));
        p.drawEllipse(handleX, handleY, kHandleSize, kHandleSize);

        // -- Tick marks --
        if (_ticksVisible && range > 0) {
            const int step = singleStep();
            p.setPen(QPen(style.border, 1));
            for (int v = minimum(); v <= maximum(); v += step) {
                const double r2 = static_cast<double>(v - minimum()) / range;
                const int x = trackLeft + static_cast<int>(trackW * r2);
                p.drawLine(x, trackY + kTrackHeight + 2, x, trackY + kTrackHeight + 2 + kTickHeight);
            }
        }
    } else {
        // Vertical
        const int trackX = (width() - kTrackHeight) / 2;
        const int trackTop = halfHandle;
        const int trackBottom = height() - halfHandle;
        const int trackH = trackBottom - trackTop;
        const int filledH = static_cast<int>(trackH * ratio);

        // Unfilled track
        p.setPen(Qt::NoPen);
        p.setBrush(style.background);
        p.drawRoundedRect(QRect(trackX, trackTop, kTrackHeight, trackH), 2, 2);

        // Filled track (from bottom)
        if (filledH > 0 && isEnabled()) {
            p.setBrush(style.foreground);
            p.drawRoundedRect(QRect(trackX, trackBottom - filledH, kTrackHeight, filledH), 2, 2);
        }

        // -- Handle --
        const int handleX = (width() - kHandleSize) / 2;
        const int handleY = trackBottom - filledH - halfHandle;
        p.setPen(QPen(style.border, 1));
        p.setBrush(Theme().Color(ColorToken::SurfaceElevated));
        p.drawEllipse(handleX, handleY, kHandleSize, kHandleSize);
    }
}

void NyanSlider::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
