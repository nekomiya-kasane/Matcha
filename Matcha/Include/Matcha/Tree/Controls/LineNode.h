#pragma once

/**
 * @file LineNode.h
 * @brief Typed WidgetNode wrapping NyanLine for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <cstdint>
#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanLine (Scheme D typed node).
 *
 * A 1px themed separator line. Orientation and color token are
 * configurable. No user interaction — purely visual structure.
 *
 * Dispatches: (none — display-only widget).
 */
class MATCHA_EXPORT LineNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    explicit LineNode(std::string id);
    ~LineNode() override;

    LineNode(const LineNode&) = delete;
    auto operator=(const LineNode&) -> LineNode& = delete;
    LineNode(LineNode&&) = delete;
    auto operator=(LineNode&&) -> LineNode& = delete;

    void SetOrientation(fw::Orientation orientation);
    [[nodiscard]] auto Orientation() const -> fw::Orientation;

    void SetColorToken(uint16_t token);
    [[nodiscard]] auto ColorToken() const -> uint16_t;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
