#pragma once

/**
 * @file StatusBarNode.h
 * @brief UiNode wrapper for item-based NyanStatusBar.
 *
 * StatusBarNode is a container node. Callers add items via AddItem(),
 * which creates a child StatusItemNode and inserts a widget into the bar.
 *
 * @see NyanStatusBar for the widget layer.
 */

#include "Matcha/Tree/UiNode.h"

#include <string_view>

namespace matcha::gui {
class NyanStatusBar;
enum class StatusBarSide : uint8_t;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

class StatusItemNode;

/**
 * @brief UiNode wrapper for item-based NyanStatusBar.
 *
 * Pre-created by WindowNode. Items are added as StatusItemNode children.
 */
class MATCHA_EXPORT StatusBarNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    explicit StatusBarNode(UiNode* parentHint = nullptr);
    ~StatusBarNode() override;

    StatusBarNode(const StatusBarNode&) = delete;
    auto operator=(const StatusBarNode&) -> StatusBarNode& = delete;
    StatusBarNode(StatusBarNode&&) = delete;
    auto operator=(StatusBarNode&&) -> StatusBarNode& = delete;

    /// @brief Get the underlying NyanStatusBar.
    [[nodiscard]] auto StatusBar() -> gui::NyanStatusBar*;

    /// @brief Get the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Item management --

    /// @brief Add a label item.
    /// @param id Unique item id.
    /// @param text Initial text.
    /// @param side Left or Right alignment.
    /// @return Non-owning pointer, or nullptr if id conflict.
    auto AddLabel(std::string_view id, std::string_view text,
                  gui::StatusBarSide side) -> StatusItemNode*;

    /// @brief Add a progress item.
    /// @param id Unique item id.
    /// @param side Left or Right alignment.
    /// @return Non-owning pointer, or nullptr if id conflict.
    auto AddProgress(std::string_view id,
                     gui::StatusBarSide side) -> StatusItemNode*;

    /// @brief Add a custom widget item (escape hatch).
    /// @param id Unique item id.
    /// @param widget The widget (ownership transferred to status bar).
    /// @param side Left or Right alignment.
    /// @return Non-owning pointer, or nullptr if id conflict.
    /// @note This accepts a raw QWidget* as an intentional escape hatch
    ///       for embedding arbitrary widgets. Prefer AddLabel/AddProgress
    ///       when a typed API exists.
    auto AddWidget(std::string_view id, QWidget* widget,
                   gui::StatusBarSide side) -> StatusItemNode*;

    /// @brief Remove an item by id. Removes the child node and the widget.
    /// @return true if found and removed.
    auto RemoveItem(std::string_view id) -> bool;

    /// @brief Find a StatusItemNode by id.
    [[nodiscard]] auto FindItem(std::string_view id) -> StatusItemNode*;

    /// @brief Number of items.
    [[nodiscard]] auto ItemCount() const -> int;

private:
    gui::NyanStatusBar* _statusBar;
};

} // namespace matcha::fw
