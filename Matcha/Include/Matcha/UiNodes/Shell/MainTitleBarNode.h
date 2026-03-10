#pragma once

/**
 * @file MainTitleBarNode.h
 * @brief UiNode wrapper for NyanMainTitleBar -- single-row title bar (Row 1).
 *
 * Exposes Main-specific features: MenuBar, QuickCommands.
 * Module combo and document tabs have been moved to DocumentToolBarNode.
 *
 * @see TitleBarNode for the base interface.
 * @see DocumentToolBarNode for the document toolbar row.
 * @see NyanMainTitleBar for the widget layer.
 */

#include "Matcha/Foundation/Types.h"
#include "Matcha/UiNodes/Shell/TitleBarNode.h"

#include <string>

namespace matcha::gui {
class NyanMainTitleBar;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

class ContainerNode;
class MenuBarNode;

/**
 * @brief UiNode wrapper for NyanMainTitleBar (28px, single-row layout).
 */
class MATCHA_EXPORT MainTitleBarNode : public TitleBarNode {
    MATCHA_DECLARE_CLASS

public:
    MainTitleBarNode(std::string id, UiNode* parentHint = nullptr);
    ~MainTitleBarNode() override;

    MainTitleBarNode(const MainTitleBarNode&)            = delete;
    MainTitleBarNode& operator=(const MainTitleBarNode&) = delete;
    MainTitleBarNode(MainTitleBarNode&&)                 = delete;
    MainTitleBarNode& operator=(MainTitleBarNode&&)      = delete;

    // -- TitleBarNode interface --

    void SetTitle(std::string_view title) override;
    [[nodiscard]] auto Title() const -> std::string override;
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Main-specific accessors --

    /// @brief Get the underlying NyanMainTitleBar widget.
    [[nodiscard]] auto MainTitleBar() -> gui::NyanMainTitleBar*;

    // -- UiNode child access --

    /// @brief Get the MenuBarNode child.
    [[nodiscard]] auto GetMenuBar() -> observer_ptr<MenuBarNode>;

    /// @brief Get the quick command container (after menu bar).
    [[nodiscard]] auto GetQuickCommandSlot() -> observer_ptr<ContainerNode>;

private:
    gui::NyanMainTitleBar* _titleBar;
};

} // namespace matcha::fw
