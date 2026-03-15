/**
 * @file MainTitleBarNode.cpp
 * @brief MainTitleBarNode implementation -- wraps NyanMainTitleBar (Row 1 only).
 */

#include "Matcha/Tree/Composition/Shell/MainTitleBarNode.h"

#include "Matcha/Tree/ContainerNode.h"
#include "Matcha/Tree/Composition/Menu/MenuBarNode.h"
#include "Matcha/Widgets/Shell/NyanMainTitleBar.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(MainTitleBarNode, TitleBarNode)

MainTitleBarNode::MainTitleBarNode(std::string id, UiNode* parentHint)
    : TitleBarNode(std::move(id))
    , _titleBar(new gui::NyanMainTitleBar(parentHint ? parentHint->Widget() : nullptr))
{
    // MenuBarNode: bind to the existing NyanMenuBar inside the title bar
    auto menuBarNode = std::make_unique<MenuBarNode>("main-menubar");
    menuBarNode->BindMenuBar(_titleBar->MenuBar());
    AddNode(std::move(menuBarNode));

    // QuickCommandContainer slot
    AddNode(ContainerNode::Wrap("quick-command-slot", _titleBar->QuickCommandContainer()));
}

MainTitleBarNode::~MainTitleBarNode() = default;

void MainTitleBarNode::SetTitle(std::string_view title)
{
    _titleBar->SetTitle(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
}

auto MainTitleBarNode::Title() const -> std::string
{
    return _titleBar->Title().toStdString();
}

auto MainTitleBarNode::Widget() -> QWidget*
{
    return _titleBar;
}

auto MainTitleBarNode::MainTitleBar() -> gui::NyanMainTitleBar*
{
    return _titleBar;
}

auto MainTitleBarNode::GetMenuBar() -> observer_ptr<MenuBarNode>
{
    for (auto* node : ChildrenOfType(NodeType::MenuBar)) {
        if (auto* mb = dynamic_cast<MenuBarNode*>(node)) {
            return make_observer(mb);
        }
    }
    return observer_ptr<MenuBarNode>{};
}

auto MainTitleBarNode::GetQuickCommandSlot() -> observer_ptr<ContainerNode>
{
    auto* found = FindById("quick-command-slot");
    if (auto* cn = dynamic_cast<ContainerNode*>(found)) {
        return make_observer(cn);
    }
    return observer_ptr<ContainerNode>{};
}

} // namespace matcha::fw
