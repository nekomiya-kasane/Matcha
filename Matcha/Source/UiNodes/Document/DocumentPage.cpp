/**
 * @file DocumentPage.cpp
 * @brief Implementation of DocumentPage UiNode.
 */

#include <Matcha/UiNodes/Document/DocumentPage.h>

#include <Matcha/UiNodes/Document/ViewportGroup.h>
#include <Matcha/UiNodes/Shell/WindowNode.h>

namespace matcha::fw {

DocumentPage::DocumentPage(std::string name, PageId pageId, DocumentId docId)
    : UiNode(std::string("page-") + std::to_string(pageId.value), NodeType::DocumentPage, std::move(name))
    , _pageId(pageId)
    , _docId(docId)
{
}

DocumentPage::~DocumentPage() = default;

auto DocumentPage::GetViewportGroup() -> observer_ptr<ViewportGroup>
{
    for (auto* child : ChildrenOfType(NodeType::ViewportGroup)) {
        return observer_ptr<ViewportGroup>(static_cast<ViewportGroup*>(child));
    }
    return observer_ptr<ViewportGroup>(nullptr);
}

void DocumentPage::SetTabTitle(std::string_view title)
{
    _tabTitle = std::string(title);
}

void DocumentPage::SetTabIcon(std::string_view iconPath)
{
    _tabIcon = std::string(iconPath);
}

void DocumentPage::SetModified(bool modified)
{
    _modified = modified;
}

auto DocumentPage::GetWindow() const -> observer_ptr<WindowNode>
{
    // Traverse parent chain to find the WindowNode ancestor.
    auto* current = ParentNode();
    while (current != nullptr) {
        if (current->Type() == NodeType::WindowNode) {
            return observer_ptr<WindowNode>(static_cast<WindowNode*>(current));
        }
        current = current->ParentNode();
    }
    return observer_ptr<WindowNode>(nullptr);
}

} // namespace matcha::fw
