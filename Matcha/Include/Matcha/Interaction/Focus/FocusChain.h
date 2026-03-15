#pragma once

/**
 * @file FocusChain.h
 * @brief Focus chain collection and traversal utilities (S4).
 *
 * FocusChain is a stateless utility (no Service). It walks a UiNode subtree,
 * collects focusable WidgetNodes, sorts by TabIndex, and provides
 * Next/Previous traversal.
 *
 * Called lazily on Tab/Shift-Tab keypress — not pre-computed.
 */

#include "Matcha/Core/Macros.h"

#include <span>
#include <vector>

namespace matcha::fw {

class UiNode;
class WidgetNode;

/**
 * @brief Collects and traverses focusable WidgetNodes in a subtree.
 *
 * Usage:
 * @code
 * auto chain = FocusChain::Collect(rootNode);
 * auto* next = FocusChain::Next(chain, currentWidget);
 * if (next) next->SetFocus();
 * @endcode
 */
class MATCHA_EXPORT FocusChain {
public:
    /**
     * @brief Walk the subtree rooted at `root` and collect focusable WidgetNodes.
     *
     * Nodes with IsFocusable() == true are included. Result is sorted by
     * TabIndex (ascending). Nodes with TabIndex == -1 (default) are ordered
     * by tree traversal order, after all explicitly indexed nodes.
     *
     * @param root Subtree root to search.
     * @return Sorted vector of focusable widget nodes.
     */
    [[nodiscard]] static auto Collect(UiNode* root) -> std::vector<WidgetNode*>;

    /**
     * @brief Get the next focusable node after `current` in the chain.
     * @param chain Previously collected focus chain.
     * @param current Currently focused node (may be nullptr).
     * @return Next node, or first node if current is last/not found. nullptr if chain empty.
     */
    [[nodiscard]] static auto Next(std::span<WidgetNode* const> chain,
                                   WidgetNode* current) -> WidgetNode*;

    /**
     * @brief Get the previous focusable node before `current` in the chain.
     * @param chain Previously collected focus chain.
     * @param current Currently focused node (may be nullptr).
     * @return Previous node, or last node if current is first/not found. nullptr if chain empty.
     */
    [[nodiscard]] static auto Previous(std::span<WidgetNode* const> chain,
                                       WidgetNode* current) -> WidgetNode*;
};

} // namespace matcha::fw
