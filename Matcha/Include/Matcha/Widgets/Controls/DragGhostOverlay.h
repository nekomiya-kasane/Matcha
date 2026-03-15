#pragma once

/**
 * @file DragGhostOverlay.h
 * @brief Transparent overlay widget that renders a drag ghost following the cursor.
 *
 * Reads DragPreviewConfig from DragDropVisualManager and renders:
 * - Ghost style: semi-transparent snapshot of the dragged element
 * - Icon style: small icon with label
 * - Compact style: compact card with label and badge count
 *
 * The overlay is a top-level frameless widget with Qt::ToolTip flags
 * (highest Z-order, no focus steal). It tracks the global cursor position
 * via a QTimer and repositions itself with the configured offset.
 *
 * Lifecycle: Created when drag starts, destroyed when drag ends.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Interaction/DragDropVisual.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

class QLabel;
class QTimer;

namespace matcha::gui {

/**
 * @brief Overlay widget rendering a drag ghost that follows the cursor.
 */
class MATCHA_EXPORT DragGhostOverlay : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit DragGhostOverlay(QWidget* parent = nullptr);
    ~DragGhostOverlay() override;

    DragGhostOverlay(const DragGhostOverlay&) = delete;
    auto operator=(const DragGhostOverlay&) -> DragGhostOverlay& = delete;
    DragGhostOverlay(DragGhostOverlay&&) = delete;
    auto operator=(DragGhostOverlay&&) -> DragGhostOverlay& = delete;

    /// @brief Configure the ghost appearance from a DragPreviewConfig.
    void SetConfig(const matcha::fw::DragPreviewConfig& config);

    /// @brief Start following the cursor. Call when drag begins.
    void StartFollowing();

    /// @brief Stop following and hide. Call when drag ends.
    void StopFollowing();

    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    void UpdatePosition();

    matcha::fw::DragPreviewConfig _config;
    QLabel* _label = nullptr;
    QLabel* _badge = nullptr;
    QTimer* _followTimer = nullptr;
};

} // namespace matcha::gui
