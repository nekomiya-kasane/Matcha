#pragma once

/**
 * @file NyanProgressRing.h
 * @brief Theme-aware circular progress ring with indeterminate mode.
 *
 * Fully custom-painted QWidget that draws a circular arc to indicate
 * progress. Supports determinate (0-100%) and indeterminate (spinning)
 * modes.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanProgressRing.h` (float min/max/value, 4 states)
 * - `old/NyanGuis/Gui/src/NyanProgressRingPrivate.cpp` (360-degree arc calc, 4px inset)
 *
 * @par Visual specification
 * - Track ring: Background4, configurable thickness
 * - Progress arc: PrimaryNormal
 * - Indeterminate: rotating arc segment using QTimer
 * - Text: centered percentage (Foreground1)
 * - Default size: 48x48
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QTimer>
#include <QWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware circular progress ring.
 *
 * Draws a circular arc for progress indication. Supports indeterminate
 * spinning mode with configurable ring thickness.
 */
class MATCHA_EXPORT NyanProgressRing : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a progress ring.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanProgressRing(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanProgressRing() override;

    NyanProgressRing(const NyanProgressRing&)            = delete;
    NyanProgressRing& operator=(const NyanProgressRing&) = delete;
    NyanProgressRing(NyanProgressRing&&)                 = delete;
    NyanProgressRing& operator=(NyanProgressRing&&)      = delete;

    /// @brief Set the current progress value.
    void SetValue(int value);

    /// @brief Get the current value.
    [[nodiscard]] auto Value() const -> int;

    /// @brief Set the valid range.
    void SetRange(int minimum, int maximum);

    /// @brief Get the minimum.
    [[nodiscard]] auto Minimum() const -> int;

    /// @brief Get the maximum.
    [[nodiscard]] auto Maximum() const -> int;

    /// @brief Enable or disable indeterminate (spinning) mode.
    void SetIndeterminate(bool indeterminate);

    /// @brief Whether indeterminate mode is active.
    [[nodiscard]] auto IsIndeterminate() const -> bool;

    /// @brief Set the ring stroke thickness in pixels.
    void SetThickness(int thickness);

    /// @brief Get the ring thickness.
    [[nodiscard]] auto Thickness() const -> int;

    /// @brief Enable or disable the text percentage overlay.
    void SetTextVisible(bool visible);

    /// @brief Whether text overlay is shown.
    [[nodiscard]] auto IsTextVisible() const -> bool;

    /// @brief Size hint: 48x48 default.
    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: ring track + progress arc + optional text.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kDefaultSize     = 48;
    static constexpr int kDefaultThickness = 4;
    static constexpr int kInset           = 4;  ///< Matches old 4px inset
    static constexpr int kSpinArcSpan     = 90; ///< Indeterminate arc span in degrees
    static constexpr int kSpinStep        = 6;  ///< Degrees per tick

    int   _minimum       = 0;
    int   _maximum       = 100;
    int   _value         = 0;
    int   _thickness     = kDefaultThickness;
    bool  _indeterminate = false;
    bool  _textVisible   = true;
    int   _spinAngle     = 0;
    QTimer _spinTimer;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
