#pragma once

/**
 * @file NyanScrollBar.h
 * @brief Theme-aware custom-painted scrollbar with auto-hide and hover expand.
 *
 * Inherits QScrollBar for Qt scrollbar semantics and ThemeAware for design
 * token integration. Thin by default (6px), expands to 10px on hover.
 * Auto-hides after 1.5s of idle (no value change, no hover).
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Track: Background3 fill, fully rounded
 * - Handle: Foreground5 fill (normal), Foreground3 (hovered), fully rounded
 * - Hidden state: opacity 0 (via QWidget::setVisible or alpha paint)
 * - Expand: 6px -> 10px width on hover, animated via AnimationToken::Quick
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken and AnimationToken.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QScrollBar>
#include <QTimer>

namespace matcha::gui {

/**
 * @brief Theme-aware scrollbar with auto-hide and hover-expand behavior.
 *
 * Used internally by NyanScrollArea to replace default Qt scrollbars.
 * Can also be used standalone.
 */
class MATCHA_EXPORT NyanScrollBar : public QScrollBar, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a vertical scrollbar.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanScrollBar(QWidget* parent = nullptr);

    /**
     * @brief Construct a scrollbar with specified orientation.
     * @param theme Theme service reference.
     * @param orientation Qt::Horizontal or Qt::Vertical.
     * @param parent Optional parent widget.
     */
    NyanScrollBar(Qt::Orientation orientation, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanScrollBar() override;

    NyanScrollBar(const NyanScrollBar&)            = delete;
    NyanScrollBar& operator=(const NyanScrollBar&) = delete;
    NyanScrollBar(NyanScrollBar&&)                 = delete;
    NyanScrollBar& operator=(NyanScrollBar&&)      = delete;

    /// @brief Set the auto-hide timeout in milliseconds. 0 disables auto-hide.
    void SetAutoHideMs(int ms);

    /// @brief Get the auto-hide timeout.
    [[nodiscard]] auto AutoHideMs() const -> int;

    /// @brief Set the thin (default) thickness in pixels.
    void SetThinThickness(int px);

    /// @brief Set the expanded (hovered) thickness in pixels.
    void SetExpandedThickness(int px);

    /// @brief Preferred size for layout calculations.
    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: themed track + handle, respects current thickness.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Start hover expand animation.
    void enterEvent(QEnterEvent* event) override;

    /// @brief Start hover collapse animation, restart auto-hide timer.
    void leaveEvent(QEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitCommon();
    void RestartIdleTimer();
    void OnIdleTimeout();
    void SetCurrentThickness(int px);

    static constexpr int kDefaultThinPx     = 6;    ///< Default thin thickness
    static constexpr int kDefaultExpandedPx = 10;   ///< Default expanded thickness
    static constexpr int kDefaultAutoHideMs = 1500;  ///< Default auto-hide delay
    static constexpr int kMinHandleLength   = 20;   ///< Minimum handle length in px

    int  _thinPx     = kDefaultThinPx;
    int  _expandedPx = kDefaultExpandedPx;
    int  _currentPx  = kDefaultThinPx;      ///< Current animated thickness
    int  _autoHideMs = kDefaultAutoHideMs;
    bool _hovered    = false;
    bool _visible    = true;                ///< Whether scrollbar is painted (auto-hide state)

    QTimer _idleTimer;
};

} // namespace matcha::gui
