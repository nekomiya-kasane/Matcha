#pragma once

/**
 * @file ColorPickerNode.h
 * @brief Typed WidgetNode wrapping NyanColorPicker for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <cstdint>
#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanColorPicker (Scheme D typed node).
 *
 * Dispatches: WidgetColorChanged (on color change, RGBA as uint32_t).
 */
class MATCHA_EXPORT ColorPickerNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using ColorChanged = matcha::fw::ColorChanged;
    };

    explicit ColorPickerNode(std::string id);
    ~ColorPickerNode() override;

    ColorPickerNode(const ColorPickerNode&) = delete;
    auto operator=(const ColorPickerNode&) -> ColorPickerNode& = delete;
    ColorPickerNode(ColorPickerNode&&) = delete;
    auto operator=(ColorPickerNode&&) -> ColorPickerNode& = delete;

    void SetColor(uint32_t rgba);
    [[nodiscard]] auto Color() const -> uint32_t;

    void SetAlphaEnabled(bool enabled);
    [[nodiscard]] auto IsAlphaEnabled() const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
