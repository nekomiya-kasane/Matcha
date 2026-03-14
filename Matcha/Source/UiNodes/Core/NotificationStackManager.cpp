/**
 * @file NotificationStackManager.cpp
 * @brief Implementation of NotificationStackManager.
 */

#include <Matcha/Foundation/NotificationStackManager.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Configuration
// ============================================================================

void NotificationStackManager::SetMaxVisible(int max)
{
    _maxVisible = std::max(1, max);
}

void NotificationStackManager::OnDismissed(DismissCallback cb)
{
    _dismissCallback = std::move(cb);
}

// ============================================================================
// Push / Dismiss
// ============================================================================

auto NotificationStackManager::Push(StackNotification notif) -> NotificationId
{
    // Duplicate suppression by code
    if (!notif.code.empty()) {
        for (auto& existing : _visible) {
            if (existing.code == notif.code) {
                existing.title = std::move(notif.title);
                existing.message = std::move(notif.message);
                existing.duration = notif.duration;
                // Reset timer for this item
                const auto idx = static_cast<std::size_t>(&existing - _visible.data());
                if (idx < _timers.size()) {
                    _timers[idx] = notif.duration;
                }
                return existing.id;
            }
        }
        for (auto& existing : _queued) {
            if (existing.code == notif.code) {
                existing.title = std::move(notif.title);
                existing.message = std::move(notif.message);
                existing.duration = notif.duration;
                return existing.id;
            }
        }
    }

    notif.id = _nextId++;

    const bool isUrgent = (notif.priority == NotificationPriority::Urgent);
    const auto maxVis = static_cast<std::size_t>(_maxVisible);

    if (isUrgent || _visible.size() < maxVis) {
        _visible.push_back(std::move(notif));
        _timers.push_back(_visible.back().duration);
    } else if (notif.priority > NotificationPriority::Normal && !_visible.empty()) {
        // Priority override: check if we can displace a lower-priority visible item
        auto lowest = std::ranges::min_element(_visible, {}, &StackNotification::priority);
        if (lowest != _visible.end() && lowest->priority < notif.priority) {
            const auto lowestIdx = static_cast<std::size_t>(
                std::distance(_visible.begin(), lowest));
            // Move displaced item to queue
            _queued.push_back(std::move(*lowest));
            *lowest = std::move(notif);
            _timers[lowestIdx] = _visible[lowestIdx].duration;
        } else {
            _queued.push_back(std::move(notif));
        }
    } else {
        _queued.push_back(std::move(notif));
    }

    return _nextId - 1;
}

auto NotificationStackManager::Dismiss(NotificationId id) -> bool
{
    // Check visible list
    for (std::size_t i = 0; i < _visible.size(); ++i) {
        if (_visible[i].id == id) {
            const auto dismissedId = _visible[i].id;
            _visible.erase(_visible.begin() + static_cast<std::ptrdiff_t>(i));
            _timers.erase(_timers.begin() + static_cast<std::ptrdiff_t>(i));
            if (_dismissCallback) {
                _dismissCallback(dismissedId);
            }
            PromoteFromQueue();
            return true;
        }
    }

    // Check queued list
    const auto it = std::ranges::find_if(_queued, [id](const StackNotification& n) {
        return n.id == id;
    });
    if (it != _queued.end()) {
        const auto dismissedId = it->id;
        _queued.erase(it);
        if (_dismissCallback) {
            _dismissCallback(dismissedId);
        }
        return true;
    }

    return false;
}

void NotificationStackManager::DismissAll()
{
    if (_dismissCallback) {
        for (const auto& n : _visible) {
            _dismissCallback(n.id);
        }
        for (const auto& n : _queued) {
            _dismissCallback(n.id);
        }
    }
    _visible.clear();
    _queued.clear();
    _timers.clear();
}

// ============================================================================
// Time advancement
// ============================================================================

void NotificationStackManager::Tick(std::chrono::milliseconds dt)
{
    std::vector<NotificationId> expired;

    for (std::size_t i = 0; i < _visible.size(); ++i) {
        if (_timers[i].count() <= 0) {
            continue; // no auto-dismiss (duration=0)
        }
        _timers[i] -= dt;
        if (_timers[i].count() <= 0) {
            expired.push_back(_visible[i].id);
        }
    }

    for (const auto id : expired) {
        Dismiss(id);
    }
}

// ============================================================================
// Query
// ============================================================================

auto NotificationStackManager::VisibleNotifications() const
    -> const std::vector<StackNotification>&
{
    return _visible;
}

auto NotificationStackManager::QueuedNotifications() const
    -> const std::vector<StackNotification>&
{
    return _queued;
}

auto NotificationStackManager::TotalCount() const -> int
{
    return static_cast<int>(_visible.size() + _queued.size());
}

auto NotificationStackManager::VisibleCount() const -> int
{
    return static_cast<int>(_visible.size());
}

auto NotificationStackManager::FindById(NotificationId id) const -> const StackNotification*
{
    for (const auto& n : _visible) {
        if (n.id == id) {
            return &n;
        }
    }
    for (const auto& n : _queued) {
        if (n.id == id) {
            return &n;
        }
    }
    return nullptr;
}

// ============================================================================
// Private
// ============================================================================

void NotificationStackManager::PromoteFromQueue()
{
    const auto maxVis = static_cast<std::size_t>(_maxVisible);
    while (!_queued.empty() && _visible.size() < maxVis) {
        // Promote highest-priority queued item
        auto best = std::ranges::max_element(_queued, {}, &StackNotification::priority);
        _visible.push_back(std::move(*best));
        _timers.push_back(_visible.back().duration);
        _queued.erase(best);
    }
}

void NotificationStackManager::SortByPriority(std::vector<StackNotification>& vec)
{
    std::ranges::stable_sort(vec, [](const StackNotification& a, const StackNotification& b) {
        return a.priority > b.priority;
    });
}

} // namespace matcha::fw
