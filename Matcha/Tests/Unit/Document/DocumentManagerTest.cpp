#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

// Keep diagnostic suppression active for entire file (doctest uses __COUNTER__ everywhere)

#include "Matcha/Services/DocumentManager.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include <string>
#include <vector>

using namespace matcha::fw;

/// @brief Helper CommandNode that captures Notifications for test assertions.
class DocMgrSpy : public matcha::CommandNode {
public:
    explicit DocMgrSpy(matcha::CommandNode* parent)
        : CommandNode(parent, "DocMgrSpy", matcha::CommandMode::Undefined) {}

    DocumentId lastCreatedDoc{};
    DocumentId lastSwitchedDoc{};
    DocumentId lastClosingDoc{};
    bool closingCancelled = false;
    bool vetoClose = false;
    DocumentId lastClosedDoc{};
    PageId lastCreatedPage{};
    DocumentId lastCreatedPageDoc{};
    PageId lastRemovedPage{};
    int createdCount = 0;

protected:
    auto AnalyseNotification(matcha::CommandNode* /*sender*/,
                              matcha::Notification& notif)
        -> matcha::PropagationMode override
    {
        if (auto* n = notif.As<DocumentCreated>()) {
            lastCreatedDoc = n->DocId();
            ++createdCount;
        } else if (auto* n = notif.As<DocumentSwitched>()) {
            lastSwitchedDoc = n->DocId();
        } else if (auto* n = notif.As<DocumentClosing>()) {
            lastClosingDoc = n->DocId();
            if (vetoClose) { n->SetCancel(true); }
        } else if (auto* n = notif.As<DocumentClosed>()) {
            lastClosedDoc = n->DocId();
        } else if (auto* n = notif.As<PageCreated>()) {
            lastCreatedPage = n->GetPageId();
            lastCreatedPageDoc = n->DocId();
        } else if (auto* n = notif.As<PageRemoved>()) {
            lastRemovedPage = n->GetPageId();
        }
        return matcha::PropagationMode::TransmitToParent;
    }
};

TEST_SUITE("DocumentManager") {

// ---- Basic document lifecycle ----

TEST_CASE("CreateDocument returns unique IDs") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    auto r2 = mgr.CreateDocument("Doc2");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());
    CHECK(r1.value() != r2.value());
}

TEST_CASE("CreateDocument auto-activates new document") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    REQUIRE(r1.has_value());
    CHECK(mgr.ActiveDocument().has_value());
    CHECK(mgr.ActiveDocument().value() == r1.value());
}

TEST_CASE("CreateDocument auto-creates first page") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    REQUIRE(r1.has_value());
    auto pages = mgr.GetDocumentPages(r1.value());
    CHECK(pages.size() == 1);
}

TEST_CASE("SwitchTo changes active document") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    auto r2 = mgr.CreateDocument("Doc2");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());
    CHECK(mgr.ActiveDocument().value() == r2.value());

    auto sw = mgr.SwitchTo(r1.value());
    CHECK(sw.has_value());
    CHECK(mgr.ActiveDocument().value() == r1.value());
}

TEST_CASE("SwitchTo returns NotFound for invalid ID") {
    DocumentManager mgr;
    auto r = mgr.SwitchTo(DocumentId::From(999));
    CHECK(!r.has_value());
    CHECK(r.error() == ErrorCode::NotFound);
}

TEST_CASE("CloseDocument removes document") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    REQUIRE(r1.has_value());
    CHECK(mgr.DocumentCount() == 1);

    auto cl = mgr.CloseDocument(r1.value());
    CHECK(cl.has_value());
    CHECK(mgr.DocumentCount() == 0);
}

TEST_CASE("CloseDocument switches active to remaining doc") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    auto r2 = mgr.CreateDocument("Doc2");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());

    auto sw2 = mgr.SwitchTo(r1.value());
    CHECK(sw2.has_value());
    CHECK(mgr.ActiveDocument().value() == r1.value());

    auto cl = mgr.CloseDocument(r1.value());
    CHECK(cl.has_value());
    CHECK(mgr.ActiveDocument().has_value());
    CHECK(mgr.ActiveDocument().value() == r2.value());
}

TEST_CASE("CloseDocument clears active when last doc closed") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    REQUIRE(r1.has_value());
    auto cl = mgr.CloseDocument(r1.value());
    CHECK(cl.has_value());
    CHECK(!mgr.ActiveDocument().has_value());
}

TEST_CASE("AllDocuments returns all open docs") {
    DocumentManager mgr;
    auto r1 = mgr.CreateDocument("Doc1");
    auto r2 = mgr.CreateDocument("Doc2");
    auto r3 = mgr.CreateDocument("Doc3");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());
    REQUIRE(r3.has_value());

    auto all = mgr.AllDocuments();
    CHECK(all.size() == 3);
}

