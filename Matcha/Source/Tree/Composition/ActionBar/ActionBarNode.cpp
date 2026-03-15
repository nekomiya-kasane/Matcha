#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"

#include "Matcha/Tree/Composition/ActionBar/ActionTabNode.h"
#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Widgets/ActionBar/NyanActionBar.h"
#include "Matcha/Widgets/ActionBar/NyanActionTab.h"

#include <QString>
#include <QWidget>

#include <cassert>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ActionBarNode, UiNode)

ActionBarNode::ActionBarNode(UiNode* parentHint)
    : UiNode("actionbar", NodeType::ActionBar)
    , _actionBar(new gui::NyanActionBar(parentHint ? parentHint->Widget() : nullptr))
    
{
    QObject::connect(_actionBar, &gui::NyanActionBar::TabSwitched,
        [this](const QString& tabId) {
            TabSwitched notif(tabId.toStdString());
            SendNotification(this, notif);
        });
    QObject::connect(_actionBar, &gui::NyanActionBar::CollapsedChanged,
        [this](bool collapsed) {
            CollapsedChanged notif(collapsed);
            SendNotification(this, notif);
        });
}

ActionBarNode::~ActionBarNode() = default;

auto ActionBarNode::ActionBar() -> gui::NyanActionBar*
{
    return _actionBar;
}

auto ActionBarNode::Widget() -> QWidget*
{
    return _actionBar;
}

auto ActionBarNode::CurrentTabId() const -> std::string
{
    return _actionBar->CurrentTabId().toStdString();
}

void ActionBarNode::SwitchTab(std::string_view tabId)
{
    _actionBar->SwitchTab(QString::fromUtf8(tabId.data(), static_cast<int>(tabId.size())));
}

auto ActionBarNode::HasTab(std::string_view tabId) const -> bool
{
    auto qId = QString::fromUtf8(tabId.data(), static_cast<int>(tabId.size()));
    return _actionBar->Tab(qId) != nullptr;
}

auto ActionBarNode::TabCount() const -> int
{
    return _actionBar->TabCount();
}

void ActionBarNode::SetDockSide(gui::DockSide side)
{
    _actionBar->SetDockSide(side);
}

auto ActionBarNode::GetDockSide() const -> gui::DockSide
{
    return _actionBar->GetDockSide();
}

void ActionBarNode::SetDocked(bool docked)
{
    _actionBar->SetDocked(docked);
}

auto ActionBarNode::IsDocked() const -> bool
{
    return _actionBar->IsDocked();
}

void ActionBarNode::SetCollapsed(bool collapsed)
{
    _actionBar->SetCollapsed(collapsed);
}

auto ActionBarNode::IsCollapsed() const -> bool
{
    return _actionBar->IsCollapsed();
}

auto ActionBarNode::AddTab(std::string id, std::string_view label) -> ActionTabNode*
{
    auto tabNode = std::make_unique<ActionTabNode>(std::move(id), this);
    tabNode->Tab()->SetLabel(QString::fromUtf8(label.data(),
                                                static_cast<int>(label.size())));
    return static_cast<ActionTabNode*>(AddNode(std::move(tabNode)));
}

void ActionBarNode::RemoveTab(std::string_view tabId)
{
    auto* node = FindById(tabId);
    if (node != nullptr) {
        (void)RemoveNode(node);
    }
}

auto ActionBarNode::FindTab(std::string_view tabId) -> ActionTabNode*
{
    auto* node = FindById(tabId);
    return dynamic_cast<ActionTabNode*>(node);
}

auto ActionBarNode::AddNode(std::unique_ptr<UiNode> child) -> UiNode*
{
    auto* tabNode = dynamic_cast<ActionTabNode*>(child.get());
    assert(tabNode != nullptr && "ActionBarNode only accepts ActionTabNode children");
    auto* raw = UiNode::AddNode(std::move(child));
    if (tabNode != nullptr && tabNode->Tab() != nullptr) {
        _actionBar->AddTab(tabNode->Tab());
    }
    return raw;
}

auto ActionBarNode::RemoveNode(UiNode* child) -> std::unique_ptr<UiNode>
{
    auto* tabNode = dynamic_cast<ActionTabNode*>(child);
    if (tabNode != nullptr && tabNode->Tab() != nullptr) {
        _actionBar->RemoveTab(tabNode->Tab());
    }
    return UiNode::RemoveNode(child);
}

} // namespace matcha::fw
