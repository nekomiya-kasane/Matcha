/**
 * @file ChoreographyEngine.cpp
 * @brief Implementation of ChoreographyEngine.
 */

#include <Matcha/Animation/ChoreographyEngine.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Stagger (§8.8.1)
// ============================================================================

auto ChoreographyEngine::ComputeStagger(int itemCount,
                                          const StaggerConfig& config)
    -> std::vector<StaggerScheduleEntry>
{
    std::vector<StaggerScheduleEntry> result;
    if (itemCount <= 0) {
        return result;
    }
    result.reserve(static_cast<std::size_t>(itemCount));

    for (int i = 0; i < itemCount; ++i) {
        const double rawDelay = config.baseDelayMs
                                + (static_cast<double>(i) * config.staggerIntervalMs);
        const double clampedDelay = std::min(rawDelay, config.maxStaggerMs);
        result.push_back({
            .index = i,
            .delayMs = clampedDelay,
            .durationMs = config.itemDurationMs,
        });
    }
    return result;
}

// ============================================================================
// Cascade (§8.8.2)
// ============================================================================

auto ChoreographyEngine::ComputeCascade(int depth,
                                          const CascadeConfig& config)
    -> std::vector<CascadeScheduleEntry>
{
    std::vector<CascadeScheduleEntry> result;
    if (depth <= 0) {
        return result;
    }
    result.reserve(static_cast<std::size_t>(depth));

    double cumulativeDelay = 0.0;

    for (int d = 0; d < depth; ++d) {
        if (d >= config.maxDepth) {
            // Beyond max depth: instant
            result.push_back({
                .depth = d,
                .delayMs = cumulativeDelay,
                .durationMs = 0.0,
                .instant = true,
            });
        } else {
            result.push_back({
                .depth = d,
                .delayMs = cumulativeDelay,
                .durationMs = config.levelDurationMs,
                .instant = false,
            });
            // Next level starts at triggerFraction of this level's duration
            cumulativeDelay += config.triggerFraction * config.levelDurationMs;
        }
    }
    return result;
}

// ============================================================================
// Sequence (§8.8.3)
// ============================================================================

auto ChoreographyEngine::ComputeSequence(const std::vector<SequenceStep>& steps)
    -> std::vector<SequenceScheduleEntry>
{
    std::vector<SequenceScheduleEntry> result;
    result.reserve(steps.size());

    double cumulativeDelay = 0.0;

    for (int i = 0; i < static_cast<int>(steps.size()); ++i) {
        const auto& step = steps[static_cast<std::size_t>(i)];
        result.push_back({
            .index = i,
            .id = step.id,
            .delayMs = cumulativeDelay,
            .durationMs = step.durationMs,
        });
        cumulativeDelay += step.durationMs;
    }
    return result;
}

// ============================================================================
// Utility: TotalDuration
// ============================================================================

auto ChoreographyEngine::TotalDuration(
    const std::vector<StaggerScheduleEntry>& schedule) -> double
{
    if (schedule.empty()) {
        return 0.0;
    }
    const auto& last = schedule.back();
    return last.delayMs + last.durationMs;
}

auto ChoreographyEngine::TotalDuration(
    const std::vector<CascadeScheduleEntry>& schedule) -> double
{
    if (schedule.empty()) {
        return 0.0;
    }
    double maxEnd = 0.0;
    for (const auto& e : schedule) {
        maxEnd = std::max(maxEnd, e.delayMs + e.durationMs);
    }
    return maxEnd;
}

auto ChoreographyEngine::TotalDuration(
    const std::vector<SequenceScheduleEntry>& schedule) -> double
{
    if (schedule.empty()) {
        return 0.0;
    }
    const auto& last = schedule.back();
    return last.delayMs + last.durationMs;
}

} // namespace matcha::fw
