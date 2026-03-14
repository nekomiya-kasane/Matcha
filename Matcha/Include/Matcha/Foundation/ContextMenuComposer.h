#pragma once

/**
 * @file ContextMenuComposer.h
 * @brief Context menu composition API: multi-source contribution, group ordering.
 *
 * Allows multiple contributors (widget, plugin, workbench) to add items
 * to a context menu, organized by named groups with explicit ordering.
 *
 * - Contributors register items into named groups
 * - Groups are ordered by priority (lower = earlier)
 * - Separators inserted automatically between groups
 * - Items within a group maintain insertion order
 *
 * This is a Foundation-layer component with zero Qt dependency.
 *
 * @see Matcha_Design_System_Specification.md (ContextMenu interactions)
 */

#include <Matcha/Foundation/Macros.h>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

// ============================================================================
// MenuItemKind
// ============================================================================

/**
 * @enum MenuItemKind
 * @brief Kind of context menu item.
 */
enum class MenuItemKind : uint8_t {
    Action    = 0,   ///< Normal clickable item
    Toggle    = 1,   ///< Checkable item
    Submenu   = 2,   ///< Opens a nested submenu
    Separator = 3,   ///< Visual separator (auto-inserted between groups)
};

// ============================================================================
// ContextMenuItem
// ============================================================================

/**
 * @struct ContextMenuItem
 * @brief A single context menu item descriptor.
 */
struct ContextMenuItem {
    std::string           id;
    std::string           label;
    std::string           iconId;       ///< Icon identifier (theme-resolved later)
    std::string           shortcut;     ///< Display-only shortcut hint
    MenuItemKind          kind     = MenuItemKind::Action;
    bool                  enabled  = true;
    bool                  checked  = false;  ///< For Toggle kind
    std::function<void()> handler;
    std::vector<ContextMenuItem> children; ///< For Submenu kind
};

// ============================================================================
// MenuGroup
// ============================================================================

/**
 * @struct MenuGroup
 * @brief A named group of context menu items with ordering priority.
 */
struct MenuGroup {
    std::string                  name;
    int                          priority = 100;  ///< Lower = earlier in menu
    std::vector<ContextMenuItem> items;
};

// ============================================================================
// ContextMenuComposer
// ============================================================================

/**
 * @class ContextMenuComposer
 * @brief Composes a context menu from multiple contributor sources.
 *
 * Usage:
 * @code
 *   ContextMenuComposer composer;
 *   composer.AddGroup("clipboard", 10);
 *   composer.AddItem("clipboard", {.id="cut", .label="Cut", .shortcut="Ctrl+X"});
 *   composer.AddItem("clipboard", {.id="copy", .label="Copy", .shortcut="Ctrl+C"});
 *   composer.AddItem("clipboard", {.id="paste", .label="Paste", .shortcut="Ctrl+V"});
 *
 *   composer.AddGroup("custom", 50);
 *   composer.AddItem("custom", {.id="refresh", .label="Refresh"});
 *
 *   auto flat = composer.Compose();
 *   // Result: Cut, Copy, Paste, [Separator], Refresh
 * @endcode
 */
class MATCHA_EXPORT ContextMenuComposer {
public:
    ContextMenuComposer() = default;

    // ====================================================================
    // Group management
    // ====================================================================

    /**
     * @brief Add or get a named group. If it already exists, returns existing.
     * @param name Group name.
     * @param priority Ordering priority (lower = earlier). Ignored if group exists.
     */
    void AddGroup(std::string name, int priority = 100);

    /**
     * @brief Set priority for an existing group.
     */
    auto SetGroupPriority(std::string_view name, int priority) -> bool;

    // ====================================================================
    // Item management
    // ====================================================================

    /**
     * @brief Add an item to a named group. Creates the group if needed.
     */
    void AddItem(std::string_view groupName, ContextMenuItem item);

    /**
     * @brief Add a separator hint within a group (rarely needed, auto-generated between groups).
     */
    void AddSeparator(std::string_view groupName);

    // ====================================================================
    // Composition
    // ====================================================================

    /**
     * @brief Compose the final ordered menu with separators between groups.
     * @return Flattened list of items with separators inserted.
     */
    [[nodiscard]] auto Compose() const -> std::vector<ContextMenuItem>;

    // ====================================================================
    // Query
    // ====================================================================

    [[nodiscard]] auto GroupCount() const -> int {
        return static_cast<int>(_groups.size());
    }

    [[nodiscard]] auto TotalItemCount() const -> int;

    /**
     * @brief Find a group by name.
     */
    [[nodiscard]] auto FindGroup(std::string_view name) const -> const MenuGroup*;

    /**
     * @brief Clear all groups and items.
     */
    void Clear();

private:
    auto FindGroupMut(std::string_view name) -> MenuGroup*;

    std::vector<MenuGroup> _groups;
};

} // namespace matcha::fw
