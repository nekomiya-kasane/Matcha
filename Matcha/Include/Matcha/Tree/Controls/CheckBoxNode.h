#pragma once

/**
 * @file CheckBoxNode.h
 * @brief Typed WidgetNode wrapping NyanCheckBox for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanCheckBox (Scheme D typed node).
 *
 * Dispatches: WidgetToggled (on toggle).
 */
class MATCHA_EXPORT CheckBoxNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Toggled = matcha::fw::Toggled;
        using Clicked = matcha::fw::Clicked;
    };

    explicit CheckBoxNode(std::string id);
    ~CheckBoxNode() override;

    CheckBoxNode(const CheckBoxNode&) = delete;
    auto operator=(const CheckBoxNode&) -> CheckBoxNode& = delete;
    CheckBoxNode(CheckBoxNode&&) = delete;
    auto operator=(CheckBoxNode&&) -> CheckBoxNode& = delete;

    void SetChecked(bool checked);
    [[nodiscard]] auto IsChecked() const -> bool;
    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
