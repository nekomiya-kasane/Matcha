#pragma once

/**
 * @file AnimationBlendLayer.h
 * @brief Animation-in-flight blending: cascade priority layer 5 (Spec §4.17).
 *
 * The style cascade has 5 layers:
 *   1. Base theme
 *   2. Component override
 *   3. Instance override
 *   4. State mapping
 *   5. Animation in-flight  <-- THIS LAYER
 *
 * When an animation is running on a property, its interpolated value takes
 * priority over the static resolved value. This component tracks which
 * properties are currently animated and their in-flight values.
 *
 * Qt-free Foundation layer.
 *
 * @see Matcha_Design_System_Specification.md §4.17
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace matcha::fw {

// ============================================================================
// BlendEntry
// ============================================================================

/**
 * @struct BlendEntry
 * @brief A single in-flight animation value for a property.
 */
struct BlendEntry {
    double   value      = 0.0;     ///< Current interpolated value
    double   weight     = 1.0;     ///< Blend weight [0, 1]
    double   progress   = 0.0;     ///< Animation progress [0, 1]
    uint64_t animId     = 0;       ///< Associated animation handle
    bool     active     = false;   ///< Whether this entry is still animating
};

// ============================================================================
// AnimationBlendLayer
// ============================================================================

/**
 * @class AnimationBlendLayer
 * @brief Tracks in-flight animation values for style cascade layer 5.
 *
 * Used by the resolve pipeline: after computing layers 1-4, check this layer
 * for any active overrides. If present, blend or replace the static value.
 *
 * Usage:
 * @code
 *   AnimationBlendLayer blend;
 *
 *   // Animation starts
 *   blend.SetEntry("opacity", {.value = 0.5, .weight = 1.0, .progress = 0.3,
 *                                .animId = 42, .active = true});
 *
 *   // During resolve
 *   if (auto* e = blend.GetEntry("opacity"); e && e->active) {
 *       resolvedOpacity = Lerp(resolvedOpacity, e->value, e->weight);
 *   }
 *
 *   // Animation completes
 *   blend.Remove("opacity");
 * @endcode
 */
class MATCHA_EXPORT AnimationBlendLayer {
public:
    AnimationBlendLayer() = default;

    // ====================================================================
    // Entry management
    // ====================================================================

    /**
     * @brief Set or update an in-flight animation entry for a property.
     */
    void SetEntry(const std::string& propertyKey, BlendEntry entry);

    /**
     * @brief Update just the value and progress of an existing entry.
     */
    auto UpdateValue(const std::string& propertyKey,
                     double value, double progress) -> bool;

    /**
     * @brief Remove an entry (animation completed or cancelled).
     */
    auto Remove(const std::string& propertyKey) -> bool;

    /**
     * @brief Get an entry, or nullptr if not present.
     */
    [[nodiscard]] auto GetEntry(const std::string& propertyKey) const -> const BlendEntry*;

    /**
     * @brief Check if a property has an active in-flight animation.
     */
    [[nodiscard]] auto HasActive(const std::string& propertyKey) const -> bool;

    // ====================================================================
    // Blending
    // ====================================================================

    /**
     * @brief Apply the blend layer to a base value.
     *
     * If the property has an active entry, returns:
     *   base * (1 - weight) + entry.value * weight
     * Otherwise returns base unchanged.
     *
     * @param propertyKey Property to check.
     * @param baseValue Value from cascade layers 1-4.
     * @return Blended value.
     */
    [[nodiscard]] auto Apply(const std::string& propertyKey,
                              double baseValue) const -> double;

    // ====================================================================
    // Bulk operations
    // ====================================================================

    /**
     * @brief Mark all entries as inactive (animation service paused/reset).
     */
    void DeactivateAll();

    /**
     * @brief Remove all inactive entries.
     */
    void PurgeInactive();

    /**
     * @brief Remove all entries.
     */
    void Clear();

    [[nodiscard]] auto EntryCount() const -> int {
        return static_cast<int>(_entries.size());
    }

    [[nodiscard]] auto ActiveCount() const -> int;

private:
    std::unordered_map<std::string, BlendEntry> _entries;
};

} // namespace matcha::fw
