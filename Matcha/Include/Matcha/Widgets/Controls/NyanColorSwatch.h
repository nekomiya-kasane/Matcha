#pragma once

/**
 * @file NyanColorSwatch.h
 * @brief Theme-aware clickable color swatch (chip) widget.
 *
 * Displays a filled color rectangle with optional title text below.
 * Shows hex color tooltip on hover. Emits `ColorClicked` on click.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanColorPanel.h` (renamed ColorPanel->ColorSwatch)
 * - Old API: SetPanelColor/GetPanelColor, SetPanelTitle/GetPanelTitle
 *
 * @par Visual specification
 * - Color rectangle with Border2 border, 1px
 * - Title below swatch in Foreground1, small font
 * - Hover: PrimaryHover border
 * - Tooltip: hex color string
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware clickable color swatch.
 *
 * A compact color chip that displays a color and optional title.
 * Emits `ColorClicked(QColor)` when clicked.
 */
class MATCHA_EXPORT NyanColorSwatch : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a color swatch.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanColorSwatch(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanColorSwatch() override;

    NyanColorSwatch(const NyanColorSwatch&)            = delete;
    NyanColorSwatch& operator=(const NyanColorSwatch&) = delete;
    NyanColorSwatch(NyanColorSwatch&&)                 = delete;
    NyanColorSwatch& operator=(NyanColorSwatch&&)      = delete;

    /// @brief Set the displayed color.
    void SetColor(const QColor& color);

    /// @brief Get the displayed color.
    [[nodiscard]] auto Color() const -> QColor;

    /// @brief Get the hex string representation of the color.
    [[nodiscard]] auto ColorHex() const -> QString;

    /// @brief Set the title text below the swatch.
    void SetTitle(const QString& title);

    /// @brief Get the title.
    [[nodiscard]] auto Title() const -> QString;

    /// @brief Size hint: 32x32 (swatch only) or 32x48 (with title).
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the swatch is clicked.
    void ColorClicked(const QColor& color);

protected:
    /// @brief Custom paint: color rectangle + optional title.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle click.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kSwatchSize   = 32; ///< Swatch square size
    static constexpr int kTitleHeight  = 16; ///< Title area height

    QColor  _color;
    QString _title;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
