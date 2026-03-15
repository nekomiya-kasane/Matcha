#pragma once

/**
 * @file ColorSwatchNode.h
 * @brief Typed WidgetNode wrapping NyanColorSwatch for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanColorSwatch (Scheme D typed node).
 *
 * Dispatches: WidgetActivated (on click).
 */
class MATCHA_EXPORT ColorSwatchNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using ColorChanged = matcha::fw::ColorChanged;
        using Activated    = matcha::fw::Activated;
    };

    explicit ColorSwatchNode(std::string id);
    ~ColorSwatchNode() override;

    ColorSwatchNode(const ColorSwatchNode&) = delete;
    auto operator=(const ColorSwatchNode&) -> ColorSwatchNode& = delete;
    ColorSwatchNode(ColorSwatchNode&&) = delete;
    auto operator=(ColorSwatchNode&&) -> ColorSwatchNode& = delete;

    void SetColor(uint32_t rgba);
    [[nodiscard]] auto Color() const -> uint32_t;

    void SetTitle(std::string_view title);
    [[nodiscard]] auto Title() const -> std::string;

    void SetSwatchSize(int w, int h);

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
