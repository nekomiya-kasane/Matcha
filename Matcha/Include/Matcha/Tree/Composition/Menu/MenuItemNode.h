#pragma once

/**
 * @file MenuItemNode.h
 * @brief UiNode wrapper for NyanMenuItem -- a single clickable menu entry.
 *
 * Leaf node in the Menu UiNode tree. Dispatches Activated on click.
 * Does NOT inherit WidgetNode (menu items are not standalone widgets;
 * they are children of a NyanMenu layout, which owns their QWidget lifetime).
 */

#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <string>
#include <string_view>

namespace matcha::gui {
class NyanMenuItem;
} // namespace matcha::gui

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanMenuItem (leaf node in menu tree).
 *
 * Dispatches: Activated (on click/trigger).
 */
class MATCHA_EXPORT MenuItemNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using Activated = matcha::fw::Activated;
    };

    explicit MenuItemNode(std::string id);
    ~MenuItemNode() override;

    MenuItemNode(const MenuItemNode&) = delete;
    auto operator=(const MenuItemNode&) -> MenuItemNode& = delete;
    MenuItemNode(MenuItemNode&&) = delete;
    auto operator=(MenuItemNode&&) -> MenuItemNode& = delete;

    /// @brief Set the display text.
    void SetText(std::string_view text);

    /// @brief Get the display text.
    [[nodiscard]] auto Text() const -> std::string;

    /// @brief Set enabled state.
    void SetEnabled(bool enabled);

    /// @brief Get enabled state.
    [[nodiscard]] auto IsEnabled() const -> bool;

    /// @brief Return the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

private:
    friend class MenuNode;

    /// @brief Bind the underlying NyanMenuItem. @internal Called by MenuNode.
    void Bind(gui::NyanMenuItem* item);

    /// @brief Get the underlying NyanMenuItem. @internal
    [[nodiscard]] auto MenuItem() const -> gui::NyanMenuItem* { return _item; }

    gui::NyanMenuItem* _item = nullptr;
    std::string _text;
    bool _enabled = true;
};

} // namespace matcha::fw
