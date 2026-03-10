#pragma once

/**
 * @file MenuBarNode.h
 * @brief UiNode wrapper for NyanMenuBar -- horizontal menu bar.
 *
 * MenuBarNode manages top-level MenuNode children. It bridges the
 * NyanMenuBar widget API to the UiNode declarative tree.
 *
 * @see NyanMenuBar for the underlying QWidget.
 * @see MenuNode for individual dropdown menus.
 */

#include "Matcha/UiNodes/Core/UiNode.h"

#include <string>
#include <string_view>

namespace matcha::gui {
class NyanMenuBar;
} // namespace matcha::gui

namespace matcha::fw {

class MenuNode;

/**
 * @brief UiNode wrapper for NyanMenuBar.
 *
 * Container node that holds top-level MenuNode children.
 */
class MATCHA_EXPORT MenuBarNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    explicit MenuBarNode(std::string id);
    ~MenuBarNode() override;

    MenuBarNode(const MenuBarNode&) = delete;
    auto operator=(const MenuBarNode&) -> MenuBarNode& = delete;
    MenuBarNode(MenuBarNode&&) = delete;
    auto operator=(MenuBarNode&&) -> MenuBarNode& = delete;

    /// @brief Add a top-level menu with the given title.
    /// @param title Menu title (use & for mnemonic, e.g., "&File").
    /// @return Non-owning pointer to the created MenuNode.
    auto AddMenu(std::string_view title) -> MenuNode*;

    /// @brief Get the number of top-level menus.
    [[nodiscard]] auto MenuCount() const -> int;

    /// @brief Bind an existing NyanMenuBar widget (e.g. from NyanMainTitleBar).
    /// MenuBarNode does NOT own the widget in this case.
    void BindMenuBar(gui::NyanMenuBar* menuBar);

    /// @brief Get the underlying NyanMenuBar widget.
    [[nodiscard]] auto MenuBar() const -> gui::NyanMenuBar*;

    /// @brief Return the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

private:
    gui::NyanMenuBar* _menuBar = nullptr;
    bool _ownsMenuBar = false;
};

} // namespace matcha::fw
