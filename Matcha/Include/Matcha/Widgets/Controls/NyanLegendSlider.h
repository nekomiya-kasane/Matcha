#pragma once

/**
 * @file NyanLegendSlider.h
 * @brief Theme-aware slider with color gradient bar for legend visualization.
 *
 * Combines a draggable handle with a horizontal color gradient bar.
 * Used for simulation result range selection with color mapping.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanLegendSlider.h` (SetRangeValue, float lower/upper)
 * - Old: single drag handle, LegendValueChanged signal
 *
 * @par Visual specification
 * - Gradient bar: user-specified color stops, 8px height
 * - Handle: Background1 fill, Border2 border, 14px diameter circle
 * - Handle hover: PrimaryHover border
 * - Value label below handle optional
 *
 * @see NyanSlider for single-handle slider.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <vector>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief A color stop for the gradient bar.
 */
struct MATCHA_EXPORT ColorStop {
    double position = 0.0; ///< Normalized position [0.0, 1.0]
    QColor color;    ///< Color at this stop
};

/**
 * @brief Theme-aware slider with color gradient bar.
 *
 * Displays a horizontal gradient bar defined by color stops, with a
 * draggable handle. Emits `ValueChanged(double)` when moved.
 */
class MATCHA_EXPORT NyanLegendSlider : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a legend slider.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanLegendSlider(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanLegendSlider() override;

    NyanLegendSlider(const NyanLegendSlider&)            = delete;
    NyanLegendSlider& operator=(const NyanLegendSlider&) = delete;
    NyanLegendSlider(NyanLegendSlider&&)                 = delete;
    NyanLegendSlider& operator=(NyanLegendSlider&&)      = delete;

    /// @brief Set the value range.
    void SetRange(double lower, double upper);

    /// @brief Get the lower bound.
    [[nodiscard]] auto Lower() const -> double;

    /// @brief Get the upper bound.
    [[nodiscard]] auto Upper() const -> double;

    /// @brief Set the current value.
    void SetValue(double value);

    /// @brief Get the current value.
    [[nodiscard]] auto Value() const -> double;

    /// @brief Set the color gradient stops.
    void SetColorMap(const std::vector<ColorStop>& stops);

    /// @brief Get the current color stops.
    [[nodiscard]] auto ColorMap() const -> const std::vector<ColorStop>&;

    /// @brief Size hint: 200x32.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the handle is dragged.
    void ValueChanged(double value);

protected:
    /// @brief Custom paint: gradient bar + handle.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Begin handle drag.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Continue handle drag.
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief End handle drag.
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Convert value to pixel X position.
    [[nodiscard]] auto ValueToX(double val) const -> int;

    /// @brief Convert pixel X to value (clamped).
    [[nodiscard]] auto XToValue(int x) const -> double;

    static constexpr int kBarHeight     = 8;
    static constexpr int kHandleSize    = 14;
    static constexpr int kDefaultWidth  = 200;
    static constexpr int kTotalHeight   = 32;

    double _lower  = 0.0;
    double _upper  = 100.0;
    double _value  = 0.0;
    bool   _dragging = false;
    std::vector<ColorStop> _colorStops;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
