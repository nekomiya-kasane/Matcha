#pragma once

/**
 * @file StrongId.h
 * @brief Tag-based strong type IDs for compile-time safety.
 */

#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>

namespace matcha::fw {

// --------------------------------------------------------------------------- //
// StrongId<Tag> — different Tags produce incompatible types
// --------------------------------------------------------------------------- //

/**
 * @brief Type-safe ID wrapper. Different Tag types produce distinct,
 *        non-interchangeable ID types at compile time.
 * @tparam Tag Empty struct used solely to differentiate ID types.
 */
template <typename Tag>
struct StrongId {
    uint64_t value;

    [[nodiscard]] constexpr auto operator<=>(const StrongId&) const = default;

    /** @brief Explicit factory — prevents implicit construction from raw integers. */
    [[nodiscard]] static constexpr auto From(uint64_t v) noexcept -> StrongId {
        return {v};
    }

    /** @brief Hash functor for use with unordered containers. */
    struct Hash {
        [[nodiscard]] constexpr auto operator()(StrongId id) const noexcept -> size_t {
            return std::hash<uint64_t>{}(id.value);
        }
    };
};

// --------------------------------------------------------------------------- //
// Tag types
// --------------------------------------------------------------------------- //

struct DocumentIdTag {};
struct ViewportIdTag {};
struct WidgetIdTag {};
struct PageIdTag {};
struct WindowIdTag {};

// --------------------------------------------------------------------------- //
// Aliases
// --------------------------------------------------------------------------- //

using DocumentId = StrongId<DocumentIdTag>;  ///< aka DocId
using ViewportId = StrongId<ViewportIdTag>;
using WidgetId   = StrongId<WidgetIdTag>;
using PageId     = StrongId<PageIdTag>;       ///< DocumentPage identity (1:N model)
using WindowId   = StrongId<WindowIdTag>;     ///< WindowNode identity

// --------------------------------------------------------------------------- //
// WidgetId bit layout: [8-bit Type | 16-bit Generation | 40-bit Index]
//
//   63       56 55         40 39                       0
//  +----------+-------------+-------------------------+
//  |   Type   |  Generation |          Index           |
//  +----------+-------------+-------------------------+
// --------------------------------------------------------------------------- //

inline constexpr unsigned kWidgetIdIndexBits      = 40;
inline constexpr unsigned kWidgetIdGenerationBits = 16;
inline constexpr unsigned kWidgetIdTypeBits       = 8;

inline constexpr uint64_t kWidgetIdIndexMask      = (uint64_t{1} << kWidgetIdIndexBits) - 1;
inline constexpr uint64_t kWidgetIdGenerationMask = (uint64_t{1} << kWidgetIdGenerationBits) - 1;
inline constexpr uint64_t kWidgetIdTypeMask       = (uint64_t{1} << kWidgetIdTypeBits) - 1;

inline constexpr unsigned kWidgetIdGenerationShift = kWidgetIdIndexBits;
inline constexpr unsigned kWidgetIdTypeShift       = kWidgetIdIndexBits + kWidgetIdGenerationBits;

/**
 * @brief Pack a WidgetId from type, generation, and index components.
 * @param type Widget type discriminator (0-255).
 * @param generation ABA protection counter (0-65535).
 * @param index Slot index (0 - 2^40-1).
 * @return Packed WidgetId.
 */
[[nodiscard]] constexpr auto MakeWidgetId(uint8_t type, uint16_t generation, uint64_t index) noexcept
    -> WidgetId {
    return WidgetId::From(
        (static_cast<uint64_t>(type) << kWidgetIdTypeShift) |
        (static_cast<uint64_t>(generation) << kWidgetIdGenerationShift) |
        (index & kWidgetIdIndexMask));
}

/** @brief Extract the type field from a WidgetId. */
[[nodiscard]] constexpr auto GetType(WidgetId id) noexcept -> uint8_t {
    return static_cast<uint8_t>((id.value >> kWidgetIdTypeShift) & kWidgetIdTypeMask);
}

/** @brief Extract the generation counter from a WidgetId. */
[[nodiscard]] constexpr auto GetGeneration(WidgetId id) noexcept -> uint16_t {
    return static_cast<uint16_t>((id.value >> kWidgetIdGenerationShift) & kWidgetIdGenerationMask);
}

/** @brief Extract the index from a WidgetId. */
[[nodiscard]] constexpr auto GetIndex(WidgetId id) noexcept -> uint64_t {
    return id.value & kWidgetIdIndexMask;
}

} // namespace matcha::fw

// --------------------------------------------------------------------------- //
// std::formatter specialization — enables std::format / std::print
// --------------------------------------------------------------------------- //

template <typename Tag>
struct std::formatter<matcha::fw::StrongId<Tag>> : std::formatter<uint64_t> {
    auto format(matcha::fw::StrongId<Tag> id, auto& ctx) const {
        return std::formatter<uint64_t>::format(id.value, ctx);
    }
};
