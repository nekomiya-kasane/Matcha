/**
 * @file DocumentToolBarNode.cpp
 * @brief DocumentToolBarNode implementation -- wraps NyanDocumentToolBar.
 */

#include "Matcha/Tree/Composition/Shell/DocumentToolBarNode.h"

#include "Matcha/Tree/ContainerNode.h"
#include "Matcha/Tree/Composition/Document/TabBarNode.h"
#include "Matcha/Widgets/Shell/NyanDocumentToolBar.h"
#include "Matcha/Widgets/Shell/NyanTabBar.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(DocumentToolBarNode, UiNode)

DocumentToolBarNode::DocumentToolBarNode(std::string id, UiNode* parentHint)
    : UiNode(std::move(id), NodeType::DocumentToolBar)
    , _toolBar(new gui::NyanDocumentToolBar(parentHint ? parentHint->Widget() : nullptr))
{
    // TabBarNode: create and install into document toolbar
    auto tabBarNode = std::make_unique<TabBarNode>("doc-tabbar", gui::TabStyle::TitleBar,
                                                    _toolBar);
    _toolBar->SetTabBar(tabBarNode->TabBar());
    AddNode(std::move(tabBarNode));

    // GlobalButtonContainer slot
    AddNode(ContainerNode::Wrap("global-button-slot", _toolBar->GlobalButtonContainer()));
}

DocumentToolBarNode::~DocumentToolBarNode() = default;

auto DocumentToolBarNode::Widget() -> QWidget*
{
    return _toolBar;
}

auto DocumentToolBarNode::DocumentToolBar() -> gui::NyanDocumentToolBar*
{
    return _toolBar;
}

void DocumentToolBarNode::SetModuleItems(std::span<const std::string> items)
{
    QStringList qItems;
    qItems.reserve(static_cast<int>(items.size()));
    for (const auto& item : items) {
        qItems.append(QString::fromUtf8(item.data(), static_cast<int>(item.size())));
    }
    _toolBar->SetModuleItems(qItems);
}

void DocumentToolBarNode::SetCurrentModule(std::string_view name)
{
    _toolBar->SetCurrentModule(QString::fromUtf8(name.data(), static_cast<int>(name.size())));
}

auto DocumentToolBarNode::CurrentModule() const -> std::string
{
    return _toolBar->CurrentModule().toStdString();
}

auto DocumentToolBarNode::GetTabBar() -> observer_ptr<TabBarNode>
{
    for (auto* node : ChildrenOfType(NodeType::TabBar)) {
        if (auto* tb = dynamic_cast<TabBarNode*>(node)) {
            return make_observer(tb);
        }
    }
    return observer_ptr<TabBarNode>{};
}

auto DocumentToolBarNode::GetGlobalButtonSlot() -> observer_ptr<ContainerNode>
{
    auto* found = FindById("global-button-slot");
    if (auto* cn = dynamic_cast<ContainerNode*>(found)) {
        return make_observer(cn);
    }
    return observer_ptr<ContainerNode>{};
}

} // namespace matcha::fw
