#pragma once

/**
 * @file NyanRangeSlider.h
 * @brief Theme-aware dual-handle range slider.
 *
 * Fully custom-painted QWidget with two draggable handles defining a
 * low/high range. Highlighted region between handles uses PrimaryNormal.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Track: Background4 full, PrimaryNormal between handles
 * - Handles: Background1 fill, Border2 border, PrimaryHover on hover, 14px
 * - Disabled: Background4 track, Foreground5 handle border
 *
 * @see NyanSlider for single-handle variant.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Theme-aware dual-handle range slider.
 *
 * Provides SetRange/SetLow/SetHigh/SetStep API. Emits `RangeChanged(int, int)`
 * when either handle is dragged. Horizontal orientation only.
 */
class MATCHA_EXPORT NyanRangeSlider : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a range slider.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanRangeSlider(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanRangeSlider() override;

    NyanRangeSlider(const NyanRangeSlider&)            = delete;
    NyanRangeSlider& operator=(const NyanRangeSlider&) = delete;
    NyanRangeSlider(NyanRangeSlider&&)                 = delete;
    NyanRangeSlider& operator=(NyanRangeSlider&&)      = delete;

    /// @brief Set the overall value range.
    void SetRange(int minimum, int maximum);

    /// @brief Set the low handle value.
    void SetLow(int value);

    /// @brief Get the low handle value.
    [[nodiscard]] auto Low() const -> int;

    /// @brief Set the high handle value.
    void SetHigh(int value);

    /// @brief Get the high handle value.
    [[nodiscard]] auto High() const -> int;

    /// @brief Set the step increment for snapping.
    void SetStep(int step);

    /// @brief Get the step increment.
    [[nodiscard]] auto Step() const -> int;

    /// @brief Get the minimum of the range.
    [[nodiscard]] auto Minimum() const -> int;

    /// @brief Get the maximum of the range.
    [[nodiscard]] auto Maximum() const -> int;

    /// @brief Size hint: 200x24.
    [[nodiscard]] auto sizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when either handle is moved.
    void RangeChanged(int low, int high);

protected:
    /// @brief Custom paint: track + highlighted region + dual handles.
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
    [[nodiscard]] auto ValueToX(int val) const -> int;

    /// @brief Convert pixel X to value (clamped and snapped).
    [[nodiscard]] auto XToValue(int x) const -> int;

    static constexpr int kTrackHeight   = 4;
    static constexpr int kHandleSize    = 14;
    static constexpr int kDefaultLength = 200;
    static constexpr int kCrossSize     = 24;

    enum class DragHandle : uint8_t { None, Low, High };

    int        _minimum    = 0;
    int        _maximum    = 100;
    int        _low        = 0;
    int        _high       = 100;
    int        _step       = 1;
    DragHandle _dragging   = DragHandle::None;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
