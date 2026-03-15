#pragma once

/**
 * @file BadgeNode.h
 * @brief Typed WidgetNode wrapping NyanBadge for UiNode tree.
 */

#include "Matcha/Tree/WidgetNode.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanBadge (Scheme D typed node).
 *
 * Pill-shaped semantic status indicator with text and optional close button.
 * The variant determines the color scheme (Success/Warning/Error/Info/Neutral).
 *
 * Dispatches: Closed (when closable and close button clicked).
 */
class MATCHA_EXPORT BadgeNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using Closed = matcha::fw::CloseRequested;
    };

    explicit BadgeNode(std::string id);
    ~BadgeNode() override;

    BadgeNode(const BadgeNode&) = delete;
    auto operator=(const BadgeNode&) -> BadgeNode& = delete;
    BadgeNode(BadgeNode&&) = delete;
    auto operator=(BadgeNode&&) -> BadgeNode& = delete;

    void SetText(std::string_view text);
    [[nodiscard]] auto Text() const -> std::string;

    void SetVariant(uint8_t variant);
    [[nodiscard]] auto Variant() const -> uint8_t;

    void SetClosable(bool closable);
    [[nodiscard]] auto IsClosable() const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
