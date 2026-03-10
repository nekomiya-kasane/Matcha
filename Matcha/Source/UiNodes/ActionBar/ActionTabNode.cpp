#include "Matcha/UiNodes/ActionBar/ActionTabNode.h"

#include "Matcha/UiNodes/ActionBar/ActionToolbarNode.h"
#include "Matcha/Widgets/ActionBar/NyanActionTab.h"
#include "Matcha/Widgets/ActionBar/NyanActionToolbar.h"

#include <QString>
#include <QWidget>

#include <cassert>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ActionTabNode, UiNode)

ActionTabNode::ActionTabNode(std::string id, UiNode* parentHint)
    : UiNode(std::move(id), NodeType::ActionTab)
    , _tab(new gui::NyanActionTab(parentHint ? parentHint->Widget() : nullptr))
    
{
}

ActionTabNode::~ActionTabNode() = default;

auto ActionTabNode::Tab() -> gui::NyanActionTab*
{
    return _tab;
}

auto ActionTabNode::Widget() -> QWidget*
{
    return _tab;
}

auto ActionTabNode::ToolbarCount() const -> int
{
    return _tab->ToolbarCount();
}

auto ActionTabNode::AddToolbar(std::string id, std::string_view label) -> ActionToolbarNode*
{
    auto toolbarNode = std::make_unique<ActionToolbarNode>(std::move(id), this);
    toolbarNode->Toolbar()->setObjectName(
        QString::fromUtf8(label.data(), static_cast<int>(label.size())));
    return static_cast<ActionToolbarNode*>(AddNode(std::move(toolbarNode)));
}

void ActionTabNode::RemoveToolbar(std::string_view toolbarId)
{
    auto* node = FindById(toolbarId);
    if (node != nullptr) {
        (void)RemoveNode(node);
    }
}

auto ActionTabNode::FindToolbar(std::string_view toolbarId) -> ActionToolbarNode*
{
    auto* node = FindById(toolbarId);
    return dynamic_cast<ActionToolbarNode*>(node);
}

auto ActionTabNode::AddNode(std::unique_ptr<UiNode> child) -> UiNode*
{
    auto* toolbarNode = dynamic_cast<ActionToolbarNode*>(child.get());
    assert(toolbarNode != nullptr && "ActionTabNode only accepts ActionToolbarNode children");
    auto* raw = UiNode::AddNode(std::move(child));
    if (toolbarNode != nullptr && toolbarNode->Toolbar() != nullptr) {
        _tab->AddToolbar(toolbarNode->Toolbar());
    }
    return raw;
}

auto ActionTabNode::RemoveNode(UiNode* child) -> std::unique_ptr<UiNode>
{
    auto* toolbarNode = dynamic_cast<ActionToolbarNode*>(child);
    if (toolbarNode != nullptr && toolbarNode->Toolbar() != nullptr) {
        _tab->RemoveToolbar(toolbarNode->Toolbar());
    }
    return UiNode::RemoveNode(child);
}

} // namespace matcha::fw
