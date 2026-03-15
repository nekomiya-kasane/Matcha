#pragma once

/**
 * @file DocumentView.h
 * @brief Per-window document view: binds DocumentManager to a tab widget + viewport container.
 *
 * Each window (Main or Floating) owns one DocumentView. The DocumentView:
 * - Filters DocumentManager events by WindowId (only reacts to pages belonging to this window)
 * - Drives a TabBarNode for tab UI
 * - Manages a QStackedWidget with ViewportGroup-backed content per page
 * - Observes (non-owning) a fw::DocumentArea that lives in the UiNode tree
 */

#include "Matcha/Core/StrongId.h"
#include "Matcha/Event/EventNode.h"

#include <memory>
#include <vector>

namespace matcha::fw {
class DocumentArea;
class DocumentPage;
class IDocumentManager;
class TabBarNode;
class UiNode;
} // namespace matcha::fw

namespace nyancad {

/**
 * @brief Per-window document view widget.
 *
 * Owns the viewport content stack. Does NOT own the tab widget
 * (that belongs to the window's title bar or tab bar).
 */
class DocumentView {
public:
    /**
     * @brief Construct a DocumentView for a specific window.
     * @param windowId The WindowId this view is bound to.
     * @param docMgr The shared DocumentManager.
     * @param tabNode The TabBarNode (not owned).
     * @param docArea The DocumentArea (not owned, must outlive DocumentView).
     */
    DocumentView(matcha::fw::WindowId windowId,
                 matcha::fw::IDocumentManager& docMgr,
                 matcha::fw::TabBarNode* tabNode,
                 matcha::fw::DocumentArea* docArea);
    ~DocumentView();

    DocumentView(const DocumentView&) = delete;
    auto operator=(const DocumentView&) -> DocumentView& = delete;
    DocumentView(DocumentView&&) = delete;
    auto operator=(DocumentView&&) -> DocumentView& = delete;

    /// @brief Get the WindowId this view is bound to.
    [[nodiscard]] auto GetWindowId() const -> matcha::fw::WindowId { return _windowId; }

    /// @brief Get the fw::DocumentArea UiNode (non-owning observer).
    [[nodiscard]] auto UiDocumentArea() -> matcha::fw::DocumentArea* { return _uiDocArea; }

private:
    void OnPageCreated(matcha::fw::PageId pageId, matcha::fw::DocumentId docId);
    void OnPageClosed(matcha::fw::PageId pageId, matcha::fw::DocumentId docId);
    void OnDocumentSwitched(matcha::fw::DocumentId docId);
    void OnPageMoved(matcha::fw::PageId pageId,
                     matcha::fw::WindowId fromWin, matcha::fw::WindowId toWin);

    void AddPageContent(matcha::fw::PageId pageId, matcha::fw::DocumentId docId);
    void RemovePageContent(matcha::fw::PageId pageId);
    void SwitchToPage(matcha::fw::PageId pageId);

    void UpdateVisibility();

    matcha::fw::WindowId _windowId;
    matcha::fw::IDocumentManager& _docMgr;
    matcha::fw::TabBarNode* _tabNode;

    matcha::fw::DocumentArea* _uiDocArea;

    bool _suppressSwitch = false;

    /// @brief Subscriptions for Notification observation.
    std::vector<matcha::ScopedSubscription> _subscriptions;
};

} // namespace nyancad
