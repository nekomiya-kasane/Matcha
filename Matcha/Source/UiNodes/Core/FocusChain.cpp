#include "Matcha/UiNodes/Core/FocusChain.h"

#include "Matcha/UiNodes/Core/UiNode.h"
#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <QWidget>

#include <algorithm>

namespace matcha::fw {

namespace {

void CollectFocusable(UiNode* node, std::vector<WidgetNode*>& out, bool isRoot)
{
    if (node == nullptr) {
        return;
    }

    // Do not descend into child focus scopes (they trap their own chain).
    // The root of the collection is always included even if it is a scope.
    if (!isRoot && node->IsFocusScope()) {
        return;
    }

    // Check if this node is a focusable WidgetNode
    auto* wn = dynamic_cast<WidgetNode*>(node);
    if (wn != nullptr && wn->IsFocusable()) {
        out.push_back(wn);
    }

    // Recurse into children
    for (size_t i = 0; i < node->NodeCount(); ++i) {
        CollectFocusable(node->NodeAt(i), out, false);
    }
}

} // anonymous namespace

auto FocusChain::Collect(UiNode* root) -> std::vector<WidgetNode*>
{
    std::vector<WidgetNode*> result;
    CollectFocusable(root, result, true);

    // Stable sort: explicit TabIndex first (ascending), then tree-order nodes (TabIndex == -1)
    std::stable_sort(result.begin(), result.end(),
                     [](const WidgetNode* a, const WidgetNode* b) {
                         const int ai = a->TabIndex();
                         const int bi = b->TabIndex();
                         // -1 means "after all explicit indices"
                         if (ai == -1 && bi == -1) { return false; } // preserve tree order
                         if (ai == -1) { return false; } // a goes after b
                         if (bi == -1) { return true; }  // a goes before b
                         return ai < bi;
                     });

    return result;
}

auto FocusChain::Next(std::span<WidgetNode* const> chain,
                      WidgetNode* current) -> WidgetNode*
{
    if (chain.empty()) {
        return nullptr;
    }

    if (current == nullptr) {
        return chain.front();
    }

    for (size_t i = 0; i < chain.size(); ++i) {
        if (chain[i] == current) {
            return chain[(i + 1) % chain.size()];
        }
    }

    // current not found in chain — return first
    return chain.front();
}

auto FocusChain::Previous(std::span<WidgetNode* const> chain,
                          WidgetNode* current) -> WidgetNode*
{
    if (chain.empty()) {
        return nullptr;
    }

    if (current == nullptr) {
        return chain.back();
    }

    for (size_t i = 0; i < chain.size(); ++i) {
        if (chain[i] == current) {
            return chain[(i + chain.size() - 1) % chain.size()];
        }
    }

    // current not found in chain — return last
    return chain.back();
}

} // namespace matcha::fw
