#pragma once

/**
 * @file IViewportRenderer.h
 * @brief Interface for external viewport renderers + InputEvent struct.
 *
 * External renderers (OpenGL, Vulkan, DirectX, software) implement this
 * interface and bind to a Viewport via `Viewport::BindRenderer()`.
 * The Viewport forwards lifecycle and input events to the bound renderer.
 *
 * @see 05_Greenfield_Plan.md ss 8.2.4 for the rendering integration spec.
 */

#include <Matcha/Core/ErrorCode.h>
#include <Matcha/Core/Macros.h>
#include <Matcha/Core/Types.h>

#include <cstdint>
#include <span>

namespace matcha::fw {

// ============================================================================
// InputEvent — unified input event forwarded from Qt
// ============================================================================

/** @brief Input event type classification. */
enum class InputEventType : uint8_t {
    MousePress,
    MouseRelease,
    MouseMove,
    MouseDoubleClick,
    Wheel,
    KeyPress,
    KeyRelease,
    TabletPress,
    TabletRelease,
    TabletMove,
};

/** @brief Mouse button flags. */
enum class MouseButton : uint8_t {
    None   = 0,
    Left   = 1U << 0U,
    Right  = 1U << 1U,
    Middle = 1U << 2U,
};

/** @brief Keyboard modifier flags. */
enum class KeyModifier : uint8_t {
    None    = 0,
    Shift   = 1U << 0U,
    Control = 1U << 1U,
    Alt     = 1U << 2U,
    Meta    = 1U << 3U,
};

/**
 * @brief Unified input event struct forwarded from Qt events.
 *
 * Covers mouse, keyboard, wheel, and tablet events.
 * Batched via `OnInputBatch(std::span<const InputEvent>)`.
 */
struct InputEvent {
    InputEventType type {};
    MouseButton    button {};
    KeyModifier    modifiers {};
    int            key = 0;         ///< Qt::Key value for keyboard events
    double         x = 0.0;         ///< Local X coordinate
    double         y = 0.0;         ///< Local Y coordinate
    double         globalX = 0.0;   ///< Global X coordinate
    double         globalY = 0.0;   ///< Global Y coordinate
    double         wheelDelta = 0.0;///< Wheel rotation in degrees
    double         pressure = 1.0;  ///< Tablet pressure (0.0 to 1.0)
    uint64_t       timestamp = 0;   ///< Event timestamp in milliseconds
};

// ============================================================================
// IViewportRenderer — external renderer interface
// ============================================================================

/**
 * @brief Interface for external renderers bound to a Viewport.
 *
 * **Lifecycle callbacks** (called by the Viewport):
 * - OnAttach: renderer is bound to a viewport (receive native handle)
 * - OnDetach: renderer is unbound
 * - OnRenderFrame: paint one frame (push model)
 * - OnResize: viewport surface resized
 * - OnDprChanged: device pixel ratio changed (DPI scaling)
 * - OnVisibilityChanged: viewport shown/hidden
 * - OnNativeHandleChanged: underlying window handle changed (rare)
 * - OnInputEvent: single input event
 * - OnInputBatch: batched input events (for high-frequency tablet input)
 *
 * **Thread safety**: All callbacks are called on the GUI thread.
 */
class MATCHA_EXPORT IViewportRenderer {
public:
    virtual ~IViewportRenderer() = default;

    IViewportRenderer(const IViewportRenderer&)            = default;
    IViewportRenderer& operator=(const IViewportRenderer&) = default;
    IViewportRenderer(IViewportRenderer&&)                 = default;
    IViewportRenderer& operator=(IViewportRenderer&&)      = default;

protected:
    IViewportRenderer() = default;

public:

    // -- Lifecycle --

    /** @brief Renderer is bound to a viewport surface. Create swapchain / render target. */
    virtual auto OnAttach(ViewportId vpId, void* nativeHandle,
                          int width, int height, double dpr) -> Expected<void> = 0;

    /** @brief Renderer is unbound. Release GPU resources. */
    virtual void OnDetach(ViewportId vpId) = 0;

    // -- Rendering --

    /** @brief Paint one frame. Called by Application::FlushDirtyViewports(). */
    virtual auto OnRenderFrame(ViewportId vpId) -> Expected<void> = 0;

    /** @brief Whether the renderer has completed initialization and can render. */
    [[nodiscard]] virtual auto IsReady() const -> bool = 0;

    // -- Surface changes --

    /** @brief Viewport surface resized. Rebuild swapchain if needed. */
    virtual void OnResize(ViewportId vpId, int width, int height, double dpr) = 0;

    /** @brief Device pixel ratio changed (DPI scaling). Independent of OnResize. */
    virtual void OnDprChanged(ViewportId vpId, double dpr) = 0;

    /** @brief Viewport shown or hidden. Pause/resume rendering. */
    virtual void OnVisibilityChanged(ViewportId vpId, bool visible) = 0;

    /** @brief Underlying native window handle changed (e.g. tab drag-out reparent). */
    virtual void OnNativeHandleChanged(ViewportId vpId, void* newHandle,
                                       int width, int height, double dpr) = 0;

    // -- Input --

    /** @brief Single input event forwarded from Qt. */
    virtual void OnInputEvent(ViewportId vpId, const InputEvent& event) = 0;

    /**
     * @brief Batched input events (for high-frequency tablet input).
     *
     * Default implementation delegates to OnInputEvent in a loop.
     * Override for custom batch processing.
     */
    virtual void OnInputBatch(ViewportId vpId, std::span<const InputEvent> events)
    {
        for (const auto& evt : events) {
            OnInputEvent(vpId, evt);
        }
    }
};

} // namespace matcha::fw
