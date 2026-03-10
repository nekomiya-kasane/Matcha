/**
 * @file Viewport.cpp
 * @brief Implementation of Viewport UiNode -- leaf node for renderer binding.
 */

#include <Matcha/UiNodes/Document/Viewport.h>

#include <Matcha/Services/IViewportRenderer.h>
#include <Matcha/UiNodes/Core/UiNodeNotification.h>
#include <Matcha/Widgets/Shell/ViewportWidget.h>

#include <QEvent>

namespace matcha::fw {

// ============================================================================
// ViewportFocusFilter -- event filter for focus in/out
// ============================================================================

class ViewportFocusFilter : public QObject {
public:
    ViewportFocusFilter(Viewport* vp, QObject* parent)
        : QObject(parent), _vp(vp) {}

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override
    {
        if (event->type() == QEvent::FocusIn) {
            _vp->OnWidgetFocusChanged(true);
        } else if (event->type() == QEvent::FocusOut) {
            _vp->OnWidgetFocusChanged(false);
        }
        return QObject::eventFilter(obj, event);
    }

private:
    Viewport* _vp;
};

Viewport::Viewport(std::string name, ViewportId vpId)
    : UiNode(std::string("viewport-") + std::to_string(vpId.value), NodeType::Viewport, std::move(name))
    , _vpId(vpId)
{
}

Viewport::~Viewport()
{
    if (_renderer != nullptr) {
        _renderer->OnDetach(_vpId);
        _renderer = nullptr;
    }
    // _widget is owned by Qt parent hierarchy -- do not delete here
}

// ============================================================================
// Renderer binding
// ============================================================================

void Viewport::BindRenderer(IViewportRenderer* renderer)
{
    if (_renderer != nullptr) {
        _renderer->OnDetach(_vpId);
    }
    _renderer = renderer;
    if (_renderer != nullptr && _widget != nullptr) {
        _widget->SetRenderer(_renderer, _vpId);
    } else if (_renderer != nullptr) {
        // No widget yet -- OnAttach with placeholder (will re-attach when CreateWidget is called)
        (void)_renderer->OnAttach(_vpId, nullptr, 0, 0, 1.0);
    }
}

void Viewport::UnbindRenderer()
{
    if (_widget != nullptr) {
        _widget->RemoveRenderer();
    }
    if (_renderer != nullptr) {
        _renderer->OnDetach(_vpId);
        _renderer = nullptr;
    }
}

void Viewport::RequestFrame()
{
    _dirty.store(true, std::memory_order_release);
}

// ============================================================================
// Dirty flag
// ============================================================================

auto Viewport::IsDirty() const -> bool
{
    return _dirty.load(std::memory_order_acquire);
}

void Viewport::ClearDirty()
{
    _dirty.store(false, std::memory_order_release);
}

// ============================================================================
// Widget
// ============================================================================

void Viewport::CreateWidget(QWidget* parent)
{
    if (_widget != nullptr) { return; }

    _widget = new ViewportWidget(parent); // NOLINT(cppcoreguidelines-owning-memory) -- Qt parent owns
    _widget->setFocusPolicy(Qt::StrongFocus);

    // Connect focus change: ViewportWidget gains/loses focus -> fire callbacks
    QObject::connect(_widget, &QWidget::destroyed, _widget, [this]() {
        _widget = nullptr;
    });

    // Install event filter for focus tracking
    _widget->installEventFilter(new class ViewportFocusFilter(this, _widget));

    // If renderer already bound, wire it to the widget
    if (_renderer != nullptr) {
        _widget->SetRenderer(_renderer, _vpId);
    }
}

auto Viewport::GetWidget() -> ViewportWidget*
{
    return _widget;
}

// ============================================================================
// Focus
// ============================================================================

auto Viewport::IsFocused() const -> bool
{
    if (_widget != nullptr) {
        return _widget->hasFocus();
    }
    return false;
}

void Viewport::OnWidgetFocusChanged(bool focused)
{
    VpFocusChanged notif(_vpId, focused);
    SendNotification(this, notif);
}

} // namespace matcha::fw
