#pragma once

/**
 * @file NyanPushButton.h
 * @brief Theme-aware multi-variant push button with size presets.
 *
 * Inherits QPushButton for Qt button semantics and ThemeAware for design
 * token integration. Supports 4 visual variants and 3 size presets.
 * Custom `paintEvent` replaces old QSS-based styling.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanPushButton.h` (8 PBTNState enum)
 * - `old/NyanGuis/Gui/src/NyanPushButtonPrivate.cpp` (QSS color map:
 *   State1=Primary, State2=Secondary, State3=Ghost, State4=Danger,
 *   border-radius:3px, 12 color slots per state: bg/fg/border x 4 interaction)
 *
 * @par Visual preservation
 * - Primary: PrimaryNormal bg, Foreground7 text, PrimaryHover/PrimaryPressed
 * - Secondary: Background4 bg, Foreground1 text, Background5/Background6
 * - Ghost: Background1 bg, Border2 border, PrimaryNormal hover text
 * - Danger: ErrorNormal bg, Foreground7 text, ErrorHover/ErrorPressed
 * - Radius 3px, min size 56x24 (old sizeHint)
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values used in painting.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Tree/FSM/WidgetEnums.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QPushButton>

namespace matcha::gui {

class PushButtonEventFilter;

/**
 * @brief Height preset for push buttons.
 *
 * The underlying value IS the pixel height.
 */
enum class ButtonSize : uint8_t {
    Small  = 24, ///< Compact toolbar button
    Medium = 32, ///< Default button height
    Large  = 40, ///< Prominent dialog button
};

/**
 * @brief Theme-aware multi-variant push button.
 *
 * Supports icon+text, icon-only, and text-only modes.
 * Checkable mode inherits from QPushButton::setCheckable().
 * All painting uses direct QPainter calls with design tokens.
 */
class MATCHA_EXPORT NyanPushButton : public QPushButton, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a push button.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanPushButton(QWidget* parent = nullptr);

    /**
     * @brief Construct a push button with text.
     * @param theme Theme service reference.
     * @param text  Button label text.
     * @param parent Optional parent widget.
     */
    explicit NyanPushButton(const QString& text, QWidget* parent = nullptr);

    /**
     * @brief Construct a push button with icon and text.
     * @param theme Theme service reference.
     * @param icon  Button icon.
     * @param text  Button label text.
     * @param parent Optional parent widget.
     */
    NyanPushButton(const QIcon& icon, const QString& text,
                   QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanPushButton() override;

    NyanPushButton(const NyanPushButton&)            = delete;
    NyanPushButton& operator=(const NyanPushButton&) = delete;
    NyanPushButton(NyanPushButton&&)                 = delete;
    NyanPushButton& operator=(NyanPushButton&&)      = delete;

    /// @brief Set the visual variant.
    void SetVariant(ButtonVariant variant);

    /// @brief Get the current visual variant.
    [[nodiscard]] auto Variant() const -> ButtonVariant;

    /// @brief Set the size preset (changes fixed height).
    void SetSize(ButtonSize size);

    /// @brief Get the current size preset.
    [[nodiscard]] auto Size() const -> ButtonSize;

    /// @brief Minimum size: 56 x height matching old sizeHint.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: rounded rect + text/icon using variant colors.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kMinWidth   = 56;  ///< Minimum button width in px
    static constexpr int kIconSize   = 16;  ///< Icon render size in px
    static constexpr int kIconGap    = 4;   ///< Gap between icon and text
    static constexpr int kHPadding   = 12;  ///< Horizontal content padding

    ButtonVariant _variant = ButtonVariant::Secondary;
    ButtonSize    _size    = ButtonSize::Medium;
    PushButtonEventFilter* _pbFilter = nullptr;
};

} // namespace matcha::gui
