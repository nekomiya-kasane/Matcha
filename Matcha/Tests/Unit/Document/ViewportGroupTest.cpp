#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <ostream>

#include "doctest.h"

#include "Matcha/UiNodes/Document/SplitTreeNode.h"
#include "Matcha/UiNodes/Document/ViewportGroup.h"

#include "Matcha/Foundation/ErrorCode.h"
#include "Matcha/Foundation/StrongId.h"
#include "Matcha/UiNodes/Document/Viewport.h"

using matcha::fw::DropZone;
using matcha::fw::ErrorCode;
using matcha::fw::NodeType;
using matcha::fw::SplitDirection;
using matcha::fw::Viewport;
using matcha::fw::ViewportGroup;
using matcha::fw::ViewportGroupState;
using matcha::fw::ViewportId;

TEST_CASE("ViewportGroup: constructor creates initial viewport") {
    ViewportGroup vg("test-vg");
    CHECK(vg.Type() == NodeType::ViewportGroup);
    CHECK(vg.ViewportCount() == 1);
    CHECK(vg.State() == ViewportGroupState::Normal);

    auto active = vg.ActiveViewport();
    CHECK(active.get() != nullptr);
}

TEST_CASE("ViewportGroup: SplitViewport creates binary split") {
    ViewportGroup vg("vg");
    auto ids = vg.AllViewportIds();
    REQUIRE(ids.size() == 1);

    auto result = vg.SplitViewport(ids[0], SplitDirection::Horizontal);
    CHECK(result.has_value());
    CHECK(result->get() != nullptr);
    CHECK(vg.ViewportCount() == 2);

    // Tree root should now be a SplitNode
    auto* root = vg.TreeRoot();
    CHECK(root != nullptr);
    CHECK(matcha::fw::IsSplit(*root));
}

TEST_CASE("ViewportGroup: SplitViewport not found") {
    ViewportGroup vg("vg");
    auto result = vg.SplitViewport(ViewportId::From(999), SplitDirection::Vertical);
    CHECK(!result.has_value());
    CHECK(result.error() == ErrorCode::NotFound);
}

TEST_CASE("ViewportGroup: RemoveViewport collapses split") {
    ViewportGroup vg("vg");
    auto ids = vg.AllViewportIds();
    auto id1 = ids[0];

    auto splitResult = vg.SplitViewport(id1, SplitDirection::Horizontal);
    REQUIRE(splitResult.has_value());
    auto id2 = splitResult->get()->GetViewportId();
    CHECK(vg.ViewportCount() == 2);

    auto result = vg.RemoveViewport(id2);
    CHECK(result.has_value());
    CHECK(vg.ViewportCount() == 1);

    // Tree root should be back to a leaf
    CHECK(matcha::fw::IsLeaf(*vg.TreeRoot()));
}

TEST_CASE("ViewportGroup: RemoveViewport last viewport fails") {
    ViewportGroup vg("vg");
    auto ids = vg.AllViewportIds();

    auto result = vg.RemoveViewport(ids[0]);
    CHECK(!result.has_value());
    CHECK(result.error() == ErrorCode::InvalidArgument);
}

TEST_CASE("ViewportGroup: RemoveViewport not found") {
    ViewportGroup vg("vg");

    auto result = vg.RemoveViewport(ViewportId::From(999));
    CHECK(!result.has_value());
    CHECK(result.error() == ErrorCode::NotFound);
}

TEST_CASE("ViewportGroup: ActiveViewport tracks initial") {
    ViewportGroup vg("vg");
    auto active = vg.ActiveViewport();
    CHECK(active.get() != nullptr);
    CHECK(active->GetViewportId() == vg.AllViewportIds()[0]);
}

TEST_CASE("ViewportGroup: SetActiveViewport changes active") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto splitResult = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id2 = splitResult->get()->GetViewportId();

    vg.SetActiveViewport(id2);
    CHECK(vg.ActiveViewport()->GetViewportId() == id2);
}

