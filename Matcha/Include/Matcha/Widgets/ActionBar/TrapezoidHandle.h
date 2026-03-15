#pragma once

/**
 * @file TrapezoidHandle.h
 * @brief Custom-painted trapezoid button for ActionBar docked-collapsed state.
 *
 * When the ActionBar is docked to a WorkspaceFrame edge and collapsed,
 * the entire ActionBar is hidden and only this trapezoid handle remains
 * visible, centered on the docked edge. Clicking it expands the ActionBar.
 *
 * The trapezoid orientation follows the dock side:
 * - Bottom-docked: wider edge at bottom, narrows upward
 * - Top-docked: wider edge at top, narrows downward
 * - Left-docked: wider edge at left, narrows rightward
 * - Right-docked: wider edge at right, narrows leftward
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Widgets/ActionBar/NyanActionBar.h>  // DockSide
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

/**
 * @brief Trapezoid-shaped expand handle for docked+collapsed ActionBar.
 */
class MATCHA_EXPORT TrapezoidHandle : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit TrapezoidHandle(QWidget* parent = nullptr);
    ~TrapezoidHandle() override;

    TrapezoidHandle(const TrapezoidHandle&)            = delete;
    TrapezoidHandle& operator=(const TrapezoidHandle&) = delete;
    TrapezoidHandle(TrapezoidHandle&&)                 = delete;
    TrapezoidHandle& operator=(TrapezoidHandle&&)      = delete;

    /// @brief Set which edge the trapezoid sits on (affects orientation).
    void SetDockSide(DockSide side);

    /// @brief Get current dock side.
    [[nodiscard]] auto GetDockSide() const -> DockSide;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the handle is clicked (expand ActionBar).
    void Clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void OnThemeChanged() override;

private:
    [[nodiscard]] auto BuildTrapezoidPath() const -> QPainterPath;

    static constexpr int kLongEdge  = 64;  ///< Wider edge length
    static constexpr int kShortEdge = 40;  ///< Narrower edge length
    static constexpr int kDepth     = 12;  ///< Height/width perpendicular to edge

    DockSide _dockSide = DockSide::Bottom;
    bool     _hovered  = false;
    bool     _pressed  = false;
};

} // namespace matcha::gui
