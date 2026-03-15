#pragma once

/**
 * @file NyanMainTitleBar.h
 * @brief Main window title bar shell (single row).
 *
 * NyanMainTitleBar provides:
 * - Single row layout (28px height)
 * - MenuBar + QuickCommandContainer + centered text logo + Min/Max/Close
 *
 * Row 2 (document toolbar) has been extracted to NyanDocumentToolBar.
 * Logo has been extracted to NyanLogoButton.
 *
 * @see NyanDocumentToolBar for the document toolbar row.
 * @see NyanLogoButton for the clickable logo.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QPushButton;

namespace matcha::gui {

class NyanMenuBar;

/**
 * @brief Main window title bar shell (single row).
 *
 * A11y role: TitleBar.
 */
class MATCHA_EXPORT NyanMainTitleBar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a main title bar.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanMainTitleBar(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanMainTitleBar() override;

    NyanMainTitleBar(const NyanMainTitleBar&)            = delete;
    NyanMainTitleBar& operator=(const NyanMainTitleBar&) = delete;
    NyanMainTitleBar(NyanMainTitleBar&&)                 = delete;
    NyanMainTitleBar& operator=(NyanMainTitleBar&&)      = delete;

    /// @brief Set the title logo (centered in title bar).
    void SetTitleLogo(const QPixmap& titleLogo);

    // -- Menu Bar --

    /// @brief Get the menu bar widget.
    [[nodiscard]] auto MenuBar() -> NyanMenuBar*;

    // -- Containers --

    /// @brief Get the quick command container (after menu bar).
    [[nodiscard]] auto QuickCommandContainer() -> QWidget*;

    // -- Title --

    /// @brief Set the window title.
    void SetTitle(const QString& title);

    /// @brief Get the window title.
    [[nodiscard]] auto Title() const -> QString;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when minimize button is clicked.
    void MinimizeRequested();

    /// @brief Emitted when maximize/restore button is clicked.
    void MaximizeRequested();

    /// @brief Emitted when close button is clicked.
    void CloseRequested();

protected:
    /// @brief Custom paint for themed title bar.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle double-click for maximize toggle.
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    /// @brief Handle mouse press for window drag initiation.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Handle mouse move for window dragging.
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void UpdateButtonStyles();

    static constexpr int kHeight     = 28;
    static constexpr int kButtonSize = 20;

    QHBoxLayout*     _layout               = nullptr;
    NyanMenuBar*     _menuBar              = nullptr;
    QWidget*         _quickCommandContainer = nullptr;
    QLabel*          _titleLogoLabel       = nullptr;
    QPushButton*     _minimizeButton       = nullptr;
    QPushButton*     _maximizeButton       = nullptr;
    QPushButton*     _closeButton          = nullptr;

    QPixmap          _titleLogo;
    QString          _title;

    // Drag state (for frameless window move)
    QPointF          _dragStartPos;
    bool             _dragging = false;
};

} // namespace matcha::gui
