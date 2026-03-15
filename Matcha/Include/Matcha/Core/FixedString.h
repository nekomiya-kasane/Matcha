#pragma once

/**
 * @file FixedString.h
 * @brief Compile-time fixed-length string for use as NTTP (Non-Type Template Parameter).
 *
 * Enables string literals as template arguments in C++23:
 * @code
 *   template <FixedString Path> struct Query { ... };
 *   auto q = Query<"WindowNode/TitleBar/MenuBar">{};
 * @endcode
 */

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace matcha {

/**
 * @brief Compile-time string that can be used as a non-type template parameter.
 *
 * @tparam N Size of the character array including null terminator.
 */
template <std::size_t N>
struct FixedString {
    char data[N]{};

    consteval FixedString() = default;

    consteval FixedString(const char (&str)[N]) { // NOLINT(google-explicit-constructor)
        std::copy_n(str, N, data);
    }

    [[nodiscard]] consteval auto size() const -> std::size_t { return N - 1; }

    [[nodiscard]] consteval auto view() const -> std::string_view {
        return {data, N - 1};
    }

    [[nodiscard]] consteval operator std::string_view() const { // NOLINT(google-explicit-constructor)
        return view();
    }

    [[nodiscard]] consteval auto operator[](std::size_t i) const -> char {
        return data[i];
    }

    template <std::size_t M>
    [[nodiscard]] consteval auto operator==(const FixedString<M>& other) const -> bool {
        return view() == other.view();
    }
};

/**
 * @brief Concatenate two FixedStrings with a '/' separator at compile time.
 */
template <std::size_t A, std::size_t B>
consteval auto ConcatPath(const FixedString<A>& lhs, const FixedString<B>& rhs) {
    constexpr std::size_t total = (A - 1) + 1 + (B - 1) + 1; // lhs + '/' + rhs + '\0'
    FixedString<total> result{};
    std::size_t pos = 0;
    for (std::size_t i = 0; i < A - 1; ++i) { result.data[pos++] = lhs.data[i]; }
    result.data[pos++] = '/';
    for (std::size_t i = 0; i < B - 1; ++i) { result.data[pos++] = rhs.data[i]; }
    result.data[pos] = '\0';
    return result;
}

} // namespace matcha
