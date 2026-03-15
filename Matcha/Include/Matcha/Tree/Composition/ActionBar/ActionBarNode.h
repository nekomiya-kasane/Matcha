#pragma once

/**
 * @file ActionBarNode.h
 * @brief UiNode wrapper for NyanActionBar -- ribbon container with tabs.
 *
 * ActionBarNode is a framework-predefined container node wrapping the
 * NyanActionBar widget. It is pre-created as a child of WindowNode.
 *
 * Hierarchy: ActionBarNode -> ActionTabNode -> ActionToolbarNode -> ActionButtonNode
 *
 * @see docs/02_Architecture_Design.md section 2.5.3 (ActionBar)
 */

#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <Matcha/Widgets/ActionBar/NyanActionBar.h>

#include <string>
#include <string_view>

class QWidget;

namespace matcha::fw {

class ActionTabNode;

/**
 * @brief UiNode wrapper for NyanActionBar.
 *
 * Pre-created by WindowNode. Business layer accesses via WindowNode or Shell.
 */
class MATCHA_EXPORT ActionBarNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using TabSwitched      = matcha::fw::TabSwitched;
        using CollapsedChanged = matcha::fw::CollapsedChanged;
    };

    /**
     * @brief Construct an ActionBarNode.
     * @param parentHint UiNode whose Widget() becomes the Qt parent.
     */
    explicit ActionBarNode(UiNode* parentHint = nullptr);
    ~ActionBarNode() override;

    ActionBarNode(const ActionBarNode&) = delete;
    auto operator=(const ActionBarNode&) -> ActionBarNode& = delete;
    ActionBarNode(ActionBarNode&&) = delete;
    auto operator=(ActionBarNode&&) -> ActionBarNode& = delete;

    // -- Widget access --

    /// @brief Get the underlying NyanActionBar widget.
    [[nodiscard]] auto ActionBar() -> gui::NyanActionBar*;

    /// @brief Get the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Imperative CRUD (creates child node + syncs widget in one call) --

    /// @brief Add a tab. Creates ActionTabNode, adds to tree, syncs widget.
    /// @param id Unique node identifier.
    /// @param label Display label for the tab.
    /// @return Non-owning pointer to the new tab node.
    auto AddTab(std::string id, std::string_view label) -> ActionTabNode*;

    /// @brief Remove a tab by ID.
    void RemoveTab(std::string_view tabId);

    /// @brief Find a tab by ID.
    /// @return Non-owning pointer, or nullptr if not found.
    [[nodiscard]] auto FindTab(std::string_view tabId) -> ActionTabNode*;

    // -- Declarative path --
    // External code can also construct ActionTabNode independently and call:
    //   actionBar->AddNode(std::move(tabNode));
    // AddNode override validates type and syncs the widget.
    // Rejects non-ActionTabNode children (assert in debug).

    // -- Tab query --

    /// @brief Get the current tab ID.
    [[nodiscard]] auto CurrentTabId() const -> std::string;

    /// @brief Switch to a tab by ID.
    void SwitchTab(std::string_view tabId);

    /// @brief Check if a tab exists.
    [[nodiscard]] auto HasTab(std::string_view tabId) const -> bool;

    /// @brief Get tab count.
    [[nodiscard]] auto TabCount() const -> int;

    // -- Dock & Collapse --

    /// @brief Set the dock side (Bottom/Left/Right/Top).
    void SetDockSide(gui::DockSide side);

    /// @brief Get the current dock side.
    [[nodiscard]] auto GetDockSide() const -> gui::DockSide;

    /// @brief Set whether the ActionBar is docked to a WorkspaceFrame edge.
    void SetDocked(bool docked);

    /// @brief Whether the ActionBar is currently docked.
    [[nodiscard]] auto IsDocked() const -> bool;

    /// @brief Set the collapsed state. Behavior depends on docked state:
    /// - Docked + collapsed: ActionBar hidden, trapezoid handle shown on edge.
    /// - Undocked + collapsed: ActionBar hidden, mini-button (64x16) shown.
    void SetCollapsed(bool collapsed);

    /// @brief Whether the ActionBar is collapsed.
    [[nodiscard]] auto IsCollapsed() const -> bool;

    // -- Tree overrides (type validation + widget sync) --
    auto AddNode(std::unique_ptr<UiNode> child) -> UiNode* override;
    auto RemoveNode(UiNode* child) -> std::unique_ptr<UiNode> override;

private:
    gui::NyanActionBar* _actionBar;
};

} // namespace matcha::fw
