#pragma once

/**
 * @file Viewport.h
 * @brief Viewport UiNode -- leaf node for renderer binding.
 *
 * A Viewport is a leaf in the ViewportGroup binary split tree.
 * It hosts a QWidget surface (QRhi or QOpenGLWidget) and forwards
 * input events to a bound IViewportRenderer.
 *
 * @see 05_Greenfield_Plan.md ss 4.2 for the three-layer model.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/Types.h>
#include <Matcha/UiNodes/Core/UiNode.h>

#include <atomic>

class QWidget;

namespace matcha::fw {

class IViewportRenderer;
class ViewportWidget;

/**
 * @brief UiNode representing a single renderable viewport surface.
 *
 * **Renderer binding**: External renderers implement IViewportRenderer
 * and bind via `BindRenderer()`. The viewport forwards lifecycle events
 * (attach, detach, resize, DPR change, input) to the bound renderer.
 *
 * **Identity**: `ViewportId` (unique within the application).
 * **Ownership**: ViewportGroup owns Viewport via the UiNode tree.
 */
class MATCHA_EXPORT Viewport : public UiNode {
public:
    explicit Viewport(std::string name, ViewportId vpId);
    ~Viewport() override;

    Viewport(const Viewport&)            = delete;
    Viewport& operator=(const Viewport&) = delete;
    Viewport(Viewport&&)                 = delete;
    Viewport& operator=(Viewport&&)      = delete;

    // -- Identity --

    [[nodiscard]] auto GetViewportId() const -> ViewportId { return _vpId; }

    // -- Renderer binding --

    /** @brief Bind an external renderer. Calls OnAttach on the renderer. */
    void BindRenderer(IViewportRenderer* renderer);

    /** @brief Unbind the current renderer. Calls OnDetach. */
    void UnbindRenderer();

    /** @brief Request a frame redraw (marks viewport dirty). Thread-safe. */
    void RequestFrame();

    // -- Dirty flag (used by Application::FlushDirtyViewports) --

    /** @brief Check if this viewport has a pending render request. */
    [[nodiscard]] auto IsDirty() const -> bool;

    /** @brief Clear the dirty flag after rendering. */
    void ClearDirty();

    // -- Widget --

    /** @brief Get the underlying ViewportWidget. May be nullptr before CreateWidget. */
    [[nodiscard]] auto GetWidget() -> ViewportWidget*;

    // -- Focus --

    [[nodiscard]] auto IsFocused() const -> bool;

private:
    friend class ViewportGroup;
    friend class ViewportFocusFilter;

    /** @brief Create the underlying ViewportWidget. @internal Called by ViewportGroup. */
    void CreateWidget(QWidget* parent);
    void OnWidgetFocusChanged(bool focused);

    ViewportId                         _vpId;
    IViewportRenderer*                 _renderer = nullptr;
    ViewportWidget*                    _widget = nullptr;
    std::atomic<bool>                  _dirty {false};
};

} // namespace matcha::fw
