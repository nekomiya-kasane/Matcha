#pragma once

/**
 * @file SearchBoxNode.h
 * @brief Typed WidgetNode wrapping NyanSearchBox for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::gui { enum class SearchMode : uint8_t; }

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanSearchBox (Scheme D typed node).
 *
 * Dispatches: WidgetTextChanged (on text change).
 */
class MATCHA_EXPORT SearchBoxNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using TextChanged      = matcha::fw::TextChanged;
        using SearchSubmitted  = matcha::fw::SearchSubmitted;
    };

    explicit SearchBoxNode(std::string id);
    ~SearchBoxNode() override;

    SearchBoxNode(const SearchBoxNode&) = delete;
    auto operator=(const SearchBoxNode&) -> SearchBoxNode& = delete;
    SearchBoxNode(SearchBoxNode&&) = delete;
    auto operator=(SearchBoxNode&&) -> SearchBoxNode& = delete;

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

    void SetPlaceholder(std::string_view text);
    [[nodiscard]] auto Placeholder() const -> std::string;

    void SetSearchMode(gui::SearchMode mode);
    [[nodiscard]] auto SearchMode() const -> gui::SearchMode;

    void Clear();

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
