#pragma once

/**
 * @file MessageNode.h
 * @brief Typed WidgetNode wrapping NyanMessage for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanMessage (Scheme D typed node).
 *
 * Inline message bar with semantic type (Info/Success/Warning/Error),
 * optional close button, and optional action button.
 *
 * Dispatches: CloseRequested (close clicked), ActionClicked (action clicked).
 */
class MATCHA_EXPORT MessageNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Closed        = matcha::fw::CloseRequested;
        using ActionClicked = matcha::fw::ActionClicked;
    };

    explicit MessageNode(std::string id);
    ~MessageNode() override;

    MessageNode(const MessageNode&) = delete;
    auto operator=(const MessageNode&) -> MessageNode& = delete;
    MessageNode(MessageNode&&) = delete;
    auto operator=(MessageNode&&) -> MessageNode& = delete;

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

    void SetType(uint8_t type);
    [[nodiscard]] auto Type() const -> uint8_t;

    void SetClosable(bool closable);
    [[nodiscard]] auto IsClosable() const -> bool;

    void SetAction(std::string_view text);

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
