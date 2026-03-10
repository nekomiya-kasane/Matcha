#pragma once

/**
 * @file CollapsibleSectionNode.h
 * @brief Typed WidgetNode wrapping NyanCollapsibleSection for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanCollapsibleSection (Scheme D typed node).
 *
 * Borderless collapsible group with title row and animated expand/collapse.
 * The content widget is set via SetContentWidget(). Unlike ContainerNode,
 * this wraps a specific visual pattern (chevron + title + animated collapse).
 *
 * Dispatches: ExpandToggled (expanded state).
 */
class MATCHA_EXPORT CollapsibleSectionNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using ExpandToggled = matcha::fw::ExpandToggled;
    };

    explicit CollapsibleSectionNode(std::string id);
    ~CollapsibleSectionNode() override;

    CollapsibleSectionNode(const CollapsibleSectionNode&) = delete;
    auto operator=(const CollapsibleSectionNode&) -> CollapsibleSectionNode& = delete;
    CollapsibleSectionNode(CollapsibleSectionNode&&) = delete;
    auto operator=(CollapsibleSectionNode&&) -> CollapsibleSectionNode& = delete;

    void SetTitle(std::string_view title);
    [[nodiscard]] auto Title() const -> std::string;

    void SetExpanded(bool expanded);
    [[nodiscard]] auto IsExpanded() const -> bool;

    void SetContentNode(UiNode* contentNode);

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
