#pragma once

/**
 * @file NyanSlider.h
 * @brief Theme-aware single-handle slider with custom-painted track and handle.
 *
 * Inherits QSlider for Qt slider semantics and ThemeAware for design
 * token integration. Supports horizontal/vertical orientation and optional
 * tick marks.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Track: PrimaryNormal (filled portion), Background4 (unfilled), 4px height
 * - Handle: Background1 fill, Border2 border, 14px diameter circle
 * - Handle hover: PrimaryHover border
 * - Disabled: Background4 track, Foreground5 handle border
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QSlider>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware single-handle slider.
 *
 * Provides SetRange/SetStep convenience API on top of QSlider.
 * Custom-painted track and circular handle using design tokens.
 */
class MATCHA_EXPORT NyanSlider : public QSlider, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a horizontal slider.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanSlider(QWidget* parent = nullptr);

    /**
     * @brief Construct a slider with specified orientation.
     * @param theme Theme service reference.
     * @param orientation Qt::Horizontal or Qt::Vertical.
     * @param parent Optional parent widget.
     */
    NyanSlider(Qt::Orientation orientation, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanSlider() override;

    NyanSlider(const NyanSlider&)            = delete;
    NyanSlider& operator=(const NyanSlider&) = delete;
    NyanSlider(NyanSlider&&)                 = delete;
    NyanSlider& operator=(NyanSlider&&)      = delete;

    /// @brief Set the valid range.
    void SetRange(int minimum, int maximum);

    /// @brief Set the single-step increment.
    void SetStep(int step);

    /// @brief Enable or disable tick marks.
    void SetTicksVisible(bool visible);

    /// @brief Whether tick marks are visible.
    [[nodiscard]] auto TicksVisible() const -> bool;

    /// @brief Size hint: 200x24 horizontal, 24x200 vertical.
    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: themed track + handle + optional ticks.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kTrackHeight   = 4;   ///< Track thickness in px
    static constexpr int kHandleSize    = 14;  ///< Handle diameter in px
    static constexpr int kDefaultLength = 200; ///< Default length in px
    static constexpr int kCrossSize     = 24;  ///< Cross-axis size in px
    static constexpr int kTickHeight    = 4;   ///< Tick mark height in px

    bool _ticksVisible = false;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
