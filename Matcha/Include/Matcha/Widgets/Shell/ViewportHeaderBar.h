#pragma once

/**
 * @file ViewportHeaderBar.h
 * @brief Thin header strip above each viewport for drag initiation and controls.
 *
 * Displays viewport name/camera label. Mouse press + 4px drag threshold
 * initiates a QDrag for viewport rearrangement. Close button (X) on the right.
 * Double-click toggles maximize/restore.
 *
 * Internal to Matcha — not part of the public UiNode API.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/Types.h>

#include <QWidget>

namespace matcha::gui {

/**
 * @brief Thin header bar widget above each viewport.
 *
 * Responsibilities:
 * - Display viewport label (left-aligned)
 * - Close button (right-aligned, visible on hover)
 * - Drag initiation on mousePress + 4px threshold -> QDrag
 * - Double-click -> maximize/restore signal
 *
 * Signals emitted via function callbacks (no Q_SIGNAL for simplicity):
 * - dragStarted(ViewportId)
 * - closeRequested(ViewportId)
 * - maximizeToggled(ViewportId)
 */
class MATCHA_EXPORT ViewportHeaderBar : public QWidget {
    Q_OBJECT

public:
    explicit ViewportHeaderBar(fw::ViewportId vpId, QWidget* parent = nullptr);
    ~ViewportHeaderBar() override;

    ViewportHeaderBar(const ViewportHeaderBar&)            = delete;
    ViewportHeaderBar& operator=(const ViewportHeaderBar&) = delete;
    ViewportHeaderBar(ViewportHeaderBar&&)                 = delete;
    ViewportHeaderBar& operator=(ViewportHeaderBar&&)      = delete;

    /** @brief Get the viewport ID this header belongs to. */
    [[nodiscard]] auto GetViewportId() const -> fw::ViewportId { return _vpId; }

    /** @brief Set the display label text. */
    void SetLabel(const QString& text);

    /** @brief Get the display label text. */
    [[nodiscard]] auto Label() const -> QString;

    /** @brief Show or hide the close button. */
    void SetCloseButtonVisible(bool visible);

    /** @brief Set ghost appearance (during drag). */
    void SetGhostMode(bool ghost);

signals:
    void dragStarted(matcha::fw::ViewportId vpId);
    void dragEnded(matcha::fw::ViewportId vpId, bool dropAccepted);
    void closeRequested(matcha::fw::ViewportId vpId);
    void maximizeToggled(matcha::fw::ViewportId vpId);
    void splitHRequested(matcha::fw::ViewportId vpId);
    void splitVRequested(matcha::fw::ViewportId vpId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    static constexpr int kHeight        = 24;
    static constexpr int kDragThreshold = 4;
    static constexpr int kCloseButtonSize = 16;

    [[nodiscard]] auto CloseButtonRect() const -> QRect;
    [[nodiscard]] auto SplitHButtonRect() const -> QRect;
    [[nodiscard]] auto SplitVButtonRect() const -> QRect;

    fw::ViewportId _vpId;
    QString        _label;
    QPoint         _dragStartPos;
    bool           _dragging       = false;
    bool           _hovered        = false;
    bool           _ghostMode      = false;
    bool           _closeVisible   = true;
    bool           _closeHovered   = false;
    bool           _splitHHovered  = false;
    bool           _splitVHovered  = false;
};

} // namespace matcha::gui