TEST_CASE("ViewportGroup: MaximizeViewport state transitions") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    CHECK(vg.MaximizeViewport(id1).has_value());
    CHECK(vg.State() == ViewportGroupState::Maximized);

    // Cannot maximize again while maximized
    CHECK(!vg.MaximizeViewport(id1).has_value());

    // Cannot split while maximized
    CHECK(!vg.SplitViewport(id1, SplitDirection::Horizontal).has_value());

    // Restore
    vg.RestoreLayout();
    CHECK(vg.State() == ViewportGroupState::Normal);
}

TEST_CASE("ViewportGroup: MaximizeViewport not found") {
    ViewportGroup vg("vg");
    CHECK(!vg.MaximizeViewport(ViewportId::From(999)).has_value());
}

TEST_CASE("ViewportGroup: SwapViewports in tree") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto splitResult = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = splitResult->get()->GetViewportId();

    // Before swap: tree is Split([id1], [id2])
    auto* root = vg.TreeRoot();
    REQUIRE(matcha::fw::IsSplit(*root));
    auto& sn = matcha::fw::AsSplit(*root);
    CHECK(matcha::fw::AsLeaf(*sn.first).viewportId == id1);
    CHECK(matcha::fw::AsLeaf(*sn.second).viewportId == id2);

    // Swap
    CHECK(vg.SwapViewports(id1, id2).has_value());

    // After swap: tree is Split([id2], [id1])
    root = vg.TreeRoot();
    auto& snAfter = matcha::fw::AsSplit(*root);
    CHECK(matcha::fw::AsLeaf(*snAfter.first).viewportId == id2);
    CHECK(matcha::fw::AsLeaf(*snAfter.second).viewportId == id1);
}

TEST_CASE("ViewportGroup: SwapViewports validation") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto splitResult = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = splitResult->get()->GetViewportId();

    CHECK(vg.SwapViewports(id1, id2).has_value());
    CHECK(!vg.SwapViewports(id1, id1).has_value());
    CHECK(!vg.SwapViewports(id1, ViewportId::From(999)).has_value());
}

TEST_CASE("ViewportGroup: SplitAndMove to right zone") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Create 2 viewports via split
    auto splitResult = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id2 = splitResult->get()->GetViewportId();
    CHECK(vg.ViewportCount() == 2);

    // Now split-and-move id1 to the right of id2
    CHECK(vg.SplitAndMove(id1, id2, DropZone::Right).has_value());
    CHECK(vg.ViewportCount() == 2);

    // id1 and id2 should both still be in the tree
    auto allIds = vg.AllViewportIds();
    CHECK(allIds.size() == 2);
}

TEST_CASE("ViewportGroup: SplitAndMove center zone swaps") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto splitResult = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = splitResult->get()->GetViewportId();

    // Center zone = swap
    CHECK(vg.SplitAndMove(id1, id2, DropZone::Center).has_value());
    CHECK(vg.ViewportCount() == 2);
}

TEST_CASE("ViewportGroup: SplitAndMove self fails") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    CHECK(!vg.SplitAndMove(id1, id1, DropZone::Top).has_value());
}

TEST_CASE("ViewportGroup: three-level deep split tree") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Split horizontally: [id1 | id2]
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();

    // Split id1 vertically: [[id1 / id3] | id2]
    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id3 = r2->get()->GetViewportId();

    CHECK(vg.ViewportCount() == 3);

    // Split id2 vertically: [[id1 / id3] | [id2 / id4]]
    auto r3 = vg.SplitViewport(id2, SplitDirection::Vertical);
    auto id4 = r3->get()->GetViewportId();

    CHECK(vg.ViewportCount() == 4);

    // Remove id3 -- collapses its parent, tree becomes: [id1 | [id2 / id4]]
    CHECK(vg.RemoveViewport(id3).has_value());
    CHECK(vg.ViewportCount() == 3);

    // Remove id4 -- collapses its parent, tree becomes: [id1 | id2]
    CHECK(vg.RemoveViewport(id4).has_value());
    CHECK(vg.ViewportCount() == 2);
}

