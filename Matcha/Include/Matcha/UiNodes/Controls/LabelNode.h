#pragma once

/**
 * @file LabelNode.h
 * @brief Typed WidgetNode wrapping NyanLabel for UiNode tree.
 */

#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <string>
#include <string_view>

#include "Matcha/Foundation/WidgetEnums.h"

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanLabel (Scheme D typed node).
 *
 * Dispatches: LinkActivated (on rich-text link click).
 */
class MATCHA_EXPORT LabelNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using LinkActivated = matcha::fw::LinkActivated;
    };

    explicit LabelNode(std::string id);
    ~LabelNode() override;

    LabelNode(const LabelNode&) = delete;
    auto operator=(const LabelNode&) -> LabelNode& = delete;
    LabelNode(LabelNode&&) = delete;
    auto operator=(LabelNode&&) -> LabelNode& = delete;

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

    void SetRole(gui::LabelRole role);
    [[nodiscard]] auto Role() const -> gui::LabelRole;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
