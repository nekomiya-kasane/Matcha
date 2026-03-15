/**
 * @file DocumentView.cpp
 * @brief Per-window document view: binds DocumentManager to tab widget + viewport container.
 *
 * DocumentView is a pure C++ object (not a QWidget). It owns a DocumentArea UiNode
 * which in turn owns the QStackedWidget container. DnD is handled via
 * DocumentArea::SetAcceptDrops + DnD Notification propagation.
 */

#include "DocumentView.h"

#include "Matcha/Tree/Composition/Document/DocumentArea.h"
#include "Matcha/Tree/Composition/Document/DocumentPage.h"
#include "Matcha/Services/IDocumentManager.h"
#include "Matcha/Tree/Composition/Document/TabBarNode.h"
#include "Matcha/Tree/Composition/Document/SplitTreeNode.h"
#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Tree/Composition/Document/Viewport.h"
#include "Matcha/Tree/Composition/Document/ViewportGroup.h"

#include <QWidget>
#include <string>

namespace nyancad {

DocumentView::DocumentView(matcha::fw::WindowId windowId,
                           matcha::fw::IDocumentManager& docMgr,
                           matcha::fw::TabBarNode* tabNode,
                           matcha::fw::DocumentArea* docArea)
    : _windowId(windowId)
    , _docMgr(docMgr)
    , _tabNode(tabNode)
    , _uiDocArea(docArea)
{
    if (_uiDocArea == nullptr) { return; }

    // -- Subscribe to DocumentManager Notifications --
    using namespace matcha::fw;
    auto* pub = &docMgr;

    _subscriptions.emplace_back(docMgr, pub, "PageCreated",
        [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
            if (auto* e = n.As<PageCreated>()) {
                OnPageCreated(e->GetPageId(), e->DocId());
            }
        });
    _subscriptions.emplace_back(docMgr, pub, "PageRemoved",
        [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
            if (auto* e = n.As<PageRemoved>()) {
                auto docOpt = _docMgr.GetPageDocument(e->GetPageId());
                if (docOpt.has_value()) {
                    OnPageClosed(e->GetPageId(), docOpt.value());
                }
            }
        });
    _subscriptions.emplace_back(docMgr, pub, "DocumentClosed",
        [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
            if (auto* e = n.As<DocumentClosed>()) {
                auto page = _uiDocArea->FindPageByDoc(e->DocId());
                if (page) {
                    auto pageId = page->GetPageId();
                    if (_uiDocArea->HasPageWidget(pageId)) {
                        RemovePageContent(pageId);
                        if (_tabNode) { _tabNode->RemoveTab(pageId); }
                    }
                }
                UpdateVisibility();
            }
        });
    _subscriptions.emplace_back(docMgr, pub, "DocumentSwitched",
        [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
            if (auto* e = n.As<DocumentSwitched>()) {
                OnDocumentSwitched(e->DocId());
            }
        });
    _subscriptions.emplace_back(docMgr, pub, "DocumentPageMoved",
        [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
            if (auto* e = n.As<DocumentPageMoved>()) {
                OnPageMoved(e->GetPageId(), e->FromWindow(), e->ToWindow());
            }
        });

    // -- Subscribe to tab node Notifications (PageId-based) --
    if (_tabNode != nullptr) {
        _subscriptions.emplace_back(*_tabNode, _tabNode, "TabPageSwitched",
            [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
                if (auto* e = n.As<TabPageSwitched>()) {
                    auto pageId = e->GetPageId();
                    SwitchToPage(pageId);
                    auto page = _uiDocArea->FindPage(pageId);
                    if (page) {
                        _suppressSwitch = true;
                        (void)_docMgr.SwitchTo(page->GetDocId());
                        _suppressSwitch = false;
                    }
                }
            });
        _subscriptions.emplace_back(*_tabNode, _tabNode, "TabPageCloseRequested",
            [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
                if (auto* e = n.As<TabPageCloseRequested>()) {
                    auto page = _uiDocArea->FindPage(e->GetPageId());
                    if (page) {
                        (void)_docMgr.CloseDocumentPage(e->GetPageId());
                    }
                }
            });
        _subscriptions.emplace_back(*_tabNode, _tabNode, "TabPageDraggedOut",
            [this](matcha::EventNode& /*s*/, matcha::Notification& n) {
                if (auto* e = n.As<TabPageDraggedOut>()) {
                    // Forward via DocumentArea so app layer can Subscribe there
                    TabPageDraggedOut fwd(e->GetPageId(), e->GlobalX(), e->GlobalY());
                    _uiDocArea->SendNotification(_uiDocArea, fwd);
                }
            });
    }
}

DocumentView::~DocumentView() = default;

void DocumentView::OnPageCreated(matcha::fw::PageId pageId, matcha::fw::DocumentId docId)
{
    // Filter: only handle pages assigned to our window
    auto winOpt = _docMgr.GetPageWindow(pageId);
    if (winOpt.has_value() && winOpt.value() != _windowId) {
        return;
    }
    if (!winOpt.has_value()) {
        if (_windowId.value != 1) {
            return;
        }
    }
    AddPageContent(pageId, docId);
}

void DocumentView::OnPageClosed(matcha::fw::PageId pageId,
                                [[maybe_unused]] matcha::fw::DocumentId docId)
{
    if (!_uiDocArea->HasPageWidget(pageId)) {
        return;
    }
    RemovePageContent(pageId);
    if (_tabNode) { _tabNode->RemoveTab(pageId); }
    UpdateVisibility();
}

void DocumentView::OnDocumentSwitched(matcha::fw::DocumentId docId)
{
    if (_suppressSwitch) { return; }

    auto page = _uiDocArea->FindPageByDoc(docId);
    if (!page) { return; }

    auto pageId = page->GetPageId();
    if (!_uiDocArea->HasPageWidget(pageId)) { return; }

    _suppressSwitch = true;
    if (_tabNode) { _tabNode->SetActiveTab(pageId); }
    SwitchToPage(pageId);
    _suppressSwitch = false;
}

void DocumentView::OnPageMoved(matcha::fw::PageId pageId,
                               matcha::fw::WindowId fromWin,
                               matcha::fw::WindowId toWin)
{
    if (fromWin == _windowId) {
        RemovePageContent(pageId);
        if (_tabNode) { _tabNode->RemoveTab(pageId); }
        UpdateVisibility();
    }
    if (toWin == _windowId) {
        auto docOpt = _docMgr.GetPageDocument(pageId);
        if (docOpt.has_value()) {
            AddPageContent(pageId, docOpt.value());
        }
    }
}

void DocumentView::AddPageContent(matcha::fw::PageId pageId,
                                  matcha::fw::DocumentId docId)
{
    using namespace matcha::fw;

    auto name = "Document " + std::to_string(docId.value);

    // Create DocumentPage in fw UiNode tree
    auto docPage = _uiDocArea->CreatePage(name, docId);
    if (!docPage) { return; }

    // Get ViewportGroup and create 2x2 viewport layout
    auto vg = docPage->GetViewportGroup();
    if (!vg) { return; }

    auto id1 = vg->AllViewportIds()[0];
    vg->FindViewport(id1)->SetName(name + " - Perspective");

    auto r1 = vg->SplitViewport(id1, SplitDirection::Vertical);
    if (r1.has_value()) {
        auto id2 = r1->get()->GetViewportId();
        r1->get()->SetName(name + " - Front");

        auto r2 = vg->SplitViewport(id1, SplitDirection::Horizontal);
        if (r2.has_value()) {
            r2->get()->SetName(name + " - Top");
        }
        auto r3 = vg->SplitViewport(id2, SplitDirection::Horizontal);
        if (r3.has_value()) {
            r3->get()->SetName(name + " - Right");
        }
    }

    // Create content widget via DocumentArea's stack
    auto* areaWidget = _uiDocArea->Widget();
    auto* pageWidget = new QWidget(areaWidget);
    vg->RebuildWidgetTree(pageWidget);
    _uiDocArea->AddPageWidget(pageId, pageWidget);

    // Add tab
    if (_tabNode) { _tabNode->AddTab(pageId, name); }

    // Switch to this page
    (void)_uiDocArea->SwitchPage(pageId);
    _uiDocArea->SwitchPageWidget(pageId);
    if (_tabNode) { _tabNode->SetActiveTab(pageId); }
}

void DocumentView::RemovePageContent(matcha::fw::PageId pageId)
{
    _uiDocArea->RemovePageWidget(pageId);
    _uiDocArea->RemovePage(pageId);
}

void DocumentView::SwitchToPage(matcha::fw::PageId pageId)
{
    if (_uiDocArea->HasPageWidget(pageId)) {
        _uiDocArea->SwitchPageWidget(pageId);
        (void)_uiDocArea->SwitchPage(pageId);
    }
}

void DocumentView::UpdateVisibility()
{
    if (_uiDocArea->PageCount() == 0) {
        _uiDocArea->ShowPlaceholder();
    }
}

} // namespace nyancad
