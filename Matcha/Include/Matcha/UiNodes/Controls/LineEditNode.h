#pragma once

/**
 * @file LineEditNode.h
 * @brief Typed WidgetNode wrapping NyanLineEdit for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanLineEdit (Scheme D typed node).
 *
 * Dispatches: WidgetTextChanged (on text change), WidgetEditingFinished (on enter/focus-out).
 */
class MATCHA_EXPORT LineEditNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using TextChanged      = matcha::fw::TextChanged;
        using EditingFinished  = matcha::fw::EditingFinished;
        using ReturnPressed    = matcha::fw::ReturnPressed;
    };

    explicit LineEditNode(std::string id);
    ~LineEditNode() override;

    LineEditNode(const LineEditNode&) = delete;
    auto operator=(const LineEditNode&) -> LineEditNode& = delete;
    LineEditNode(LineEditNode&&) = delete;
    auto operator=(LineEditNode&&) -> LineEditNode& = delete;

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

    void SetPlaceholder(std::string_view text);
    [[nodiscard]] auto Placeholder() const -> std::string;
    void SetReadOnly(bool readOnly);
    [[nodiscard]] auto IsReadOnly() const -> bool;
    void SetMaxLength(int length);
    [[nodiscard]] auto MaxLength() const -> int;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
