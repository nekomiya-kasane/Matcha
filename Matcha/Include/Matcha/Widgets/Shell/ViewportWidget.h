#pragma once

/**
 * @file ViewportWidget.h
 * @brief Internal QWidget for viewport rendering surface + overlay.
 *
 * ViewportWidget creates a native window handle, forwards Qt events
 * (mouse, key, wheel, resize, show/hide, DPR) to IViewportRenderer
 * callbacks via InputEvent structs. An overlay QWidget sits above the
 * render surface for selection rubber-band, drag preview, etc.
 *
 * Internal to Matcha — not part of the public UiNode API.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/Types.h>
#include <Matcha/Services/IViewportRenderer.h>

#include <QWidget>

namespace matcha::fw {

/**
 * @brief QWidget subclass that hosts a native render surface.
 *
 * One ViewportWidget per Viewport UiNode. The widget:
 * - Forces native window handle creation (WA_NativeWindow)
 * - Forwards Qt events to bound IViewportRenderer
 * - Provides an overlay child widget for 2D annotations
 */
class MATCHA_EXPORT ViewportWidget : public QWidget {
    Q_OBJECT

public:
    explicit ViewportWidget(QWidget* parent = nullptr);
    ~ViewportWidget() override;

    ViewportWidget(const ViewportWidget&)            = delete;
    ViewportWidget& operator=(const ViewportWidget&) = delete;
    ViewportWidget(ViewportWidget&&)                 = delete;
    ViewportWidget& operator=(ViewportWidget&&)      = delete;

    // -- Renderer binding --

    /** @brief Bind a renderer. Calls OnAttach with native handle + size + dpr. */
    void SetRenderer(IViewportRenderer* renderer, ViewportId vpId);

    /** @brief Unbind the current renderer. Calls OnDetach. */
    void RemoveRenderer();

    /** @brief Get the native window handle (HWND on Windows). */
    [[nodiscard]] auto NativeHandle() -> void*;

    /** @brief Get the transparent overlay widget above the render surface. */
    [[nodiscard]] auto OverlayWidget() -> QWidget*;

    /** @brief Trigger OnRenderFrame on the bound renderer. */
    void RenderFrame();

    /** @brief Get the bound viewport ID. */
    [[nodiscard]] auto GetViewportId() const -> ViewportId { return _vpId; }

protected:
    // -- Qt event overrides --
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    void ForwardMouseEvent(QMouseEvent* event, InputEventType type);
    void ConnectDprSignal();

    IViewportRenderer* _renderer = nullptr;
    ViewportId         _vpId {};
    QWidget*           _overlay = nullptr;
};

} // namespace matcha::fw
