#pragma once

/**
 * @file NyanInputDialog.h
 * @brief Theme-aware input prompt dialog replacing QInputDialog.
 *
 * Provides static methods that open a themed modal dialog and return
 * `std::optional<T>`, where `std::nullopt` indicates cancellation.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanInputDialog.h`
 * - `old/NyanGuis/Gui/inc/NyanInputDialogPrivate.h` (eliminated)
 *
 * @par Visual specification
 * - Themed dialog chrome: Background1 fill, Border4 border, RadiusToken::Default
 * - Title bar with message label
 * - Input area: NyanLineEdit for text/int, NyanComboBox for item selection
 * - OK/Cancel buttons (NyanPushButton Primary + Secondary)
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QDialog>

#include <optional>
#include <vector>

namespace matcha::gui {

/**
 * @brief Theme-aware input dialog with static convenience methods.
 *
 * Each static method creates a modal dialog, runs exec(), and returns
 * the user input wrapped in optional (nullopt on cancel).
 */
class MATCHA_EXPORT NyanInputDialog : public QDialog, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct an input dialog.
     * @param theme Theme service reference (must outlive this dialog).
     * @param parent Optional parent widget.
     */
    explicit NyanInputDialog(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanInputDialog() override;

    NyanInputDialog(const NyanInputDialog&)            = delete;
    NyanInputDialog& operator=(const NyanInputDialog&) = delete;
    NyanInputDialog(NyanInputDialog&&)                 = delete;
    NyanInputDialog& operator=(NyanInputDialog&&)      = delete;

    /**
     * @brief Prompt for a text string.
     * @param theme Theme service.
     * @param parent Parent widget.
     * @param title Dialog title.
     * @param label Prompt label.
     * @param defaultValue Initial text value.
     * @return User input, or nullopt if cancelled.
     */
    [[nodiscard]] static auto GetText(
        QWidget* parent,
        const QString& title,
        const QString& label,
        const QString& defaultValue = {}) -> std::optional<QString>;

    /**
     * @brief Prompt for an integer.
     * @param theme Theme service.
     * @param parent Parent widget.
     * @param title Dialog title.
     * @param label Prompt label.
     * @param defaultValue Initial value.
     * @param minValue Minimum allowed value.
     * @param maxValue Maximum allowed value.
     * @return User input, or nullopt if cancelled.
     */
    [[nodiscard]] static auto GetInt(
        QWidget* parent,
        const QString& title,
        const QString& label,
        int defaultValue = 0,
        int minValue = -2147483647,
        int maxValue = 2147483647) -> std::optional<int>;

    /**
     * @brief Prompt for item selection from a list.
     * @param theme Theme service.
     * @param parent Parent widget.
     * @param title Dialog title.
     * @param label Prompt label.
     * @param items List of items to choose from.
     * @param currentIndex Initial selection index.
     * @return Selected item text, or nullopt if cancelled.
     */
    [[nodiscard]] static auto GetItem(
        QWidget* parent,
        const QString& title,
        const QString& label,
        const std::vector<QString>& items,
        int currentIndex = 0) -> std::optional<QString>;

protected:
    /// @brief Custom paint: themed dialog chrome.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;
};

} // namespace matcha::gui
