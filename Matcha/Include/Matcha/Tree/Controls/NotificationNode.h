#pragma once

/**
 * @file NotificationNode.h
 * @brief Typed WidgetNode wrapping NyanNotification for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanNotification (Scheme D typed node).
 *
 * Toast notification popup with slide-in animation and auto-dismiss.
 * Supports semantic types (Info/Success/Warning/Error), optional action
 * button, and configurable duration.
 *
 * Dispatches: Dismissed (auto or manual), ActionClicked (action button).
 */
class MATCHA_EXPORT NotificationNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Dismissed     = matcha::fw::Dismissed;
        using ActionClicked = matcha::fw::ActionClicked;
    };

    explicit NotificationNode(std::string id);
    ~NotificationNode() override;

    NotificationNode(const NotificationNode&) = delete;
    auto operator=(const NotificationNode&) -> NotificationNode& = delete;
    NotificationNode(NotificationNode&&) = delete;
    auto operator=(NotificationNode&&) -> NotificationNode& = delete;

    void SetMessage(std::string_view message);
    [[nodiscard]] auto Message() const -> std::string;

    void SetType(uint8_t type);
    [[nodiscard]] auto Type() const -> uint8_t;

    void SetDurationMs(int ms);
    [[nodiscard]] auto DurationMs() const -> int;

    void SetAction(std::string_view text);
    void ClearAction();

    void ShowAt(int globalX, int globalY);
    void Dismiss();

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
