#pragma once

/**
 * @file TabBarNode.h
 * @brief Unified UiNode wrapper for NyanTabBar — document tab bar.
 *
 * Replaces both DocumentBarNode and FloatingTabBarNode with a single
 * implementation. Each tab is represented as a TabItemNode child.
 *
 * @see NyanTabBar for the widget layer.
 * @see TabItemNode for individual tab nodes.
 */

#include "Matcha/Core/StrongId.h"
#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <string_view>

namespace matcha::gui {
enum class TabStyle : uint8_t;
class NyanTabBar;
} // namespace matcha::gui

class QWidget;

namespace matcha::fw {

class TabItemNode;

/**
 * @brief Unified UiNode wrapper for NyanTabBar.
 *
 * Manages document tabs. Each tab is a TabItemNode child.
 * Fires TabPageSwitched, TabPageCloseRequested, TabPageDraggedOut.
 */
class MATCHA_EXPORT TabBarNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification {
        using TabPageSwitched       = matcha::fw::TabPageSwitched;
        using TabPageCloseRequested = matcha::fw::TabPageCloseRequested;
        using TabPageDraggedOut     = matcha::fw::TabPageDraggedOut;
        using TabDroppedIn          = matcha::fw::TabDroppedIn;
        using TabReordered          = matcha::fw::TabReordered;
    };

    /// @brief Construct and create a new NyanTabBar widget.
    TabBarNode(std::string id, gui::TabStyle style, QWidget* parentWidget = nullptr);
    ~TabBarNode() override;

    TabBarNode(const TabBarNode&)            = delete;
    TabBarNode& operator=(const TabBarNode&) = delete;
    TabBarNode(TabBarNode&&)                 = delete;
    TabBarNode& operator=(TabBarNode&&)      = delete;

    /// @brief Get the underlying NyanTabBar widget.
    [[nodiscard]] auto TabBar() -> gui::NyanTabBar*;

    /// @brief Get the underlying QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Tab management (PageId-based) --

    /// @brief Add a tab. Creates a TabItemNode child. Returns the tab index.
    auto AddTab(PageId pageId, std::string_view title) -> int;

    /// @brief Remove a tab by PageId. Removes the TabItemNode child.
    void RemoveTab(PageId pageId);

    /// @brief Set the active tab by PageId.
    void SetActiveTab(PageId pageId);

    /// @brief Set tab title by PageId.
    void SetTabTitle(PageId pageId, std::string_view title);

    /// @brief Move a tab from one index to another (reorder).
    void MoveTab(int fromIndex, int toIndex);

    /// @brief Get tab count.
    [[nodiscard]] auto TabCount() const -> int;

    /// @brief Find a TabItemNode by PageId.
    [[nodiscard]] auto FindTab(PageId pageId) -> TabItemNode*;

private:
    gui::NyanTabBar* _tabBar;
};

} // namespace matcha::fw
