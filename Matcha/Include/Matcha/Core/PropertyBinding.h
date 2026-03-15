#pragma once

/**
 * @file PropertyBinding.h
 * @brief RAII binding from Observable<T> to a setter function.
 *
 * Zero Qt dependency. Single-threaded (GUI thread).
 * Part of the Matcha Foundation layer.
 *
 * @par Usage
 * @code
 *   Observable<std::string> title{"Untitled"};
 *   PropertyBinding<std::string> binding(title,
 *       [&label](const std::string& val) { label.SetText(val); });
 *   // label is now synced to title. When binding destructs, observer is removed.
 * @endcode
 */

#include "Matcha/Core/Observable.h"

#include <functional>
#include <memory>
#include <utility>

namespace matcha::fw {

/**
 * @brief RAII binding from Observable<T> to a setter function.
 *
 * On construction, performs an initial sync (calls setter with current value)
 * and registers an observer. On destruction, unregisters the observer.
 *
 * The setter is stored in a shared_ptr so the observer callback (which
 * captures a weak_ptr) remains valid after move operations.
 *
 * Move-only. Not copyable.
 */
template <typename T>
class PropertyBinding {
    using Setter = std::move_only_function<void(const T&)>;

public:
    PropertyBinding() = default;

    PropertyBinding(Observable<T>& source, Setter setter)
        : _source(&source)
        , _setter(std::make_shared<Setter>(std::move(setter)))
    {
        (*_setter)(source.Get());
        std::weak_ptr<Setter> weak = _setter;
        _handle = source.Observe([weak](const T& /*old*/, const T& newVal) {
            if (auto sp = weak.lock()) {
                (*sp)(newVal);
            }
        });
    }

    ~PropertyBinding() {
        Release();
    }

    PropertyBinding(PropertyBinding&& o) noexcept
        : _source(std::exchange(o._source, nullptr))
        , _setter(std::move(o._setter))
        , _handle(std::exchange(o._handle, ObserverHandle{}))
    {}

    auto operator=(PropertyBinding&& o) noexcept -> PropertyBinding& {
        if (this != &o) {
            Release();
            _source = std::exchange(o._source, nullptr);
            _setter = std::move(o._setter);
            _handle = std::exchange(o._handle, ObserverHandle{});
        }
        return *this;
    }

    PropertyBinding(const PropertyBinding&) = delete;
    auto operator=(const PropertyBinding&) -> PropertyBinding& = delete;

    /// @brief Manually unbind before destruction.
    void Release() {
        if (_source) {
            _source->Unobserve(_handle);
            _source = nullptr;
            _setter.reset();
        }
    }

    /// @brief Check if binding is active.
    [[nodiscard]] auto IsActive() const -> bool { return _source != nullptr; }

private:
    Observable<T>* _source = nullptr;
    std::shared_ptr<Setter> _setter;
    ObserverHandle _handle = {};
};

} // namespace matcha::fw
