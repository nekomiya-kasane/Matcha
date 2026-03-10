#pragma once

/**
 * @file DocumentManager.h
 * @brief Concrete implementation of IDocumentManager with 1:N Document:DocumentPage model.
 *
 * @see IDocumentManager.h for interface documentation.
 * @see docs/02_Architecture_Design.md section 2.5.5
 */

#include "Matcha/Services/IDocumentManager.h"

#include <string>
#include <unordered_map>

namespace matcha::fw {

/**
 * @brief Internal document record.
 */
struct DocumentRecord {
    DocumentId id;
    std::string name;
    std::vector<PageId> pages;
};

/**
 * @brief Concrete IDocumentManager -- manages documents and pages via Notifications.
 */
class MATCHA_EXPORT DocumentManager final : public IDocumentManager {
public:
    DocumentManager();
    ~DocumentManager() override;

    DocumentManager(const DocumentManager&) = delete;
    auto operator=(const DocumentManager&) -> DocumentManager& = delete;
    DocumentManager(DocumentManager&&) = delete;
    auto operator=(DocumentManager&&) -> DocumentManager& = delete;

    // ---- IDocumentManager interface ----

    auto CreateDocument(std::string_view name)
        -> Expected<DocumentId> override;

    auto CreateDocument(std::string_view name, WindowId targetWindow)
        -> Expected<DocumentId> override;

    auto SwitchTo(DocumentId docId)
        -> Expected<void> override;

    auto CloseDocument(DocumentId docId)
        -> Expected<void> override;

    [[nodiscard]] auto ActiveDocument() const
        -> std::optional<DocumentId> override;

    [[nodiscard]] auto AllDocuments() const
        -> std::vector<DocumentId> override;

    auto CreateDocumentPage(DocumentId docId)
        -> Expected<PageId> override;

    auto CreateDocumentPage(DocumentId docId, WindowId targetWindow)
        -> Expected<PageId> override;

    auto CloseDocumentPage(PageId pageId)
        -> Expected<void> override;

    [[nodiscard]] auto GetDocumentPages(DocumentId docId) const
        -> std::vector<PageId> override;

    [[nodiscard]] auto GetPageDocument(PageId pageId) const
        -> std::optional<DocumentId> override;

    [[nodiscard]] auto DocumentCount() const -> size_t override;

    // ---- Page / Window queries ----

    [[nodiscard]] auto GetDocumentPage(DocumentId docId) const
        -> std::optional<PageId> override;

    [[nodiscard]] auto GetDocumentPageByPageId(PageId pageId) const
        -> std::optional<PageId> override;

    [[nodiscard]] auto GetPageWindow(PageId pageId) const
        -> std::optional<WindowId> override;

    [[nodiscard]] auto ActiveWindow() const
        -> std::optional<WindowId> override;

    auto MoveDocumentPage(PageId pageId, WindowId targetWindow)
        -> Expected<void> override;

private:
    auto FindDocument(DocumentId docId) -> DocumentRecord*;
    [[nodiscard]] auto FindDocument(DocumentId docId) const -> const DocumentRecord*;

    uint64_t _nextDocId = 1;
    uint64_t _nextPageId = 1;
    std::optional<DocumentId> _activeDocument;
    std::unordered_map<uint64_t, DocumentRecord> _documents;
    std::unordered_map<uint64_t, DocumentId> _pageToDoc;
    std::unordered_map<uint64_t, WindowId> _pageToWindow;
    std::optional<WindowId> _activeWindow;
};

} // namespace matcha::fw
