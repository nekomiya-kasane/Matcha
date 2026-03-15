#pragma once

/**
 * @file VariantNameRegistry.h
 * @brief Variant name ↔ index registry (Spec §4.7.2).
 *
 * Each WidgetKind has a canonical set of variant names (e.g. PushButton has
 * "primary", "secondary", "ghost", "danger"). This registry maps between
 * the human-readable variant name and the integer index used internally
 * by WidgetStyleSheet::variants[].
 *
 * Qt-free Foundation layer.
 *
 * @see Matcha_Design_System_Specification.md §4.7.2
 */

#include <Matcha/Core/Macros.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace matcha::fw {

// ============================================================================
// VariantNameRegistry
// ============================================================================

/**
 * @class VariantNameRegistry
 * @brief Maps variant names ↔ indices per widget kind.
 *
 * Usage:
 * @code
 *   VariantNameRegistry reg;
 *   reg.RegisterKind("PushButton", {"primary", "secondary", "ghost", "danger"});
 *
 *   auto idx = reg.IndexOf("PushButton", "ghost");  // -> 2
 *   auto name = reg.NameOf("PushButton", 2);         // -> "ghost"
 * @endcode
 */
class MATCHA_EXPORT VariantNameRegistry {
public:
    VariantNameRegistry() = default;

    /**
     * @brief Register canonical variant names for a widget kind.
     *
     * Order determines indices: names[0] → index 0, names[1] → index 1, etc.
     * Replaces any previous registration for this kind.
     *
     * @param widgetKind Widget kind identifier (e.g. "PushButton").
     * @param variantNames Ordered list of canonical variant names.
     */
    void RegisterKind(std::string_view widgetKind,
                      std::vector<std::string> variantNames);

    /**
     * @brief Look up the index for a variant name.
     * @return Index, or std::nullopt if not found.
     */
    [[nodiscard]] auto IndexOf(std::string_view widgetKind,
                                std::string_view variantName) const -> std::optional<int>;

    /**
     * @brief Look up the variant name for an index.
     * @return Name, or std::nullopt if out of range.
     */
    [[nodiscard]] auto NameOf(std::string_view widgetKind,
                               int index) const -> std::optional<std::string_view>;

    /**
     * @brief Get all variant names for a widget kind.
     * @return Variant name list, or empty span if kind not registered.
     */
    [[nodiscard]] auto VariantNames(std::string_view widgetKind) const
        -> const std::vector<std::string>*;

    /**
     * @brief Get the number of variants for a widget kind.
     */
    [[nodiscard]] auto VariantCount(std::string_view widgetKind) const -> int;

    /**
     * @brief Check if a widget kind is registered.
     */
    [[nodiscard]] auto HasKind(std::string_view widgetKind) const -> bool;

    /**
     * @brief Get all registered widget kind names.
     */
    [[nodiscard]] auto RegisteredKinds() const -> std::vector<std::string_view>;

    /**
     * @brief Remove registration for a widget kind.
     */
    auto UnregisterKind(std::string_view widgetKind) -> bool;

    /**
     * @brief Remove all registrations.
     */
    void Clear();

private:
    struct KindEntry {
        std::vector<std::string> names;
        std::unordered_map<std::string, int> nameToIndex;
    };

    std::unordered_map<std::string, KindEntry> _kinds;
};

} // namespace matcha::fw
