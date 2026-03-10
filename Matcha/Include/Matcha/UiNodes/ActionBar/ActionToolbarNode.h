#pragma once

/**
 * @file ActionToolbarNode.h
 * @brief UiNode wrapper for NyanActionToolbar -- button group within ActionTab.
 *
 * @see ActionTabNode.h
 */

#include "Matcha/UiNodes/Core/UiNode.h"

namespace matcha::gui {
class IThemeService;
class NyanActionToolbar;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

class ActionButtonNode;

/**
 * @brief UiNode wrapper for NyanActionToolbar. Child of ActionTabNode.
 */
class MATCHA_EXPORT ActionToolbarNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    ActionToolbarNode(std::string id, UiNode* parentHint = nullptr);
    ~ActionToolbarNode() override;

    ActionToolbarNode(const ActionToolbarNode&) = delete;
    auto operator=(const ActionToolbarNode&) -> ActionToolbarNode& = delete;
    ActionToolbarNode(ActionToolbarNode&&) = delete;
    auto operator=(ActionToolbarNode&&) -> ActionToolbarNode& = delete;

    [[nodiscard]] auto Toolbar() -> gui::NyanActionToolbar*;
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Imperative CRUD (creates child node + syncs widget in one call) --

    /// @brief Add a button. Creates ActionButtonNode, adds to tree, syncs widget.
    auto AddButton(std::string id, std::string_view icon,
                   std::string_view tooltip = "") -> ActionButtonNode*;

    /// @brief Remove a button by its node ID.
    void RemoveButton(std::string_view buttonId);

    /// @brief Find a button by ID.
    [[nodiscard]] auto FindButton(std::string_view buttonId) -> ActionButtonNode*;

    [[nodiscard]] auto ButtonCount() const -> int;

    // -- Declarative path --
    // External code can also construct ActionButtonNode independently and call:
    //   toolbar->AddNode(std::move(buttonNode));
    // AddNode override validates type and syncs the widget.
    // Rejects non-ActionButtonNode children (assert in debug).

    // -- Positional query --

    /// @brief Get this toolbar's index within its parent ActionTabNode.
    /// @return 0-based index, or -1 if not attached.
    [[nodiscard]] auto Index() const -> int;

    // -- Tree overrides (type validation + widget sync) --
    auto AddNode(std::unique_ptr<UiNode> child) -> UiNode* override;
    auto RemoveNode(UiNode* child) -> std::unique_ptr<UiNode> override;

private:
    gui::NyanActionToolbar* _toolbar;
};

} // namespace matcha::fw
