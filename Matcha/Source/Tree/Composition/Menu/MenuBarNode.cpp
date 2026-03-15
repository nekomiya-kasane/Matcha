#include "Matcha/Tree/Composition/Menu/MenuBarNode.h"
#include "Matcha/Tree/Composition/Menu/MenuNode.h"

#include "Matcha/Widgets/Menu/NyanMenuBar.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(MenuBarNode, UiNode)

MenuBarNode::MenuBarNode(std::string id)
    : UiNode(std::move(id), NodeType::MenuBar)
{
}

MenuBarNode::~MenuBarNode()
{
    if (_ownsMenuBar) {
        delete _menuBar;
    }
}

auto MenuBarNode::AddMenu(std::string_view title) -> MenuNode*
{
    if (!_menuBar) {
        return nullptr;
    }

    // Create the widget-level menu via NyanMenuBar
    auto qTitle = QString::fromUtf8(title.data(), static_cast<int>(title.size()));
    auto* widgetMenu = _menuBar->AddMenu(qTitle);

    // Create the UiNode and bind it to the already-created widget
    auto node = std::make_unique<MenuNode>(std::string(title));
    node->BindMenu(widgetMenu);

    auto* raw = node.get();
    AddNode(std::move(node));
    return raw;
}

auto MenuBarNode::MenuCount() const -> int
{
    return static_cast<int>(NodeCount());
}

void MenuBarNode::BindMenuBar(gui::NyanMenuBar* menuBar)
{
    if (_ownsMenuBar) {
        delete _menuBar;
    }
    _menuBar = menuBar;
    _ownsMenuBar = false;
}

auto MenuBarNode::MenuBar() const -> gui::NyanMenuBar*
{
    return _menuBar;
}

auto MenuBarNode::Widget() -> QWidget*
{
    return _menuBar;
}

} // namespace matcha::fw
