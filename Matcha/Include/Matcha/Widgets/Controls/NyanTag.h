#pragma once

/**
 * @file NyanTag.h
 * @brief Theme-aware closable tag/chip widget for multi-select display.
 *
 * A compact QWidget that paints a rounded rectangle with text, optional
 * prefix icon, and optional suffix close button. Used by NyanCascader and
 * NyanTransfer (Tier 2) to display selected items.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget from `specs/details/25-tag.md`.
 *
 * @par Visual specification
 * - Height: fixed 24px
 * - Width: auto from content, clamped to max 152px
 * - Radius: RadiusToken::Small (2px)
 * - Text overflow: ellipsis
 * - Close button: suffix position, hover-reveal when not selected
 * - Background: Background3 (normal), PrimaryDefault (selected)
 * - Text: Foreground1 (normal), OnPrimary (selected)
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QIcon>
#include <QWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware closable tag/chip widget.
 *
 * Displays a short text label in a rounded rectangle. Optional prefix icon
 * and optional close button. Close button visibility depends on selection
 * and hover state per specs/details/25-tag.md.
 */
class MATCHA_EXPORT NyanTag : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a tag.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanTag(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanTag() override;

    NyanTag(const NyanTag&)            = delete;
    NyanTag& operator=(const NyanTag&) = delete;
    NyanTag(NyanTag&&)                 = delete;
    NyanTag& operator=(NyanTag&&)      = delete;

    /// @brief Set the tag text.
    void SetText(const QString& text);

    /// @brief Get the tag text.
    [[nodiscard]] auto Text() const -> QString;

    /// @brief Set the tag background color (non-selected state).
    void SetColor(const QColor& color);

    /// @brief Get the tag background color.
    [[nodiscard]] auto Color() const -> QColor;

    /// @brief Set optional prefix icon.
    void SetIcon(const QIcon& icon);

    /// @brief Get the prefix icon.
    [[nodiscard]] auto Icon() const -> QIcon;

    /// @brief Enable or disable the close (x) button.
    void SetClosable(bool closable);

    /// @brief Whether the close button is enabled.
    [[nodiscard]] auto IsClosable() const -> bool;

    /// @brief Set the selected state.
    void SetSelected(bool selected);

    /// @brief Whether the tag is selected.
    [[nodiscard]] auto IsSelected() const -> bool;

    /// @brief Size hint based on text width + icon + close button.
    [[nodiscard]] auto sizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the close button is clicked.
    void Closed();

    /// @brief Emitted when the tag body is clicked.
    void Clicked();

protected:
    /// @brief Custom paint: rounded rect + text + optional icon + close.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle close button click and tag body click.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Track hover for close button reveal.
    void enterEvent(QEnterEvent* event) override;

    /// @brief Track hover for close button reveal.
    void leaveEvent(QEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Compute the close button hit rect.
    [[nodiscard]] auto CloseButtonRect() const -> QRect;

    /// @brief Whether the close button should be visually shown.
    [[nodiscard]] auto IsCloseVisible() const -> bool;

    static constexpr int kFixedHeight = 24;   ///< Fixed tag height (spec: 24px)
    static constexpr int kMaxWidth    = 152;  ///< Max tag width (spec: 152px)
    static constexpr int kHPadding    = 8;    ///< Horizontal content padding
    static constexpr int kIconSize    = 16;   ///< Prefix icon size
    static constexpr int kIconGap     = 4;    ///< Gap between icon and text
    static constexpr int kCloseSize   = 16;   ///< Close button area size
    static constexpr int kCloseGap    = 4;    ///< Gap before close button

    QString _text;
    QColor  _color;    ///< User-specified background color (non-selected)
    QIcon   _icon;
    bool    _closable = false;
    bool    _selected = false;
    bool    _hovered  = false;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
