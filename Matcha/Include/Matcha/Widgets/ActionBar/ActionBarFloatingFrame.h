#pragma once

/**
 * @file ActionBarFloatingFrame.h
 * @brief Frameless top-level wrapper for undocked ActionBar.
 *
 * When the ActionBar is undocked, it is reparented into this frame.
 * The frame provides:
 * - Frameless window with shadow/border
 * - A small drag grip handle at the top for repositioning
 * - Resize support (resizing reflows toolbar buttons)
 * - Edge snap detection: when dragged near a container edge, emits a signal
 */

#include <Matcha/Foundation/Macros.h>

#include <QWidget>

class QVBoxLayout;

namespace matcha::gui {

class NyanActionBar;

/**
 * @brief Frameless floating wrapper for undocked ActionBar.
 */
class MATCHA_EXPORT ActionBarFloatingFrame : public QWidget {
    Q_OBJECT

public:
    explicit ActionBarFloatingFrame(QWidget* parent = nullptr);
    ~ActionBarFloatingFrame() override;

    ActionBarFloatingFrame(const ActionBarFloatingFrame&)            = delete;
    ActionBarFloatingFrame& operator=(const ActionBarFloatingFrame&) = delete;
    ActionBarFloatingFrame(ActionBarFloatingFrame&&)                 = delete;
    ActionBarFloatingFrame& operator=(ActionBarFloatingFrame&&)      = delete;

    /// @brief Set the ActionBar widget to host inside this frame.
    void SetActionBar(NyanActionBar* bar);

    /// @brief Get the hosted ActionBar (may be nullptr).
    [[nodiscard]] auto GetActionBar() const -> NyanActionBar*;

    /// @brief Remove the ActionBar from this frame (does NOT delete it).
    /// Returns the ActionBar pointer so caller can reparent it.
    auto TakeActionBar() -> NyanActionBar*;

    /// @brief Set the original container widget (for edge snap detection during drag).
    void SetDockTarget(QWidget* container);

    /// @brief Get the dock target container.
    [[nodiscard]] auto DockTarget() const -> QWidget*;

Q_SIGNALS:
    /// @brief Emitted when the frame is dragged close to the dock target edge.
    /// @param side The detected edge, or std::nullopt if center.
    void DragNearEdge(QPoint globalPos);

    /// @brief Emitted when the frame drag ends (mouse release on grip).
    void DragFinished(QPoint globalPos);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    static constexpr int kGripHeight = 8;
    static constexpr int kBorderRadius = 6;

    [[nodiscard]] auto IsInGripArea(QPoint pos) const -> bool;

    QVBoxLayout*    _layout    = nullptr;
    NyanActionBar*  _actionBar = nullptr;
    QWidget*        _dockTarget = nullptr;

    bool   _dragging     = false;
    QPoint _dragStartPos;
};

} // namespace matcha::gui