TEST_CASE("ViewportGroup: RemoveViewport updates active") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    // Set active to id2, then remove it
    vg.SetActiveViewport(id2);
    CHECK(vg.ActiveViewport()->GetViewportId() == id2);

    CHECK(vg.RemoveViewport(id2).has_value());
    // Active should fallback to first remaining
    auto active = vg.ActiveViewport();
    CHECK(active.get() != nullptr);
    CHECK(active->GetViewportId() == id1);
}

TEST_CASE("ViewportGroup: FindViewport returns correct pointer") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    auto* vp1 = vg.FindViewport(id1);
    auto* vp2 = vg.FindViewport(id2);
    CHECK(vp1 != nullptr);
    CHECK(vp2 != nullptr);
    CHECK(vp1 != vp2);
    CHECK(vp1->GetViewportId() == id1);
    CHECK(vp2->GetViewportId() == id2);

    CHECK(vg.FindViewport(ViewportId::From(999)) == nullptr);
}

// ============================================================================
// Simulated user interaction scenarios
// ============================================================================

TEST_CASE("Scenario: rapid split then remove all but one") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // User splits 5 times rapidly
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();
    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id3 = r2->get()->GetViewportId();
    auto r3 = vg.SplitViewport(id2, SplitDirection::Vertical);
    auto id4 = r3->get()->GetViewportId();
    auto r4 = vg.SplitViewport(id3, SplitDirection::Horizontal);
    auto id5 = r4->get()->GetViewportId();
    CHECK(vg.ViewportCount() == 5);

    // User closes all but id1
    CHECK(vg.RemoveViewport(id5).has_value());
    CHECK(vg.RemoveViewport(id4).has_value());
    CHECK(vg.RemoveViewport(id3).has_value());
    CHECK(vg.RemoveViewport(id2).has_value());
    CHECK(vg.ViewportCount() == 1);
    CHECK(matcha::fw::IsLeaf(*vg.TreeRoot()));
    CHECK(vg.AllViewportIds()[0] == id1);
}

TEST_CASE("Scenario: 5-level deep binary tree") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Build a left-leaning chain: each split adds depth on the first child
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();

    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id3 = r2->get()->GetViewportId();

    auto r3 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id4 = r3->get()->GetViewportId();

    auto r4 = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id5 = r4->get()->GetViewportId();

    CHECK(vg.ViewportCount() == 5);

    // Verify all can be found
    CHECK(vg.FindViewport(id1) != nullptr);
    CHECK(vg.FindViewport(id2) != nullptr);
    CHECK(vg.FindViewport(id3) != nullptr);
    CHECK(vg.FindViewport(id4) != nullptr);
    CHECK(vg.FindViewport(id5) != nullptr);

    // Remove the deepest leaf (id5), tree collapses one level
    CHECK(vg.RemoveViewport(id5).has_value());
    CHECK(vg.ViewportCount() == 4);

    // Remove id4, further collapse
    CHECK(vg.RemoveViewport(id4).has_value());
    CHECK(vg.ViewportCount() == 3);
}

