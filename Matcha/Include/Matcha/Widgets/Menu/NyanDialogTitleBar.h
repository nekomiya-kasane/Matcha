#pragma once

/**
 * @file NyanDialogTitleBar.h
 * @brief Dialog title bar with icon, title, and window buttons.
 *
 * NyanDialogTitleBar provides:
 * - Icon + title text display
 * - Min/Max/Close buttons (configurable visibility)
 * - Drag-to-move support
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QPushButton;

namespace matcha::gui {

/**
 * @brief Title bar button visibility flags.
 */
enum class TitleBarButton : uint8_t {
    None     = 0,
    Minimize = 1U << 0U,
    Maximize = 1U << 1U,
    Close    = 1U << 2U,
    All      = Minimize | Maximize | Close
};

/// @brief Enable bitwise operations on TitleBarButton.
[[nodiscard]] constexpr auto operator|(TitleBarButton a, TitleBarButton b) noexcept -> TitleBarButton {
    return static_cast<TitleBarButton>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

[[nodiscard]] constexpr auto operator&(TitleBarButton a, TitleBarButton b) noexcept -> TitleBarButton {
    return static_cast<TitleBarButton>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

[[nodiscard]] constexpr auto operator~(TitleBarButton a) noexcept -> TitleBarButton {
    return static_cast<TitleBarButton>(~static_cast<uint8_t>(a));
}

/**
 * @brief Dialog title bar with icon, title, and window buttons.
 *
 * A11y role: TitleBar.
 */
class MATCHA_EXPORT NyanDialogTitleBar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a dialog title bar.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanDialogTitleBar(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanDialogTitleBar() override;

    NyanDialogTitleBar(const NyanDialogTitleBar&)            = delete;
    NyanDialogTitleBar& operator=(const NyanDialogTitleBar&) = delete;
    NyanDialogTitleBar(NyanDialogTitleBar&&)                 = delete;
    NyanDialogTitleBar& operator=(NyanDialogTitleBar&&)      = delete;

    // -- Title --

    /// @brief Set the title text.
    void SetTitle(const QString& title);

    /// @brief Get the title text.
    [[nodiscard]] auto Title() const -> QString;

    /// @brief Set the title icon.
    void SetIcon(const QIcon& icon);

    /// @brief Get the title icon.
    [[nodiscard]] auto Icon() const -> QIcon;

    // -- Buttons --

    /// @brief Set visible buttons.
    void SetVisibleButtons(TitleBarButton buttons);

    /// @brief Get visible buttons.
    [[nodiscard]] auto VisibleButtons() const -> TitleBarButton;

    /// @brief Set minimize button visible.
    void SetMinimizeVisible(bool visible);

    /// @brief Check if minimize button is visible.
    [[nodiscard]] auto IsMinimizeVisible() const -> bool;

    /// @brief Set maximize button visible.
    void SetMaximizeVisible(bool visible);

    /// @brief Check if maximize button is visible.
    [[nodiscard]] auto IsMaximizeVisible() const -> bool;

    /// @brief Set close button visible.
    void SetCloseVisible(bool visible);

    /// @brief Check if close button is visible.
    [[nodiscard]] auto IsCloseVisible() const -> bool;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when minimize button is clicked.
    void MinimizeClicked();

    /// @brief Emitted when maximize button is clicked.
    void MaximizeClicked();

    /// @brief Emitted when close button is clicked.
    void CloseClicked();

    /// @brief Emitted when drag starts.
    void DragStarted(const QPoint& globalPos);

    /// @brief Emitted during drag.
    void DragMoved(const QPoint& globalPos);

protected:
    /// @brief Custom paint for themed title bar.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle mouse press for drag.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Handle mouse move for drag.
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief Handle mouse release.
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// @brief Handle double-click for maximize toggle.
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void UpdateButtonStyles();

    static constexpr int kHeight     = 32;
    static constexpr int kIconSize   = 16;
    static constexpr int kButtonSize = 28;

    QHBoxLayout*   _layout          = nullptr;
    QLabel*        _iconLabel       = nullptr;
    QLabel*        _titleLabel      = nullptr;
    QPushButton*   _minimizeButton  = nullptr;
    QPushButton*   _maximizeButton  = nullptr;
    QPushButton*   _closeButton     = nullptr;

    QIcon          _icon;
    TitleBarButton _visibleButtons  = TitleBarButton::Close;
    bool           _dragging        = false;
    QPoint         _dragStartPos;
};

} // namespace matcha::gui
