#pragma once

/**
 * @file SliderNode.h
 * @brief Typed WidgetNode wrapping NyanSlider for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanSlider (Scheme D typed node).
 *
 * Dispatches: WidgetIntValueChanged (on value change).
 */
class MATCHA_EXPORT SliderNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using ValueChanged   = matcha::fw::IntValueChanged;
        using SliderPressed  = matcha::fw::SliderPressed;
        using SliderReleased = matcha::fw::SliderReleased;
    };

    explicit SliderNode(std::string id);
    ~SliderNode() override;

    SliderNode(const SliderNode&) = delete;
    auto operator=(const SliderNode&) -> SliderNode& = delete;
    SliderNode(SliderNode&&) = delete;
    auto operator=(SliderNode&&) -> SliderNode& = delete;

    void SetValue(int value);
    [[nodiscard]] auto Value() const -> int;

    void SetRange(int minimum, int maximum);
    [[nodiscard]] auto Minimum() const -> int;
    [[nodiscard]] auto Maximum() const -> int;

    void SetStep(int step);
    [[nodiscard]] auto Step() const -> int;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
