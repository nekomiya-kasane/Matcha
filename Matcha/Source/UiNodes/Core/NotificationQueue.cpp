#include "Matcha/UiNodes/Core/NotificationQueue.h"

#include "Matcha/UiNodes/Core/CommandNode.h"

namespace matcha {

void NotificationQueue::Enqueue(CommandNode* sender,
                                CommandNode* target,
                                std::shared_ptr<Notification> notif,
                                std::weak_ptr<void> guard)
{
    if (!target || !notif) { return; }
    // A default-constructed weak_ptr has no owner block.
    // A weak_ptr from an (even expired) shared_ptr has an owner block.
    // owner_before detects this: two default-constructed weak_ptrs are equivalent.
    std::weak_ptr<void> empty;
    bool hasGuard = guard.owner_before(empty) || empty.owner_before(guard);
    _entries.push_back(QueueEntry{
        .sender       = sender,
        .target       = target,
        .senderToken  = sender ? sender->AliveToken() : std::weak_ptr<void>{},
        .targetToken  = target->AliveToken(),
        .guardToken   = std::move(guard),
        .hasGuard     = hasGuard,
        .notification = std::move(notif),
    });
}

void NotificationQueue::FlushPending()
{
    // Process entries FIFO. New entries may be added during dispatch
    // (by callbacks). We process them in the same flush pass.
    while (!_entries.empty()) {
        auto entry = std::move(_entries.front());
        _entries.pop_front();

        // Check target liveness
        if (entry.targetToken.expired()) {
            continue;  // target destroyed — discard
        }

        // Check external lifetime guard (if one was provided)
        if (entry.hasGuard && entry.guardToken.expired()) {
            continue;  // guard object destroyed — discard
        }

        // Check sender liveness — if expired, use nullptr (wildcard match)
        CommandNode* sender = nullptr;
        if (!entry.senderToken.expired()) {
            sender = entry.sender;
        }

        // Perform sync-style tree propagation on the target
        auto* current = entry.target;
        while (current) {
            current->DispatchCallbacks(*entry.notification, sender);
            auto result = current->AnalyseNotification(sender, *entry.notification);
            if (result == PropagationMode::DontTransmitToParent) {
                break;
            }
            current = current->Parent();
        }
    }
}

void NotificationQueue::Clear()
{
    _entries.clear();
}

} // namespace matcha
