#pragma once

/**
 * @file NyanDialogFootBar.h
 * @brief Dialog footer bar with Confirm/Apply/Cancel buttons.
 *
 * NyanDialogFootBar provides:
 * - Confirm, Apply, Cancel buttons (configurable visibility/text)
 * - Custom widget area
 * - Themed styling
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

class QHBoxLayout;
class QPushButton;

namespace matcha::gui {

/**
 * @brief Dialog button code for signal identification.
 */
enum class DialogButtonCode : uint8_t {
    Confirm,
    Apply,
    Cancel
};

/**
 * @brief Dialog footer bar with Confirm/Apply/Cancel buttons.
 *
 * A11y role: ToolBar.
 */
class MATCHA_EXPORT NyanDialogFootBar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a dialog footer bar.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanDialogFootBar(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanDialogFootBar() override;

    NyanDialogFootBar(const NyanDialogFootBar&)            = delete;
    NyanDialogFootBar& operator=(const NyanDialogFootBar&) = delete;
    NyanDialogFootBar(NyanDialogFootBar&&)                 = delete;
    NyanDialogFootBar& operator=(NyanDialogFootBar&&)      = delete;

    // -- Confirm button --

    /// @brief Set confirm button text.
    void SetConfirmText(const QString& text);

    /// @brief Get confirm button text.
    [[nodiscard]] auto ConfirmText() const -> QString;

    /// @brief Set confirm button visible.
    void SetConfirmVisible(bool visible);

    /// @brief Check if confirm button is visible.
    [[nodiscard]] auto IsConfirmVisible() const -> bool;

    /// @brief Set confirm button enabled.
    void SetConfirmEnabled(bool enabled);

    /// @brief Check if confirm button is enabled.
    [[nodiscard]] auto IsConfirmEnabled() const -> bool;

    // -- Apply button --

    /// @brief Set apply button text.
    void SetApplyText(const QString& text);

    /// @brief Get apply button text.
    [[nodiscard]] auto ApplyText() const -> QString;

    /// @brief Set apply button visible.
    void SetApplyVisible(bool visible);

    /// @brief Check if apply button is visible.
    [[nodiscard]] auto IsApplyVisible() const -> bool;

    /// @brief Set apply button enabled.
    void SetApplyEnabled(bool enabled);

    /// @brief Check if apply button is enabled.
    [[nodiscard]] auto IsApplyEnabled() const -> bool;

    // -- Cancel button --

    /// @brief Set cancel button text.
    void SetCancelText(const QString& text);

    /// @brief Get cancel button text.
    [[nodiscard]] auto CancelText() const -> QString;

    /// @brief Set cancel button visible.
    void SetCancelVisible(bool visible);

    /// @brief Check if cancel button is visible.
    [[nodiscard]] auto IsCancelVisible() const -> bool;

    /// @brief Set cancel button enabled.
    void SetCancelEnabled(bool enabled);

    /// @brief Check if cancel button is enabled.
    [[nodiscard]] auto IsCancelEnabled() const -> bool;

    // -- Custom widget --

    /// @brief Set custom widget (left side of buttons).
    void SetCustomWidget(QWidget* widget);

    /// @brief Get custom widget.
    [[nodiscard]] auto CustomWidget() const -> QWidget*;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when confirm button is clicked.
    void ConfirmClicked();

    /// @brief Emitted when apply button is clicked.
    void ApplyClicked();

    /// @brief Emitted when cancel button is clicked.
    void CancelClicked();

    /// @brief Emitted when any button is clicked.
    void ButtonClicked(DialogButtonCode code);

protected:
    /// @brief Custom paint for themed footer bar.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void UpdateButtonStyles();

    static constexpr int kHeight      = 48;
    static constexpr int kButtonWidth = 80;

    QHBoxLayout*   _layout         = nullptr;
    QWidget*       _customWidget   = nullptr;
    QPushButton*   _confirmButton  = nullptr;
    QPushButton*   _applyButton    = nullptr;
    QPushButton*   _cancelButton   = nullptr;
};

} // namespace matcha::gui
