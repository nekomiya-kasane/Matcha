#pragma once

/**
 * @file RangeSliderNode.h
 * @brief Typed WidgetNode wrapping NyanRangeSlider for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanRangeSlider (Scheme D typed node).
 *
 * Dual-handle range slider defining a [low, high] sub-interval within
 * a [minimum, maximum] domain. Essential for CAD/CAE range selection
 * (mesh size bounds, stress filter ranges, etc.).
 *
 * Dispatches: RangeChanged (low, high).
 */
class MATCHA_EXPORT RangeSliderNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using RangeChanged = matcha::fw::RangeChanged;
    };

    explicit RangeSliderNode(std::string id);
    ~RangeSliderNode() override;

    RangeSliderNode(const RangeSliderNode&) = delete;
    auto operator=(const RangeSliderNode&) -> RangeSliderNode& = delete;
    RangeSliderNode(RangeSliderNode&&) = delete;
    auto operator=(RangeSliderNode&&) -> RangeSliderNode& = delete;

    void SetRange(int minimum, int maximum);
    [[nodiscard]] auto Minimum() const -> int;
    [[nodiscard]] auto Maximum() const -> int;

    void SetLow(int value);
    [[nodiscard]] auto Low() const -> int;

    void SetHigh(int value);
    [[nodiscard]] auto High() const -> int;

    void SetStep(int step);
    [[nodiscard]] auto Step() const -> int;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
