#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/Tree/Composition/Document/SplitTreeNode.h"

using matcha::fw::AsLeaf;
using matcha::fw::AsSplit;
using matcha::fw::CollectLeafIds;
using matcha::fw::CountLeaves;
using matcha::fw::DropZone;
using matcha::fw::DropZoneToDirection;
using matcha::fw::FindLeaf;
using matcha::fw::FindParentOf;
using matcha::fw::IsLeaf;
using matcha::fw::IsSourceFirst;
using matcha::fw::IsSplit;
using matcha::fw::LeafNode;
using matcha::fw::MakeLeaf;
using matcha::fw::MakeSplit;
using matcha::fw::SplitDirection;
using matcha::fw::SplitNode;
using matcha::fw::TreeNode;
using matcha::fw::ViewportId;

// ============================================================================
// MakeLeaf / MakeSplit factory tests
// ============================================================================

TEST_CASE("SplitTreeNode: MakeLeaf creates leaf") {
    auto leaf = MakeLeaf(ViewportId::From(1), nullptr);
    REQUIRE(leaf != nullptr);
    CHECK(IsLeaf(*leaf));
    CHECK(!IsSplit(*leaf));
    CHECK(AsLeaf(*leaf).viewportId == ViewportId::From(1));
    CHECK(AsLeaf(*leaf).viewport == nullptr);
}

TEST_CASE("SplitTreeNode: MakeSplit creates split") {
    auto a = MakeLeaf(ViewportId::From(1), nullptr);
    auto b = MakeLeaf(ViewportId::From(2), nullptr);
    auto split = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(a), std::move(b));

    REQUIRE(split != nullptr);
    CHECK(IsSplit(*split));
    CHECK(!IsLeaf(*split));

    const auto& sn = AsSplit(*split);
    CHECK(sn.direction == SplitDirection::Horizontal);
    CHECK(sn.ratio == doctest::Approx(0.5));
    REQUIRE(sn.first != nullptr);
    REQUIRE(sn.second != nullptr);
    CHECK(AsLeaf(*sn.first).viewportId == ViewportId::From(1));
    CHECK(AsLeaf(*sn.second).viewportId == ViewportId::From(2));
}

// ============================================================================
// FindLeaf tests
// ============================================================================

TEST_CASE("SplitTreeNode: FindLeaf in single leaf") {
    auto leaf = MakeLeaf(ViewportId::From(42), nullptr);
    CHECK(FindLeaf(leaf.get(), ViewportId::From(42)) == leaf.get());
    CHECK(FindLeaf(leaf.get(), ViewportId::From(99)) == nullptr);
}

TEST_CASE("SplitTreeNode: FindLeaf in nullptr") {
    CHECK(FindLeaf(nullptr, ViewportId::From(1)) == nullptr);
}

TEST_CASE("SplitTreeNode: FindLeaf in split tree") {
    auto a = MakeLeaf(ViewportId::From(1), nullptr);
    auto b = MakeLeaf(ViewportId::From(2), nullptr);
    auto* bPtr = b.get();
    auto split = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(a), std::move(b));

    CHECK(FindLeaf(split.get(), ViewportId::From(2)) == bPtr);
    CHECK(FindLeaf(split.get(), ViewportId::From(99)) == nullptr);
}

TEST_CASE("SplitTreeNode: FindLeaf in 3-level tree") {
    //       Split(H)
    //      /        \
    //   Split(V)    [3]
    //   /     \
    // [1]    [2]
    auto l1 = MakeLeaf(ViewportId::From(1), nullptr);
    auto l2 = MakeLeaf(ViewportId::From(2), nullptr);
    auto l3 = MakeLeaf(ViewportId::From(3), nullptr);
    auto* l2Ptr = l2.get();
    auto innerSplit = MakeSplit(SplitDirection::Vertical, 0.5, std::move(l1), std::move(l2));
    auto root = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(innerSplit), std::move(l3));

    CHECK(FindLeaf(root.get(), ViewportId::From(2)) == l2Ptr);
    CHECK(FindLeaf(root.get(), ViewportId::From(3)) != nullptr);
    CHECK(FindLeaf(root.get(), ViewportId::From(99)) == nullptr);
}

