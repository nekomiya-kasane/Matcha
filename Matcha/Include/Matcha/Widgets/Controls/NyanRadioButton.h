#pragma once

/**
 * @file NyanRadioButton.h
 * @brief Theme-aware radio button with custom-painted circular indicator.
 *
 * Inherits QRadioButton for Qt radio semantics (mutual exclusion within
 * a QButtonGroup) and ThemeAware for design token integration. The circular
 * indicator is custom-painted to match the Matcha visual language.
 *
 * @par Old project reference
 * No old NyanGuis equivalent exists. Visual language follows the same
 * token set as NyanCheckBox (Background1, Border3, PrimaryNormal, Border4).
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values used in painting.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QRadioButton>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware radio button with custom-painted circular indicator.
 *
 * Visual states:
 * - **Unselected**: circular outline (Background1 fill, Border3 border)
 * - **Selected**: PrimaryNormal filled circle with white inner dot
 * - **Disabled**: Background1 fill, Border4 border
 *
 * All colors respond to theme changes via ThemeAware.
 */
class MATCHA_EXPORT NyanRadioButton : public QRadioButton, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a radio button.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanRadioButton(QWidget* parent = nullptr);

    /**
     * @brief Construct a radio button with text label.
     * @param theme Theme service reference.
     * @param text  Radio button label text.
     * @param parent Optional parent widget.
     */
    explicit NyanRadioButton(const QString& text, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanRadioButton() override;

    NyanRadioButton(const NyanRadioButton&)            = delete;
    NyanRadioButton& operator=(const NyanRadioButton&) = delete;
    NyanRadioButton(NyanRadioButton&&)                 = delete;
    NyanRadioButton& operator=(NyanRadioButton&&)      = delete;

protected:
    /// @brief Custom paint: circular indicator + text using design tokens.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kIndicatorSize = 16;  ///< Indicator diameter in px
    static constexpr int kFixedHeight   = 24;  ///< Widget fixed height in px
    static constexpr int kIndicatorInset = 2;  ///< Inset from indicator rect edge
    static constexpr int kTextGap       = 4;   ///< Gap between indicator and text
    static constexpr int kInnerDotInset = 4;   ///< Inset for inner selected dot
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
