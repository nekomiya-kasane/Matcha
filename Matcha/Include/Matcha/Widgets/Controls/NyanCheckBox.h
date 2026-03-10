#pragma once

/**
 * @file NyanCheckBox.h
 * @brief Theme-aware checkbox with tristate support and custom-painted indicator.
 *
 * Inherits QCheckBox for Qt checkbox semantics and ThemeAware for design token
 * integration. The indicator is custom-painted to match the Matcha visual
 * language (rounded rect, PrimaryNormal check, Line3 border).
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanCheckBox.h` (QCheckBox base, Q_D pattern)
 * - `old/NyanGuis/Gui/inc/NyanCheckBoxStyle.h` (QProxyStyle drawControl)
 * - `old/NyanGuis/Gui/src/NyanCheckBoxStyle.cpp` (indicator paint: 16x16, radius 3,
 *   Background1 fill, Line3 border, PrimaryHover hover, checked via icon pixmap,
 *   partial = PrimaryClick inner rect, disabled = Line4 border)
 *
 * @par Visual preservation
 * New paint must produce identical visual output for unchecked/checked/partial
 * states in Normal/Hover/Disabled. Fixed height 24px preserved.
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values used in painting.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QCheckBox>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Theme-aware checkbox with custom-painted indicator.
 *
 * Supports tristate (`setTristate(true)`) with three visual states:
 * - **Unchecked**: rounded rect outline (Background1 fill, Border3 border)
 * - **Checked**: PrimaryNormal filled rect with white check mark
 * - **Partial**: outline rect with PrimaryPressed inner rect
 *
 * All colors respond to theme changes via ThemeAware.
 */
class MATCHA_EXPORT NyanCheckBox : public QCheckBox, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a checkbox.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanCheckBox(QWidget* parent = nullptr);

    /**
     * @brief Construct a checkbox with text label.
     * @param theme Theme service reference.
     * @param text  Checkbox label text.
     * @param parent Optional parent widget.
     */
    explicit NyanCheckBox(const QString& text, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanCheckBox() override;

    NyanCheckBox(const NyanCheckBox&)            = delete;
    NyanCheckBox& operator=(const NyanCheckBox&) = delete;
    NyanCheckBox(NyanCheckBox&&)                 = delete;
    NyanCheckBox& operator=(NyanCheckBox&&)      = delete;

protected:
    /// @brief Custom paint: indicator + text using design tokens.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kIndicatorSize = 16;  ///< Indicator width/height in px
    static constexpr int kFixedHeight   = 24;  ///< Widget fixed height in px
    static constexpr int kIndicatorInset = 2;  ///< Inset from indicator rect edge
    static constexpr int kTextGap       = 4;   ///< Gap between indicator and text
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
