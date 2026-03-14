#pragma once

/**
 * @file BreakpointObserver.h
 * @brief Responsive breakpoint observer for priority-based panel collapse.
 *
 * Tracks current window dimensions and resolves which UI elements should
 * hide/collapse based on a priority-ordered list of breakpoint rules.
 *
 * Qt-free model. Shell-level code feeds window resize events into this.
 *
 * @see Matcha_Design_System_Specification.md §6.3
 */

#include "Matcha/Foundation/Macros.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace matcha::fw {

/**
 * @brief Action to take when a breakpoint threshold is crossed.
 */
enum class CollapseAction : uint8_t {
    AutoHide,        ///< Element hides entirely (toggle button remains)
    IconOnly,        ///< Hide text labels, show icons only
    HideSection,     ///< Hide a specific section of the element
    OverflowMenu,    ///< Overflow excess items into a "..." menu
    SingleMode,      ///< Switch to single-item mode (e.g. single viewport)
};

/**
 * @brief Dimension axis for a breakpoint rule.
 */
enum class BreakpointAxis : uint8_t {
    Width,   ///< Collapse when width < threshold (default)
    Height,  ///< Collapse when height < threshold
};

/**
 * @brief A single breakpoint rule: when dimension < threshold, take action.
 */
struct BreakpointRule {
    int            priority  = 0;        ///< Lower = first to collapse (1-based)
    std::string    elementId;            ///< Identifier for the target element
    int            threshold = 0;        ///< Dimension (px) below which the action fires
    CollapseAction action    = CollapseAction::AutoHide;
    BreakpointAxis axis      = BreakpointAxis::Width; ///< Which dimension to check
};

/**
 * @brief Result of breakpoint evaluation for a single element.
 */
struct CollapseState {
    std::string    elementId;
    CollapseAction action    = CollapseAction::AutoHide;
    bool           collapsed = false; ///< true if currently below threshold
};

/**
 * @brief Unified rule + state entry (eliminates parallel-array fragility).
 */
struct RuleState {
    BreakpointRule rule;
    bool           collapsed = false;
};

/**
 * @brief Evaluates responsive breakpoint rules against current dimensions.
 *
 * Rules are evaluated in priority order (ascending). Each rule independently
 * checks whether the current dimension is below its threshold.
 *
 * Callers register rules once, then call `Evaluate(width, height)` on resize.
 * A callback fires when any collapse states change.
 */
class MATCHA_EXPORT BreakpointObserver {
public:
    using ChangedCallback = std::function<void(const std::vector<CollapseState>& allStates)>;

    BreakpointObserver() = default;

    /**
     * @brief Add a breakpoint rule. Rules are kept sorted by priority.
     */
    void AddRule(const BreakpointRule& rule);

    /**
     * @brief Remove all rules for an element.
     */
    void RemoveRules(const std::string& elementId);

    /**
     * @brief Clear all rules.
     */
    void ClearRules();

    /**
     * @brief Evaluate all rules against the given dimensions.
     * @param width  Current container width.
     * @param height Current container height (default 0 = ignore height rules).
     * Fires callback if any collapse states changed.
     * @return Snapshot of current collapse states.
     */
    auto Evaluate(int width, int height = 0) -> std::vector<CollapseState>;

    /**
     * @brief Get current collapse states without re-evaluating.
     */
    [[nodiscard]] auto States() const -> std::vector<CollapseState>;

    /**
     * @brief Check if a specific element is currently collapsed.
     */
    [[nodiscard]] auto IsCollapsed(const std::string& elementId) const -> bool;

    /**
     * @brief Set callback for collapse state changes.
     */
    void OnChanged(ChangedCallback cb) { _callback = std::move(cb); }

    /**
     * @brief Get the number of registered rules.
     */
    [[nodiscard]] auto RuleCount() const -> int { return static_cast<int>(_entries.size()); }

private:
    std::vector<RuleState> _entries;
    ChangedCallback _callback;
    int _lastWidth  = -1;
    int _lastHeight = -1;
};

} // namespace matcha::fw
