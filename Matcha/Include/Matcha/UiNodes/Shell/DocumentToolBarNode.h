#pragma once

/**
 * @file DocumentToolBarNode.h
 * @brief UiNode wrapper for NyanDocumentToolBar -- module combo + doc tabs + global buttons.
 *
 * Extracted from the former Row 2 of MainTitleBarNode.
 * Owns: TabBarNode, GlobalButtonContainer slot.
 *
 * @see NyanDocumentToolBar for the widget layer.
 */

#include "Matcha/Foundation/Types.h"
#include "Matcha/UiNodes/Core/UiNode.h"

#include <span>
#include <string>

namespace matcha::gui {
class NyanDocumentToolBar;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

class ContainerNode;
class TabBarNode;

/**
 * @brief UiNode wrapper for NyanDocumentToolBar (36px row).
 */
class MATCHA_EXPORT DocumentToolBarNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    explicit DocumentToolBarNode(std::string id, UiNode* parentHint = nullptr);
    ~DocumentToolBarNode() override;

    DocumentToolBarNode(const DocumentToolBarNode&)            = delete;
    DocumentToolBarNode& operator=(const DocumentToolBarNode&) = delete;
    DocumentToolBarNode(DocumentToolBarNode&&)                 = delete;
    DocumentToolBarNode& operator=(DocumentToolBarNode&&)      = delete;

    /// @brief Get the underlying widget.
    [[nodiscard]] auto Widget() -> QWidget* override;

    /// @brief Get the underlying NyanDocumentToolBar widget.
    [[nodiscard]] auto DocumentToolBar() -> gui::NyanDocumentToolBar*;

    // -- Module combo --

    /// @brief Set module combo items (Qt-free).
    void SetModuleItems(std::span<const std::string> items);

    /// @brief Set the current module by name.
    void SetCurrentModule(std::string_view name);

    /// @brief Get the current module name.
    [[nodiscard]] auto CurrentModule() const -> std::string;

    // -- UiNode child access --

    /// @brief Get the TabBarNode child (document tabs).
    [[nodiscard]] auto GetTabBar() -> observer_ptr<TabBarNode>;

    /// @brief Get the global button container (right side).
    [[nodiscard]] auto GetGlobalButtonSlot() -> observer_ptr<ContainerNode>;

private:
    gui::NyanDocumentToolBar* _toolBar;
};

} // namespace matcha::fw
