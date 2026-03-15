#pragma once

/**
 * @file IDocumentManager.h
 * @brief Abstract interface for document lifecycle management (1:N model).
 *
 * IDocumentManager is a CommandNode-based service in the Matcha framework.
 * It manages Document creation/switching/closing and supports the 1:N
 * Document:DocumentPage model (CATIA multi-view):
 *  - One Document can have multiple DocumentPages across different windows
 *  - Each DocumentPage has its own ViewportGroup (independent camera/layout)
 *  - Undo/Redo operates at Document level, propagating to all Pages
 *  - Closing a non-last Page only destroys that Page
 *  - Closing the last Page triggers the Document close flow (with veto)
 *
 * All lifecycle events are communicated via Notification dispatch through
 * the command tree (DocumentCreated, DocumentSwitched, DocumentClosing,
 * DocumentClosed, DocumentPageMoved). Business layer receives these by
 * overriding AnalyseNotification() in a parent command.
 *
 * @note Accessed via Shell::GetDocumentManager().
 * @see docs/02_Architecture_Design.md section 2.5.5
 */

#include "Matcha/Core/ErrorCode.h"
#include "Matcha/Core/Macros.h"
#include "Matcha/Core/StrongId.h"
#include "Matcha/Core/Types.h"
#include "Matcha/Event/CommandNode.h"

#include <optional>
#include <string_view>
#include <vector>

namespace matcha::fw {

/**
 * @brief Abstract interface for document lifecycle management.
 *
 * Supports 1:N Document:DocumentPage model. Implementation in DocumentManager.
 * Inherits CommandNode so it can dispatch Notifications through the command tree.
 */
class MATCHA_EXPORT IDocumentManager : public CommandNode {
public:
    ~IDocumentManager() override = default;

    IDocumentManager(const IDocumentManager&) = delete;
    auto operator=(const IDocumentManager&) -> IDocumentManager& = delete;
    IDocumentManager(IDocumentManager&&) = delete;
    auto operator=(IDocumentManager&&) -> IDocumentManager& = delete;

    // ---- Basic document lifecycle ----

    /**
     * @brief Create a new document.
     * @param name Human-readable document name.
     * @return DocumentId on success, ErrorCode on failure.
     */
    virtual auto CreateDocument(std::string_view name)
        -> Expected<DocumentId> = 0;

    /**
     * @brief Create a new document in a specific window.
     * @param name Human-readable document name.
     * @param targetWindow Window to create the document page in.
     * @return DocumentId on success, ErrorCode on failure.
     */
    virtual auto CreateDocument(std::string_view name, WindowId targetWindow)
        -> Expected<DocumentId> = 0;

    /**
     * @brief Switch the active document.
     * @param docId Document to make active.
     * @return void on success, ErrorCode::NotFound if docId not found.
     */
    virtual auto SwitchTo(DocumentId docId)
        -> Expected<void> = 0;

    /**
     * @brief Switch the active document (alias for SwitchTo).
     * @param docId Document to make active.
     * @return void on success, ErrorCode::NotFound if docId not found.
     */
    virtual auto SwitchDocument(DocumentId docId)
        -> Expected<void> { return SwitchTo(docId); }

    /**
     * @brief Close a document (closes all its pages).
     * @param docId Document to close.
     * @return void on success, ErrorCode::Cancelled if vetoed, NotFound if not found.
     */
    virtual auto CloseDocument(DocumentId docId)
        -> Expected<void> = 0;

    /**
     * @brief Get the currently active document.
     * @return Active DocumentId, or nullopt if no document is active.
     */
    [[nodiscard]] virtual auto ActiveDocument() const
        -> std::optional<DocumentId> = 0;

    /**
     * @brief Get all open documents.
     * @return Vector of all DocumentIds.
     */
    [[nodiscard]] virtual auto AllDocuments() const
        -> std::vector<DocumentId> = 0;

    // ---- 1:N Document:DocumentPage extensions ----

    /**
     * @brief Create an additional DocumentPage for an existing document.
     *
     * This enables CATIA-style multi-view: same document, different window,
     * independent camera/layout. Right-click tab "New View" uses this.
     *
     * @param docId Document to create a new page for.
     * @return PageId on success, ErrorCode on failure.
     */
    virtual auto CreateDocumentPage(DocumentId docId)
        -> Expected<PageId> = 0;

    /**
     * @brief Create an additional DocumentPage in a specific window.
     * @param docId Document to create a new page for.
     * @param targetWindow Window to host the new page.
     * @return PageId on success, ErrorCode on failure.
     */
    virtual auto CreateDocumentPage(DocumentId docId, WindowId targetWindow)
        -> Expected<PageId> = 0;

    /**
     * @brief Close a specific DocumentPage.
     *
     * If this is the last page for the document, triggers Document close flow
     * (with veto via DocumentClosing notification).
     * If not the last page, only destroys this page.
     *
     * @param pageId Page to close.
     * @return void on success, ErrorCode on failure.
     */
    virtual auto CloseDocumentPage(PageId pageId)
        -> Expected<void> = 0;

    /**
     * @brief Get all pages for a document.
     * @param docId Document to query.
     * @return Vector of PageIds for this document.
     */
    [[nodiscard]] virtual auto GetDocumentPages(DocumentId docId) const
        -> std::vector<PageId> = 0;

    /**
     * @brief Get the document that a page belongs to.
     * @param pageId Page to query.
     * @return DocumentId, or nullopt if page not found.
     */
    [[nodiscard]] virtual auto GetPageDocument(PageId pageId) const
        -> std::optional<DocumentId> = 0;

    /**
     * @brief Get the number of open documents.
     */
    [[nodiscard]] virtual auto DocumentCount() const -> size_t = 0;

    // ---- Page / Window queries ----

    /**
     * @brief Get the primary DocumentPage for a document (first page).
     * @param docId Document to query.
     * @return PageId of the primary page, or nullopt if not found.
     */
    [[nodiscard]] virtual auto GetDocumentPage(DocumentId docId) const
        -> std::optional<PageId> = 0;

    /**
     * @brief Get a DocumentPage by its PageId (existence check + lookup).
     * @param pageId Page to look up.
     * @return PageId if found, nullopt if not.
     */
    [[nodiscard]] virtual auto GetDocumentPageByPageId(PageId pageId) const
        -> std::optional<PageId> = 0;

    /**
     * @brief Get the window hosting a specific page.
     * @param pageId Page to query.
     * @return WindowId of the hosting window, or nullopt if not found.
     */
    [[nodiscard]] virtual auto GetPageWindow(PageId pageId) const
        -> std::optional<WindowId> = 0;

    /**
     * @brief Get the currently active window (the one with focus).
     * @return WindowId of the active window, or nullopt.
     */
    [[nodiscard]] virtual auto ActiveWindow() const
        -> std::optional<WindowId> = 0;

    /**
     * @brief Move a DocumentPage to a different window.
     * @param pageId Page to move.
     * @param targetWindow Destination window.
     * @return void on success, ErrorCode on failure.
     */
    virtual auto MoveDocumentPage(PageId pageId, WindowId targetWindow)
        -> Expected<void> = 0;

protected:
    IDocumentManager()
        : CommandNode(nullptr, "DocumentManager", CommandMode::Undefined) {}
};

} // namespace matcha::fw
