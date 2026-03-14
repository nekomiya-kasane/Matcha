#pragma once

/**
 * @file InteractionTimingRegistry.h
 * @brief Registry of interaction timing tokens (non-animation intervals).
 *
 * Implements §8.7 Interaction Timing Tokens from the Matcha Design System Spec.
 * These tokens define when interactions trigger, debounce, or timeout —
 * distinct from animation duration tokens (§8.1) which control visual motion.
 *
 * Platform overrides (§8.7.2): On Windows, certain tokens are queried from
 * the OS at startup via SystemParametersInfo(). Fallback values apply on
 * other platforms or when the query fails.
 *
 * This is a Foundation-layer component with zero Qt dependency.
 *
 * @see Matcha_Design_System_Specification.md §8.7
 */

#include <Matcha/Foundation/Macros.h>

#include <array>
#include <chrono>
#include <cstdint>

namespace matcha::fw {

// ============================================================================
// TimingToken — 15 named interaction timing tokens (§8.7.1)
// ============================================================================

/**
 * @enum TimingToken
 * @brief Named interaction timing intervals.
 *
 * | Token                | Default  | Use Case                                          |
 * |----------------------|:--------:|---------------------------------------------------|
 * | HoverDelay           | 200ms    | Delay before hover state activates                |
 * | TooltipDelay         | 500ms    | Delay before tooltip appears                      |
 * | TooltipDismissDelay  | 100ms    | Grace period when mouse leaves tooltip trigger     |
 * | LongPressThreshold   | 500ms    | Duration for long-press recognition               |
 * | DoubleClickWindow    | 400ms    | Max interval between clicks for double-click       |
 * | DebounceSearch       | 300ms    | Debounce for search-as-you-type                   |
 * | DebounceResize       | 100ms    | Debounce for window resize layout recalc           |
 * | AutoSaveInterval     | 30000ms  | Interval for auto-save                            |
 * | IdleTimeout          | 60000ms  | Time before UI enters idle mode                   |
 * | RepeatKeyInitial     | 500ms    | Delay before key repeat starts                    |
 * | RepeatKeyInterval    | 33ms     | Interval between key repeats (~30/s)              |
 * | DragInitDelay        | 150ms    | Hold duration before drag initiates               |
 * | ToastDismissTimeout  | 5000ms   | Auto-dismiss delay for Toast notifications        |
 * | MenuOpenDelay        | 200ms    | Delay before submenu opens on hover               |
 * | MenuCloseDelay       | 300ms    | Grace period before submenu closes                |
 */
enum class TimingToken : uint8_t {
    HoverDelay = 0,
    TooltipDelay,
    TooltipDismissDelay,
    LongPressThreshold,
    DoubleClickWindow,
    DebounceSearch,
    DebounceResize,
    AutoSaveInterval,
    IdleTimeout,
    RepeatKeyInitial,
    RepeatKeyInterval,
    DragInitDelay,
    ToastDismissTimeout,
    MenuOpenDelay,
    MenuCloseDelay,

    _Count  ///< Sentinel — do not use as a token.
};

/// Number of defined timing tokens.
inline constexpr int kTimingTokenCount = static_cast<int>(TimingToken::_Count);

// ============================================================================
// InteractionTimingRegistry
// ============================================================================

/**
 * @class InteractionTimingRegistry
 * @brief Central registry for interaction timing values.
 *
 * **Thread safety**: Not thread-safe. All calls must be from the GUI thread.
 *
 * Usage:
 * @code
 *   InteractionTimingRegistry reg;                       // defaults from §8.7.1
 *   reg.ApplyPlatformOverrides();                        // query OS (Windows)
 *   auto delay = reg.Get(TimingToken::TooltipDelay);     // 500ms
 *   reg.Set(TimingToken::TooltipDelay, 300ms);           // override
 *   reg.ResetToDefault(TimingToken::TooltipDelay);       // back to 500ms
 *   reg.ResetAll();                                      // all to spec defaults
 * @endcode
 */
class MATCHA_EXPORT InteractionTimingRegistry {
public:
    using Milliseconds = std::chrono::milliseconds;

    /**
     * @brief Construct with all spec-default values (§8.7.1).
     */
    InteractionTimingRegistry();

    // ====================================================================
    // Access
    // ====================================================================

    /**
     * @brief Get the current value of a timing token.
     * @param token The timing token to query.
     * @return Duration in milliseconds.
     */
    [[nodiscard]] auto Get(TimingToken token) const -> Milliseconds;

    /**
     * @brief Get the current value as a raw integer (ms).
     */
    [[nodiscard]] auto GetMs(TimingToken token) const -> int;

    /**
     * @brief Get the spec-defined default value of a timing token.
     */
    [[nodiscard]] static auto DefaultValue(TimingToken token) -> Milliseconds;

    // ====================================================================
    // Mutation
    // ====================================================================

    /**
     * @brief Override a timing token with a custom value.
     * @param token The token to set.
     * @param value New duration in milliseconds.
     */
    void Set(TimingToken token, Milliseconds value);

    /**
     * @brief Reset a single token to its spec-default value.
     */
    void ResetToDefault(TimingToken token);

    /**
     * @brief Reset all tokens to spec-default values.
     */
    void ResetAll();

    // ====================================================================
    // Platform integration (§8.7.2)
    // ====================================================================

    /**
     * @brief Query OS-specific timing values and apply them.
     *
     * On Windows, queries SystemParametersInfo for:
     * - DoubleClickWindow (GetDoubleClickTime)
     * - RepeatKeyInitial  (SPI_GETKEYBOARDDELAY)
     * - RepeatKeyInterval (SPI_GETKEYBOARDSPEED)
     * - HoverDelay        (SPI_GETMOUSEHOVERTIME)
     *
     * On other platforms, this is a no-op (spec defaults remain).
     *
     * @return Number of tokens that were updated from OS values.
     */
    auto ApplyPlatformOverrides() -> int;

private:
    std::array<Milliseconds, kTimingTokenCount> _values;
};

} // namespace matcha::fw
