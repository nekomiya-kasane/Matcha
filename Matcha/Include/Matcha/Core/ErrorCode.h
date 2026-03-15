#pragma once

/**
 * @file ErrorCode.h
 * @brief ErrorCode enum and Expected<T> alias for type-safe error handling.
 */

#include <cstdint>
#include <expected>
#include <format>
#include <string_view>

namespace matcha::fw {

// --------------------------------------------------------------------------- //
// ErrorCode
// --------------------------------------------------------------------------- //

/** @brief Enumeration of all framework-level error conditions. */
enum class ErrorCode : uint8_t {
    Ok = 0,
    NotFound,
    AlreadyExists,
    InvalidArgument,
    StaleHandle,
    PluginLoadFailed,
    Timeout,
    AccessDenied,
    Cancelled,
};

// --------------------------------------------------------------------------- //
// Expected<T>
// --------------------------------------------------------------------------- //

/**
 * @brief Alias for std::expected using ErrorCode as the error type.
 * @tparam T The success value type.
 */
template <typename T>
using Expected = std::expected<T, ErrorCode>;

// --------------------------------------------------------------------------- //
// ToString
// --------------------------------------------------------------------------- //

/**
 * @brief Convert an ErrorCode to its string representation.
 * @param code The error code.
 * @return A compile-time string view of the error name.
 */
[[nodiscard]] constexpr auto ToString(ErrorCode code) noexcept -> std::string_view {
    switch (code) {
        case ErrorCode::Ok:               return "Ok";
        case ErrorCode::NotFound:         return "NotFound";
        case ErrorCode::AlreadyExists:    return "AlreadyExists";
        case ErrorCode::InvalidArgument:  return "InvalidArgument";
        case ErrorCode::StaleHandle:      return "StaleHandle";
        case ErrorCode::PluginLoadFailed: return "PluginLoadFailed";
        case ErrorCode::Timeout:          return "Timeout";
        case ErrorCode::AccessDenied:     return "AccessDenied";
        case ErrorCode::Cancelled:       return "Cancelled";
    }
    return "Unknown";
}

} // namespace matcha::fw

// --------------------------------------------------------------------------- //
// std::formatter specialization — enables std::format / std::print
// --------------------------------------------------------------------------- //

template <>
struct std::formatter<matcha::fw::ErrorCode> : std::formatter<std::string_view> {
    auto format(matcha::fw::ErrorCode code, auto& ctx) const {
        return std::formatter<std::string_view>::format(matcha::fw::ToString(code), ctx);
    }
};
