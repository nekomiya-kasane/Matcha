#pragma once

/**
 * @file FloatingTitleBarNode.h
 * @brief UiNode wrapper for NyanFloatingTitleBar -- lightweight floating window title bar.
 *
 * @see TitleBarNode for the base interface.
 * @see NyanFloatingTitleBar for the widget layer.
 */

#include "Matcha/Tree/Composition/Shell/TitleBarNode.h"

namespace matcha::gui {
class NyanFloatingTitleBar;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanFloatingTitleBar (28px, single row).
 */
class MATCHA_EXPORT FloatingTitleBarNode : public TitleBarNode {
    MATCHA_DECLARE_CLASS

public:
    FloatingTitleBarNode(std::string id, UiNode* parentHint = nullptr);
    ~FloatingTitleBarNode() override;

    FloatingTitleBarNode(const FloatingTitleBarNode&)            = delete;
    FloatingTitleBarNode& operator=(const FloatingTitleBarNode&) = delete;
    FloatingTitleBarNode(FloatingTitleBarNode&&)                 = delete;
    FloatingTitleBarNode& operator=(FloatingTitleBarNode&&)      = delete;

    // -- TitleBarNode interface --

    void SetTitle(std::string_view title) override;
    [[nodiscard]] auto Title() const -> std::string override;
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Floating-specific --

    /// @brief Get the underlying NyanFloatingTitleBar widget.
    [[nodiscard]] auto FloatingTitleBar() -> gui::NyanFloatingTitleBar*;

private:
    gui::NyanFloatingTitleBar* _titleBar;
};

} // namespace matcha::fw
