#pragma once

#include <mutex>
#include <unordered_map>

#include <QWidget>

namespace matcha::gui {

/// RAII guard that disables widget updates during batch operations.
/// Nestable: inner guards on the same widget are no-ops.
/// Move-only.
///
/// Usage:
/// ```cpp
/// {
///     auto guard = UpdateGuard::Create(widget);
///     // ... batch modifications ...
/// } // updates re-enabled here
/// ```
class UpdateGuard final {
public:
    /// Create a guard for the given widget.
    /// Returns a guard that will re-enable updates on destruction.
    [[nodiscard]] static auto Create(QWidget* widget) -> UpdateGuard;

    ~UpdateGuard();

    // Move-only
    UpdateGuard(UpdateGuard&& other) noexcept;
    auto operator=(UpdateGuard&& other) noexcept -> UpdateGuard&;

    UpdateGuard(const UpdateGuard&) = delete;
    auto operator=(const UpdateGuard&) -> UpdateGuard& = delete;

    /// Release the guard early, re-enabling updates if this is the last guard.
    void Release();

    /// Check if this guard is still active.
    [[nodiscard]] auto IsActive() const noexcept -> bool;

private:
    explicit UpdateGuard(QWidget* widget);

    QWidget* _widget = nullptr;

    // Static reference counting for nesting support
    static std::mutex _mutex;
    static std::unordered_map<QWidget*, int> _refCounts;
};

} // namespace matcha::gui