// ---- 1:N Document:DocumentPage model ----

TEST_CASE("CreateDocumentPage adds a second page") {
    DocumentManager mgr;
    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());

    auto page2 = mgr.CreateDocumentPage(doc.value());
    REQUIRE(page2.has_value());

    auto pages = mgr.GetDocumentPages(doc.value());
    CHECK(pages.size() == 2);
    CHECK(pages[0] != pages[1]);
}

TEST_CASE("CreateDocumentPage returns NotFound for invalid doc") {
    DocumentManager mgr;
    auto r = mgr.CreateDocumentPage(DocumentId::From(999));
    CHECK(!r.has_value());
    CHECK(r.error() == ErrorCode::NotFound);
}

TEST_CASE("GetPageDocument returns correct document") {
    DocumentManager mgr;
    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());

    auto pages = mgr.GetDocumentPages(doc.value());
    REQUIRE(pages.size() == 1);

    auto result = mgr.GetPageDocument(pages[0]);
    REQUIRE(result.has_value());
    CHECK(result.value() == doc.value());
}

TEST_CASE("CloseDocumentPage on non-last page keeps document alive") {
    DocumentManager mgr;
    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());

    auto page2 = mgr.CreateDocumentPage(doc.value());
    REQUIRE(page2.has_value());

    auto pages = mgr.GetDocumentPages(doc.value());
    REQUIRE(pages.size() == 2);

    auto cl = mgr.CloseDocumentPage(pages[0]);
    CHECK(cl.has_value());
    CHECK(mgr.DocumentCount() == 1);

    auto remaining = mgr.GetDocumentPages(doc.value());
    CHECK(remaining.size() == 1);
    CHECK(remaining[0] == page2.value());
}

TEST_CASE("CloseDocumentPage on last page triggers document close") {
    DocumentManager mgr;
    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());

    auto pages = mgr.GetDocumentPages(doc.value());
    REQUIRE(pages.size() == 1);

    auto cl = mgr.CloseDocumentPage(pages[0]);
    CHECK(cl.has_value());
    CHECK(mgr.DocumentCount() == 0);
}

// ---- Notification-based tests ----

TEST_CASE("DocumentCreated notification fires") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);

    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());
    CHECK(spy.lastCreatedDoc == doc.value());
}

TEST_CASE("DocumentSwitched notification fires") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);
    auto d1 = mgr.CreateDocument("Doc1");
    auto d2 = mgr.CreateDocument("Doc2");
    REQUIRE(d1.has_value());
    REQUIRE(d2.has_value());

    auto sw = mgr.SwitchTo(d1.value());
    CHECK(sw.has_value());
    CHECK(spy.lastSwitchedDoc == d1.value());
}

TEST_CASE("DocumentClosing veto prevents close") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);
    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());

    spy.vetoClose = true;

    auto cl = mgr.CloseDocument(doc.value());
    CHECK(!cl.has_value());
    CHECK(cl.error() == ErrorCode::Cancelled);
    CHECK(mgr.DocumentCount() == 1);
}

TEST_CASE("DocumentClosed notification fires after close") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);

    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());
    auto docId = doc.value();

    auto cl = mgr.CloseDocument(docId);
    CHECK(cl.has_value());
    CHECK(spy.lastClosedDoc == docId);
}

TEST_CASE("PageCreated notification fires for auto-created first page") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);

    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());
    CHECK(spy.createdCount == 1);
}

TEST_CASE("PageCreated notification fires for additional page") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);

    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());
    CHECK(spy.createdCount == 1);

    auto page2 = mgr.CreateDocumentPage(doc.value());
    REQUIRE(page2.has_value());
    CHECK(spy.lastCreatedPage == page2.value());
}

TEST_CASE("PageRemoved notification fires for non-last page close") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);

    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());
    auto page2 = mgr.CreateDocumentPage(doc.value());
    REQUIRE(page2.has_value());

    auto pages = mgr.GetDocumentPages(doc.value());
    auto firstPage = pages[0];
    auto cl = mgr.CloseDocumentPage(firstPage);
    CHECK(cl.has_value());
    CHECK(spy.lastRemovedPage == firstPage);
}

TEST_CASE("DocumentClosing veto also prevents last-page close") {
    DocMgrSpy spy(nullptr);
    DocumentManager mgr;
    mgr.SetParent(&spy);
    auto doc = mgr.CreateDocument("Doc1");
    REQUIRE(doc.has_value());

    spy.vetoClose = true;

    auto pages = mgr.GetDocumentPages(doc.value());
    REQUIRE(pages.size() == 1);

    auto cl = mgr.CloseDocumentPage(pages[0]);
    CHECK(!cl.has_value());
    CHECK(cl.error() == ErrorCode::Cancelled);
    CHECK(mgr.DocumentCount() == 1);
}

} // TEST_SUITE
