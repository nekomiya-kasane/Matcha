#pragma once

/**
 * @file Observable.h
 * @brief Reactive observable value with change notification.
 *
 * Zero Qt dependency. Single-threaded (GUI thread), no locking.
 * Part of the Matcha Foundation layer.
 *
 * @par Usage
 * @code
 *   Observable<std::string> name{"Untitled"};
 *   auto handle = name.Observe([](const std::string& old, const std::string& cur) {
 *       fmt::print("name changed: {} -> {}\n", old, cur);
 *   });
 *   name.Set("My Document"); // fires observer
 *   name.Unobserve(handle);
 * @endcode
 */

#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace matcha::fw {

/// @brief Handle returned by Observable::Observe(), used for Unobserve().
enum class ObserverHandle : uint64_t {};

/**
 * @brief Reactive observable value with change notification.
 *
 * @tparam T Value type. Must support `operator==` for change detection.
 *
 * Thread model: single-thread (GUI thread), no locking.
 * Reentrancy: Set() during observer dispatch is silently ignored (no infinite loop).
 */
template <typename T>
class Observable {
public:
    using Callback = std::move_only_function<void(const T& oldVal, const T& newVal)>;

    explicit Observable(T initial = {}) : _value(std::move(initial)) {}

    /// @brief Get current value.
    [[nodiscard]] auto Get() const -> const T& { return _value; }

    /// @brief Implicit conversion for read access.
    [[nodiscard]] operator const T&() const { return _value; } // NOLINT(google-explicit-constructor)

    /// @brief Set value. Fires observers only if value changed.
    /// Reentrant calls (Set inside observer callback) are silently ignored.
    void Set(T newValue) {
        if (_dispatching) { return; }
        if (_value == newValue) { return; }
        T old = std::exchange(_value, std::move(newValue));
        _dispatching = true;
        for (auto& [id, cb] : _observers) {
            cb(old, _value);
        }
        _dispatching = false;
    }

    /// @brief Observe changes. Returns handle for removal.
    [[nodiscard]] auto Observe(Callback cb) -> ObserverHandle {
        auto id = static_cast<ObserverHandle>(_nextId++);
        _observers.emplace_back(id, std::move(cb));
        return id;
    }

    /// @brief Remove observer by handle.
    void Unobserve(ObserverHandle handle) {
        std::erase_if(_observers, [handle](auto& p) { return p.first == handle; });
    }

    /// @brief Return the number of active observers.
    [[nodiscard]] auto ObserverCount() const -> size_t { return _observers.size(); }

private:
    T _value;
    std::vector<std::pair<ObserverHandle, Callback>> _observers;
    uint64_t _nextId = 1;
    bool _dispatching = false;
};

} // namespace matcha::fw