TEST_CASE("Scenario: chain of SplitAndMove operations") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Create 3 viewports: [id1 | id2], then split id2 -> [id1 | [id2 / id3]]
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();
    auto r2 = vg.SplitViewport(id2, SplitDirection::Vertical);
    auto id3 = r2->get()->GetViewportId();
    CHECK(vg.ViewportCount() == 3);

    // Move id3 to the left of id1: detach id3, split id1 with id3 on left
    CHECK(vg.SplitAndMove(id3, id1, DropZone::Left).has_value());
    CHECK(vg.ViewportCount() == 3);

    // Now move id1 to the bottom of id2
    CHECK(vg.SplitAndMove(id1, id2, DropZone::Bottom).has_value());
    CHECK(vg.ViewportCount() == 3);

    // All three IDs should still exist
    auto ids = vg.AllViewportIds();
    CHECK(ids.size() == 3);
    bool found1 = false, found2 = false, found3 = false;
    for (auto vid : ids) {
        if (vid == id1) { found1 = true; }
        if (vid == id2) { found2 = true; }
        if (vid == id3) { found3 = true; }
    }
    CHECK(found1);
    CHECK(found2);
    CHECK(found3);
}

TEST_CASE("Scenario: swap then split then remove") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();

    // Swap
    CHECK(vg.SwapViewports(id1, id2).has_value());
    auto ids = vg.AllViewportIds();
    CHECK(ids[0] == id2);
    CHECK(ids[1] == id1);

    // Split the swapped id2 (now at position 0)
    auto r2 = vg.SplitViewport(id2, SplitDirection::Vertical);
    (void)r2;
    CHECK(vg.ViewportCount() == 3);

    // Remove id1 (still in tree at position 1 of root split)
    CHECK(vg.RemoveViewport(id1).has_value());
    CHECK(vg.ViewportCount() == 2);
    ids = vg.AllViewportIds();
    CHECK(ids.size() == 2);
}

TEST_CASE("Scenario: maximize during complex tree, restore, then modify") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Build 2x2 quad
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();
    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    (void)r2;
    auto r3 = vg.SplitViewport(id2, SplitDirection::Vertical);
    auto id4 = r3->get()->GetViewportId();
    CHECK(vg.ViewportCount() == 4);

    auto idsBefore = vg.AllViewportIds();

    // Maximize id1
    CHECK(vg.MaximizeViewport(id1).has_value());
    CHECK(vg.State() == ViewportGroupState::Maximized);

    // Cannot split or remove while maximized
    CHECK(!vg.SplitViewport(id1, SplitDirection::Horizontal).has_value());
    CHECK(!vg.RemoveViewport(id2).has_value());

    // Restore
    vg.RestoreLayout();
    CHECK(vg.State() == ViewportGroupState::Normal);

    // Tree should be identical
    CHECK(vg.AllViewportIds() == idsBefore);

    // Now modify: remove id4
    CHECK(vg.RemoveViewport(id4).has_value());
    CHECK(vg.ViewportCount() == 3);
}

TEST_CASE("Scenario: alternating split directions build balanced tree") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // H split: [id1 | id2]
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();

    // V split id1: [[id1 / id3] | id2]
    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    (void)r2;

    // V split id2: [[id1 / id3] | [id2 / id4]]
    auto r3 = vg.SplitViewport(id2, SplitDirection::Vertical);
    (void)r3;

    CHECK(vg.ViewportCount() == 4);

    // Verify tree structure
    const auto* root = vg.TreeRoot();
    REQUIRE(matcha::fw::IsSplit(*root));
    const auto& rootSplit = matcha::fw::AsSplit(*root);
    CHECK(rootSplit.direction == SplitDirection::Horizontal);

    // Both children should be vertical splits
    REQUIRE(matcha::fw::IsSplit(*rootSplit.first));
    REQUIRE(matcha::fw::IsSplit(*rootSplit.second));
    CHECK(matcha::fw::AsSplit(*rootSplit.first).direction == SplitDirection::Vertical);
    CHECK(matcha::fw::AsSplit(*rootSplit.second).direction == SplitDirection::Vertical);
}

TEST_CASE("Scenario: SplitAndMove with only 2 viewports reverses order") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    // Tree: [id1 | id2]
    auto ids = vg.AllViewportIds();
    CHECK(ids[0] == id1);
    CHECK(ids[1] == id2);

    // Move id2 to the left of id1
    CHECK(vg.SplitAndMove(id2, id1, DropZone::Left).has_value());
    ids = vg.AllViewportIds();
    CHECK(ids[0] == id2);
    CHECK(ids[1] == id1);
}

