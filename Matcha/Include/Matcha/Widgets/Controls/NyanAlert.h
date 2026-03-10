#pragma once

/**
 * @file NyanAlert.h
 * @brief Theme-aware blocking alert dialog (info/warning/error/confirm).
 *
 * Replaces QMessageBox. Static methods for common dialog patterns.
 * Themed to match application.
 *
 * @see 05_Greenfield_Plan.md ss 3.4, widget #67.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QDialog>

#include <cstdint>
#include <string_view>

class QLabel;
class QPushButton;

namespace matcha::gui {

class InteractionEventFilter;

/** @brief Alert dialog type. */
enum class AlertType : uint8_t {
    Info,
    Warning,
    Error,
    Confirm,
};

/** @brief Alert button result. */
enum class AlertButton : uint8_t {
    Ok,
    Cancel,
    Yes,
    No,
};

/**
 * @brief Theme-aware blocking alert dialog.
 *
 * A11y role: AlertDialog.
 */
class MATCHA_EXPORT NyanAlert : public QDialog, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanAlert(QWidget* parent = nullptr);
    ~NyanAlert() override;

    NyanAlert(const NyanAlert&)            = delete;
    NyanAlert& operator=(const NyanAlert&) = delete;
    NyanAlert(NyanAlert&&)                 = delete;
    NyanAlert& operator=(NyanAlert&&)      = delete;

    /// @brief Show a blocking alert dialog. Returns the button pressed.
    static auto Show(AlertType type,
                     std::string_view title, std::string_view message,
                     QWidget* parent = nullptr) -> AlertButton;

    /// @brief Show a confirm dialog (Yes/No). Returns true if Yes.
    static auto Confirm(
                        std::string_view title, std::string_view message,
                        QWidget* parent = nullptr) -> bool;

    /// @brief Set the alert type.
    void SetAlertType(AlertType type);

    /// @brief Set the title text.
    void SetTitle(std::string_view title);

    /// @brief Set the message text.
    void SetMessage(std::string_view message);

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    void SetupUi();
    void UpdateStyle();

    AlertType _alertType = AlertType::Info;

    QLabel* _iconLabel = nullptr;
    QLabel* _titleLabel = nullptr;
    QLabel* _messageLabel = nullptr;
    QPushButton* _okBtn = nullptr;
    QPushButton* _cancelBtn = nullptr;
    QPushButton* _yesBtn = nullptr;
    QPushButton* _noBtn = nullptr;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
