/**
 * @file ViewportWidget.cpp
 * @brief Implementation of ViewportWidget -- native render surface + overlay.
 */

#include <Matcha/Widgets/Shell/ViewportWidget.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include <chrono>

namespace matcha::fw {

namespace {

auto ToMouseButton(Qt::MouseButton btn) -> MouseButton
{
    switch (btn) {
    case Qt::LeftButton:   return MouseButton::Left;
    case Qt::RightButton:  return MouseButton::Right;
    case Qt::MiddleButton: return MouseButton::Middle;
    default:               return MouseButton::None;
    }
}

auto ToKeyModifiers(Qt::KeyboardModifiers mods) -> KeyModifier
{
    auto result = static_cast<uint8_t>(KeyModifier::None);
    if (mods & Qt::ShiftModifier)   { result |= static_cast<uint8_t>(KeyModifier::Shift); }
    if (mods & Qt::ControlModifier) { result |= static_cast<uint8_t>(KeyModifier::Control); }
    if (mods & Qt::AltModifier)     { result |= static_cast<uint8_t>(KeyModifier::Alt); }
    if (mods & Qt::MetaModifier)    { result |= static_cast<uint8_t>(KeyModifier::Meta); }
    return static_cast<KeyModifier>(result);
}

auto CurrentTimestampMs() -> uint64_t
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::steady_clock;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

} // anonymous namespace

// ============================================================================
// ViewportWidget
// ============================================================================

ViewportWidget::ViewportWidget(QWidget* parent)
    : QWidget(parent)
    , _overlay(new QWidget(this))
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    // Configure overlay: transparent, above render surface
    _overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    _overlay->setAttribute(Qt::WA_TranslucentBackground);
    _overlay->setGeometry(rect());
    _overlay->raise();
    _overlay->show();
}

ViewportWidget::~ViewportWidget()
{
    RemoveRenderer();
}

// ============================================================================
// Renderer binding
// ============================================================================

void ViewportWidget::SetRenderer(IViewportRenderer* renderer, ViewportId vpId)
{
    if (_renderer != nullptr) {
        _renderer->OnDetach(_vpId);
    }
    _renderer = renderer;
    _vpId = vpId;
    if (_renderer != nullptr) {
        auto dpr = devicePixelRatio();
        auto* handle = reinterpret_cast<void*>(winId()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
        (void)_renderer->OnAttach(_vpId, handle, width(), height(), dpr);
    }
}

void ViewportWidget::RemoveRenderer()
{
    if (_renderer != nullptr) {
        _renderer->OnDetach(_vpId);
        _renderer = nullptr;
    }
}

auto ViewportWidget::NativeHandle() -> void*
{
    return reinterpret_cast<void*>(winId()); // NOLINT(performance-no-int-to-ptr)
}

auto ViewportWidget::OverlayWidget() -> QWidget*
{
    return _overlay;
}

void ViewportWidget::RenderFrame()
{
    if (_renderer != nullptr && _renderer->IsReady()) {
        (void)_renderer->OnRenderFrame(_vpId);
    }
}

// ============================================================================
// Qt event overrides
// ============================================================================

void ViewportWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (_renderer != nullptr) {
        _renderer->OnVisibilityChanged(_vpId, true);
    }
}

void ViewportWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    if (_renderer != nullptr) {
        _renderer->OnVisibilityChanged(_vpId, false);
    }
}

void ViewportWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // Keep overlay matching viewport size
    if (_overlay != nullptr) {
        _overlay->setGeometry(rect());
    }

    if (_renderer != nullptr) {
        _renderer->OnResize(_vpId, width(), height(), devicePixelRatio());
    }
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    ForwardMouseEvent(event, InputEventType::MousePress);
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    ForwardMouseEvent(event, InputEventType::MouseRelease);
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    ForwardMouseEvent(event, InputEventType::MouseMove);
}

void ViewportWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    ForwardMouseEvent(event, InputEventType::MouseDoubleClick);
}

void ViewportWidget::wheelEvent(QWheelEvent* event)
{
    if (_renderer == nullptr) { return; }

    InputEvent evt {};
    evt.type = InputEventType::Wheel;
    evt.modifiers = ToKeyModifiers(event->modifiers());
    evt.x = event->position().x();
    evt.y = event->position().y();
    evt.globalX = event->globalPosition().x();
    evt.globalY = event->globalPosition().y();
    evt.wheelDelta = event->angleDelta().y() / 8.0;
    evt.timestamp = CurrentTimestampMs();

    _renderer->OnInputEvent(_vpId, evt);
}

void ViewportWidget::keyPressEvent(QKeyEvent* event)
{
    if (_renderer == nullptr) { return; }

    InputEvent evt {};
    evt.type = InputEventType::KeyPress;
    evt.modifiers = ToKeyModifiers(event->modifiers());
    evt.key = event->key();
    evt.timestamp = CurrentTimestampMs();

    _renderer->OnInputEvent(_vpId, evt);
}

void ViewportWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (_renderer == nullptr) { return; }

    InputEvent evt {};
    evt.type = InputEventType::KeyRelease;
    evt.modifiers = ToKeyModifiers(event->modifiers());
    evt.key = event->key();
    evt.timestamp = CurrentTimestampMs();

    _renderer->OnInputEvent(_vpId, evt);
}

// ============================================================================
// Internal helpers
// ============================================================================

void ViewportWidget::ForwardMouseEvent(QMouseEvent* event, InputEventType type)
{
    if (_renderer == nullptr) { return; }

    InputEvent evt {};
    evt.type = type;
    evt.button = ToMouseButton(event->button());
    evt.modifiers = ToKeyModifiers(event->modifiers());
    evt.x = event->position().x();
    evt.y = event->position().y();
    evt.globalX = event->globalPosition().x();
    evt.globalY = event->globalPosition().y();
    evt.timestamp = CurrentTimestampMs();

    _renderer->OnInputEvent(_vpId, evt);
}

void ViewportWidget::ConnectDprSignal()
{
    // DPR change is detected via resizeEvent -- Qt sends a resize when DPR changes.
    // No separate signal needed. This method is kept as a hook point for future use.
}

} // namespace matcha::fw
