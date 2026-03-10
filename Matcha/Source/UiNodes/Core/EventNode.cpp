#include "Matcha/UiNodes/Core/EventNode.h"

#include "Matcha/UiNodes/Core/CommandNode.h"

#include <algorithm>

namespace matcha {

MATCHA_IMPLEMENT_CLASS(EventNode, BaseObject)

// --------------------------------------------------------------------------- //
// EventNode
// --------------------------------------------------------------------------- //

EventNode::EventNode() = default;

EventNode::~EventNode()
{
    // As publisher: proactively remove all callbacks in subscribers
    // that filter on us. This prevents stale publisher pointers.
    CleanupAsPublisher();

    // As subscriber: _callbacks cleared by vector destructor.
    // Also remove our reverse refs from any publishers we subscribed to.
    for (auto& cb : _callbacks) {
        if (cb.publisher != nullptr && !cb.publisherToken.expired()) {
            cb.publisher->RemoveReverseRef(this, cb.id);
        }
    }
}

auto EventNode::Subscribe(EventNode* publisher,
                          std::string_view notificationClassName,
                          NotificationCallback callback) -> CallbackId {
    auto id = CallbackId{_nextCallbackId++};
    _callbacks.push_back(CallbackEntry{
        .id = id,
        .publisher = publisher,
        .publisherToken = publisher ? publisher->AliveToken() : std::weak_ptr<void>{},
        .notificationClassName = std::string(notificationClassName),
        .callback = std::move(callback),
    });
    // Bidirectional: register reverse reference on publisher
    if (publisher != nullptr) {
        publisher->AddReverseRef(this, id);
    }
    return id;
}

void EventNode::Unsubscribe(CallbackId id) {
    // Find the entry first to remove reverse ref from publisher
    for (auto& entry : _callbacks) {
        if (entry.id == id) {
            if (entry.publisher != nullptr && !entry.publisherToken.expired()) {
                entry.publisher->RemoveReverseRef(this, id);
            }
            break;
        }
    }
    std::erase_if(_callbacks, [id](const CallbackEntry& e) {
        return e.id == id;
    });
}

void EventNode::UnsubscribeAll() {
    // Remove reverse refs from all publishers
    for (auto& entry : _callbacks) {
        if (entry.publisher != nullptr && !entry.publisherToken.expired()) {
            entry.publisher->RemoveReverseRef(this, entry.id);
        }
    }
    _callbacks.clear();
}

auto EventNode::SubscriptionCount() const -> size_t {
    return _callbacks.size();
}

void EventNode::DispatchCallbacks(Notification& notif, EventNode* sender) {
    auto className = notif.ClassName();
    // Iterate by index — callbacks may be added/removed during dispatch
    for (size_t i = 0; i < _callbacks.size(); ++i) {
        auto& entry = _callbacks[i];
        // Match publisher: nullptr = wildcard (any sender)
        if (entry.publisher != nullptr) {
            // If publisher was destroyed, the token expires.
            // A dead publisher can never match any sender -> skip.
            if (entry.publisherToken.expired()) {
                continue;
            }
            if (entry.publisher != sender) {
                continue;
            }
        }
        // Match notification type: "*" = wildcard (any type)
        if (entry.notificationClassName != "*" &&
            entry.notificationClassName != className) {
            continue;
        }
        entry.callback(*this, notif);
    }
}

// --------------------------------------------------------------------------- //
// ScopedSubscription
// --------------------------------------------------------------------------- //

ScopedSubscription::ScopedSubscription(EventNode& subscriber,
                                       EventNode* publisher,
                                       std::string_view notificationClassName,
                                       NotificationCallback callback)
    : _subscriber(&subscriber)
    , _subscriberToken(subscriber.AliveToken())
    , _id(subscriber.Subscribe(publisher, notificationClassName, std::move(callback))) {}

ScopedSubscription::~ScopedSubscription() {
    Release();
}

ScopedSubscription::ScopedSubscription(ScopedSubscription&& other) noexcept
    : _subscriber(other._subscriber)
    , _subscriberToken(std::move(other._subscriberToken))
    , _id(other._id) {
    other._subscriber = nullptr;
    other._id = {};
}

auto ScopedSubscription::operator=(ScopedSubscription&& other) noexcept -> ScopedSubscription& {
    if (this != &other) {
        Release();
        _subscriber = other._subscriber;
        _subscriberToken = std::move(other._subscriberToken);
        _id = other._id;
        other._subscriber = nullptr;
        other._id = {};
    }
    return *this;
}

void ScopedSubscription::Release() {
    if (_subscriber) {
        // If subscriber is already destroyed, skip Unsubscribe to avoid
        // use-after-free. The subscriber's destructor already cleared
        // its _callbacks vector.
        if (!_subscriberToken.expired()) {
            _subscriber->Unsubscribe(_id);
        }
        _subscriber = nullptr;
        _subscriberToken.reset();
        _id = {};
    }
}

// --------------------------------------------------------------------------- //
// Detach cleanup
// --------------------------------------------------------------------------- //

namespace {

/// Check whether `candidate` is a descendant of (or equal to) `root`
/// by walking up the CommandNode parent chain. Works for EventNode because
/// CommandNode inherits EventNode and CommandNode::Parent() returns the tree parent.
auto IsInSubtree(const EventNode* candidate, const EventNode* subtreeRoot) -> bool {
    const auto* node = dynamic_cast<const CommandNode*>(candidate);
    const auto* rootCmd = dynamic_cast<const CommandNode*>(subtreeRoot);
    while (node != nullptr) {
        if (node == rootCmd) { return true; }
        node = node->Parent();
    }
    return false;
}

} // anonymous namespace

void EventNode::DetachExternalSubscriptions(const EventNode* subtreeRoot) {
    // Remove callbacks whose publisher is OUTSIDE the detached subtree.
    // Wildcard (nullptr) publishers are kept — they match any sender.
    std::vector<CallbackId> toRemove;
    for (auto& entry : _callbacks) {
        if (entry.publisher == nullptr) { continue; } // wildcard — keep
        if (entry.publisherToken.expired()) {
            toRemove.push_back(entry.id);
            continue;
        }
        if (!IsInSubtree(entry.publisher, subtreeRoot)) {
            toRemove.push_back(entry.id);
        }
    }
    for (auto id : toRemove) {
        Unsubscribe(id); // handles reverse ref cleanup via Layer 2
    }

    // Also clean up reverse refs (_subscribedBy) where the subscriber
    // is OUTSIDE the detached subtree — those subscribers have callbacks
    // filtering on us, but after detach we're unreachable via propagation.
    std::vector<std::pair<EventNode*, CallbackId>> externalSubscribers;
    for (auto& ref : _subscribedBy) {
        if (ref.subscriberToken.expired()) { continue; }
        if (!IsInSubtree(ref.subscriber, subtreeRoot)) {
            externalSubscribers.emplace_back(ref.subscriber, ref.callbackId);
        }
    }
    for (auto& [subscriber, cbId] : externalSubscribers) {
        subscriber->Unsubscribe(cbId); // also removes our reverse ref via Layer 2
    }
}

// --------------------------------------------------------------------------- //
// Bidirectional tracking helpers
// --------------------------------------------------------------------------- //

void EventNode::AddReverseRef(EventNode* subscriber, CallbackId id) {
    _subscribedBy.push_back(SubscriberRef{
        .subscriber = subscriber,
        .subscriberToken = subscriber->AliveToken(),
        .callbackId = id,
    });
}

void EventNode::RemoveReverseRef(EventNode* subscriber, CallbackId id) {
    std::erase_if(_subscribedBy, [subscriber, id](const SubscriberRef& ref) {
        return ref.subscriber == subscriber && ref.callbackId == id;
    });
}

void EventNode::CleanupAsPublisher() {
    for (auto& ref : _subscribedBy) {
        if (ref.subscriberToken.expired()) { continue; }
        // Remove the specific callback from subscriber that references us
        std::erase_if(ref.subscriber->_callbacks, [this, &ref](const CallbackEntry& e) {
            return e.id == ref.callbackId && e.publisher == this;
        });
    }
    _subscribedBy.clear();
}

} // namespace matcha
