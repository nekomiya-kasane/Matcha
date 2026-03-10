#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <ostream>

#include "doctest.h"

#include "Matcha/Foundation/StrongId.h"
#include "Matcha/UiNodes/Document/DocumentArea.h"
#include "Matcha/UiNodes/Document/DocumentPage.h"
#include "Matcha/UiNodes/Document/ViewportGroup.h"
#include "Matcha/UiNodes/Document/Viewport.h"

using matcha::fw::DocumentArea;
using matcha::fw::DocumentId;
using matcha::fw::DocumentPage;
using matcha::fw::PageId;
using matcha::fw::ViewportGroup;
using matcha::fw::Viewport;
using matcha::fw::ViewportId;

TEST_SUITE("DocumentLifecycle") {

TEST_CASE("Create document page and add to document area") {
    DocumentArea da("da");
    auto page = std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100));
    da.AddPage(std::move(page));
    CHECK(da.PageCount() == 1);
    CHECK(da.ActivePage().get() != nullptr);
    CHECK(da.ActivePage()->GetPageId() == PageId::From(1));
    CHECK(da.ActivePage()->GetDocId() == DocumentId::From(100));
}

TEST_CASE("Add viewport group to document page and split") {
    DocumentPage page("p1", PageId::From(1), DocumentId::From(100));

    auto vg = std::make_unique<ViewportGroup>("vg1");
    auto initialId = vg->AllViewportIds()[0];
    page.AddNode(std::move(vg));

    auto vpGroup = page.GetViewportGroup();
    REQUIRE(vpGroup.get() != nullptr);
    CHECK(vpGroup->ViewportCount() == 1);

    auto result = vpGroup->SplitViewport(initialId, matcha::fw::SplitDirection::Horizontal);
    CHECK(result.has_value());
    CHECK(vpGroup->ViewportCount() == 2);
}

TEST_CASE("Switch between document pages") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    da.AddPage(std::make_unique<DocumentPage>("p2", PageId::From(2), DocumentId::From(200)));
    CHECK(da.PageCount() == 2);

    auto result = da.SwitchPage(PageId::From(2));
    CHECK(result.has_value());
}

TEST_CASE("Close document page removes from area") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    da.AddPage(std::make_unique<DocumentPage>("p2", PageId::From(2), DocumentId::From(200)));
    CHECK(da.PageCount() == 2);

    auto removed = da.RemovePage(PageId::From(1));
    CHECK(removed != nullptr);
    CHECK(da.PageCount() == 1);
    CHECK(da.ActivePage()->GetPageId() == PageId::From(2));
}

TEST_CASE("Close all document pages leaves empty area") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    CHECK(da.PageCount() == 1);

    auto removed = da.RemovePage(PageId::From(1));
    CHECK(removed != nullptr);
    CHECK(da.PageCount() == 0);
    CHECK(da.ActivePage().get() == nullptr);
}

TEST_CASE("Multiple pages for same document (1:N model)") {
    DocumentArea da("da");
    da.AddPage(std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100)));
    da.AddPage(std::make_unique<DocumentPage>("p2", PageId::From(2), DocumentId::From(100)));
    CHECK(da.PageCount() == 2);

    // Both pages reference same document
    // Remove one page, other remains
    da.RemovePage(PageId::From(1));
    CHECK(da.PageCount() == 1);
    CHECK(da.ActivePage()->GetDocId() == DocumentId::From(100));
}

TEST_CASE("Document page with viewport group hierarchy") {
    DocumentArea da("da");

    auto page = std::make_unique<DocumentPage>("p1", PageId::From(1), DocumentId::From(100));

    auto vg = std::make_unique<ViewportGroup>("vg");
    auto initialId = vg->AllViewportIds()[0];
    page->AddNode(std::move(vg));

    auto* rawPage = page.get();
    da.AddPage(std::move(page));

    auto vpGroup = rawPage->GetViewportGroup();
    REQUIRE(vpGroup.get() != nullptr);

    auto active = vpGroup->ActiveViewport();
    CHECK(active.get() != nullptr);
    CHECK(active->GetViewportId() == initialId);
}

TEST_CASE("Viewport split and remove lifecycle") {
    ViewportGroup vg("vg");
    auto initialId = vg.AllViewportIds()[0];

    // Split creates second viewport
    auto splitResult = vg.SplitViewport(initialId, matcha::fw::SplitDirection::Vertical);
    CHECK(splitResult.has_value());
    CHECK(vg.ViewportCount() == 2);

    auto newId = splitResult->get()->GetViewportId();

    // Remove original viewport
    auto removeResult = vg.RemoveViewport(initialId);
    CHECK(removeResult.has_value());
    CHECK(vg.ViewportCount() == 1);

    // Cannot remove last viewport
    auto lastRemove = vg.RemoveViewport(newId);
    CHECK(!lastRemove.has_value());
    CHECK(vg.ViewportCount() == 1);
}

} // TEST_SUITE
