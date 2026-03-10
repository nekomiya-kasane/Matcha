/**
 * @file NyanScrollArea.cpp
 * @brief Implementation of NyanScrollArea themed scroll container.
 */

#include <Matcha/Widgets/Shell/NyanScrollArea.h>
#include <Matcha/Widgets/Shell/NyanScrollBar.h>

#include <QWheelEvent>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanScrollArea::NyanScrollArea(QWidget* parent)
    : QScrollArea(parent)
    , ThemeAware(WidgetKind::ScrollArea)
{
    // Replace default scrollbars with themed NyanScrollBar instances.
    _vBar = new NyanScrollBar( Qt::Vertical, this);
    _hBar = new NyanScrollBar( Qt::Horizontal, this);

    setVerticalScrollBar(_vBar);
    setHorizontalScrollBar(_hBar);

    // Transparent background by default -- content determines appearance.
    setFrameShape(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
}

NyanScrollArea::~NyanScrollArea() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanScrollArea::SetScrollBarPolicy(Qt::ScrollBarPolicy horizontal,
                                        Qt::ScrollBarPolicy vertical)
{
    setHorizontalScrollBarPolicy(horizontal);
    setVerticalScrollBarPolicy(vertical);
}

void NyanScrollArea::SetWheelStep(int pixels)
{
    _wheelStep = pixels;
}

auto NyanScrollArea::WheelStep() const -> int
{
    return _wheelStep;
}

auto NyanScrollArea::VerticalBar() const -> NyanScrollBar*
{
    return _vBar;
}

auto NyanScrollArea::HorizontalBar() const -> NyanScrollBar*
{
    return _hBar;
}

auto NyanScrollArea::sizeHint() const -> QSize
{
    return {kDefaultSize, kDefaultSize};
}

// ============================================================================
// Events
// ============================================================================

void NyanScrollArea::wheelEvent(QWheelEvent* event)
{
    // Use configurable wheel step instead of default.
    const QPoint angleDelta = event->angleDelta();
    if (angleDelta.isNull()) {
        QScrollArea::wheelEvent(event);
        return;
    }

    // Standard wheel: angleDelta.y() is +-120 per notch.
    // Scale to our pixel step: each notch = _wheelStep pixels.
    const int notches = angleDelta.y() / 120;
    if (notches != 0) {
        QScrollBar* bar = verticalScrollBar();
        if (bar != nullptr) {
            bar->setValue(bar->value() - (notches * _wheelStep));
        }
    }

    // Horizontal wheel.
    const int hNotches = angleDelta.x() / 120;
    if (hNotches != 0) {
        QScrollBar* bar = horizontalScrollBar();
        if (bar != nullptr) {
            bar->setValue(bar->value() - (hNotches * _wheelStep));
        }
    }

    event->accept();
}

void NyanScrollArea::OnThemeChanged()
{
    // NyanScrollBars handle their own theme updates via ThemeAware.
    // Just ensure viewport repaints.
    if (viewport() != nullptr) {
        viewport()->update();
    }
}

} // namespace matcha::gui
