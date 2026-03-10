#include <Matcha/Widgets/Core/UpdateGuard.h>

namespace matcha::gui {

std::mutex UpdateGuard::_mutex;
std::unordered_map<QWidget*, int> UpdateGuard::_refCounts;

auto UpdateGuard::Create(QWidget* widget) -> UpdateGuard
{
    return UpdateGuard(widget);
}

UpdateGuard::UpdateGuard(QWidget* widget)
    : _widget(widget)
{
    if (_widget == nullptr) {
        return;
    }

    std::lock_guard lock(_mutex);
    auto& count = _refCounts[_widget];
    if (count == 0) {
        _widget->setUpdatesEnabled(false);
    }
    ++count;
}

UpdateGuard::~UpdateGuard()
{
    Release();
}

UpdateGuard::UpdateGuard(UpdateGuard&& other) noexcept
    : _widget(other._widget)
{
    other._widget = nullptr;
}

auto UpdateGuard::operator=(UpdateGuard&& other) noexcept -> UpdateGuard&
{
    if (this != &other) {
        Release();
        _widget = other._widget;
        other._widget = nullptr;
    }
    return *this;
}

void UpdateGuard::Release()
{
    if (_widget == nullptr) {
        return;
    }

    std::lock_guard lock(_mutex);
    auto it = _refCounts.find(_widget);
    if (it != _refCounts.end()) {
        --it->second;
        if (it->second <= 0) {
            _widget->setUpdatesEnabled(true);
            _refCounts.erase(it);
        }
    }
    _widget = nullptr;
}

auto UpdateGuard::IsActive() const noexcept -> bool
{
    return _widget != nullptr;
}

} // namespace matcha::gui
