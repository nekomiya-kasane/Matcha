#pragma once

/**
 * @file SpinBoxNode.h
 * @brief Typed WidgetNode wrapping NyanSpinBox for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanSpinBox (Scheme D typed node).
 *
 * Dispatches: WidgetIntValueChanged (on value change).
 */
class MATCHA_EXPORT SpinBoxNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using ValueChanged    = matcha::fw::IntValueChanged;
        using EditingFinished = matcha::fw::EditingFinished;
    };

    explicit SpinBoxNode(std::string id);
    ~SpinBoxNode() override;

    SpinBoxNode(const SpinBoxNode&) = delete;
    auto operator=(const SpinBoxNode&) -> SpinBoxNode& = delete;
    SpinBoxNode(SpinBoxNode&&) = delete;
    auto operator=(SpinBoxNode&&) -> SpinBoxNode& = delete;

    void SetValue(int value);
    [[nodiscard]] auto Value() const -> int;
    void SetRange(int min, int max);
    [[nodiscard]] auto Minimum() const -> int;
    [[nodiscard]] auto Maximum() const -> int;

    void SetSuffix(std::string_view suffix);
    [[nodiscard]] auto Suffix() const -> std::string;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
