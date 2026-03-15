#pragma once

/**
 * @file NotificationQueue.h
 * @brief Deferred notification dispatch queue.
 *
 * Owned by Application. Stores queued notifications (shared_ptr) and
 * dispatches them during Application::Tick() via FlushPending().
 *
 * Each queued entry stores:
 *  - shared_ptr<Notification>: keeps the notification alive
 *  - weak_ptr<void>: target node's alive token (expires when node is destroyed)
 *  - CommandNode* target: raw pointer, only dereferenced after token check
 *  - CommandNode* sender: raw pointer stored for DispatchCallbacks sender matching
 *  - weak_ptr<void>: sender node's alive token
 *
 * At flush time, if the target's token has expired, the entry is silently
 * discarded. If the sender's token has expired, the sender pointer is set
 * to nullptr (wildcard) for callback matching.
 */

#include "Matcha/Core/Macros.h"

#include <deque>
#include <memory>

namespace matcha {

class CommandNode;
class Notification;

/**
 * @brief Deferred notification dispatch queue.
 *
 * Thread model: single-threaded (GUI thread only). No locking.
 */
class MATCHA_EXPORT NotificationQueue {
public:
    NotificationQueue() = default;
    ~NotificationQueue() = default;

    NotificationQueue(const NotificationQueue&) = delete;
    auto operator=(const NotificationQueue&) -> NotificationQueue& = delete;
    NotificationQueue(NotificationQueue&&) = delete;
    auto operator=(NotificationQueue&&) -> NotificationQueue& = delete;

    /**
     * @brief Enqueue a notification for deferred dispatch.
     *
     * @param sender  The command that is sending the notification.
     * @param target  The command to receive the notification.
     * @param notif   Shared ownership of the notification.
     * @param guard   Optional lifetime guard. If non-empty and expired at
     *                flush time, the entry is silently discarded. Use this
     *                to bind the notification to an external object's lifetime
     *                (e.g. a subscription owner or a lambda capture target).
     */
    void Enqueue(CommandNode* sender,
                 CommandNode* target,
                 std::shared_ptr<Notification> notif,
                 std::weak_ptr<void> guard = {});

    /**
     * @brief Dispatch all pending notifications.
     *
     * For each entry:
     *  1. Check target alive token — if expired, discard.
     *  2. Check sender alive token — if expired, use nullptr as sender.
     *  3. Perform sync-style tree propagation (DispatchCallbacks + AnalyseNotification).
     *
     * Called by Application::Tick().
     *
     * @note New notifications may be enqueued during flush (by callbacks).
     *       They will be dispatched in the same flush pass (breadth-first).
     */
    void FlushPending();

    /** @brief Return the number of pending notifications. */
    [[nodiscard]] auto PendingCount() const -> size_t { return _entries.size(); }

    /** @brief Discard all pending notifications without dispatching. */
    void Clear();

private:
    struct QueueEntry {
        CommandNode* sender;
        CommandNode* target;
        std::weak_ptr<void> senderToken;
        std::weak_ptr<void> targetToken;
        std::weak_ptr<void> guardToken;  ///< Optional external lifetime guard.
        bool hasGuard = false;           ///< True if guardToken was explicitly set.
        std::shared_ptr<Notification> notification;
    };

    std::deque<QueueEntry> _entries;
};

} // namespace matcha
