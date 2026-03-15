#pragma once

/**
 * @file NyanFloatingTitleBar.h
 * @brief Lightweight title bar for Floating Tab Windows.
 *
 * Single-row (28px) self-drawn title bar with:
 * - Optional window icon (16x16)
 * - Window title label
 * - Minimize / MaximizeRestore / Close buttons
 * - Frameless window drag support
 *
 * Used by FloatingWindowNode (kind=Floating) instead of NyanMainTitleBar.
 *
 * @see 05_Greenfield_Plan.md "Qt Widget Hierarchy (Floating Tab Window)"
 */

#include <Matcha/Core/Macros.h>

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QPushButton;

namespace matcha::gui {

/**
 * @brief Lightweight title bar for floating (detached-tab) windows.
 *
 * A11y role: TitleBar.
 */
class MATCHA_EXPORT NyanFloatingTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit NyanFloatingTitleBar(QWidget* parent = nullptr);
    ~NyanFloatingTitleBar() override;

    NyanFloatingTitleBar(const NyanFloatingTitleBar&)            = delete;
    NyanFloatingTitleBar& operator=(const NyanFloatingTitleBar&) = delete;
    NyanFloatingTitleBar(NyanFloatingTitleBar&&)                 = delete;
    NyanFloatingTitleBar& operator=(NyanFloatingTitleBar&&)      = delete;

    /** @brief Set the window title text. */
    void SetTitle(const QString& title);

    /** @brief Get the window title text. */
    [[nodiscard]] auto Title() const -> QString;

    /** @brief Set the window icon (16x16). */
    void SetIcon(const QIcon& icon);

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    void MinimizeRequested();
    void MaximizeRequested();
    void CloseRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void InitLayout();

    static constexpr int kHeight     = 28;
    static constexpr int kButtonW    = 28;
    static constexpr int kButtonH    = 20;
    static constexpr int kIconSize   = 16;

    QHBoxLayout* _layout         = nullptr;
    QLabel*      _iconLabel      = nullptr;
    QLabel*      _titleLabel     = nullptr;
    QPushButton* _minimizeButton = nullptr;
    QPushButton* _maximizeButton = nullptr;
    QPushButton* _closeButton    = nullptr;

    QPointF      _dragStartPos;
    bool         _dragging = false;
};

} // namespace matcha::gui
