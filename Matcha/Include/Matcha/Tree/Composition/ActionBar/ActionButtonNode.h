#pragma once

/**
 * @file ActionButtonNode.h
 * @brief UiNode wrapper for a single action button within ActionToolbar.
 *
 * @see ActionToolbarNode.h
 */

#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <string>
#include <string_view>

namespace matcha::gui {
class IThemeService;
class NyanToolButton;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

/**
 * @brief UiNode wrapper for a single NyanToolButton. Child of ActionToolbarNode.
 */
class MATCHA_EXPORT ActionButtonNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using Clicked = matcha::fw::ButtonClicked;
    };

    ActionButtonNode(std::string id, gui::NyanToolButton* button);
    ~ActionButtonNode() override;

    ActionButtonNode(const ActionButtonNode&) = delete;
    auto operator=(const ActionButtonNode&) -> ActionButtonNode& = delete;
    ActionButtonNode(ActionButtonNode&&) = delete;
    auto operator=(ActionButtonNode&&) -> ActionButtonNode& = delete;

    [[nodiscard]] auto Button() -> gui::NyanToolButton*;
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- State --

    void SetEnabled(bool enabled);
    [[nodiscard]] auto IsEnabled() const -> bool;

    void SetCheckable(bool checkable);
    [[nodiscard]] auto IsCheckable() const -> bool;

    void SetChecked(bool checked);
    [[nodiscard]] auto IsChecked() const -> bool;

    void SetText(std::string_view text);
    void SetToolTip(std::string_view tip);

    void SetIcon(std::string_view iconId, fw::IconSize size = fw::IconSize::Sm);

    // -- Positional query --

    /// @brief Get this button's index within its parent ActionToolbarNode.
    /// @return 0-based index, or -1 if not attached.
    [[nodiscard]] auto Index() const -> int;

private:
    gui::NyanToolButton* _button;
};

} // namespace matcha::fw