// ============================================================================
// FindParentOf tests
// ============================================================================

TEST_CASE("SplitTreeNode: FindParentOf returns nullptr for root leaf") {
    auto leaf = MakeLeaf(ViewportId::From(1), nullptr);
    CHECK(FindParentOf(leaf.get(), ViewportId::From(1)) == nullptr);
}

TEST_CASE("SplitTreeNode: FindParentOf returns nullptr for nullptr") {
    CHECK(FindParentOf(nullptr, ViewportId::From(1)) == nullptr);
}

TEST_CASE("SplitTreeNode: FindParentOf direct child") {
    auto a = MakeLeaf(ViewportId::From(1), nullptr);
    auto b = MakeLeaf(ViewportId::From(2), nullptr);
    auto split = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(a), std::move(b));

    CHECK(FindParentOf(split.get(), ViewportId::From(1)) == split.get());
    CHECK(FindParentOf(split.get(), ViewportId::From(2)) == split.get());
    CHECK(FindParentOf(split.get(), ViewportId::From(99)) == nullptr);
}

TEST_CASE("SplitTreeNode: FindParentOf in 3-level tree") {
    auto l1 = MakeLeaf(ViewportId::From(1), nullptr);
    auto l2 = MakeLeaf(ViewportId::From(2), nullptr);
    auto l3 = MakeLeaf(ViewportId::From(3), nullptr);
    auto innerSplit = MakeSplit(SplitDirection::Vertical, 0.5, std::move(l1), std::move(l2));
    auto* innerPtr = innerSplit.get();
    auto root = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(innerSplit), std::move(l3));

    // Parent of l1 and l2 is the inner split
    CHECK(FindParentOf(root.get(), ViewportId::From(1)) == innerPtr);
    CHECK(FindParentOf(root.get(), ViewportId::From(2)) == innerPtr);
    // Parent of l3 is root
    CHECK(FindParentOf(root.get(), ViewportId::From(3)) == root.get());
}

// ============================================================================
// CountLeaves tests
// ============================================================================

TEST_CASE("SplitTreeNode: CountLeaves nullptr") {
    CHECK(CountLeaves(nullptr) == 0);
}

TEST_CASE("SplitTreeNode: CountLeaves single leaf") {
    auto leaf = MakeLeaf(ViewportId::From(1), nullptr);
    CHECK(CountLeaves(leaf.get()) == 1);
}

TEST_CASE("SplitTreeNode: CountLeaves split with 2 leaves") {
    auto a = MakeLeaf(ViewportId::From(1), nullptr);
    auto b = MakeLeaf(ViewportId::From(2), nullptr);
    auto split = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(a), std::move(b));
    CHECK(CountLeaves(split.get()) == 2);
}

TEST_CASE("SplitTreeNode: CountLeaves 4 leaves") {
    auto l1 = MakeLeaf(ViewportId::From(1), nullptr);
    auto l2 = MakeLeaf(ViewportId::From(2), nullptr);
    auto l3 = MakeLeaf(ViewportId::From(3), nullptr);
    auto l4 = MakeLeaf(ViewportId::From(4), nullptr);
    auto left = MakeSplit(SplitDirection::Vertical, 0.5, std::move(l1), std::move(l2));
    auto right = MakeSplit(SplitDirection::Vertical, 0.5, std::move(l3), std::move(l4));
    auto root = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(left), std::move(right));
    CHECK(CountLeaves(root.get()) == 4);
}

// ============================================================================
// CollectLeafIds tests
// ============================================================================

TEST_CASE("SplitTreeNode: CollectLeafIds nullptr") {
    std::vector<ViewportId> ids;
    CollectLeafIds(nullptr, ids);
    CHECK(ids.empty());
}

TEST_CASE("SplitTreeNode: CollectLeafIds single leaf") {
    auto leaf = MakeLeaf(ViewportId::From(7), nullptr);
    std::vector<ViewportId> ids;
    CollectLeafIds(leaf.get(), ids);
    REQUIRE(ids.size() == 1);
    CHECK(ids[0] == ViewportId::From(7));
}

