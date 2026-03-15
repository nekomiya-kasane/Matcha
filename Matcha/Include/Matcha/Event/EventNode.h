#pragma once

/**
 * @file EventNode.h
 * @brief Event publish/subscribe base class.
 *
 * Matcha equivalent of CATEventSubscriber. Provides:
 *  - Typed callback registration with (publisher, notificationType) filtering
 *  - Callback dispatch triggered during SendNotification propagation
 *  - Automatic cleanup on destruction (no dangling callbacks)
 *
 * Subscription model (3DEXPERIENCE-style):
 *  - Callbacks are registered on the SUBSCRIBER (the node that wants to receive)
 *  - Each callback specifies a (publisher, notificationType) pair to filter on
 *  - publisher=nullptr means wildcard (match any sender)
 *  - notificationType="*" means wildcard (match any notification class)
 *  - During SendNotification propagation, at each node the framework calls
 *    DispatchCallbacks(notif, sender) which fires matching callbacks
 *
 * Key differences from CATEventSubscriber:
 *  - std::move_only_function instead of C-style CATSubscriberMethod
 *  - Type-safe subscription via Notification::ClassName() matching
 *  - No CATCallbackManager indirection (callbacks stored inline)
 *  - No COM dependency (no AddRef/Release/QueryInterface)
 *  - Thread model: single-thread only (Qt GUI thread), no locking
 */

#include "Matcha/Event/BaseObject.h"
#include "Matcha/Event/Notification.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace matcha {

// Forward declarations for callback signature
class EventNode;

/**
 * @brief Opaque callback identifier, returned by Subscribe() for later removal.
 *
 * Equivalent to CATCallback in the 3DEXPERIENCE architecture.
 */
enum class CallbackId : uint64_t {};

/**
 * @brief Callback signature for notification handlers.
 *
 * @param sender  The EventNode that dispatched the notification.
 * @param notif   The notification being dispatched.
 *
 * Equivalent to CATSubscriberMethod / CATCommandMethod.
 */
using NotificationCallback = std::move_only_function<void(EventNode& sender, Notification& notif)>;

/**
 * @brief Event publish/subscribe base class.
 *
 * Equivalent to CATEventSubscriber in the 3DEXPERIENCE architecture.
 * Callbacks are stored on the subscriber (this node). Each callback
 * specifies a (publisher, notificationType) filter.
 *
 * @par Subscribing
 * Call Subscribe() on this node (the subscriber), providing an optional
 * publisher pointer and the notification class name. When a notification
 * propagates to this node via SendNotification, callbacks whose
 * (publisher, notificationType) match (sender, notif.ClassName()) fire.
 *
 * @par Lifetime Safety
 * On destruction, an EventNode automatically clears all callbacks stored
 * on it. Use ScopedSubscription for RAII unsubscribe.
 *
 * @par Thread Safety
 * All operations are single-threaded (Qt GUI thread). No internal locking.
 */
class MATCHA_EXPORT EventNode : public BaseObject {
    MATCHA_DECLARE_CLASS
public:
    EventNode();
    ~EventNode() override;

    // Non-copyable, non-movable (identity-bearing object)
    EventNode(const EventNode&) = delete;
    auto operator=(const EventNode&) -> EventNode& = delete;
    EventNode(EventNode&&) = delete;
    auto operator=(EventNode&&) -> EventNode& = delete;

    // -- Subscription (subscriber side) --------------------------------------

    /**
     * @brief Register a callback on this node (subscriber) for a specific
     *        (publisher, notificationType) pair.
     *
     * Equivalent to CATEventSubscriber::AddCallback().
     *
     * @param publisher              The expected sender. nullptr = wildcard (any sender).
     * @param notificationClassName  The ClassName() of the Notification to listen for.
     *                               Use "*" to match ALL notification types.
     * @param callback               The callback to invoke when a matching notification arrives.
     * @return A CallbackId that can be passed to Unsubscribe().
     */
    auto Subscribe(EventNode* publisher,
                   std::string_view notificationClassName,
                   NotificationCallback callback) -> CallbackId;

    /**
     * @brief Remove a specific callback by its ID.
     *
     * Equivalent to CATEventSubscriber::RemoveCallback().
     *
     * @param id The CallbackId returned by Subscribe().
     */
    void Unsubscribe(CallbackId id);

    /**
     * @brief Remove all callbacks registered on this publisher.
     *
     * Equivalent to CATEventSubscriber::RemoveSubscriberCallbacks().
     */
    void UnsubscribeAll();

    /**
     * @brief Return the number of active subscriptions on this publisher.
     */
    [[nodiscard]] auto SubscriptionCount() const -> size_t;

    // -- Detach cleanup -------------------------------------------------------

