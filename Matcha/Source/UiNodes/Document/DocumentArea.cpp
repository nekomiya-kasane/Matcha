/**
 * @file DocumentArea.cpp
 * @brief Implementation of DocumentArea UiNode.
 */

#include <Matcha/UiNodes/Document/DocumentArea.h>

#include <Matcha/UiNodes/Document/DocumentPage.h>
#include <Matcha/UiNodes/Core/UiNodeNotification.h>
#include <Matcha/UiNodes/Document/ViewportGroup.h>

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace matcha::fw {

DocumentArea::DocumentArea(std::string name)
    : WidgetNode("document-area", NodeType::DocumentArea)
{
    SetName(std::move(name));
}

DocumentArea::~DocumentArea() = default;

auto DocumentArea::CreateWidget(QWidget* parent) -> QWidget*
{
    _container = new QWidget(parent);
    auto* lay = new QVBoxLayout(_container);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    _contentStack = new QStackedWidget(_container);

    // Empty placeholder
    _placeholder = new QWidget(_contentStack);
    _placeholder->setStyleSheet("background: #E8C84A;");
    auto* placeholderLayout = new QVBoxLayout(_placeholder);
    auto* placeholderLabel = new QLabel("No document open", _placeholder);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet(
        "color: #555; font-size: 18px; font-weight: bold; background: transparent;");
    placeholderLayout->addWidget(placeholderLabel);
    _contentStack->addWidget(_placeholder);

    lay->addWidget(_contentStack, /*stretch=*/1);
    _contentStack->setCurrentWidget(_placeholder);

    return _container;
}

auto DocumentArea::CreatePage(std::string name, DocumentId docId)
    -> observer_ptr<DocumentPage>
{
    auto pageId = PageId::From(_nextPageId++);
    auto page = std::make_unique<DocumentPage>(std::move(name), pageId, docId);

    // Attach a ViewportGroup child so the page has a viewport layout
    auto vg = std::make_unique<ViewportGroup>(
        "vg_page_" + std::to_string(pageId.value));
    page->AddNode(std::move(vg));

    auto* raw = page.get();
    AddPage(std::move(page));

    PageCreated notif(pageId, docId);
    SendNotification(this, notif);

    return observer_ptr<DocumentPage>(raw);
}

void DocumentArea::AddPage(std::unique_ptr<DocumentPage> page)
{
    if (!page) {
        return;
    }
    auto id = page->GetPageId();
    AddNode(std::move(page));

    // If this is the first page, make it active
    if (!_activePageId) {
        _activePageId = id;
    }
}

auto DocumentArea::RemovePage(PageId id) -> std::unique_ptr<DocumentPage>
{
    for (auto* child : ChildrenOfType(NodeType::DocumentPage)) {
        auto* page = static_cast<DocumentPage*>(child);
        if (page->GetPageId() == id) {
            auto owned = RemoveNode(page);

            // If we removed the active page, switch to another
            if (_activePageId && *_activePageId == id) {
                _activePageId.reset();
                for (auto* remaining : ChildrenOfType(NodeType::DocumentPage)) {
                    _activePageId = static_cast<DocumentPage*>(remaining)->GetPageId();
                    break;
                }
            }

            PageRemoved notif(id);
            SendNotification(this, notif);

            return std::unique_ptr<DocumentPage>(
                static_cast<DocumentPage*>(owned.release()));
        }
    }
    return nullptr;
}

auto DocumentArea::ActivePage() -> observer_ptr<DocumentPage>
{
    if (!_activePageId) {
        return observer_ptr<DocumentPage>(nullptr);
    }
    return FindPage(*_activePageId);
}

auto DocumentArea::SwitchPage(PageId id) -> Expected<void>
{
    auto page = FindPage(id);
    if (!page) {
        return std::unexpected(ErrorCode::NotFound);
    }
    _activePageId = id;

    PageSwitched notif(id);
    SendNotification(this, notif);

    return {};
}

auto DocumentArea::PageCount() const -> std::size_t
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < NodeCount(); ++i) {
        if (NodeAt(i)->Type() == NodeType::DocumentPage) {
            ++count;
        }
    }
    return count;
}

auto DocumentArea::FindPage(PageId id) -> observer_ptr<DocumentPage>
{
    for (auto* child : ChildrenOfType(NodeType::DocumentPage)) {
        auto* page = static_cast<DocumentPage*>(child);
        if (page->GetPageId() == id) {
            return observer_ptr<DocumentPage>(page);
        }
    }
    return observer_ptr<DocumentPage>(nullptr);
}

auto DocumentArea::FindPageByDoc(DocumentId docId) -> observer_ptr<DocumentPage>
{
    for (auto* child : ChildrenOfType(NodeType::DocumentPage)) {
        auto* page = static_cast<DocumentPage*>(child);
        if (page->GetDocId() == docId) {
            return observer_ptr<DocumentPage>(page);
        }
    }
    return observer_ptr<DocumentPage>(nullptr);
}

// =========================================================================== //
//  Page widget management
// =========================================================================== //

void DocumentArea::AddPageWidget(PageId pageId, QWidget* pageWidget)
{
    EnsureWidget();
    if (_contentStack == nullptr || pageWidget == nullptr) { return; }
    _contentStack->addWidget(pageWidget);
    _pageWidgets[pageId.value] = pageWidget;
    _contentStack->setCurrentWidget(pageWidget);
}

void DocumentArea::RemovePageWidget(PageId pageId)
{
    auto it = _pageWidgets.find(pageId.value);
    if (it == _pageWidgets.end()) { return; }
    if (_contentStack != nullptr) {
        _contentStack->removeWidget(it->second);
    }
    delete it->second;
    _pageWidgets.erase(it);
}

void DocumentArea::SwitchPageWidget(PageId pageId)
{
    auto it = _pageWidgets.find(pageId.value);
    if (it != _pageWidgets.end() && _contentStack != nullptr) {
        _contentStack->setCurrentWidget(it->second);
    }
}

void DocumentArea::ShowPlaceholder()
{
    if (_contentStack != nullptr && _placeholder != nullptr) {
        _contentStack->setCurrentWidget(_placeholder);
    }
}

auto DocumentArea::HasPageWidget(PageId pageId) const -> bool
{
    return _pageWidgets.contains(pageId.value);
}

} // namespace matcha::fw
