#pragma once

/**
 * @file TreeItemNode.h
 * @brief Lightweight tree data node for TreeWidgetNode.
 *
 * TreeItemNode is a pure data structure (not a UiNode/WidgetNode) that
 * represents a single node in a hierarchical tree. TreeWidgetNode owns
 * root TreeItemNodes and internally syncs them to a QStandardItemModel.
 *
 * This keeps Qt types out of the public TreeWidgetNode API entirely.
 */

#include <Matcha/Foundation/Macros.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

/**
 * @brief A single node in the tree data model.
 *
 * Each node holds display text, an optional icon path, and ordered children.
 * Ownership: parent TreeItemNode (or TreeWidgetNode for roots) owns children
 * via unique_ptr. The `_parent` back-pointer is non-owning.
 */
class MATCHA_EXPORT TreeItemNode {
public:
    explicit TreeItemNode(std::string text);
    ~TreeItemNode();

    TreeItemNode(const TreeItemNode&) = delete;
    auto operator=(const TreeItemNode&) -> TreeItemNode& = delete;
    TreeItemNode(TreeItemNode&&) = delete;
    auto operator=(TreeItemNode&&) -> TreeItemNode& = delete;

    // -- Text --

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> const std::string&;

    // -- Icon (optional resource path, not QIcon) --

    void SetIconPath(std::string_view path);
    [[nodiscard]] auto IconPath() const -> const std::string&;

    // -- Children --

    /// @brief Add a child node. Returns raw pointer for chaining.
    auto AddChild(std::unique_ptr<TreeItemNode> child) -> TreeItemNode*;

    /// @brief Insert a child at @p index. Returns raw pointer.
    auto InsertChild(int index, std::unique_ptr<TreeItemNode> child) -> TreeItemNode*;

    /// @brief Remove and destroy child at @p index.
    void RemoveChild(int index);

    /// @brief Number of direct children.
    [[nodiscard]] auto ChildCount() const -> int;

    /// @brief Access child at @p index (nullptr if invalid).
    [[nodiscard]] auto Child(int index) const -> TreeItemNode*;

    /// @brief Non-owning parent pointer (nullptr for root items).
    [[nodiscard]] auto Parent() const -> TreeItemNode*;

    /// @brief Index of this node among its parent's children (-1 if root).
    [[nodiscard]] auto IndexInParent() const -> int;

private:
    std::string _text;
    std::string _iconPath;
    TreeItemNode* _parent = nullptr;
    std::vector<std::unique_ptr<TreeItemNode>> _children;
};

} // namespace matcha::fw
