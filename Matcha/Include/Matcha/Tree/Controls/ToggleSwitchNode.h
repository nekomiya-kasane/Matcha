#pragma once

/**
 * @file ToggleSwitchNode.h
 * @brief Typed WidgetNode wrapping NyanToggleSwitch for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanToggleSwitch (Scheme D typed node).
 *
 * Dispatches: WidgetToggled (on toggle).
 */
class MATCHA_EXPORT ToggleSwitchNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Toggled = matcha::fw::Toggled;
    };

    explicit ToggleSwitchNode(std::string id);
    ~ToggleSwitchNode() override;

    ToggleSwitchNode(const ToggleSwitchNode&) = delete;
    auto operator=(const ToggleSwitchNode&) -> ToggleSwitchNode& = delete;
    ToggleSwitchNode(ToggleSwitchNode&&) = delete;
    auto operator=(ToggleSwitchNode&&) -> ToggleSwitchNode& = delete;

    void SetChecked(bool checked);
    [[nodiscard]] auto IsChecked() const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
