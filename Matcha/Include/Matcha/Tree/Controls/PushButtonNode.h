#pragma once

/**
 * @file PushButtonNode.h
 * @brief Typed WidgetNode wrapping NyanPushButton for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>
#include <string_view>

#include "Matcha/Tree/FSM/WidgetEnums.h"

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanPushButton (Scheme D typed node).
 *
 * Dispatches: WidgetActivated (on click).
 */
class MATCHA_EXPORT PushButtonNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Activated = matcha::fw::Activated;
        using Pressed   = matcha::fw::Pressed;
        using Released  = matcha::fw::Released;
    };

    explicit PushButtonNode(std::string id);
    ~PushButtonNode() override;

    PushButtonNode(const PushButtonNode&) = delete;
    auto operator=(const PushButtonNode&) -> PushButtonNode& = delete;
    PushButtonNode(PushButtonNode&&) = delete;
    auto operator=(PushButtonNode&&) -> PushButtonNode& = delete;

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

    void SetVariant(gui::ButtonVariant variant);
    [[nodiscard]] auto Variant() const -> gui::ButtonVariant;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
    void OnIconChanged() override;
};

} // namespace matcha::fw
