#pragma once

/**
 * @file MenuNode.h
 * @brief UiNode wrapper for NyanMenu -- dropdown/submenu container.
 *
 * MenuNode manages the menu's UiNode tree (MenuItemNode children and
 * sub-MenuNode children). It performs UiNode-layer event routing:
 * when a child submenu's mouse exits toward the parent menu area,
 * the routing propagates through the MenuNode tree rather than
 * through direct QWidget parent-child relationships.
 *
 * @see NyanMenu for the underlying QWidget.
 */

#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <string>
#include <string_view>

class QPoint;

namespace matcha::gui {
class NyanMenu;
class NyanMenuCheckItem;
class NyanMenuItem;
} // namespace matcha::gui

namespace matcha::fw {

class MenuItemNode;

/**
 * @brief UiNode wrapper for NyanMenu.
 *
 * Owns a NyanMenu widget and manages child MenuItemNode / MenuNode
 * sub-trees. Handles mouse-exit-from-submenu routing at the UiNode layer.
 *
 * Dispatches: Activated (when any descendant item is triggered).
 */
class MATCHA_EXPORT MenuNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using Activated = matcha::fw::Activated;
    };

    explicit MenuNode(std::string id);
    ~MenuNode() override;

    MenuNode(const MenuNode&) = delete;
    auto operator=(const MenuNode&) -> MenuNode& = delete;
    MenuNode(MenuNode&&) = delete;
    auto operator=(MenuNode&&) -> MenuNode& = delete;

    // -- Item Management (declarative API) --

    /// @brief Add a menu item. Creates a MenuItemNode child and a NyanMenuItem in the widget.
    /// @return Non-owning pointer to the created MenuItemNode.
    auto AddItem(std::string_view text) -> MenuItemNode*;

    /// @brief Add a separator to the menu widget.
    void AddSeparator();

    /// @brief Add a check item to the menu widget.
    /// @return Non-owning pointer to the underlying NyanMenuCheckItem.
    auto AddCheckItem(std::string_view text, bool checked = false) -> gui::NyanMenuCheckItem*;

    /// @brief Add a submenu. Creates a child MenuNode and a NyanMenu submenu in the widget.
    /// @return Non-owning pointer to the created child MenuNode.
    auto AddSubmenu(std::string_view text) -> MenuNode*;

    /// @brief Add an arbitrary UiNode as a child and install its widget into the menu layout.
    /// @param node UiNode to add (ownership transferred to this MenuNode).
    /// @return Non-owning pointer to the installed node.
    auto InstallNode(std::unique_ptr<UiNode> node) -> UiNode*;

    /// @brief Remove all child nodes and clear the menu widget.
    void ClearAll();

    // -- Popup Control --

    /// @brief Show the menu at the specified global position.
    void Popup(const QPoint& globalPos);

    /// @brief Close the menu.
    void Close();

    /// @brief Check if the menu is currently visible.
    [[nodiscard]] auto IsOpen() const -> bool;

    // -- Widget Access --

    /// @brief Get the underlying NyanMenu widget.
    [[nodiscard]] auto Menu() const -> gui::NyanMenu*;

    /// @brief Return the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

private:
    /// @brief Handle MouseExitedToward from a direct child submenu's NyanMenu.
    /// Routes the event upward through the UiNode tree.
    void OnChildSubmenuMouseExited(const QPoint& globalPos);

    gui::NyanMenu* _menu = nullptr;
    bool _ownsMenu = true;

    // Allow MenuBarNode to set the menu pointer without ownership
    friend class MenuBarNode;
    void BindMenu(gui::NyanMenu* menu);
};

} // namespace matcha::fw
