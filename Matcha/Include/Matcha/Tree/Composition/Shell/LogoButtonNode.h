#pragma once

/**
 * @file LogoButtonNode.h
 * @brief UiNode wrapper for NyanLogoButton -- clickable logo spanning two shell rows.
 *
 * Emits LogoClicked notification when the user clicks the logo.
 *
 * @see NyanLogoButton for the widget layer.
 */

#include "Matcha/Tree/UiNode.h"

#include <cstdint>
#include <span>
#include <string>

namespace matcha::gui {
class NyanLogoButton;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanLogoButton (56x56 clickable logo).
 */
class MATCHA_EXPORT LogoButtonNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    explicit LogoButtonNode(std::string id, UiNode* parentHint = nullptr);
    ~LogoButtonNode() override;

    LogoButtonNode(const LogoButtonNode&)            = delete;
    LogoButtonNode& operator=(const LogoButtonNode&) = delete;
    LogoButtonNode(LogoButtonNode&&)                 = delete;
    LogoButtonNode& operator=(LogoButtonNode&&)      = delete;

    /// @brief Get the underlying widget.
    [[nodiscard]] auto Widget() -> QWidget* override;

    /// @brief Get the underlying NyanLogoButton widget.
    [[nodiscard]] auto LogoButton() -> gui::NyanLogoButton*;

    /// @brief Set the logo image from raw pixel data (PNG bytes).
    void SetLogoFromData(std::span<const uint8_t> pngData);

    /// @brief Set the vertical split point (Row 1 height).
    void SetSplitY(int row1Height);

    /// @brief Set the top/bottom background colors.
    void SetColors(uint32_t topArgb, uint32_t bottomArgb);

private:
    gui::NyanLogoButton* _logoButton;
};

} // namespace matcha::fw
