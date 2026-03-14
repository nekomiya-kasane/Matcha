#pragma once

/**
 * @file NyanComboBox.h
 * @brief Theme-aware combo box with custom popup styling and animated arrow.
 *
 * Inherits QComboBox for Qt combo semantics and ThemeAware for design token
 * integration. The combo frame, arrow indicator, and popup items are
 * custom-painted to match the Matcha visual language.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanComboBox.h` (QComboBox base, 2 ComboBoxState)
 * - `old/NyanGuis/Gui/inc/NyanComboBoxStyle.h` (QProxyStyle: drawComplexControl,
 *   drawControl, ItemHeight=28, ArrowRotate animation, ShadowWidth=6,
 *   Border=Line4, Background=Background2, Focus border=PrimaryClick,
 *   Item selection=PrimaryLight/PrimaryFocus, Text=Font1)
 * - `old/NyanGuis/Gui/src/NyanComboBox.cpp` (showPopup/hidePopup with animation,
 *   sizeHint min 140x24)
 *
 * @par Visual preservation
 * Combo frame must use Border4 normal / PrimaryPressed focus border,
 * Background2 fill, Radius=3, min size 140x24. Popup item height 28px.
 * Arrow icon 16x16 rotates on popup show/hide.
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values used in painting.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QComboBox>

namespace matcha::gui {

class ComboBoxEventFilter;

/**
 * @brief Theme-aware combo box with custom-painted frame and popup.
 *
 * Features:
 * - Custom-painted combo frame with themed border/background
 * - Animated arrow rotation on popup show/hide
 * - Popup items with themed selection highlight
 * - Placeholder text support
 * - showPopup/hidePopup overrides for animation control
 */
class MATCHA_EXPORT NyanComboBox : public QComboBox, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a combo box.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanComboBox(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanComboBox() override;

    NyanComboBox(const NyanComboBox&)            = delete;
    NyanComboBox& operator=(const NyanComboBox&) = delete;
    NyanComboBox(NyanComboBox&&)                 = delete;
    NyanComboBox& operator=(NyanComboBox&&)      = delete;

    /// @brief Set placeholder text for empty selection.
    void SetPlaceholder(const QString& text);

    /// @brief Get the placeholder text.
    [[nodiscard]] auto Placeholder() const -> QString;

    /// @brief Enable or disable search/filter mode.
    /// When enabled, typing in the combo filters visible items.
    void SetSearchEnabled(bool enabled);

    /// @brief Whether search/filter mode is enabled.
    [[nodiscard]] auto IsSearchEnabled() const -> bool;

    /// @brief Enable or disable editable mode.
    /// When enabled, user can type free-form text (not just select from list).
    void SetEditableMode(bool editable);

    /// @brief Whether editable mode is enabled.
    [[nodiscard]] auto IsEditableMode() const -> bool;

    /// @brief Animated popup show override.
    void showPopup() override;

    /// @brief Animated popup hide override.
    void hidePopup() override;

    /// @brief Minimum size: 140x24 matching old sizeHint.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: combo frame + arrow using design tokens.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kMinWidth      = 140; ///< Minimum combo width in px
    static constexpr int kFixedHeight   = 24;  ///< Fixed combo height in px
    static constexpr int kArrowSize     = 16;  ///< Arrow icon area size in px
    static constexpr int kArrowMargin   = 20;  ///< Right margin for arrow area
    static constexpr int kTextPadding   = 8;   ///< Left text padding in px
    static constexpr int kItemHeight    = 28;  ///< Popup item height in px

    QString _placeholder;              ///< Placeholder text
    qreal   _arrowAngle = 0.0;         ///< Current arrow rotation angle
    bool    _popupVisible = false;      ///< Popup state for hide guard
    bool    _searchEnabled = false;     ///< Search/filter mode
    bool    _editableMode = false;      ///< Free-form editable mode
    ComboBoxEventFilter* _cbFilter = nullptr;
};

} // namespace matcha::gui
