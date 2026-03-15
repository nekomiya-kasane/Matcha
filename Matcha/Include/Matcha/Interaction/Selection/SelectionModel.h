#pragma once

/**
 * @file SelectionModel.h
 * @brief Qt-free generic selection model for list/tree/table widgets.
 *
 * Implements §7.1 Selection Model from the Matcha Design System Specification.
 * Supports Single, Multi, Range, and None modes with anchor/focus tracking,
 * modifier-key semantics (Ctrl+Click toggle, Shift+Click range, Ctrl+A),
 * and observer notification.
 *
 * This is a fw-layer component with zero Qt dependency.
 *
 * @see Matcha_Design_System_Specification.md §7.1
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <functional>
#include <vector>

namespace matcha::fw {

// ============================================================================
// Selection Mode
// ============================================================================

/**
 * @brief Selection behaviour mode.
 *
 * | Mode   | Description                                       |
 * |--------|---------------------------------------------------|
 * | None   | Selection disabled — clicks have no effect.       |
 * | Single | At most one item selected at a time.              |
 * | Multi  | Multiple discrete items via Ctrl+Click / Ctrl+A.  |
 * | Range  | Contiguous range [anchor, focus] via Shift+Click. |
 */
enum class SelectionMode : uint8_t {
    None   = 0,
    Single = 1,
    Multi  = 2,
    Range  = 3,
};

// ============================================================================
// Click Modifier Flags
// ============================================================================

/**
 * @brief Modifier flags passed to Click() to determine selection behaviour.
 */
enum class ClickModifier : uint8_t {
    None  = 0,
    Ctrl  = 1U << 0U,
    Shift = 1U << 1U,
};

[[nodiscard]] constexpr auto operator|(ClickModifier a, ClickModifier b) -> ClickModifier {
    return static_cast<ClickModifier>(
        static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
[[nodiscard]] constexpr auto operator&(ClickModifier a, ClickModifier b) -> ClickModifier {
    return static_cast<ClickModifier>(
        static_cast<unsigned>(a) & static_cast<unsigned>(b));
}

// ============================================================================
// SelectionModel
// ============================================================================

/**
 * @brief Generic selection model for indexed item collections.
 *
 * **Thread safety**: Not thread-safe. All calls must be from the GUI thread.
 *
 * **Usage**:
 * @code
 *   SelectionModel sel(SelectionMode::Multi);
 *   sel.SetItemCount(100);
 *   sel.OnChanged([](const SelectionModel& m) {
 *       for (int idx : m.SelectedIndices()) { ... }
 *   });
 *   sel.Click(5, ClickModifier::None);    // select item 5
 *   sel.Click(8, ClickModifier::Ctrl);    // toggle item 8
 *   sel.Click(12, ClickModifier::Shift);  // range [5, 12]
 * @endcode
 */
class MATCHA_EXPORT SelectionModel {
public:
    using ChangedCallback = std::function<void(const SelectionModel&)>;

    explicit SelectionModel(SelectionMode mode = SelectionMode::Single);
    ~SelectionModel() = default;

    SelectionModel(const SelectionModel&)            = delete;
    SelectionModel& operator=(const SelectionModel&) = delete;
    SelectionModel(SelectionModel&&)                 = default;
    SelectionModel& operator=(SelectionModel&&)      = default;

    // ====================================================================
    // Configuration
    // ====================================================================

    /**
     * @brief Set the total number of selectable items.
     *
     * Clears selection if item count changes. Indices are [0, count).
     */
    void SetItemCount(int count);

    [[nodiscard]] auto ItemCount() const -> int { return _itemCount; }

    void SetMode(SelectionMode mode);

    [[nodiscard]] auto Mode() const -> SelectionMode { return _mode; }

    // ====================================================================
    // Interaction API (§7.1.2 Modifier Key Semantics)
    // ====================================================================

    /**
     * @brief Process a click on an item with optional modifier keys.
     *
     * | Modifier | Effect (Mode=Multi/Range)                        |
     * |----------|--------------------------------------------------|
     * | None     | Clear all, select index, anchor=focus=index      |
     * | Ctrl     | Toggle index in selection set, anchor=index      |
     * | Shift    | Select range [anchor, index], focus=index        |
     *
     * In Single mode, Ctrl/Shift are ignored — always single selection.
     *
     * @param index Item index [0, itemCount).
     * @param mod   Modifier flags.
     */
    void Click(int index, ClickModifier mod = ClickModifier::None);

    /**
     * @brief Select all items. Sets anchor=0, focus=last.
     *
     * No-op in Single or None mode.
     */
    void SelectAll();

    /**
     * @brief Clear all selection. Resets anchor and focus to -1.
     */
    void Clear();

    /**
     * @brief Move focus by delta (for arrow key navigation).
     *
     * @param delta    Typically +1 (down/right) or -1 (up/left).
     * @param extend   If true (Shift held), extend range selection.
     */
    void MoveFocus(int delta, bool extend = false);

    // ====================================================================
    // Query API
    // ====================================================================

    /**
     * @brief Get sorted list of selected indices.
     */
    [[nodiscard]] auto SelectedIndices() const -> const std::vector<int>&;

    /**
     * @brief Check if a specific index is selected.
     */
    [[nodiscard]] auto IsSelected(int index) const -> bool;

    /**
     * @brief Get the number of selected items.
     */
    [[nodiscard]] auto SelectionCount() const -> int {
        return static_cast<int>(_selected.size());
    }

    /**
     * @brief Is nothing selected?
     */
    [[nodiscard]] auto IsEmpty() const -> bool { return _selected.empty(); }

    /**
     * @brief Get the anchor index (starting point of range selection).
     *
     * Returns -1 if no anchor.
     */
    [[nodiscard]] auto Anchor() const -> int { return _anchor; }

    /**
     * @brief Get the focus index (most recently interacted item).
     *
     * Returns -1 if no focus.
     */
    [[nodiscard]] auto Focus() const -> int { return _focus; }

    // ====================================================================
    // Observer
    // ====================================================================

    /**
     * @brief Register a callback invoked when selection changes.
     *
     * Only one callback is supported. Setting a new one replaces the old.
     */
    void OnChanged(ChangedCallback cb);

private:
    void SetRange(int from, int to);
    void NotifyChanged();
    [[nodiscard]] auto ValidIndex(int idx) const -> bool {
        return idx >= 0 && idx < _itemCount;
    }

    SelectionMode    _mode      = SelectionMode::Single;
    int              _itemCount = 0;
    int              _anchor    = -1;
    int              _focus     = -1;
    std::vector<int> _selected;
    ChangedCallback  _callback;
};

} // namespace matcha::fw
