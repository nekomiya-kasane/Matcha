#pragma once

/**
 * @file SplitTreeNode.h
 * @brief Binary split tree data structure for viewport layout topology.
 *
 * This is a pure data structure describing how viewports are spatially arranged.
 * It is NOT part of the UiNode child list. ViewportGroup owns the tree root
 * and uses it to drive QSplitter widget creation.
 *
 * TreeNode = SplitNode | LeafNode (tagged union via variant)
 *
 * @see ViewportGroup.h for the owning UiNode.
 * @see 05_Greenfield_Plan.md "Multi-Viewport State Machine" section.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/StrongId.h>

#include <memory>
#include <optional>
#include <variant>

namespace matcha::fw {

class Viewport;

/** @brief Split direction for binary viewport tree. */
enum class SplitDirection : uint8_t {
    Horizontal, ///< Left | Right
    Vertical,   ///< Top / Bottom
};

/** @brief Drop zone indicating where a viewport is dropped relative to target. */
enum class DropZone : uint8_t {
    Center = 0, ///< Swap
    Top    = 1,
    Bottom = 2,
    Left   = 3,
    Right  = 4,
};

// Forward declarations
struct SplitNode;
struct LeafNode;

/**
 * @brief A node in the binary split tree: either a SplitNode or a LeafNode.
 *
 * Leaf nodes represent individual viewports.
 * Split nodes represent a binary split with a direction and ratio.
 */
using TreeNode = std::variant<SplitNode, LeafNode>;

/**
 * @brief Leaf node — represents a single viewport in the split tree.
 */
struct MATCHA_EXPORT LeafNode {
    ViewportId viewportId {};
    Viewport*  viewport = nullptr; ///< Non-owning. Lifetime managed by UiNode tree.
};

/**
 * @brief Split node — binary split with direction and ratio.
 *
 * `first` gets `ratio * totalSize`, `second` gets the remainder.
 */
struct MATCHA_EXPORT SplitNode {
    SplitDirection             direction = SplitDirection::Horizontal;
    double                     ratio     = 0.5; ///< [0.1, 0.9]
    std::unique_ptr<TreeNode>  first;
    std::unique_ptr<TreeNode>  second;
};

// ============================================================================
// Free functions for tree manipulation
// ============================================================================

/** @brief Create a leaf node. */
[[nodiscard]] inline auto MakeLeaf(ViewportId id, Viewport* vp)
    -> std::unique_ptr<TreeNode>
{
    return std::make_unique<TreeNode>(LeafNode{id, vp});
}

/** @brief Create a split node from two children. */
[[nodiscard]] inline auto MakeSplit(SplitDirection dir, double ratio,
                                    std::unique_ptr<TreeNode> first,
                                    std::unique_ptr<TreeNode> second)
    -> std::unique_ptr<TreeNode>
{
    SplitNode sn;
    sn.direction = dir;
    sn.ratio     = ratio;
    sn.first     = std::move(first);
    sn.second    = std::move(second);
    return std::make_unique<TreeNode>(std::move(sn));
}

/** @brief Check if a TreeNode is a leaf. */
[[nodiscard]] inline auto IsLeaf(const TreeNode& node) -> bool
{
    return std::holds_alternative<LeafNode>(node);
}

/** @brief Check if a TreeNode is a split. */
[[nodiscard]] inline auto IsSplit(const TreeNode& node) -> bool
{
    return std::holds_alternative<SplitNode>(node);
}

/** @brief Get the leaf from a TreeNode. Undefined if not a leaf. */
[[nodiscard]] inline auto AsLeaf(TreeNode& node) -> LeafNode&
{
    return std::get<LeafNode>(node);
}

[[nodiscard]] inline auto AsLeaf(const TreeNode& node) -> const LeafNode&
{
    return std::get<LeafNode>(node);
}

/** @brief Get the split from a TreeNode. Undefined if not a split. */
[[nodiscard]] inline auto AsSplit(TreeNode& node) -> SplitNode&
{
    return std::get<SplitNode>(node);
}

[[nodiscard]] inline auto AsSplit(const TreeNode& node) -> const SplitNode&
{
    return std::get<SplitNode>(node);
}

/**
 * @brief Find a leaf node by ViewportId in the tree.
 * @return Pointer to the TreeNode containing the leaf, or nullptr if not found.
 */
[[nodiscard]] MATCHA_EXPORT auto FindLeaf(TreeNode* root, ViewportId id) -> TreeNode*;

/**
 * @brief Find the parent SplitNode of a given ViewportId.
 * @return Pointer to the parent TreeNode (which is a SplitNode), or nullptr
 *         if the viewport is the root or not found.
 */
[[nodiscard]] MATCHA_EXPORT auto FindParentOf(TreeNode* root, ViewportId id) -> TreeNode*;

/**
 * @brief Count the total number of leaf nodes in the tree.
 */
[[nodiscard]] MATCHA_EXPORT auto CountLeaves(const TreeNode* root) -> int;

/**
 * @brief Collect all leaf ViewportIds in depth-first order.
 */
MATCHA_EXPORT void CollectLeafIds(const TreeNode* root, std::vector<ViewportId>& out);

/**
 * @brief Convert a DropZone to the corresponding SplitDirection.
 * Center has no direction — caller must handle swap separately.
 */
[[nodiscard]] inline auto DropZoneToDirection(DropZone zone) -> SplitDirection
{
    switch (zone) {
    case DropZone::Top:
    case DropZone::Bottom:
        return SplitDirection::Vertical;
    default:
        return SplitDirection::Horizontal;
    }
}

/**
 * @brief Check if the source should be the first child for a given drop zone.
 * Top/Left: source is first. Bottom/Right: source is second.
 */
[[nodiscard]] inline auto IsSourceFirst(DropZone zone) -> bool
{
    return zone == DropZone::Top || zone == DropZone::Left;
}

/**
 * @brief Get the ViewportId of the first (leftmost) leaf in a subtree.
 * @return The ViewportId if found, std::nullopt if the subtree is empty.
 */
[[nodiscard]] inline auto GetFirstLeafId(const TreeNode* node) -> std::optional<ViewportId>
{
    if (node == nullptr) {
        return std::nullopt;
    }
    if (IsLeaf(*node)) {
        return AsLeaf(*node).viewportId;
    }
    const auto& sn = AsSplit(*node);
    return GetFirstLeafId(sn.first.get());
}

} // namespace matcha::fw
