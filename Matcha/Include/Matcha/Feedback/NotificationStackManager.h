#pragma once

/**
 * @file NotificationStackManager.h
 * @brief Notification stack manager: FIFO queue, max visible, priority override.
 *
 * Manages a stack of toast/notification items with:
 * - FIFO ordering for same-priority items
 * - Priority override: higher priority notifications jump the queue
 * - Max visible limit with overflow queue
 * - Auto-dismiss tracking via duration per notification
 * - Duplicate suppression by notification code
 *
 * This is a Foundation-layer component with zero Qt dependency.
 * Timer ticking is driven externally via Tick(dt).
 *
 * @see Matcha_Design_System_Specification.md §7.15 (Error Boundary toast integration)
 */

#include <Matcha/Core/Macros.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace matcha::fw {

// ============================================================================
// NotificationPriority
// ============================================================================

/**
 * @enum NotificationPriority
 * @brief Priority levels for notification stack ordering.
 *
 * Higher priority notifications displace lower ones when max visible is reached.
 */
enum class NotificationPriority : uint8_t {
    Low    = 0,
    Normal = 1,
    High   = 2,
    Urgent = 3,   ///< Always visible, bypasses max visible limit
};

// ============================================================================
// StackNotification
// ============================================================================

using NotificationId = uint64_t;

/**
 * @struct StackNotification
 * @brief A single notification in the stack.
 */
struct StackNotification {
    NotificationId       id       = 0;
    NotificationPriority priority = NotificationPriority::Normal;
    std::string          code;             ///< For duplicate suppression
    std::string          title;
    std::string          message;
    std::chrono::milliseconds duration{5000}; ///< 0 = no auto-dismiss
    std::string          actionLabel;      ///< Optional action button text
    std::function<void()> actionCallback;  ///< Optional action callback
};

// ============================================================================
// NotificationStackManager
// ============================================================================

/**
 * @class NotificationStackManager
 * @brief Manages a visible notification stack with overflow queue.
 *
 * Usage:
 * @code
 *   NotificationStackManager mgr;
 *   mgr.SetMaxVisible(3);
 *
 *   auto id = mgr.Push({
 *       .priority = NotificationPriority::Normal,
 *       .title = "Saved",
 *       .message = "File saved successfully.",
 *       .duration = std::chrono::milliseconds{5000},
 *   });
 *
 *   // In frame loop:
 *   mgr.Tick(std::chrono::milliseconds{16});
 *
 *   // Query visible notifications for rendering:
 *   for (const auto& n : mgr.VisibleNotifications()) { ... }
 * @endcode
 */
class MATCHA_EXPORT NotificationStackManager {
public:
    using DismissCallback = std::function<void(NotificationId)>;

    NotificationStackManager() = default;

    // ====================================================================
    // Configuration
    // ====================================================================

    void SetMaxVisible(int max);
    [[nodiscard]] auto MaxVisible() const -> int { return _maxVisible; }

    /**
     * @brief Register callback fired when a notification is dismissed.
     */
    void OnDismissed(DismissCallback cb);

    // ====================================================================
    // Push / Dismiss
    // ====================================================================

    /**
     * @brief Push a notification. Returns its assigned ID.
     *
     * If a notification with the same non-empty code already exists,
     * the existing one is updated instead of adding a duplicate.
     */
    auto Push(StackNotification notif) -> NotificationId;

    /**
     * @brief Manually dismiss a notification by ID.
     */
    auto Dismiss(NotificationId id) -> bool;

    /**
     * @brief Dismiss all notifications.
     */
    void DismissAll();

    // ====================================================================
    // Time advancement
    // ====================================================================

    /**
     * @brief Advance auto-dismiss timers.
     * @param dt Elapsed time since last tick.
     */
    void Tick(std::chrono::milliseconds dt);

    // ====================================================================
    // Query
    // ====================================================================

    /**
     * @brief Currently visible notifications (ordered, most recent first).
     */
    [[nodiscard]] auto VisibleNotifications() const
        -> const std::vector<StackNotification>&;

    /**
     * @brief Queued (overflow) notifications not yet visible.
     */
    [[nodiscard]] auto QueuedNotifications() const
        -> const std::vector<StackNotification>&;

    /**
     * @brief Total count (visible + queued).
     */
    [[nodiscard]] auto TotalCount() const -> int;

    /**
     * @brief Count of visible notifications.
     */
    [[nodiscard]] auto VisibleCount() const -> int;

    /**
     * @brief Find a notification by ID (visible or queued).
     */
    [[nodiscard]] auto FindById(NotificationId id) const -> const StackNotification*;

private:
    void PromoteFromQueue();
    static void SortByPriority(std::vector<StackNotification>& vec);

    int                           _maxVisible = 5;
    NotificationId                _nextId     = 1;
    std::vector<StackNotification> _visible;
    std::vector<StackNotification> _queued;
    std::vector<std::chrono::milliseconds> _timers; ///< Remaining time per visible item
    DismissCallback               _dismissCallback;
};

} // namespace matcha::fw
