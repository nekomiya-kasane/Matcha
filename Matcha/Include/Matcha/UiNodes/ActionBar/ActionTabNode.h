#pragma once

/**
 * @file ActionTabNode.h
 * @brief UiNode wrapper for NyanActionTab -- tab within ActionBar.
 *
 * @see ActionBarNode.h
 */

#include "Matcha/UiNodes/Core/UiNode.h"

namespace matcha::gui {
class IThemeService;
class NyanActionTab;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

class ActionToolbarNode;

/**
 * @brief UiNode wrapper for NyanActionTab. Child of ActionBarNode.
 */
class MATCHA_EXPORT ActionTabNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    ActionTabNode(std::string id, UiNode* parentHint = nullptr);
    ~ActionTabNode() override;

    ActionTabNode(const ActionTabNode&) = delete;
    auto operator=(const ActionTabNode&) -> ActionTabNode& = delete;
    ActionTabNode(ActionTabNode&&) = delete;
    auto operator=(ActionTabNode&&) -> ActionTabNode& = delete;

    [[nodiscard]] auto Tab() -> gui::NyanActionTab*;
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Imperative CRUD (creates child node + syncs widget in one call) --

    /// @brief Add a toolbar. Creates ActionToolbarNode, adds to tree, syncs widget.
    auto AddToolbar(std::string id, std::string_view label) -> ActionToolbarNode*;

    /// @brief Remove a toolbar by ID.
    void RemoveToolbar(std::string_view toolbarId);

    /// @brief Find a toolbar by ID.
    [[nodiscard]] auto FindToolbar(std::string_view toolbarId) -> ActionToolbarNode*;

    [[nodiscard]] auto ToolbarCount() const -> int;

    // -- Declarative path --
    // External code can also construct ActionToolbarNode independently and call:
    //   tab->AddNode(std::move(toolbarNode));
    // AddNode override validates type and syncs the widget.
    // Rejects non-ActionToolbarNode children (assert in debug).

    // -- Tree overrides (type validation + widget sync) --
    auto AddNode(std::unique_ptr<UiNode> child) -> UiNode* override;
    auto RemoveNode(UiNode* child) -> std::unique_ptr<UiNode> override;

private:
    gui::NyanActionTab* _tab;
};

} // namespace matcha::fw