TEST_CASE("SplitTreeNode: CollectLeafIds DFS order") {
    //       Split(H)
    //      /        \
    //   Split(V)    [3]
    //   /     \
    // [1]    [2]
    auto l1 = MakeLeaf(ViewportId::From(1), nullptr);
    auto l2 = MakeLeaf(ViewportId::From(2), nullptr);
    auto l3 = MakeLeaf(ViewportId::From(3), nullptr);
    auto innerSplit = MakeSplit(SplitDirection::Vertical, 0.5, std::move(l1), std::move(l2));
    auto root = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(innerSplit), std::move(l3));

    std::vector<ViewportId> ids;
    CollectLeafIds(root.get(), ids);
    REQUIRE(ids.size() == 3);
    CHECK(ids[0] == ViewportId::From(1));
    CHECK(ids[1] == ViewportId::From(2));
    CHECK(ids[2] == ViewportId::From(3));
}

TEST_CASE("SplitTreeNode: CollectLeafIds 4-leaf quad") {
    auto l1 = MakeLeaf(ViewportId::From(10), nullptr);
    auto l2 = MakeLeaf(ViewportId::From(20), nullptr);
    auto l3 = MakeLeaf(ViewportId::From(30), nullptr);
    auto l4 = MakeLeaf(ViewportId::From(40), nullptr);
    auto left = MakeSplit(SplitDirection::Vertical, 0.5, std::move(l1), std::move(l2));
    auto right = MakeSplit(SplitDirection::Vertical, 0.5, std::move(l3), std::move(l4));
    auto root = MakeSplit(SplitDirection::Horizontal, 0.5, std::move(left), std::move(right));

    std::vector<ViewportId> ids;
    CollectLeafIds(root.get(), ids);
    REQUIRE(ids.size() == 4);
    CHECK(ids[0] == ViewportId::From(10));
    CHECK(ids[1] == ViewportId::From(20));
    CHECK(ids[2] == ViewportId::From(30));
    CHECK(ids[3] == ViewportId::From(40));
}

// ============================================================================
// DropZone utility tests
// ============================================================================

TEST_CASE("SplitTreeNode: DropZoneToDirection") {
    CHECK(DropZoneToDirection(DropZone::Top) == SplitDirection::Vertical);
    CHECK(DropZoneToDirection(DropZone::Bottom) == SplitDirection::Vertical);
    CHECK(DropZoneToDirection(DropZone::Left) == SplitDirection::Horizontal);
    CHECK(DropZoneToDirection(DropZone::Right) == SplitDirection::Horizontal);
    CHECK(DropZoneToDirection(DropZone::Center) == SplitDirection::Horizontal);
}

TEST_CASE("SplitTreeNode: IsSourceFirst") {
    CHECK(IsSourceFirst(DropZone::Top) == true);
    CHECK(IsSourceFirst(DropZone::Left) == true);
    CHECK(IsSourceFirst(DropZone::Bottom) == false);
    CHECK(IsSourceFirst(DropZone::Right) == false);
    CHECK(IsSourceFirst(DropZone::Center) == false);
}

// ============================================================================
// AsLeaf / AsSplit accessor tests
// ============================================================================

TEST_CASE("SplitTreeNode: AsLeaf mutable access") {
    auto leaf = MakeLeaf(ViewportId::From(5), nullptr);
    auto& data = AsLeaf(*leaf);
    data.viewportId = ViewportId::From(99);
    CHECK(AsLeaf(*leaf).viewportId == ViewportId::From(99));
}

TEST_CASE("SplitTreeNode: AsSplit mutable access") {
    auto a = MakeLeaf(ViewportId::From(1), nullptr);
    auto b = MakeLeaf(ViewportId::From(2), nullptr);
    auto split = MakeSplit(SplitDirection::Horizontal, 0.3, std::move(a), std::move(b));
    auto& sn = AsSplit(*split);
    sn.ratio = 0.7;
    CHECK(AsSplit(*split).ratio == doctest::Approx(0.7));
}
