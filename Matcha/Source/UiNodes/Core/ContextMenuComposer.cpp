/**
 * @file ContextMenuComposer.cpp
 * @brief Implementation of ContextMenuComposer.
 */

#include <Matcha/Foundation/ContextMenuComposer.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Group management
// ============================================================================

void ContextMenuComposer::AddGroup(std::string name, int priority)
{
    if (FindGroupMut(name) != nullptr) {
        return; // already exists
    }
    _groups.push_back({.name = std::move(name), .priority = priority, .items = {}});
}

auto ContextMenuComposer::SetGroupPriority(std::string_view name, int priority) -> bool
{
    auto* g = FindGroupMut(name);
    if (g == nullptr) {
        return false;
    }
    g->priority = priority;
    return true;
}

// ============================================================================
// Item management
// ============================================================================

void ContextMenuComposer::AddItem(std::string_view groupName, ContextMenuItem item)
{
    auto* g = FindGroupMut(groupName);
    if (g == nullptr) {
        AddGroup(std::string(groupName));
        g = FindGroupMut(groupName);
    }
    g->items.push_back(std::move(item));
}

void ContextMenuComposer::AddSeparator(std::string_view groupName)
{
    AddItem(groupName, {
        .id = {}, .label = {}, .iconId = {}, .shortcut = {},
        .kind = MenuItemKind::Separator,
        .enabled = true, .checked = false,
        .handler = {}, .children = {},
    });
}

// ============================================================================
// Composition
// ============================================================================

auto ContextMenuComposer::Compose() const -> std::vector<ContextMenuItem>
{
    // Sort groups by priority (stable sort preserves insertion order for equal priority)
    auto sorted = _groups;
    std::ranges::stable_sort(sorted, [](const MenuGroup& a, const MenuGroup& b) {
        return a.priority < b.priority;
    });

    std::vector<ContextMenuItem> result;
    bool needSeparator = false;

    for (const auto& group : sorted) {
        if (group.items.empty()) {
            continue;
        }
        if (needSeparator) {
            result.push_back({
                .id = {}, .label = {}, .iconId = {}, .shortcut = {},
                .kind = MenuItemKind::Separator,
                .enabled = true, .checked = false,
                .handler = {}, .children = {},
            });
        }
        for (const auto& item : group.items) {
            result.push_back(item);
        }
        needSeparator = true;
    }

    return result;
}

// ============================================================================
// Query
// ============================================================================

auto ContextMenuComposer::TotalItemCount() const -> int
{
    int count = 0;
    for (const auto& g : _groups) {
        count += static_cast<int>(g.items.size());
    }
    return count;
}

auto ContextMenuComposer::FindGroup(std::string_view name) const -> const MenuGroup*
{
    for (const auto& g : _groups) {
        if (g.name == name) {
            return &g;
        }
    }
    return nullptr;
}

void ContextMenuComposer::Clear()
{
    _groups.clear();
}

// ============================================================================
// Private
// ============================================================================

auto ContextMenuComposer::FindGroupMut(std::string_view name) -> MenuGroup*
{
    for (auto& g : _groups) {
        if (g.name == name) {
            return &g;
        }
    }
    return nullptr;
}

} // namespace matcha::fw
