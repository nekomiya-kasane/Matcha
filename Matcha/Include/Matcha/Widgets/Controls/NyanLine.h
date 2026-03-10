#pragma once

/**
 * @file NyanLine.h
 * @brief Theme-aware 1px separator line (horizontal or vertical).
 *
 * A minimal QWidget that paints a single-pixel line using a configurable
 * `ColorToken`. Defaults to `ColorToken::Separator`.
 *
 * @par Old project reference
 * No old NyanGuis equivalent exists. This is a new utility widget.
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken::Separator.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Theme-aware 1px separator line.
 *
 * Renders a horizontal or vertical line using design tokens.
 * Fixed to 1px in the cross-axis dimension.
 */
class MATCHA_EXPORT NyanLine : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a horizontal separator line.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanLine(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanLine() override;

    NyanLine(const NyanLine&)            = delete;
    NyanLine& operator=(const NyanLine&) = delete;
    NyanLine(NyanLine&&)                 = delete;
    NyanLine& operator=(NyanLine&&)      = delete;

    /// @brief Set the line orientation.
    void SetOrientation(Qt::Orientation orientation);

    /// @brief Get the current orientation.
    [[nodiscard]] auto Orientation() const -> Qt::Orientation;

    /// @brief Set the line color token.
    void SetColor(ColorToken token);

    /// @brief Get the current color token.
    [[nodiscard]] auto Color() const -> ColorToken;

    /// @brief Preferred size: full span in main axis, 1px in cross axis.
    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: 1px line using themed color.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    Qt::Orientation _orientation = Qt::Horizontal;
    ColorToken      _colorToken  = ColorToken::Separator;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
