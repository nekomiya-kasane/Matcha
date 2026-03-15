#pragma once

/**
 * @file ComboBoxNode.h
 * @brief Typed WidgetNode wrapping NyanComboBox for UiNode tree.
 *
 * Items are UiNode-based: call AddItemNode() to inject any WidgetNode
 * subtree as a combo item.  AddItem(text) is a convenience alias that
 * creates a LabelNode internally.
 *
 * The underlying QComboBox popup is replaced with a QListWidget so that
 * every item node's real widget is embedded via setItemWidget().
 */

#include "Matcha/Tree/WidgetNode.h"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanComboBox (Scheme D typed node).
 *
 * Dispatches: IndexChanged, TextActivated.
 */
class MATCHA_EXPORT ComboBoxNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using IndexChanged  = matcha::fw::IndexChanged;
        using TextActivated = matcha::fw::TextActivated;
    };

    explicit ComboBoxNode(std::string id);
    ~ComboBoxNode() override;

    ComboBoxNode(const ComboBoxNode&) = delete;
    auto operator=(const ComboBoxNode&) -> ComboBoxNode& = delete;
    ComboBoxNode(ComboBoxNode&&) = delete;
    auto operator=(ComboBoxNode&&) -> ComboBoxNode& = delete;

    // -- UiNode-based Item API --

    /// @brief Add any UiNode subtree as a combo item.
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
    [[nodiscard]] auto CurrentText() const -> std::string;

    void SetEditable(bool editable);
    [[nodiscard]] auto IsEditable() const -> bool;

    void SetPlaceholder(std::string_view text);

    void Clear();

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;

private:
    /// @brief Ordered list of item nodes (ownership held here, not in UiNode child tree).
    std::vector<std::unique_ptr<UiNode>> _itemNodes;

    /// @brief Synchronise a single item node's widget into the popup QListWidget.
    void SyncItemWidget(int index);
};

} // namespace matcha::fw
