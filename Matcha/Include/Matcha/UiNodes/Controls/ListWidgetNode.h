#pragma once

/**
 * @file ListWidgetNode.h
 * @brief Typed WidgetNode wrapping NyanListWidget for UiNode tree.
 *
 * Items are UiNode-based: call AddItemNode() to inject any WidgetNode
 * subtree as a list item.  AddItem(text) is a convenience alias that
 * creates a LabelNode internally.  Each item's real widget is embedded
 * into the QListWidget row via setItemWidget().
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanListWidget (Scheme D typed node).
 *
 * Dispatches: IndexChanged, ItemDoubleClicked.
 */
class MATCHA_EXPORT ListWidgetNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using IndexChanged      = matcha::fw::IndexChanged;
        using ItemDoubleClicked = matcha::fw::ItemDoubleClicked;
    };

    explicit ListWidgetNode(std::string id);
    ~ListWidgetNode() override;

    ListWidgetNode(const ListWidgetNode&) = delete;
    auto operator=(const ListWidgetNode&) -> ListWidgetNode& = delete;
    ListWidgetNode(ListWidgetNode&&) = delete;
    auto operator=(ListWidgetNode&&) -> ListWidgetNode& = delete;

    // -- UiNode-based Item API --

    /// @brief Add any UiNode subtree as a list item.
    void AddItemNode(std::unique_ptr<UiNode> node);

    /// @brief Insert a UiNode item at a specific position.
    void InsertItemNode(int index, std::unique_ptr<UiNode> node);

    /// @brief Convenience: create a LabelNode with @p text and add it.
    void AddItem(std::string_view text);

    /// @brief Convenience: add multiple text items.
    void AddItems(std::span<const std::string> items);

    // -- Item access / mutation --

    /// @brief Access the item UiNode at @p index (nullptr if invalid).
    [[nodiscard]] auto ItemNode(int index) const -> UiNode*;

    void RemoveItem(int index);
    [[nodiscard]] auto ItemCount() const -> int;

    // -- Selection --

    void SetCurrentIndex(int index);
    [[nodiscard]] auto CurrentIndex() const -> int;

    void Clear();

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;

private:
    /// @brief Ordered list of item nodes (ownership held here).
    std::vector<std::unique_ptr<UiNode>> _itemNodes;

    /// @brief Synchronise a single item node's widget into the QListWidget row.
    void SyncItemWidget(int index);
};

} // namespace matcha::fw
