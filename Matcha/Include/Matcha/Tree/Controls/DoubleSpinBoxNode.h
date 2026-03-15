#pragma once

/**
 * @file DoubleSpinBoxNode.h
 * @brief Typed WidgetNode wrapping NyanDoubleSpinBox for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanDoubleSpinBox (Scheme D typed node).
 *
 * Dispatches: WidgetDoubleValueChanged (on value change).
 */
class MATCHA_EXPORT DoubleSpinBoxNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using ValueChanged    = matcha::fw::DoubleValueChanged;
        using EditingFinished = matcha::fw::EditingFinished;
    };

    explicit DoubleSpinBoxNode(std::string id);
    ~DoubleSpinBoxNode() override;

    DoubleSpinBoxNode(const DoubleSpinBoxNode&) = delete;
    auto operator=(const DoubleSpinBoxNode&) -> DoubleSpinBoxNode& = delete;
    DoubleSpinBoxNode(DoubleSpinBoxNode&&) = delete;
    auto operator=(DoubleSpinBoxNode&&) -> DoubleSpinBoxNode& = delete;

    void SetValue(double value);
    [[nodiscard]] auto Value() const -> double;
    void SetRange(double min, double max);
    [[nodiscard]] auto Minimum() const -> double;
    [[nodiscard]] auto Maximum() const -> double;
    void SetStep(double step);
    [[nodiscard]] auto Step() const -> double;
    void SetPrecision(int decimals);
    [[nodiscard]] auto Precision() const -> int;
    void SetSuffix(std::string_view suffix);
    [[nodiscard]] auto Suffix() const -> std::string;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
