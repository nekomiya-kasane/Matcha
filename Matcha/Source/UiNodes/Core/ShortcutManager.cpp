/**
 * @file ShortcutManager.cpp
 * @brief Implementation of ShortcutManager.
 */

#include <Matcha/Foundation/ShortcutManager.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Registration
// ============================================================================

auto ShortcutManager::Register(ShortcutEntry entry) -> bool
{
    // Reject duplicate ID
    if (FindById(entry.id) != nullptr) {
        return false;
    }
    _entries.push_back(std::move(entry));
    return true;
}

auto ShortcutManager::Unregister(std::string_view id) -> bool
{
    const auto it = std::ranges::find_if(_entries, [id](const ShortcutEntry& e) {
        return e.id == id;
    });
    if (it == _entries.end()) {
        return false;
    }
    _entries.erase(it);
    return true;
}

auto ShortcutManager::Rebind(std::string_view id, std::string newKeySequence) -> bool
{
    for (auto& e : _entries) {
        if (e.id == id) {
            e.keySequence = std::move(newKeySequence);
            return true;
        }
    }
    return false;
}

auto ShortcutManager::SetEnabled(std::string_view id, bool enabled) -> bool
{
    for (auto& e : _entries) {
        if (e.id == id) {
            e.enabled = enabled;
            return true;
        }
    }
    return false;
}

// ============================================================================
// Dispatch
// ============================================================================

auto ShortcutManager::Dispatch(std::string_view keySequence, ShortcutScope activeScope) -> bool
{
    // Search from narrowest (activeScope) to broadest (Global).
    // ShortcutScope values: Dialog=0, Panel=1, Workbench=2, Global=3.
    // We iterate from activeScope value down to... wait, narrowest is smallest enum value.
    // activeScope is the narrowest scope currently active.
    // We search: activeScope, activeScope+1, ..., Global (3).

    for (auto scopeVal = static_cast<uint8_t>(activeScope);
         scopeVal <= static_cast<uint8_t>(ShortcutScope::Global);
         ++scopeVal) {
        const auto scope = static_cast<ShortcutScope>(scopeVal);
        for (auto& entry : _entries) {
            if (entry.scope == scope &&
                entry.keySequence == keySequence &&
                entry.enabled) {
                if (entry.handler) {
                    entry.handler();
                }
                return true;
            }
        }
    }
    return false;
}

// ============================================================================
// Query
// ============================================================================

auto ShortcutManager::FindById(std::string_view id) const -> const ShortcutEntry*
{
    for (const auto& e : _entries) {
        if (e.id == id) {
            return &e;
        }
    }
    return nullptr;
}

auto ShortcutManager::FindByKey(std::string_view keySequence) const
    -> std::vector<const ShortcutEntry*>
{
    std::vector<const ShortcutEntry*> result;
    for (const auto& e : _entries) {
        if (e.keySequence == keySequence) {
            result.push_back(&e);
        }
    }
    return result;
}

auto ShortcutManager::EntriesInScope(ShortcutScope scope) const
    -> std::vector<const ShortcutEntry*>
{
    std::vector<const ShortcutEntry*> result;
    for (const auto& e : _entries) {
        if (e.scope == scope) {
            result.push_back(&e);
        }
    }
    return result;
}

// ============================================================================
// Conflict detection
// ============================================================================

auto ShortcutManager::DetectConflicts() const -> std::vector<ShortcutConflict>
{
    std::vector<ShortcutConflict> conflicts;
    for (std::size_t i = 0; i < _entries.size(); ++i) {
        for (std::size_t j = i + 1; j < _entries.size(); ++j) {
            if (_entries[i].keySequence == _entries[j].keySequence &&
                _entries[i].scope == _entries[j].scope) {
                conflicts.push_back({
                    .existingId = _entries[i].id,
                    .newId = _entries[j].id,
                    .keySequence = _entries[i].keySequence,
                    .scope = _entries[i].scope,
                });
            }
        }
    }
    return conflicts;
}

auto ShortcutManager::WouldConflict(std::string_view keySequence,
                                     ShortcutScope scope) const -> bool
{
    return std::ranges::any_of(_entries, [keySequence, scope](const ShortcutEntry& e) {
        return e.keySequence == keySequence && e.scope == scope;
    });
}

// ============================================================================
// Bulk operations
// ============================================================================

auto ShortcutManager::AllEntries() const -> const std::vector<ShortcutEntry>&
{
    return _entries;
}

void ShortcutManager::Clear()
{
    _entries.clear();
}

} // namespace matcha::fw
