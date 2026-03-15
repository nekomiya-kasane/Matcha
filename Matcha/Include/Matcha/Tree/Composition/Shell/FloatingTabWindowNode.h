#pragma once

/**
 * @file FloatingTabWindowNode.h
 * @brief Floating Tab Window -- detached-tab window with TabBar + WorkspaceFrame.
 *
 * Created when the user drags a document tab out of the Main Window's
 * DocumentBar. Inherits the lightweight frame from FloatingWindowNode
 * and fills it with TabBar, WorkspaceFrame (ActionBar + DocumentArea +
 * ControlBar), and StatusBar.
 *
 * @see FloatingWindowNode for the base frame.
 * @see WindowNode for the root abstraction.
 */

#include "Matcha/Tree/Composition/Shell/FloatingWindowNode.h"

namespace matcha::fw {

class TabBarNode;

/**
 * @brief Floating Tab Window UiNode -- subclass of FloatingWindowNode.
 *
 * Overrides BuildContent() to create:
 * - TabBarNode (floating-style tab bar below title bar)
 * - WorkspaceFrame (ActionBar + DocumentArea + ControlBar)
 * - StatusBarNode
 */
class MATCHA_EXPORT FloatingTabWindowNode : public FloatingWindowNode {
    MATCHA_DECLARE_CLASS

public:
    FloatingTabWindowNode(std::string id, WindowId windowId);
    ~FloatingTabWindowNode() override;

    FloatingTabWindowNode(const FloatingTabWindowNode&)            = delete;
    auto operator=(const FloatingTabWindowNode&) -> FloatingTabWindowNode& = delete;
    FloatingTabWindowNode(FloatingTabWindowNode&&)                 = delete;
    auto operator=(FloatingTabWindowNode&&) -> FloatingTabWindowNode&      = delete;

    /** @brief Get the TabBarNode child. */
    [[nodiscard]] auto GetTabBarNode() -> observer_ptr<TabBarNode>;

protected:
    void BuildContent(QWidget* contentParent, QVBoxLayout* layout) override;
};

} // namespace matcha::fw
