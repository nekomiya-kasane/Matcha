#pragma once

/**
 * @file ChoreographyEngine.h
 * @brief Choreography patterns: Stagger, Cascade, Sequence.
 *
 * Coordinates multi-element animations in three patterns (Spec §8.8):
 *
 * - **Stagger**: N sibling elements animate with incremental delay.
 *   delay_i = baseDelay + i * staggerInterval, capped at maxStagger.
 *
 * - **Cascade**: Hierarchical reveal. Child starts when parent reaches
 *   a trigger fraction (default 60%). Max depth 3; beyond that, instant.
 *
 * - **Sequence**: Strict serial chain. Step N+1 starts when step N finishes.
 *
 * This is a Foundation-layer component with zero Qt dependency.
 * It computes timing schedules; actual animation is driven by callbacks.
 *
 * @see Matcha_Design_System_Specification.md §8.8
 */

#include <Matcha/Foundation/Macros.h>

#include <string>
#include <vector>

namespace matcha::fw {

// ============================================================================
// StaggerConfig
// ============================================================================

/**
 * @struct StaggerConfig
 * @brief Configuration for a stagger animation pattern.
 */
struct StaggerConfig {
    double baseDelayMs      = 0.0;    ///< Delay before the first item starts
    double staggerIntervalMs = 30.0;  ///< Delay increment per item
    double maxStaggerMs     = 300.0;  ///< Cap on total delay (not per-item)
    double itemDurationMs   = 150.0;  ///< Duration of each item's animation
};

// ============================================================================
// StaggerSchedule
// ============================================================================

/**
 * @struct StaggerScheduleEntry
 * @brief Computed timing for a single stagger element.
 */
struct StaggerScheduleEntry {
    int    index     = 0;
    double delayMs   = 0.0;   ///< When this element should start
    double durationMs = 0.0;  ///< How long this element animates
};

// ============================================================================
// CascadeConfig
// ============================================================================

/**
 * @struct CascadeConfig
 * @brief Configuration for a cascade animation pattern.
 */
struct CascadeConfig {
    double triggerFraction    = 0.6;   ///< Child starts at this fraction of parent progress
    int    maxDepth           = 3;     ///< Beyond this depth, elements appear instantly (delay=0, duration=0)
    double levelDurationMs    = 200.0; ///< Animation duration per level
};

/**
 * @struct CascadeScheduleEntry
 * @brief Computed timing for a single cascade level.
 */
struct CascadeScheduleEntry {
    int    depth      = 0;
    double delayMs    = 0.0;
    double durationMs = 0.0;
    bool   instant    = false;  ///< True if beyond max depth
};

// ============================================================================
// SequenceConfig
// ============================================================================

/**
 * @struct SequenceStep
 * @brief A single step in a sequence animation.
 */
struct SequenceStep {
    std::string id;
    double      durationMs = 150.0;
};

/**
 * @struct SequenceScheduleEntry
 * @brief Computed timing for a single sequence step.
 */
struct SequenceScheduleEntry {
    int         index      = 0;
    std::string id;
    double      delayMs    = 0.0;   ///< Cumulative delay from sequence start
    double      durationMs = 0.0;
};

// ============================================================================
// ChoreographyEngine
// ============================================================================

/**
 * @class ChoreographyEngine
 * @brief Computes timing schedules for choreography patterns.
 *
 * Pure computation — no side effects, no Qt dependency.
 * Call ComputeStagger/ComputeCascade/ComputeSequence to get timing,
 * then feed the results to your animation service.
 *
 * Usage:
 * @code
 *   ChoreographyEngine engine;
 *
 *   // Stagger: 8 menu items
 *   auto stagger = engine.ComputeStagger(8, {.staggerIntervalMs = 30});
 *   for (auto& e : stagger) {
 *       scheduleAt(e.delayMs, [=]{ animate(item[e.index], e.durationMs); });
 *   }
 *
 *   // Cascade: 4-level tree
 *   auto cascade = engine.ComputeCascade(4, {.triggerFraction = 0.6});
 *
 *   // Sequence: fade out → resize → fade in
 *   auto seq = engine.ComputeSequence({
 *       {.id = "fadeOut", .durationMs = 150},
 *       {.id = "resize",  .durationMs = 200},
 *       {.id = "fadeIn",  .durationMs = 150},
 *   });
 * @endcode
 */
class MATCHA_EXPORT ChoreographyEngine {
public:
    ChoreographyEngine() = default;

    // ====================================================================
    // Stagger (§8.8.1)
    // ====================================================================

    /**
     * @brief Compute stagger timing for N elements.
     * @param itemCount Number of sibling elements.
     * @param config Stagger parameters.
     * @return Schedule entries sorted by delay.
     */
    [[nodiscard]] static auto ComputeStagger(int itemCount,
                                              const StaggerConfig& config = {})
        -> std::vector<StaggerScheduleEntry>;

    // ====================================================================
    // Cascade (§8.8.2)
    // ====================================================================

    /**
     * @brief Compute cascade timing for a hierarchy of depth levels.
     * @param depth Number of hierarchy levels (1 = root only).
     * @param config Cascade parameters.
     * @return Schedule entries for each level.
     */
    [[nodiscard]] static auto ComputeCascade(int depth,
                                              const CascadeConfig& config = {})
        -> std::vector<CascadeScheduleEntry>;

    // ====================================================================
    // Sequence (§8.8.3)
    // ====================================================================

    /**
     * @brief Compute sequence timing for ordered steps.
     * @param steps Step definitions with durations.
     * @return Schedule entries with cumulative delays.
     */
    [[nodiscard]] static auto ComputeSequence(const std::vector<SequenceStep>& steps)
        -> std::vector<SequenceScheduleEntry>;

    // ====================================================================
    // Utility
    // ====================================================================

    /**
     * @brief Total duration of a stagger schedule (last item's delay + duration).
     */
    [[nodiscard]] static auto TotalDuration(
        const std::vector<StaggerScheduleEntry>& schedule) -> double;

    /**
     * @brief Total duration of a cascade schedule.
     */
    [[nodiscard]] static auto TotalDuration(
        const std::vector<CascadeScheduleEntry>& schedule) -> double;

    /**
     * @brief Total duration of a sequence schedule.
     */
    [[nodiscard]] static auto TotalDuration(
        const std::vector<SequenceScheduleEntry>& schedule) -> double;
};

} // namespace matcha::fw
