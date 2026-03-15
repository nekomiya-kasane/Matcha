#pragma once

/**
 * @file PaginatorNode.h
 * @brief Typed WidgetNode wrapping NyanPaginator for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanPaginator (Scheme D typed node).
 *
 * Prev/next page navigation with page indicator and optional reset button.
 *
 * Dispatches: PageChanged (page index), ResetClicked.
 */
class MATCHA_EXPORT PaginatorNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using PageChanged  = matcha::fw::PageChanged;
        using ResetClicked = matcha::fw::ResetClicked;
    };

    explicit PaginatorNode(std::string id);
    ~PaginatorNode() override;

    PaginatorNode(const PaginatorNode&) = delete;
    auto operator=(const PaginatorNode&) -> PaginatorNode& = delete;
    PaginatorNode(PaginatorNode&&) = delete;
    auto operator=(PaginatorNode&&) -> PaginatorNode& = delete;

    void SetCount(int count);
    [[nodiscard]] auto Count() const -> int;

    void SetCurrent(int page);
    [[nodiscard]] auto Current() const -> int;

    void SetResetButtonVisible(bool visible);
    [[nodiscard]] auto IsResetButtonVisible() const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
