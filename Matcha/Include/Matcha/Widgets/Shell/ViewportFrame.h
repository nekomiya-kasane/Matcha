#pragma once

/**
 * @file ViewportFrame.h
 * @brief Composite widget wrapping ViewportHeaderBar + ViewportWidget vertically.
 *
 * Each leaf in the splitter tree is a ViewportFrame containing:
 * - ViewportHeaderBar (top, 24px): label, drag handle, close button
 * - ViewportWidget (fill remaining): native render surface
 * - DropZoneOverlay (stacked on top, transparent): drop zone hints during drag
 *
 * Internal to Matcha — not part of the public UiNode API.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/Types.h>

#include <QWidget>

namespace matcha::fw {
class ViewportWidget;
} // namespace matcha::fw

namespace matcha::gui {

class DropZoneOverlay;
class ViewportHeaderBar;

/**
 * @brief Composite widget: header bar + viewport widget + drop zone overlay.
 *
 * Layout:
 * ```
 * ┌──────────────────────────────┐
 * │ ViewportHeaderBar (24px)     │  <- drag handle, label, close [X]
 * ├──────────────────────────────┤
 * │                              │
 * │ ViewportWidget (fill)        │  <- native render surface
 * │                              │
 * └──────────────────────────────┘
 * [DropZoneOverlay stacked on top, transparent until drag active]
 * ```
 */
class MATCHA_EXPORT ViewportFrame : public QWidget {
    Q_OBJECT

public:
    explicit ViewportFrame(fw::ViewportId vpId, fw::ViewportWidget* vpWidget,
                           QWidget* parent = nullptr);
    ~ViewportFrame() override;

    ViewportFrame(const ViewportFrame&)            = delete;
    ViewportFrame& operator=(const ViewportFrame&) = delete;
    ViewportFrame(ViewportFrame&&)                 = delete;
    ViewportFrame& operator=(ViewportFrame&&)      = delete;

    /** @brief Get the viewport ID. */
    [[nodiscard]] auto GetViewportId() const -> fw::ViewportId { return _vpId; }

    /** @brief Get the header bar. */
    [[nodiscard]] auto HeaderBar() -> ViewportHeaderBar* { return _header; }

    /** @brief Get the drop zone overlay. */
    [[nodiscard]] auto Overlay() -> DropZoneOverlay* { return _overlay; }

    /** @brief Get the inner viewport widget. */
    [[nodiscard]] auto InnerWidget() -> fw::ViewportWidget* { return _vpWidget; }

    /** @brief Set the header label text. */
    void SetLabel(const QString& text);

    /** @brief Show the drop zone overlay (during drag). */
    void ShowOverlay();

    /** @brief Hide the drop zone overlay. */
    void HideOverlay();

signals:
    void dragStarted(matcha::fw::ViewportId vpId);
    void dragEnded(matcha::fw::ViewportId vpId, bool dropAccepted);
    void closeRequested(matcha::fw::ViewportId vpId);
    void maximizeToggled(matcha::fw::ViewportId vpId);
    void viewportDropped(matcha::fw::ViewportId sourceId,
                         matcha::fw::ViewportId targetId,
                         int dropZone);
    void splitHRequested(matcha::fw::ViewportId vpId);
    void splitVRequested(matcha::fw::ViewportId vpId);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    fw::ViewportId        _vpId;
    ViewportHeaderBar*    _header   = nullptr;
    fw::ViewportWidget*   _vpWidget = nullptr;
    DropZoneOverlay*      _overlay  = nullptr;
};

} // namespace matcha::gui
