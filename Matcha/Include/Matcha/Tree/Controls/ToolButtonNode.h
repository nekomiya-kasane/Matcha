#pragma once

/**
 * @file ToolButtonNode.h
 * @brief Typed WidgetNode wrapping NyanToolButton for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanToolButton (Scheme D typed node).
 *
 * Dispatches: WidgetActivated (on click).
 */
class MATCHA_EXPORT ToolButtonNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Activated     = matcha::fw::Activated;
        using RightClicked  = matcha::fw::RightClicked;
    };

    explicit ToolButtonNode(std::string id);
    ~ToolButtonNode() override;

    ToolButtonNode(const ToolButtonNode&) = delete;
    auto operator=(const ToolButtonNode&) -> ToolButtonNode& = delete;
    ToolButtonNode(ToolButtonNode&&) = delete;
    auto operator=(ToolButtonNode&&) -> ToolButtonNode& = delete;

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

    void SetCheckable(bool checkable);
    [[nodiscard]] auto IsCheckable() const -> bool;

    void SetChecked(bool checked);
    [[nodiscard]] auto IsChecked() const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
    void OnIconChanged() override;
};

} // namespace matcha::fw
