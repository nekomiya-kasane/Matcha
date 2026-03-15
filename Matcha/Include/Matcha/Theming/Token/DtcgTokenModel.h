#pragma once

/**
 * @file DtcgTokenModel.h
 * @brief W3C Design Token Community Group (DTCG) token data model.
 *
 * Implements the token representation from Spec §4.4.1:
 * - Token types: color, dimension, transition, fontFamily, fontWeight, number, string
 * - Token value: tagged union (DtcgValue)
 * - Token descriptor: $type, $value, $description, optional alias ($value = "{ref}")
 * - Token group: hierarchical container
 * - Import/export as DTCG-compatible JSON strings (Qt-free, uses no external JSON lib)
 *
 * The export produces valid JSON. The import parses a minimal subset of JSON
 * sufficient for DTCG token files.
 *
 * @see Matcha_Design_System_Specification.md §4.4.1
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace matcha::fw {

// ============================================================================
// DTCG Token Types
// ============================================================================

/**
 * @enum DtcgType
 * @brief W3C DTCG $type values.
 */
enum class DtcgType : uint8_t {
    Color,
    Dimension,
    Transition,
    FontFamily,
    FontWeight,
    Number,
    String,
};

[[nodiscard]] MATCHA_EXPORT auto DtcgTypeToString(DtcgType t) -> std::string;
[[nodiscard]] MATCHA_EXPORT auto DtcgTypeFromString(std::string_view s) -> std::optional<DtcgType>;

// ============================================================================
// DTCG Values
// ============================================================================

/**
 * @struct DtcgDimension
 * @brief A dimension value with unit.
 */
struct DtcgDimension {
    double      value = 0.0;
    std::string unit  = "px";
};

/**
 * @struct DtcgTransition
 * @brief A transition value with duration, delay, and timing function.
 */
struct DtcgTransition {
    double duration = 150.0;   ///< ms
    double delay    = 0.0;     ///< ms
    std::vector<double> timingFunction = {0.33, 0.0, 0.67, 1.0}; ///< cubic-bezier
};

/**
 * @brief Tagged union for DTCG token values.
 */
using DtcgValue = std::variant<
    std::string,       ///< Color (hex string) or String or FontFamily or alias ref
    DtcgDimension,     ///< Dimension
    DtcgTransition,    ///< Transition
    double,            ///< Number or FontWeight
    std::monostate     ///< Unset
>;

// ============================================================================
// DtcgToken
// ============================================================================

/**
 * @struct DtcgToken
 * @brief A single design token in DTCG format.
 */
struct DtcgToken {
    std::string name;
    DtcgType    type = DtcgType::String;
    DtcgValue   value;
    std::string description;
    std::string aliasRef;   ///< Non-empty if $value is "{some.ref}"

    [[nodiscard]] auto IsAlias() const -> bool { return !aliasRef.empty(); }
};

// ============================================================================
// DtcgTokenGroup
// ============================================================================

/**
 * @struct DtcgTokenGroup
 * @brief A hierarchical group of tokens (maps to a JSON object with children).
 */
struct DtcgTokenGroup {
    std::string name;
    std::vector<DtcgToken>      tokens;
    std::vector<DtcgTokenGroup> children;
};

// ============================================================================
// DtcgTokenFile
// ============================================================================

/**
 * @struct DtcgTokenFile
 * @brief Root-level representation of a DTCG token file.
 */
struct DtcgTokenFile {
    std::string schema;    ///< $schema URL
    std::vector<DtcgTokenGroup> groups;
};

// ============================================================================
// DtcgSerializer
// ============================================================================

/**
 * @class DtcgSerializer
 * @brief Serializes/deserializes DtcgTokenFile to/from JSON strings.
 *
 * Uses minimal hand-rolled JSON generation (no external lib dependency).
 * Import uses a simple recursive-descent parser for the DTCG subset.
 */
class MATCHA_EXPORT DtcgSerializer {
public:
    DtcgSerializer() = default;

    /**
     * @brief Export a DtcgTokenFile to a JSON string.
     * @param file The token file to serialize.
     * @param indent Number of spaces per indent level (0 = compact).
     * @return JSON string.
     */
    [[nodiscard]] static auto Export(const DtcgTokenFile& file, int indent = 2) -> std::string;

    /**
     * @brief Import a DtcgTokenFile from a JSON string.
     * @param json The JSON string to parse.
     * @return Parsed token file, or std::nullopt on parse error.
     */
    [[nodiscard]] static auto Import(std::string_view json) -> std::optional<DtcgTokenFile>;

    /**
     * @brief Flatten a DtcgTokenFile into a flat list of tokens with dot-separated paths.
     * @param file The token file.
     * @return Map of "group.subgroup.name" -> DtcgToken.
     */
    [[nodiscard]] static auto Flatten(const DtcgTokenFile& file)
        -> std::unordered_map<std::string, DtcgToken>;
};

} // namespace matcha::fw
