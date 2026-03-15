#pragma once

/**
 * @file RadioButtonNode.h
 * @brief Typed WidgetNode wrapping NyanRadioButton for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanRadioButton (Scheme D typed node).
 *
 * Dispatches: WidgetToggled (on toggle).
 */
class MATCHA_EXPORT RadioButtonNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Toggled = matcha::fw::Toggled;
        using Clicked = matcha::fw::Clicked;
    };

    explicit RadioButtonNode(std::string id);
    ~RadioButtonNode() override;

    RadioButtonNode(const RadioButtonNode&) = delete;
    auto operator=(const RadioButtonNode&) -> RadioButtonNode& = delete;
    RadioButtonNode(RadioButtonNode&&) = delete;
    auto operator=(RadioButtonNode&&) -> RadioButtonNode& = delete;

    void SetChecked(bool checked);
    [[nodiscard]] auto IsChecked() const -> bool;
    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
