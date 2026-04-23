#pragma once

/**
 * @file NyanLabel.h
 * @brief Theme-aware label with role-based font selection and elide support.
 *
 * Inherits QLabel for Qt label semantics and ThemeAware for design token
 * integration. The font is automatically resolved from `LabelRole` ->
 * `FontRole` mapping via `IThemeService::Font()`.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanLabel.h` (3 LabelState enum)
 * - `old/NyanGuis/Gui/src/NyanLabel.cpp` (paintEvent: Title=Font1+Medium+Bold,
 *   Name=Font1+Medium, Normal=Font2+Normal, default alignment Right|VCenter)
 *
 * @par Visual preservation
 * Font weights and colors must match old paintEvent exactly:
 * - Title  -> Heading  (12pt DemiBold, Foreground1)
 * - Name   -> BodyMedium (9pt Medium, Foreground1)
 * - Body   -> Body     (9pt Normal, Foreground2)
 * - Caption -> Caption  (8pt Normal, Foreground2)
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for FontRole enum.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Tree/FSM/WidgetEnums.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QLabel>

#include <cstdint>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Map LabelRole to FontRole for theme font resolution.
 * @param role Label role.
 * @return Corresponding FontRole.
 */
[[nodiscard]] constexpr auto ToFontRole(LabelRole role) noexcept -> FontRole
{
    switch (role) {
    case LabelRole::Title:   return FontRole::fontLineHeight;
    case LabelRole::Name:    return FontRole::fontWeightMedium;
    case LabelRole::Body:    return FontRole::fontSizeMD;
    case LabelRole::Caption: return FontRole::fontSizeXS;
    default:                 return FontRole::fontSizeMD;
    }
}

/**
 * @brief Theme-aware label with role-based font and elide support.
 *
 * The label automatically picks the correct font from the theme based on
 * its `LabelRole`. Text that overflows the widget width is elided with
 * the configured `Qt::TextElideMode`.
 */
class MATCHA_EXPORT NyanLabel : public QLabel, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a label.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanLabel(QWidget* parent = nullptr);

    /**
     * @brief Construct a label with text.
     * @param theme Theme service reference.
     * @param text  Label display text.
     * @param parent Optional parent widget.
     */
    explicit NyanLabel(const QString& text, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanLabel() override;

    NyanLabel(const NyanLabel&)            = delete;
    NyanLabel& operator=(const NyanLabel&) = delete;
    NyanLabel(NyanLabel&&)                 = delete;
    NyanLabel& operator=(NyanLabel&&)      = delete;

    /// @brief Set the label role (changes font automatically).
    void SetRole(LabelRole role);

    /// @brief Get the current label role.
    [[nodiscard]] auto Role() const -> LabelRole;

    /// @brief Set the text elide mode for overflow.
    void SetElideMode(Qt::TextElideMode mode);

    /// @brief Get the current elide mode.
    [[nodiscard]] auto ElideMode() const -> Qt::TextElideMode;

    /// @brief Set the buddy widget for mnemonic activation.
    /// When the label text contains '&', Alt+key will focus the buddy.
    void SetBuddy(QWidget* buddy);

    /// @brief Get the buddy widget.
    [[nodiscard]] auto Buddy() const -> QWidget*;

    /// @brief Override setText to re-parse mnemonic and re-register.
    void setText(const QString& text);

protected:
    /// @brief Custom paint: role-based font + elide + themed color.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void UpdateMnemonicRegistration();
    void UnregisterMnemonic();

    LabelRole          _role = LabelRole::Body;
    Qt::TextElideMode  _elideMode = Qt::ElideRight;
    QWidget*           _buddy = nullptr;
    uint64_t           _mnemonicId = 0;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
