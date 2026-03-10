#pragma once

/**
 * @file TabItemNode.h
 * @brief UiNode representing a single tab in a TabBarNode.
 *
 * Each tab is a child UiNode of TabBarNode, making it addressable
 * and queryable via the UiNode tree.
 *
 * @see TabBarNode for the parent node.
 */

#include "Matcha/Foundation/StrongId.h"
#include "Matcha/UiNodes/Core/UiNode.h"

#include <string>

namespace matcha::gui { class NyanTabItem; }

namespace matcha::fw {

/**
 * @brief UiNode representing a single document tab.
 *
 * Binds to a NyanTabItem widget. Widget() returns the tab item widget.
 */
class MATCHA_EXPORT TabItemNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    TabItemNode(std::string id, PageId pageId, std::string title);
    ~TabItemNode() override;

    TabItemNode(const TabItemNode&)            = delete;
    TabItemNode& operator=(const TabItemNode&) = delete;
    TabItemNode(TabItemNode&&)                 = delete;
    TabItemNode& operator=(TabItemNode&&)      = delete;

    /// @brief Get the bound NyanTabItem widget (may be nullptr before binding).
    [[nodiscard]] auto Widget() -> QWidget* override;

    /// @brief Get the typed NyanTabItem widget.
    [[nodiscard]] auto GetTabItem() const -> gui::NyanTabItem* { return _tabItem; }

    /// @brief Bind the widget created by NyanTabBar::AddTab.
    void SetTabItem(gui::NyanTabItem* item);

    /// @brief Get the PageId this tab represents.
    [[nodiscard]] auto GetPageId() const -> PageId { return _pageId; }

    /// @brief Get the tab title.
    [[nodiscard]] auto Title() const -> std::string_view { return _title; }

    /// @brief Set the tab title.
    void SetTitle(std::string_view title);

private:
    PageId _pageId;
    std::string _title;
    gui::NyanTabItem* _tabItem = nullptr;
};

} // namespace matcha::fw
