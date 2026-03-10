/**
 * @file TabBarNode.cpp
 * @brief TabBarNode implementation — unified tab bar UiNode.
 */

#include "Matcha/UiNodes/Document/TabBarNode.h"
#include "Matcha/UiNodes/Document/TabItemNode.h"
#include "Matcha/Widgets/Shell/NyanTabBar.h"
#include "Matcha/Widgets/Shell/NyanTabItem.h"

#include <QCursor>
#include <QPoint>
#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(TabBarNode, UiNode)

TabBarNode::TabBarNode(std::string id, gui::TabStyle style, QWidget* parentWidget)
    : UiNode(std::move(id), NodeType::TabBar)
    , _tabBar(new gui::NyanTabBar(style, parentWidget))
{
    QObject::connect(_tabBar, &gui::NyanTabBar::TabPressed,
        [this](fw::PageId pageId) {
            Notification::TabPageSwitched notif(pageId);
            SendNotification(this, notif);
        });

    QObject::connect(_tabBar, &gui::NyanTabBar::TabCloseRequested,
        [this](fw::PageId pageId) {
            Notification::TabPageCloseRequested notif(pageId);
            SendNotification(this, notif);
        });

    QObject::connect(_tabBar, &gui::NyanTabBar::TabDraggedToVoid,
        [this](fw::PageId pageId, QPoint globalPos) {
            Notification::TabPageDraggedOut notif(pageId, globalPos.x(), globalPos.y());
            SendNotification(this, notif);
        });

    QObject::connect(_tabBar, &gui::NyanTabBar::TabDropReceived,
        [this](fw::PageId pageId, int insertIndex) {
            Notification::TabDroppedIn notif(pageId, insertIndex);
            SendNotification(this, notif);
        });

    QObject::connect(_tabBar, &gui::NyanTabBar::TabReordered,
        [this](fw::PageId pageId, int oldIndex, int newIndex) {
            Notification::TabReordered notif(pageId, oldIndex, newIndex);
            SendNotification(this, notif);
        });
}

TabBarNode::~TabBarNode() = default;

auto TabBarNode::TabBar() -> gui::NyanTabBar*
{
    return _tabBar;
}

auto TabBarNode::Widget() -> QWidget*
{
    return _tabBar;
}

auto TabBarNode::AddTab(PageId pageId, std::string_view title) -> int
{
    auto titleStr = QString::fromUtf8(title.data(), static_cast<int>(title.size()));
    auto* tabItem = _tabBar->AddTab(pageId, titleStr);

    // Create TabItemNode child, bind the widget
    auto tabId = std::string("tab-") + std::to_string(pageId.value);
    auto tabNode = std::make_unique<TabItemNode>(std::move(tabId), pageId, std::string(title));
    tabNode->SetTabItem(tabItem);
    AddNode(std::move(tabNode));

    return _tabBar->IndexOfPage(pageId);
}

void TabBarNode::RemoveTab(PageId pageId)
{
    _tabBar->RemoveTab(pageId);

    // Remove TabItemNode child
    auto* tabNode = FindTab(pageId);
    if (tabNode != nullptr) {
        RemoveNode(tabNode);
    }
}

void TabBarNode::SetActiveTab(PageId pageId)
{
    _tabBar->SetActiveTab(pageId);
}

void TabBarNode::SetTabTitle(PageId pageId, std::string_view title)
{
    auto titleStr = QString::fromUtf8(title.data(), static_cast<int>(title.size()));
    _tabBar->SetTabTitle(pageId, titleStr);

    // Update TabItemNode metadata
    auto* tabNode = FindTab(pageId);
    if (tabNode != nullptr) {
        tabNode->SetTitle(title);
    }
}

void TabBarNode::MoveTab(int fromIndex, int toIndex)
{
    _tabBar->MoveTab(fromIndex, toIndex);
}

auto TabBarNode::TabCount() const -> int
{
    return _tabBar->TabCount();
}

auto TabBarNode::FindTab(PageId pageId) -> TabItemNode*
{
    for (auto* node : ChildrenOfType(NodeType::TabItem)) {
        auto* tab = dynamic_cast<TabItemNode*>(node);
        if (tab != nullptr && tab->GetPageId() == pageId) {
            return tab;
        }
    }
    return nullptr;
}

} // namespace matcha::fw
