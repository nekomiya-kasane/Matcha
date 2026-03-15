#pragma once

/**
 * @file NyanMenuItem.h
 * @brief Clickable menu item with icon, text, and shortcut.
 *
 * NyanMenuItem provides:
 * - Icon (16x16) on the left
 * - Text label
 * - Shortcut text (right-aligned)
 * - Optional submenu arrow indicator
 *
 * @par Old project reference
 * Replaces old NyanMenuStyle CE_MenuItem drawing with standalone widget.
 *
 * @par Visual specification (from old NyanMenuStyle)
 * - Height: 26px, Min width: 152px
 * - Icon: 16x16 at x+4
 * - Text: left-aligned after icon
 * - Shortcut: right-aligned
 * - Submenu arrow: 16x16 at right-20
 * - Colors:
 *   - Normal: Background1, Font1
 *   - Hovered: Background3, Font1
 *   - Selected: PrimaryLight, PrimaryNormal
 *   - Disabled: Background1, Font5
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QKeySequence>
#include <QWidget>

namespace matcha::gui {

/**
 * @brief Clickable menu item with icon, text, and shortcut.
 *
 * A11y role: MenuItem.
 */
class MATCHA_EXPORT NyanMenuItem : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a menu item.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanMenuItem(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanMenuItem() override;

    NyanMenuItem(const NyanMenuItem&)            = delete;
    NyanMenuItem& operator=(const NyanMenuItem&) = delete;
    NyanMenuItem(NyanMenuItem&&)                 = delete;
    NyanMenuItem& operator=(NyanMenuItem&&)      = delete;

    // -- Properties --

    /// @brief Set the menu item text.
    void SetText(const QString& text);

    /// @brief Get the menu item text.
    [[nodiscard]] auto Text() const -> QString;

    /// @brief Set the menu item icon.
    void SetIcon(const QIcon& icon);

    /// @brief Get the menu item icon.
    [[nodiscard]] auto Icon() const -> QIcon;

    /// @brief Set the keyboard shortcut.
    void SetShortcut(const QKeySequence& shortcut);

    /// @brief Get the keyboard shortcut.
    [[nodiscard]] auto Shortcut() const -> QKeySequence;

    /// @brief Set whether the item is enabled.
    void SetEnabled(bool enabled);

    /// @brief Check if the item is enabled.
    [[nodiscard]] auto IsEnabled() const -> bool;

    /// @brief Set whether to show submenu arrow indicator.
    void SetSubmenuIndicator(bool show);

    /// @brief Check if submenu indicator is shown.
    [[nodiscard]] auto HasSubmenuIndicator() const -> bool;

    // -- Size hints --

    /// @brief Size hint: min width 152, height 26.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Minimum size hint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the item is clicked.
    void Triggered();

protected:
    /// @brief Custom paint for themed menu item.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle mouse enter for hover state.
    void enterEvent(QEnterEvent* event) override;

    /// @brief Handle mouse leave.
    void leaveEvent(QEvent* event) override;

    /// @brief Handle mouse press.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Handle mouse release.
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

    /// @brief Draw the item content (can be overridden by subclasses).
    virtual void DrawContent(QPainter& painter, const QRect& rect, bool hovered, bool pressed) const;

private:
    static constexpr int kHeight     = 26;
    static constexpr int kMinWidth   = 152;
    static constexpr int kIconSize   = 16;
    static constexpr int kIconLeft   = 4;
    static constexpr int kTextLeft   = 24;  ///< After icon
    static constexpr int kArrowSize  = 16;
    static constexpr int kArrowRight = 20;
    static constexpr int kRadius     = 3;

    QString      _text;
    QIcon        _icon;
    QKeySequence _shortcut;
    bool         _enabled          = true;
    bool         _hovered          = false;
    bool         _pressed          = false;
    bool         _submenuIndicator = false;
};

} // namespace matcha::gui
