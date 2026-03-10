#pragma once

/**
 * @file DropZoneOverlay.h
 * @brief Transparent overlay that paints drop zone hints during viewport drag.
 *
 * Shown on top of a ViewportFrame when a drag is active. Highlights the
 * zone (Top/Bottom/Left/Right/Center) under the cursor. Does not consume
 * mouse events -- purely visual.
 *
 * Internal to Matcha -- not part of the public UiNode API.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/UiNodes/Document/SplitTreeNode.h>

#include <QWidget>

namespace matcha::gui {

/**
 * @brief Transparent overlay painting T/B/L/R/Center drop zone highlights.
 *
 * The overlay divides its area into 5 zones:
 * - Center: inner 40% rectangle
 * - Top/Bottom/Left/Right: outer 30% strips
 *
 * Call `SetActiveZone()` to highlight a specific zone, or `ClearZone()`
 * to hide all highlights. The overlay is transparent to mouse events.
 */
class MATCHA_EXPORT DropZoneOverlay : public QWidget {
    Q_OBJECT

public:
    explicit DropZoneOverlay(QWidget* parent = nullptr);
    ~DropZoneOverlay() override;

    DropZoneOverlay(const DropZoneOverlay&)            = delete;
    DropZoneOverlay& operator=(const DropZoneOverlay&) = delete;
    DropZoneOverlay(DropZoneOverlay&&)                 = delete;
    DropZoneOverlay& operator=(DropZoneOverlay&&)      = delete;

    /** @brief Set which zone to highlight. */
    void SetActiveZone(fw::DropZone zone);

    /** @brief Clear all zone highlights. */
    void ClearZone();

    /** @brief Determine which zone a local point falls in. */
    [[nodiscard]] auto ZoneAtPoint(const QPoint& pos) const -> fw::DropZone;

    /** @brief Whether any zone is currently highlighted. */
    [[nodiscard]] auto HasActiveZone() const -> bool { return _hasZone; }

    /** @brief Get the currently active zone. */
    [[nodiscard]] auto ActiveZone() const -> fw::DropZone { return _zone; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    [[nodiscard]] auto ZoneRect(fw::DropZone zone) const -> QRect;

    fw::DropZone _zone    = fw::DropZone::Center;
    bool         _hasZone = false;
};

} // namespace matcha::gui