TEST_CASE("Scenario: remove middle viewport in 3-deep left chain") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Build: [id1 | id2]
    auto r1 = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r1->get()->GetViewportId();

    // Split id1 vertically: [[id1 / id3] | id2]
    auto r2 = vg.SplitViewport(id1, SplitDirection::Vertical);
    auto id3 = r2->get()->GetViewportId();
    CHECK(vg.ViewportCount() == 3);

    // Remove id3 (second child of inner split) -> inner split collapses
    CHECK(vg.RemoveViewport(id3).has_value());
    CHECK(vg.ViewportCount() == 2);

    // Tree should be [id1 | id2] again
    auto ids = vg.AllViewportIds();
    CHECK(ids[0] == id1);
    CHECK(ids[1] == id2);

    // Now remove id1 (first child of root split) -> root collapses to leaf
    CHECK(vg.RemoveViewport(id1).has_value());
    CHECK(vg.ViewportCount() == 1);
    CHECK(matcha::fw::IsLeaf(*vg.TreeRoot()));
    CHECK(vg.AllViewportIds()[0] == id2);
}

// ============================================================================
// Drag-and-drop framework API tests
// ============================================================================

TEST_CASE("ViewportGroup: BeginDrag requires Normal state and >1 viewport") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];

    // Cannot drag with only 1 viewport
    CHECK(!vg.BeginDrag(id1));
    CHECK(vg.State() == ViewportGroupState::Normal);

    // Split to get 2 viewports
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    // Can drag now
    CHECK(vg.BeginDrag(id1));
    CHECK(vg.State() == ViewportGroupState::Dragging);

    // Cannot start another drag while dragging
    CHECK(!vg.BeginDrag(id2));
}

TEST_CASE("ViewportGroup: HandleDrop completes drag with SplitAndMove") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    CHECK(vg.BeginDrag(id2));
    auto result = vg.HandleDrop(id2, id1, DropZone::Top);
    CHECK(result.has_value());
    CHECK(vg.State() == ViewportGroupState::Normal);
    CHECK(vg.ViewportCount() == 2);
}

TEST_CASE("ViewportGroup: HandleDrop Center swaps viewports") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    auto idsBefore = vg.AllViewportIds();
    CHECK(vg.BeginDrag(id1));
    auto result = vg.HandleDrop(id1, id2, DropZone::Center);
    CHECK(result.has_value());

    auto idsAfter = vg.AllViewportIds();
    CHECK(idsAfter[0] == idsBefore[1]);
    CHECK(idsAfter[1] == idsBefore[0]);
}

TEST_CASE("ViewportGroup: HandleDrop fails if not dragging") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    auto id2 = r->get()->GetViewportId();

    auto result = vg.HandleDrop(id1, id2, DropZone::Left);
    CHECK(!result.has_value());
}

TEST_CASE("ViewportGroup: CancelDrag returns to Normal") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    (void)r;

    CHECK(vg.BeginDrag(id1));
    CHECK(vg.State() == ViewportGroupState::Dragging);

    vg.CancelDrag();
    CHECK(vg.State() == ViewportGroupState::Normal);
}

TEST_CASE("ViewportGroup: CancelDrag is no-op if not dragging") {
    ViewportGroup vg("vg");
    vg.CancelDrag(); // should not crash
    CHECK(vg.State() == ViewportGroupState::Normal);
}

TEST_CASE("ViewportGroup: BeginDrag fails when maximized") {
    ViewportGroup vg("vg");
    auto id1 = vg.AllViewportIds()[0];
    auto r = vg.SplitViewport(id1, SplitDirection::Horizontal);
    (void)r;

    CHECK(vg.MaximizeViewport(id1).has_value());
    CHECK(!vg.BeginDrag(id1));
}
