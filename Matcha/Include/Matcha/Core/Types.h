#pragma once

/**
 * @file Types.h
 * @brief Convenience header — re-exports all Foundation types.
 *
 * Includes: observer_ptr<T>, StrongId aliases (DocumentId, ViewportId, WidgetId),
 * ErrorCode, Expected<T>.
 */

#include "ErrorCode.h" // IWYU pragma: export
#include "StringId.h"  // IWYU pragma: export
#include "StrongId.h"  // IWYU pragma: export

#include <memory>
#include <utility>

namespace matcha::fw {

// --------------------------------------------------------------------------- //
// observer_ptr<T> — non-owning smart pointer
// --------------------------------------------------------------------------- //

/**
 * @brief Non-owning smart pointer that explicitly signals "I don't own this".
 *
 * Modeled after std::experimental::observer_ptr (Library Fundamentals TS v2).
 * Use in public APIs where a raw T* would otherwise appear.
 *
 * @tparam T Pointee type.
 */
template <typename T>
class observer_ptr final {
public:
    constexpr observer_ptr() noexcept = default;
    constexpr explicit observer_ptr(std::nullptr_t) noexcept {}

    /** @brief Explicit construction from raw pointer. */
    constexpr explicit observer_ptr(T* p) noexcept : _ptr(p) {}

    /**
     * @brief Implicit construction from std::unique_ptr (convenience).
     * @tparam Deleter The unique_ptr's deleter type.
     * @param u The unique_ptr to observe.
     */
    template <typename Deleter>
    constexpr observer_ptr(const std::unique_ptr<T, Deleter>& u) noexcept // NOLINT(google-explicit-constructor)
        : _ptr(u.get()) {}

    /** @brief Access the raw pointer. */
    [[nodiscard]] constexpr auto get() const noexcept -> T* { return _ptr; }

    /** @brief Dereference. */
    [[nodiscard]] constexpr auto operator*() const noexcept -> T& { return *_ptr; }

    /** @brief Member access. */
    [[nodiscard]] constexpr auto operator->() const noexcept -> T* { return _ptr; }

    /** @brief Boolean conversion — true if non-null. */
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return _ptr != nullptr; }

    /**
     * @brief Reset the observed pointer.
     * @param p New pointer to observe (default nullptr).
     */
    constexpr void reset(T* p = nullptr) noexcept { _ptr = p; }

    /** @brief Swap with another observer_ptr. */
    constexpr void swap(observer_ptr& other) noexcept { std::swap(_ptr, other._ptr); }

    constexpr auto operator<=>(const observer_ptr&) const = default;

private:
    T* _ptr = nullptr;
};

// --------------------------------------------------------------------------- //
// make_observer — factory function
// --------------------------------------------------------------------------- //

/**
 * @brief Create an observer_ptr from a raw pointer.
 * @tparam T Pointee type.
 * @param p Raw pointer to observe.
 * @return An observer_ptr wrapping p.
 */
template <typename T>
[[nodiscard]] constexpr auto make_observer(T* p) noexcept -> observer_ptr<T> {
    return observer_ptr<T>(p);
}

} // namespace matcha::fw
