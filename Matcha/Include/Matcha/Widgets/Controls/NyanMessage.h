#pragma once

/**
 * @file NyanMessage.h
 * @brief Theme-aware inline message bar (info/warning/error).
 *
 * Colored background bar with icon + text + optional action button
 * + optional close. Used in dialog panels, form validation feedback.
 *
 * @see 05_Greenfield_Plan.md ss 3.4, widget #66.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <functional>
#include <string>
#include <string_view>

class QLabel;
class QPushButton;

namespace matcha::gui {

class SimpleWidgetEventFilter;

/** @brief Message bar type. */
enum class MessageType : uint8_t {
    Info,
    Success,
    Warning,
    Error,
};

/**
 * @brief Theme-aware inline message bar.
 *
 * A11y role: Alert.
 */
class MATCHA_EXPORT NyanMessage : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanMessage(QWidget* parent = nullptr);
    ~NyanMessage() override;

    NyanMessage(const NyanMessage&)            = delete;
    NyanMessage& operator=(const NyanMessage&) = delete;
    NyanMessage(NyanMessage&&)                 = delete;
    NyanMessage& operator=(NyanMessage&&)      = delete;

    /// @brief Set the message type (affects color scheme and icon).
    void SetType(MessageType type);

    /// @brief Get the current message type.
    [[nodiscard]] auto Type() const -> MessageType { return _type; }

    /// @brief Set the message text.
    void SetText(std::string_view text);

    /// @brief Get the message text.
    [[nodiscard]] auto Text() const -> std::string;

    /// @brief Set whether the message can be dismissed.
    void SetClosable(bool closable);

    /// @brief Whether the message is closable.
    [[nodiscard]] auto IsClosable() const -> bool { return _closable; }

    /// @brief Set an action button with callback.
    void SetAction(std::string_view text, std::function<void()> callback);

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the close button is clicked.
    void Closed();

    /// @brief Emitted when the action button is clicked.
    void ActionClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    void SetupUi();
    void UpdateStyle();

    MessageType _type = MessageType::Info;
    bool _closable = false;

    QLabel* _iconLabel = nullptr;
    QLabel* _textLabel = nullptr;
    QPushButton* _actionBtn = nullptr;
    QPushButton* _closeBtn = nullptr;
    std::function<void()> _actionCallback;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
