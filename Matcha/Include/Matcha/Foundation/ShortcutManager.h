#pragma once

/**
 * @file ShortcutManager.h
 * @brief Keyboard shortcut registration, scope hierarchy, and conflict detection.
 *
 * Implements the shortcut management aspect of the Matcha interaction system.
 * - Scoped shortcuts: Global, Workbench, Panel, Dialog
 * - Chord sequences: e.g., "Ctrl+K, Ctrl+C"
 * - Conflict detection at registration time
 * - Priority-based dispatch (narrowest scope wins)
 *
 * This is a Foundation-layer component with zero Qt dependency.
 * Shortcut keys are represented as plain strings (e.g., "Ctrl+S").
 *
 * @see Matcha_Design_System_Specification.md §7.9.11 (Mnemonic/Keyboard)
 */

#include <Matcha/Foundation/Macros.h>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

// ============================================================================
// ShortcutScope
// ============================================================================

/**
 * @enum ShortcutScope
 * @brief Scope hierarchy for shortcut resolution (narrowest wins).
 *
 * | Scope     | Priority | Example                        |
 * |-----------|:--------:|--------------------------------|
 * | Dialog    | 0 (high) | Escape closes dialog           |
 * | Panel     | 1        | F2 renames in tree panel       |
 * | Workbench | 2        | Ctrl+R runs mesh operation     |
 * | Global    | 3 (low)  | Ctrl+S saves, Ctrl+Z undoes    |
 */
enum class ShortcutScope : uint8_t {
    Dialog    = 0,
    Panel     = 1,
    Workbench = 2,
    Global    = 3,
};

// ============================================================================
// ShortcutEntry
// ============================================================================

/**
 * @struct ShortcutEntry
 * @brief A registered shortcut binding.
 */
struct ShortcutEntry {
    std::string           id;           ///< Unique ID (e.g., command header ID)
    std::string           keySequence;  ///< Key sequence string (e.g., "Ctrl+S")
    ShortcutScope         scope = ShortcutScope::Global;
    std::string           description;  ///< Human-readable description
    std::function<void()> handler;      ///< Action to invoke
    bool                  enabled = true;
};

// ============================================================================
// ShortcutConflict
// ============================================================================

/**
 * @struct ShortcutConflict
 * @brief Describes a conflict between two shortcut entries.
 */
struct ShortcutConflict {
    std::string existingId;
    std::string newId;
    std::string keySequence;
    ShortcutScope scope = ShortcutScope::Global;
};

// ============================================================================
// ShortcutManager
// ============================================================================

/**
 * @class ShortcutManager
 * @brief Central registry and dispatcher for keyboard shortcuts.
 *
 * **Thread safety**: Not thread-safe. All calls from GUI thread.
 *
 * Usage:
 * @code
 *   ShortcutManager mgr;
 *   mgr.Register({
 *       .id = "file.save",
 *       .keySequence = "Ctrl+S",
 *       .scope = ShortcutScope::Global,
 *       .description = "Save file",
 *       .handler = []{ saveFile(); },
 *   });
 *
 *   // Dispatch a key event
 *   bool handled = mgr.Dispatch("Ctrl+S", ShortcutScope::Global);
 *
 *   // Check conflicts
 *   auto conflicts = mgr.DetectConflicts();
 * @endcode
 */
class MATCHA_EXPORT ShortcutManager {
public:
    ShortcutManager() = default;

    // ====================================================================
    // Registration
    // ====================================================================

    /**
     * @brief Register a shortcut. Returns false if ID already exists.
     */
    auto Register(ShortcutEntry entry) -> bool;

    /**
     * @brief Unregister a shortcut by ID.
     */
    auto Unregister(std::string_view id) -> bool;

    /**
     * @brief Update the key sequence for an existing shortcut.
     */
    auto Rebind(std::string_view id, std::string newKeySequence) -> bool;

    /**
     * @brief Enable or disable a shortcut by ID.
     */
    auto SetEnabled(std::string_view id, bool enabled) -> bool;

    // ====================================================================
    // Dispatch
    // ====================================================================

    /**
     * @brief Dispatch a key sequence at a given active scope.
     *
     * Searches from the given scope upward (narrowest to broadest).
     * The first enabled matching entry is invoked.
     *
     * @param keySequence The key sequence string (e.g., "Ctrl+S").
     * @param activeScope The narrowest active scope to start searching.
     * @return true if a matching shortcut was found and invoked.
     */
    auto Dispatch(std::string_view keySequence, ShortcutScope activeScope) -> bool;

    // ====================================================================
    // Query
    // ====================================================================

    /**
     * @brief Find a shortcut entry by ID.
     */
    [[nodiscard]] auto FindById(std::string_view id) const -> const ShortcutEntry*;

    /**
     * @brief Find all shortcuts bound to a key sequence.
     */
    [[nodiscard]] auto FindByKey(std::string_view keySequence) const
        -> std::vector<const ShortcutEntry*>;

    /**
     * @brief Get all shortcuts in a given scope.
     */
    [[nodiscard]] auto EntriesInScope(ShortcutScope scope) const
        -> std::vector<const ShortcutEntry*>;

    /**
     * @brief Total number of registered shortcuts.
     */
    [[nodiscard]] auto Count() const -> int {
        return static_cast<int>(_entries.size());
    }

    // ====================================================================
    // Conflict detection
    // ====================================================================

    /**
     * @brief Detect all conflicts (same keySequence + same scope).
     */
    [[nodiscard]] auto DetectConflicts() const -> std::vector<ShortcutConflict>;

    /**
     * @brief Check if registering a new entry would conflict.
     */
    [[nodiscard]] auto WouldConflict(std::string_view keySequence,
                                      ShortcutScope scope) const -> bool;

    // ====================================================================
    // Bulk operations
    // ====================================================================

    /**
     * @brief Get all registered entries (for serialization/UI display).
     */
    [[nodiscard]] auto AllEntries() const -> const std::vector<ShortcutEntry>&;

    /**
     * @brief Remove all entries.
     */
    void Clear();

private:
    std::vector<ShortcutEntry> _entries;
};

} // namespace matcha::fw
