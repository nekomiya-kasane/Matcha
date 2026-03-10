#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/Foundation/ErrorCode.h"
#include "Matcha/UiNodes/Document/SplitTreeNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Document/Viewport.h"
#include "Matcha/UiNodes/Document/ViewportGroup.h"

using matcha::fw::DropZone;
using matcha::fw::ErrorCode;
using matcha::fw::SplitDirection;
using matcha::fw::Viewport;
using matcha::fw::ViewportGroup;
using matcha::fw::ViewportId;

/// @brief Notification spy that captures ViewportGroup notifications.
class VpGroupSpy : public matcha::CommandNode {
public:
    explicit VpGroupSpy() : CommandNode(nullptr, "VpGroupSpy", matcha::CommandMode::Undefined) {}

    ViewportId lastActiveVpId{};
    bool activeVpCalled = false;
    std::vector<ViewportId> movedIds;

protected:
    auto AnalyseNotification(matcha::CommandNode* /*sender*/,
                              matcha::Notification& notif)
        -> matcha::PropagationMode override
    {
        using namespace matcha::fw;
        if (auto* n = notif.As<ActiveVpChanged>()) {
            lastActiveVpId = n->GetViewportId();
            activeVpCalled = true;
        } else if (auto* n = notif.As<VpMoved>()) {
            movedIds.push_back(n->GetViewportId());
        }
        return matcha::PropagationMode::TransmitToParent;
    }
};

TEST_CASE("ViewportGroup: SplitAndMove top zone -- source becomes first") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    // Move id2 to top of id1
    CHECK(vg.SplitAndMove(id2, id1, DropZone::Top).has_value());
    CHECK(vg.ViewportCount() == 2);

    // The tree should now have id2 on top (first) and id1 on bottom (second)
    // in a vertical split
    auto allIds = vg.AllViewportIds();
    CHECK(allIds.size() == 2);
    // id2 should come first (top) in DFS
    CHECK(allIds[0] == id2);
    CHECK(allIds[1] == id1);
}

TEST_CASE("ViewportGroup: SplitAndMove bottom zone -- source becomes second") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    // Move id2 to bottom of id1
    CHECK(vg.SplitAndMove(id2, id1, DropZone::Bottom).has_value());
    CHECK(vg.ViewportCount() == 2);

    auto allIds = vg.AllViewportIds();
    CHECK(allIds[0] == id1);
    CHECK(allIds[1] == id2);
}

TEST_CASE("ViewportGroup: SplitAndMove left zone -- source becomes first") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id2 = r->get()->GetViewportId();

    CHECK(vg.SplitAndMove(id2, id1, DropZone::Left).has_value());
    CHECK(vg.ViewportCount() == 2);

    auto allIds = vg.AllViewportIds();
    CHECK(allIds[0] == id2);
    CHECK(allIds[1] == id1);
}

TEST_CASE("ViewportGroup: SplitAndMove not found") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    CHECK(!vg.SplitAndMove(id1, ViewportId::From(999), DropZone::Top).has_value());
    CHECK(!vg.SplitAndMove(ViewportId::From(999), id1, DropZone::Top).has_value());
}

TEST_CASE("ViewportGroup: ActiveVpChanged notification fires on SetActive") {
    VpGroupSpy spy;
    ViewportGroup vg("vg");
    vg.SetParent(&spy);
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    vg.SetActiveViewport(id2);
    CHECK(spy.activeVpCalled);
    CHECK(spy.lastActiveVpId == id2);
}

TEST_CASE("ViewportGroup: VpMoved notification fires on SplitAndMove") {
    VpGroupSpy spy;
    ViewportGroup vg("vg");
    vg.SetParent(&spy);
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    spy.movedIds.clear();
    (void)vg.SplitAndMove(id1, id2, DropZone::Right);
    CHECK(spy.movedIds.size() == 1);
    CHECK(spy.movedIds[0] == id1);
}

TEST_CASE("ViewportGroup: VpMoved notification fires on swap") {
    VpGroupSpy spy;
    ViewportGroup vg("vg");
    vg.SetParent(&spy);
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    spy.movedIds.clear();
    (void)vg.SwapViewports(id1, id2);
    CHECK(spy.movedIds.size() == 2);
}

TEST_CASE("ViewportGroup: maximize then restore preserves tree") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    (void)r;

    auto idsBefore = vg.AllViewportIds();

    CHECK(vg.MaximizeViewport(id1).has_value());
    // Tree should be unchanged
    CHECK(vg.ViewportCount() == 2);

    vg.RestoreLayout();
    auto idsAfter = vg.AllViewportIds();
    CHECK(idsBefore == idsAfter);
}

TEST_CASE("ViewportGroup: cannot remove while maximized") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    CHECK(vg.MaximizeViewport(id1).has_value());
    CHECK(!vg.RemoveViewport(id2).has_value());
    CHECK(vg.ViewportCount() == 2);
}

TEST_CASE("ViewportGroup: 4-viewport quad layout via splits") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Split H: [id1 | id2]
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();

    // Split id1 V: [[id1 / id3] | id2]
    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id3 = r2->get()->GetViewportId();

    // Split id2 V: [[id1 / id3] | [id2 / id4]]
    auto r3 = vg.SplitViewport(id2, SplitDirection::Vertical);
    auto id4 = r3->get()->GetViewportId();

    CHECK(vg.ViewportCount() == 4);

    // Verify DFS order: id1, id3, id2, id4
    auto ids = vg.AllViewportIds();
    CHECK(ids.size() == 4);
    CHECK(ids[0] == id1);
    CHECK(ids[1] == id3);
    CHECK(ids[2] == id2);
    CHECK(ids[3] == id4);

    // Swap id1 and id4, verify
    CHECK(vg.SwapViewports(id1, id4).has_value());
    ids = vg.AllViewportIds();
    CHECK(ids[0] == id4);
    CHECK(ids[3] == id1);
}

TEST_CASE("ViewportGroup: SplitAndMove in 3-viewport tree") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // [id1 | id2]
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();

    // [[id1 / id3] | id2]
    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id3 = r2->get()->GetViewportId();
    CHECK(vg.ViewportCount() == 3);

    // Move id3 to the right of id2: id3 detaches, id1 collapses,
    // then id2 splits into [id2 | id3]
    // Tree becomes: [id1 | [id2 | id3]]
    CHECK(vg.SplitAndMove(id3, id2, DropZone::Right).has_value());
    CHECK(vg.ViewportCount() == 3);

    auto ids = vg.AllViewportIds();
    CHECK(ids.size() == 3);
    // id1 first, then id2 and id3 in the right sub-split
    CHECK(ids[0] == id1);
    CHECK(ids[1] == id2);
    CHECK(ids[2] == id3);
}
