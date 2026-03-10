#pragma once

/**
 * @file LayoutBoundsOverlay.h
 * @brief Transparent overlay that paints red bounds around all visible widgets.
 *
 * Non-intrusive: uses WA_TransparentForMouseEvents so it never intercepts
 * user interaction. Paints on a separate top-level widget that tracks
 * the target window's geometry.
 */

#include <QWidget>

class QTimer;

namespace nyancad {

/**
 * @brief Transparent overlay painting layout bounds (Android-style).
 *
 * Call SetTargetWindow() then SetVisible(true) to activate.
 * Internally uses a QTimer to periodically repaint at ~15 fps.
 */
class LayoutBoundsOverlay : public QWidget {
    Q_OBJECT

public:
    explicit LayoutBoundsOverlay(QWidget* parent = nullptr);
    ~LayoutBoundsOverlay() override;

    /// @brief Set the window whose widgets will be outlined.
    void SetTargetWindow(QWidget* target);

    /// @brief Toggle overlay visibility.
    void SetOverlayVisible(bool visible);

    /// @brief Check if overlay is active.
    [[nodiscard]] auto IsOverlayVisible() const -> bool;

    /// @brief Set the widget currently hovered by the picker (highlighted in blue).
    void SetPickedWidget(QWidget* widget);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void Reposition();

    QWidget* _target = nullptr;
    QWidget* _picked = nullptr;
    QTimer*  _refreshTimer = nullptr;
};

} // namespace nyancad
