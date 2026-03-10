#pragma once

/**
 * @file FloatingWindowNode.h
 * @brief Lightweight floating window base -- frame + title bar + content area.
 *
 * FloatingWindowNode provides a frameless QWidget with a NyanFloatingTitleBar
 * and a central content area. Subclasses override BuildContent() to fill
 * the content area with their specific UiNode children.
 *
 * @see FloatingTabWindowNode -- subclass for detached document tab windows.
 * @see WindowNode for the base class.
 */

#include "Matcha/UiNodes/Shell/WindowNode.h"

class QVBoxLayout;

namespace matcha::gui {
class NyanFloatingTitleBar;
} // namespace matcha::gui

namespace matcha::fw {

class FloatingTitleBarNode;

/**
 * @brief Lightweight floating window UiNode -- subclass of WindowNode.
 *
 * BuildWindow() creates the window frame:
 * - Frameless QWidget with minimum size
 * - NyanFloatingTitleBar (28px, minimize/maximize/close)
 * - Central content area (QWidget)
 *
 * Subclasses override BuildContent() to populate the content area.
 */
class MATCHA_EXPORT FloatingWindowNode : public WindowNode {
    MATCHA_DECLARE_CLASS

public:
    FloatingWindowNode(std::string id, WindowId windowId,
                       WindowKind kind = WindowKind::Floating);
    ~FloatingWindowNode() override;

    FloatingWindowNode(const FloatingWindowNode&)            = delete;
    auto operator=(const FloatingWindowNode&) -> FloatingWindowNode& = delete;
    FloatingWindowNode(FloatingWindowNode&&)                 = delete;
    auto operator=(FloatingWindowNode&&) -> FloatingWindowNode&      = delete;

    /** @brief Get the FloatingTitleBarNode child. */
    [[nodiscard]] auto GetFloatingTitleBar() -> observer_ptr<FloatingTitleBarNode>;

    /** @brief Build the floating window frame + call BuildContent(). */
    void BuildWindow(QWidget* parent) override;

protected:
    /**
     * @brief Populate the central content area with UiNode children.
     *
     * Called by BuildWindow() after the frame and title bar are created.
     * Override in subclasses to add specific content (tab bars, log areas, etc.).
     *
     * @param contentParent  Qt parent widget for content widgets.
     * @param layout         The VBoxLayout of the window (append widgets here).
     */
    virtual void BuildContent(QWidget* contentParent, QVBoxLayout* layout);
};

} // namespace matcha::fw
