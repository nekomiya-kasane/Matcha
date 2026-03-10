#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <ostream>

#include "doctest.h"

#include "Matcha/UiNodes/Document/DocumentArea.h"

#include "Matcha/Foundation/ErrorCode.h"
#include "Matcha/Foundation/StrongId.h"
#include "Matcha/UiNodes/Document/DocumentPage.h"

using matcha::fw::DocumentArea;
using matcha::fw::DocumentId;
using matcha::fw::DocumentPage;
using matcha::fw::ErrorCode;
using matcha::fw::PageId;

TEST_CASE("DocumentArea: constructor") {
    DocumentArea da("test-da");
    CHECK(da.Type() == matcha::fw::NodeType::DocumentArea);
    CHECK(da.PageCount() == 0);
}

TEST_CASE("DocumentArea: AddPage increases count") {
    DocumentArea da("da");
    auto page = std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100));
    da.AddPage(std::move(page));
    CHECK(da.PageCount() == 1);
}

TEST_CASE("DocumentArea: AddPage nullptr is no-op") {
    DocumentArea da("da");
    da.AddPage(nullptr);
    CHECK(da.PageCount() == 0);
}

TEST_CASE("DocumentArea: RemovePage by PageId") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    da.AddPage(std::make_unique<DocumentPage>("p2", PageId::From(2), DocumentId::From(100)));
    CHECK(da.PageCount() == 2);

    auto removed = da.RemovePage(PageId::From(1));
    CHECK(removed != nullptr);
    CHECK(da.PageCount() == 1);
}

TEST_CASE("DocumentArea: RemovePage not found returns nullptr") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    auto removed = da.RemovePage(PageId::From(999));
    CHECK(removed == nullptr);
    CHECK(da.PageCount() == 1);
}

TEST_CASE("DocumentArea: ActivePage returns first page") {
    DocumentArea da("da");
    CHECK(da.ActivePage().get() == nullptr);

    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    auto active = da.ActivePage();
    CHECK(active.get() != nullptr);
    CHECK(active->GetPageId() == PageId::From(1));
}

TEST_CASE("DocumentArea: SwitchPage success and not found") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));

    auto result = da.SwitchPage(PageId::From(1));
    CHECK(result.has_value());

    auto bad = da.SwitchPage(PageId::From(999));
    CHECK(!bad.has_value());
    CHECK(bad.error() == ErrorCode::NotFound);
}

TEST_CASE("DocumentArea: ActivePage tracks SwitchPage") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    da.AddPage(std::make_unique<DocumentPage>("p2", PageId::From(2), DocumentId::From(200)));

    // First added page is active by default
    CHECK(da.ActivePageId().has_value());
    CHECK(da.ActivePageId()->value == 1);
    CHECK(da.ActivePage()->GetPageId() == PageId::From(1));

    // Switch to second page
    auto r = da.SwitchPage(PageId::From(2));
    CHECK(r.has_value());
    CHECK(da.ActivePageId()->value == 2);
    CHECK(da.ActivePage()->GetPageId() == PageId::From(2));
}

TEST_CASE("DocumentArea: RemovePage updates active to another page") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    da.AddPage(std::make_unique<DocumentPage>("p2", PageId::From(2), DocumentId::From(200)));
    CHECK(da.ActivePageId()->value == 1);

    // Remove the active page
    auto removed = da.RemovePage(PageId::From(1));
    CHECK(removed != nullptr);
    CHECK(da.PageCount() == 1);
    // Active should fall back to remaining page
    CHECK(da.ActivePageId().has_value());
    CHECK(da.ActivePageId()->value == 2);
}

TEST_CASE("DocumentArea: RemovePage last page clears active") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    auto removed = da.RemovePage(PageId::From(1));
    CHECK(removed != nullptr);
    CHECK(da.PageCount() == 0);
    CHECK(!da.ActivePageId().has_value());
    CHECK(da.ActivePage().get() == nullptr);
}

TEST_CASE("DocumentArea: FindPage and FindPageByDoc") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    da.AddPage(std::make_unique<DocumentPage>("p2", PageId::From(2), DocumentId::From(200)));

    auto found = da.FindPage(PageId::From(2));
    CHECK(found.get() != nullptr);
    CHECK(found->GetPageId() == PageId::From(2));

    auto notFound = da.FindPage(PageId::From(999));
    CHECK(notFound.get() == nullptr);

    auto byDoc = da.FindPageByDoc(DocumentId::From(200));
    CHECK(byDoc.get() != nullptr);
    CHECK(byDoc->GetPageId() == PageId::From(2));

    auto byDocNotFound = da.FindPageByDoc(DocumentId::From(999));
    CHECK(byDocNotFound.get() == nullptr);
}

TEST_CASE("DocumentArea: CreatePage generates PageId and attaches ViewportGroup") {
    DocumentArea da("da");
    auto page = da.CreatePage("test page", DocumentId::From(100));
    CHECK(page.get() != nullptr);
    CHECK(da.PageCount() == 1);
    CHECK(page->GetDocId() == DocumentId::From(100));
    // PageId should be auto-generated (starting from 1)
    CHECK(page->GetPageId().value >= 1);
    // Should have a ViewportGroup child
    CHECK(page->GetViewportGroup().get() != nullptr);
    // Should be active (first page)
    CHECK(da.ActivePageId().has_value());
    CHECK(da.ActivePageId().value() == page->GetPageId());
}

TEST_CASE("DocumentArea: CreatePage multiple pages get unique PageIds") {
    DocumentArea da("da");
    auto p1 = da.CreatePage("page1", DocumentId::From(100));
    auto p2 = da.CreatePage("page2", DocumentId::From(200));
    CHECK(p1->GetPageId() != p2->GetPageId());
    CHECK(da.PageCount() == 2);
}
