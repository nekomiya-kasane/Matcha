/**
 * @file TabItemNode.cpp
 * @brief TabItemNode implementation — a single tab in a TabBarNode.
 */

#include "Matcha/UiNodes/Document/TabItemNode.h"
#include "Matcha/Widgets/Shell/NyanTabItem.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(TabItemNode, UiNode)

TabItemNode::TabItemNode(std::string id, PageId pageId, std::string title)
    : UiNode(std::move(id), NodeType::TabItem)
    , _pageId(pageId)
    , _title(std::move(title))
{
}

TabItemNode::~TabItemNode() = default;

auto TabItemNode::Widget() -> QWidget*
{
    return _tabItem;
}

void TabItemNode::SetTabItem(gui::NyanTabItem* item)
{
    _tabItem = item;
}

void TabItemNode::SetTitle(std::string_view title)
{
    _title = std::string(title);
}

} // namespace matcha::fw
