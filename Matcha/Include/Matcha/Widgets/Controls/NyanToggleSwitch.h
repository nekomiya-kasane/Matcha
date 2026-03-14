#pragma once

/**
 * @file NyanToggleSwitch.h
 * @brief Theme-aware toggle switch with animated sliding knob.
 *
 * A standalone QWidget that paints a horizontal track with a sliding knob.
 * The knob slides left/right on check state change, animated via AnimationToken.
 * Optional on/off text labels displayed beside the track.
 *
 * @par Old project reference
 * No old NyanGuis equivalent exists. This is a new widget.
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken and AnimationToken.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware toggle switch with animated sliding knob.
 *
 * Visual states:
 * - **Off**: track = Background4, knob = Background1 with Border3 outline
 * - **On**: track = PrimaryNormal, knob = white (slides right)
 * - **Disabled**: track = Background4, knob = Background3, Border4 outline
 *
 * Emits `Toggled(bool)` when the user clicks to change state.
 */
class MATCHA_EXPORT NyanToggleSwitch : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a toggle switch.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanToggleSwitch(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanToggleSwitch() override;

    NyanToggleSwitch(const NyanToggleSwitch&)            = delete;
    NyanToggleSwitch& operator=(const NyanToggleSwitch&) = delete;
    NyanToggleSwitch(NyanToggleSwitch&&)                 = delete;
    NyanToggleSwitch& operator=(NyanToggleSwitch&&)      = delete;

    /// @brief Set the checked (on/off) state.
    void SetChecked(bool checked);

    /// @brief Get the current checked state.
    [[nodiscard]] auto IsChecked() const -> bool;

    /// @brief Set the text displayed when the switch is ON.
    void SetOnText(const QString& text);

    /// @brief Set the text displayed when the switch is OFF.
    void SetOffText(const QString& text);

Q_SIGNALS:
    /// @brief Emitted when the user toggles the switch.
    void Toggled(bool checked);

protected:
    /// @brief Custom paint: track + sliding knob using design tokens.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle click to toggle state.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Preferred size for layout calculations.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kTrackWidth  = 36; ///< Track width in px
    static constexpr int kTrackHeight = 18; ///< Track height in px
    static constexpr int kKnobSize    = 14; ///< Knob diameter in px
    static constexpr int kKnobMargin  = 2;  ///< Knob inset from track edge
    static constexpr int kTextGap     = 6;  ///< Gap between track and text

    bool    _checked = false;   ///< Current on/off state
    qreal   _knobPos = 0.0;    ///< Animated knob X position (0.0=left, 1.0=right)
    QString _onText;            ///< Text displayed when ON
    QString _offText;           ///< Text displayed when OFF
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
