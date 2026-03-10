/**
 * @file SplitTreeNode.cpp
 * @brief Implementation of binary split tree traversal functions.
 */

#include <Matcha/UiNodes/Document/SplitTreeNode.h>

#include <vector>

namespace matcha::fw {

auto FindLeaf(TreeNode* root, ViewportId id) -> TreeNode*
{
    if (root == nullptr) {
        return nullptr;
    }
    if (IsLeaf(*root)) {
        if (AsLeaf(*root).viewportId == id) {
            return root;
        }
        return nullptr;
    }
    auto& sn = AsSplit(*root);
    if (auto* found = FindLeaf(sn.first.get(), id)) {
        return found;
    }
    return FindLeaf(sn.second.get(), id);
}

auto FindParentOf(TreeNode* root, ViewportId id) -> TreeNode*
{
    if (root == nullptr || IsLeaf(*root)) {
        return nullptr;
    }
    auto& sn = AsSplit(*root);

    // Check if either child is the target leaf
    if (sn.first && IsLeaf(*sn.first) && AsLeaf(*sn.first).viewportId == id) {
        return root;
    }
    if (sn.second && IsLeaf(*sn.second) && AsLeaf(*sn.second).viewportId == id) {
        return root;
    }

    // Recurse into split children
    if (sn.first && IsSplit(*sn.first)) {
        if (auto* found = FindParentOf(sn.first.get(), id)) {
            return found;
        }
    }
    if (sn.second && IsSplit(*sn.second)) {
        if (auto* found = FindParentOf(sn.second.get(), id)) {
            return found;
        }
    }
    return nullptr;
}

auto CountLeaves(const TreeNode* root) -> int
{
    if (root == nullptr) {
        return 0;
    }
    if (IsLeaf(*root)) {
        return 1;
    }
    const auto& sn = AsSplit(*root);
    return CountLeaves(sn.first.get()) + CountLeaves(sn.second.get());
}

void CollectLeafIds(const TreeNode* root, std::vector<ViewportId>& out)
{
    if (root == nullptr) {
        return;
    }
    if (IsLeaf(*root)) {
        out.push_back(AsLeaf(*root).viewportId);
        return;
    }
    const auto& sn = AsSplit(*root);
    CollectLeafIds(sn.first.get(), out);
    CollectLeafIds(sn.second.get(), out);
}

} // namespace matcha::fw
