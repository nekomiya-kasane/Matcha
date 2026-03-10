#include "Matcha/Services/DocumentManager.h"

#include "Matcha/UiNodes/Document/DocumentNotification.h"

namespace matcha::fw {

// ---------------------------------------------------------------------------
// DocumentManager
// ---------------------------------------------------------------------------

DocumentManager::DocumentManager() = default;
DocumentManager::~DocumentManager() = default;

// ---- Basic document lifecycle ----

auto DocumentManager::CreateDocument(std::string_view name)
    -> Expected<DocumentId>
{
    auto docId = DocumentId::From(_nextDocId++);
    auto pageId = PageId::From(_nextPageId++);

    DocumentRecord rec{
        .id = docId,
        .name = std::string(name),
        .pages = {pageId},
    };

    _documents.emplace(docId.value, std::move(rec));
    _pageToDoc.emplace(pageId.value, docId);

    // Auto-switch to newly created document
    _activeDocument = docId;

    DocumentCreated notif(docId, std::string(name));
    SendNotification(this, notif);

    // The initial page was created inline -- notify subscribers.
    PageCreated pageNotif(pageId, docId);
    SendNotification(this, pageNotif);

    return docId;
}

auto DocumentManager::CreateDocument(std::string_view name, WindowId targetWindow)
    -> Expected<DocumentId>
{
    auto docId = DocumentId::From(_nextDocId++);
    auto pageId = PageId::From(_nextPageId++);

    DocumentRecord rec{
        .id = docId,
        .name = std::string(name),
        .pages = {pageId},
    };

    _documents.emplace(docId.value, std::move(rec));
    _pageToDoc.emplace(pageId.value, docId);
    _pageToWindow.emplace(pageId.value, targetWindow);

    _activeDocument = docId;

    DocumentCreated notif(docId, std::string(name));
    SendNotification(this, notif);

    // The initial page was created inline -- notify subscribers.
    PageCreated pageNotif(pageId, docId);
    SendNotification(this, pageNotif);

    return docId;
}

auto DocumentManager::SwitchTo(DocumentId docId)
    -> Expected<void>
{
    if (FindDocument(docId) == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    if (_activeDocument.has_value() && _activeDocument.value() == docId) {
        return {}; // already active
    }

    _activeDocument = docId;

    DocumentSwitched notif(docId);
    SendNotification(this, notif);

    return {};
}

auto DocumentManager::CloseDocument(DocumentId docId)
    -> Expected<void>
{
    auto* rec = FindDocument(docId);
    if (rec == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    // Veto check via Notification
    DocumentClosing closingNotif(docId);
    SendNotification(this, closingNotif);
    if (closingNotif.IsCancelled()) {
        return std::unexpected(ErrorCode::Cancelled);
    }

    // Remove all pages
    for (auto pageId : rec->pages) {
        _pageToDoc.erase(pageId.value);
    }

    // Remove document
    _documents.erase(docId.value);

    // If active document was closed, switch to another
    if (_activeDocument.has_value() && _activeDocument.value() == docId) {
        _activeDocument.reset();
        if (!_documents.empty()) {
            _activeDocument = _documents.begin()->second.id;
        }
    }

    DocumentClosed closedNotif(docId);
    SendNotification(this, closedNotif);

    return {};
}

auto DocumentManager::ActiveDocument() const
    -> std::optional<DocumentId>
{
    return _activeDocument;
}

auto DocumentManager::AllDocuments() const
    -> std::vector<DocumentId>
{
    std::vector<DocumentId> result;
    result.reserve(_documents.size());
    for (const auto& [key, rec] : _documents) {
        result.push_back(rec.id);
    }
    return result;
}

// ---- 1:N Document:DocumentPage extensions ----

auto DocumentManager::CreateDocumentPage(DocumentId docId)
    -> Expected<PageId>
{
    auto* rec = FindDocument(docId);
    if (rec == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    auto pageId = PageId::From(_nextPageId++);
    rec->pages.push_back(pageId);
    _pageToDoc.emplace(pageId.value, docId);

    PageCreated notif(pageId, docId);
    SendNotification(this, notif);

    return pageId;
}

auto DocumentManager::CreateDocumentPage(DocumentId docId, WindowId targetWindow)
    -> Expected<PageId>
{
    auto* rec = FindDocument(docId);
    if (rec == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    auto pageId = PageId::From(_nextPageId++);
    rec->pages.push_back(pageId);
    _pageToDoc.emplace(pageId.value, docId);
    _pageToWindow.emplace(pageId.value, targetWindow);

    PageCreated notif(pageId, docId);
    SendNotification(this, notif);

    return pageId;
}

auto DocumentManager::CloseDocumentPage(PageId pageId)
    -> Expected<void>
{
    auto it = _pageToDoc.find(pageId.value);
    if (it == _pageToDoc.end()) {
        return std::unexpected(ErrorCode::NotFound);
    }

    auto docId = it->second;
    auto* rec = FindDocument(docId);
    if (rec == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    if (rec->pages.size() == 1) {
        // Last page -- trigger full document close
        return CloseDocument(docId);
    }

    // Non-last page -- just remove this page
    std::erase(rec->pages, pageId);
    _pageToDoc.erase(it);
    _pageToWindow.erase(pageId.value);

    PageRemoved notif(pageId);
    SendNotification(this, notif);

    return {};
}

auto DocumentManager::GetDocumentPages(DocumentId docId) const
    -> std::vector<PageId>
{
    const auto* rec = FindDocument(docId);
    if (rec == nullptr) {
        return {};
    }
    return rec->pages;
}

auto DocumentManager::GetPageDocument(PageId pageId) const
    -> std::optional<DocumentId>
{
    auto it = _pageToDoc.find(pageId.value);
    if (it == _pageToDoc.end()) {
        return std::nullopt;
    }
    return it->second;
}

auto DocumentManager::DocumentCount() const -> size_t
{
    return _documents.size();
}

// ---- Page / Window queries ----

auto DocumentManager::GetDocumentPage(DocumentId docId) const
    -> std::optional<PageId>
{
    const auto* rec = FindDocument(docId);
    if (rec == nullptr || rec->pages.empty()) {
        return std::nullopt;
    }
    return rec->pages.front();
}

auto DocumentManager::GetDocumentPageByPageId(PageId pageId) const
    -> std::optional<PageId>
{
    if (!_pageToDoc.contains(pageId.value)) {
        return std::nullopt;
    }
    return pageId;
}

auto DocumentManager::GetPageWindow(PageId pageId) const
    -> std::optional<WindowId>
{
    auto it = _pageToWindow.find(pageId.value);
    if (it == _pageToWindow.end()) {
        return std::nullopt;
    }
    return it->second;
}

auto DocumentManager::ActiveWindow() const
    -> std::optional<WindowId>
{
    return _activeWindow;
}

auto DocumentManager::MoveDocumentPage(PageId pageId, WindowId targetWindow)
    -> Expected<void>
{
    auto docIt = _pageToDoc.find(pageId.value);
    if (docIt == _pageToDoc.end()) {
        return std::unexpected(ErrorCode::NotFound);
    }

    auto winIt = _pageToWindow.find(pageId.value);
    WindowId fromWindow{};
    if (winIt != _pageToWindow.end()) {
        fromWindow = winIt->second;
        winIt->second = targetWindow;
    } else {
        _pageToWindow.emplace(pageId.value, targetWindow);
    }

    fw::DocumentPageMoved notif(pageId, fromWindow, targetWindow);
    SendNotification(this, notif);

    return {};
}

// ---- Internal ----

auto DocumentManager::FindDocument(DocumentId docId) -> DocumentRecord*
{
    auto it = _documents.find(docId.value);
    if (it == _documents.end()) {
        return nullptr;
    }
    return &it->second;
}

auto DocumentManager::FindDocument(DocumentId docId) const -> const DocumentRecord*
{
    auto it = _documents.find(docId.value);
    if (it == _documents.end()) {
        return nullptr;
    }
    return &it->second;
}

} // namespace matcha::fw
