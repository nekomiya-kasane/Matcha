#pragma once

/**
 * @file TreeWidgetNode.h
 * @brief Typed WidgetNode wrapping NyanStructureTree for UiNode tree.
 *
 * Tree data is provided via TreeItemNode objects — no Qt types leak
 * through the public API. Internally a QStandardItemModel is built
 * and kept in sync with the TreeItemNode tree.
 */

#include "Matcha/UiNodes/Controls/TreeItemNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <memory>
#include <string>
#include <vector>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanStructureTree (Scheme D typed node).
 *
 * Data model is entirely described by TreeItemNode objects.
 * No Qt types appear in the public header.
 */
class MATCHA_EXPORT TreeWidgetNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using SelectionChanged    = matcha::fw::SelectionChanged;
        using ItemDoubleClicked   = matcha::fw::ItemDoubleClicked;
        using CollapsedChanged    = matcha::fw::CollapsedChanged;
        using ContextMenuRequest   = matcha::fw::ContextMenuRequest;
    };

    explicit TreeWidgetNode(std::string id);
    ~TreeWidgetNode() override;

    TreeWidgetNode(const TreeWidgetNode&) = delete;
    auto operator=(const TreeWidgetNode&) -> TreeWidgetNode& = delete;
    TreeWidgetNode(TreeWidgetNode&&) = delete;
    auto operator=(TreeWidgetNode&&) -> TreeWidgetNode& = delete;

    // -- Tree data API (replaces SetModel) --

    /// @brief Add a root-level tree item. Returns raw pointer for chaining.
    auto AddRootItem(std::unique_ptr<TreeItemNode> item) -> TreeItemNode*;

    /// @brief Remove and destroy root item at @p index.
    void RemoveRootItem(int index);

    /// @brief Access root item at @p index (nullptr if invalid).
    [[nodiscard]] auto RootItem(int index) const -> TreeItemNode*;

    /// @brief Number of root-level items.
    [[nodiscard]] auto RootItemCount() const -> int;

    /// @brief Remove all root items and clear the internal model.
    void Clear();

    /// @brief Set the column header label (single-column tree).
    void SetHeaderLabel(std::string_view label);

    /// @brief Rebuild the internal QStandardItemModel from the TreeItemNode tree.
    /// Called automatically by AddRootItem/RemoveRootItem/Clear.
    /// Call explicitly after mutating TreeItemNode children directly.
    void SyncModel();

    // -- Selection --

    /// @brief Get the path from root to selected node as a vector of child indices.
    /// Empty if no selection. E.g. {0, 2} = first root's third child.
    [[nodiscard]] auto SelectedPath() const -> std::vector<int>;

    // -- Display --

    void SetTitle(std::string_view title);
    [[nodiscard]] auto Title() const -> std::string;

    void ExpandAll();
    void CollapseAll();

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;

private:
    std::vector<std::unique_ptr<TreeItemNode>> _rootItems;
    std::string _headerLabel;

    void RebuildModel();
};

} // namespace matcha::fw
