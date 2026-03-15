/**
 * @file UiNodeQuery.cpp
 * @brief Runtime functions for the UiNode query system.
 */

#include <Matcha/Tree/UiNodeQuery.h>

#include <algorithm>
#include <format>
#include <string>
#include <vector>

namespace matcha::fw {

auto NodePath(const UiNode* node) -> std::string {
    if (!node) { return ""; }

    // Build chain from root to node
    std::vector<const UiNode*> chain;
    for (const auto* cur = node; cur; cur = cur->ParentNode()) {
        chain.push_back(cur);
    }
    std::ranges::reverse(chain);

    std::string result;
    for (std::size_t i = 0; i < chain.size(); ++i) {
        if (i > 0) { result += '/'; }

        const auto* n = chain[i];
        result += NodeTypeName(n->Type());

        // Add [index] if parent has multiple children of the same type
        if (auto* parent = n->ParentNode()) {
            int sameTypeCount = 0;
            int myIndex = 0;
            for (std::size_t j = 0; j < parent->NodeCount(); ++j) {
                auto* sib = parent->NodeAt(j);
                if (sib->Type() == n->Type()) {
                    if (sib == n) { myIndex = sameTypeCount; }
                    ++sameTypeCount;
                }
            }
            if (sameTypeCount > 1) {
                result += std::format("[{}]", myIndex);
            }
        }
    }

    return result;
}

} // namespace matcha::fw