    /**
     * @brief Remove all callbacks on this node whose publisher is NOT in the
     *        subtree rooted at @p subtreeRoot.
     *
     * Called by the framework when this node is detached from the command tree
     * (via RemoveChild). Ensures no stale cross-tree subscriptions remain.
     *
     * Also removes reverse references (in publishers) that point to this node
     * for those cleaned-up callbacks.
     *
     * @param subtreeRoot The root of the subtree being detached (typically the
     *        node returned by RemoveChild). Only subscriptions to publishers
     *        outside this subtree are removed.
     */
    void DetachExternalSubscriptions(const EventNode* subtreeRoot);

    // -- Dispatch (called by framework during propagation) --------------------

    /**
     * @brief Dispatch callbacks registered on this node that match (sender, notif).
     *
     * Called by the framework (CommandNode::SendNotification) at each node
     * during upward propagation. Fires callbacks whose (publisher, notifType)
     * match the given (sender, notif.ClassName()).
     *
     * @param notif  The notification being propagated.
     * @param sender The original sender (CommandNode that called SendNotification).
     *               nullptr matches only wildcard-publisher callbacks.
     */
    void DispatchCallbacks(Notification& notif, EventNode* sender);

    // -- Lifetime Token (async safety) --

    /**
     * @brief Return a weak reference to this node's lifetime token.
     *
     * Used by the notification queue to verify that a target node is still
     * alive before dispatching a queued notification. The token is a
     * shared_ptr<void> that is destroyed when this EventNode is destroyed,
     * causing all weak_ptr copies to expire.
     */
    [[nodiscard]] auto AliveToken() const -> std::weak_ptr<void> { return _aliveToken; }

private:
    friend class ScopedSubscription; // needs access to AddReverseRef/RemoveReverseRef

    struct CallbackEntry {
        CallbackId id;
        EventNode* publisher;            ///< Expected sender (nullptr = wildcard).
        std::weak_ptr<void> publisherToken; ///< Lifetime token. Expired => treat as wildcard.
        std::string notificationClassName;
        NotificationCallback callback;
    };

    /// @brief Reverse reference: a subscriber that registered a callback filtering on us.
    struct SubscriberRef {
        EventNode* subscriber;
        std::weak_ptr<void> subscriberToken;
        CallbackId callbackId;
    };

    /// @brief Called by Subscribe(): publisher records that subscriber registered a callback on it.
    void AddReverseRef(EventNode* subscriber, CallbackId id);

    /// @brief Called by Unsubscribe(): publisher removes the reverse reference.
    void RemoveReverseRef(EventNode* subscriber, CallbackId id);

    /// @brief Called by ~EventNode(): as publisher, remove all callbacks in subscribers that reference us.
    void CleanupAsPublisher();

    std::vector<CallbackEntry> _callbacks;
    std::vector<SubscriberRef> _subscribedBy; ///< Reverse refs: who subscribed to me as publisher.
    uint64_t _nextCallbackId = 1;
    std::shared_ptr<void> _aliveToken = std::make_shared<int>(0);
};

/**
 * @brief RAII helper that automatically unsubscribes on destruction.
 *
 * The subscription is stored on the subscriber (the node passed as first argument).
 *
 * Usage:
 * @code
 *   ScopedSubscription sub(subscriber, &publisher, "MyNotification",
 *                          [](auto& s, auto& n) { ... });
 * @endcode
 */
class MATCHA_EXPORT ScopedSubscription final {
public:
    ScopedSubscription() = default;

    /**
     * @brief Subscribe on subscriber for (publisher, notifType) pair.
     * @param subscriber The node to register the callback on.
     * @param publisher  Expected sender (nullptr = wildcard).
     * @param notificationClassName Notification type filter ("*" = wildcard).
     * @param callback   Callback to invoke.
     */
    ScopedSubscription(EventNode& subscriber,
                       EventNode* publisher,
                       std::string_view notificationClassName,
                       NotificationCallback callback);

    ~ScopedSubscription();

    // Move-only
    ScopedSubscription(ScopedSubscription&& other) noexcept;
    auto operator=(ScopedSubscription&& other) noexcept -> ScopedSubscription&;
    ScopedSubscription(const ScopedSubscription&) = delete;
    auto operator=(const ScopedSubscription&) -> ScopedSubscription& = delete;

    /** @brief Manually unsubscribe before destruction. */
    void Release();

    /** @brief Check if this subscription is active. */
    [[nodiscard]] auto IsActive() const -> bool { return _subscriber != nullptr; }

private:
    EventNode* _subscriber = nullptr;
    std::weak_ptr<void> _subscriberToken;
    CallbackId _id = {};
};

} // namespace matcha
