#pragma once

/**
 * @file StringId.h
 * @brief Tag-based strong type string IDs for compile-time safety.
 *
 * Unlike StrongId<Tag> (uint64_t), StringId<Tag> wraps a std::string.
 * Different Tag types produce distinct, non-interchangeable types at
 * compile time, preventing accidental mixing of WorkshopId and WorkbenchId.
 */

#include <functional>
#include <ostream>
#include <string>
#include <string_view>

namespace matcha::fw {

// --------------------------------------------------------------------------- //
// StringId<Tag> — different Tags produce incompatible string-based ID types
// --------------------------------------------------------------------------- //

/**
 * @brief Type-safe string ID wrapper. Different Tag types produce distinct,
 *        non-interchangeable ID types at compile time.
 * @tparam Tag Empty struct used solely to differentiate ID types.
 */
template <typename Tag>
struct StringId {
    std::string value;

    [[nodiscard]] auto operator==(const StringId& other) const -> bool = default;
    [[nodiscard]] auto operator<=>(const StringId& other) const = default;

    /** @brief Check if the ID holds a non-empty string. */
    [[nodiscard]] auto IsValid() const noexcept -> bool { return !value.empty(); }

    /** @brief Get the string value as a view. */
    [[nodiscard]] auto View() const noexcept -> std::string_view { return value; }

    /** @brief Explicit factory from string literal, string_view, or std::string. */
    [[nodiscard]] static auto From(std::string_view sv) -> StringId {
        return StringId{std::string(sv)};
    }

    /** @brief Hash functor for use with unordered containers. */
    struct Hash {
        [[nodiscard]] auto operator()(const StringId& id) const noexcept -> size_t {
            return std::hash<std::string>{}(id.value);
        }
    };
};

// --------------------------------------------------------------------------- //
// Tag types for Workshop/Workbench architecture
// --------------------------------------------------------------------------- //

struct WorkshopIdTag {};
struct WorkbenchIdTag {};
struct CmdHeaderIdTag {};

// --------------------------------------------------------------------------- //
// Aliases
// --------------------------------------------------------------------------- //

using WorkshopId  = StringId<WorkshopIdTag>;   ///< Workshop identity
using WorkbenchId = StringId<WorkbenchIdTag>;   ///< Workbench identity
using CmdHeaderId = StringId<CmdHeaderIdTag>;   ///< Command header identity

// --------------------------------------------------------------------------- //
// Stream output — enables doctest / gtest / ostream printing
// --------------------------------------------------------------------------- //

template <typename Tag>
auto operator<<(std::ostream& os, const StringId<Tag>& id) -> std::ostream& {
    return os << id.value;
}

} // namespace matcha::fw

// --------------------------------------------------------------------------- //
// std::formatter specialization — enables std::format / std::print
// --------------------------------------------------------------------------- //

template <typename Tag>
struct std::formatter<matcha::fw::StringId<Tag>> : std::formatter<std::string_view> {
    auto format(const matcha::fw::StringId<Tag>& id, auto& ctx) const {
        return std::formatter<std::string_view>::format(id.value, ctx);
    }
};
