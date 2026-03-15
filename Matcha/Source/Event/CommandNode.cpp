#include "Matcha/Event/CommandNode.h"

#include "Matcha/Event/NotificationQueue.h"

#include <algorithm>

namespace matcha {

MATCHA_IMPLEMENT_CLASS(CommandNode, EventNode)

// --------------------------------------------------------------------------- //
// Construction / Destruction
// --------------------------------------------------------------------------- //

CommandNode::CommandNode(CommandNode* /*parent*/, std::string id, CommandMode mode)
    : _id(std::move(id))
    , _mode(mode) {
    // NOTE: parent parameter is intentionally ignored.
    // With unique_ptr ownership, the parent must call AddChild() to take
    // ownership. Constructing with a parent raw pointer is not safe because
    // the parent would not own this object.
    // Kept in signature for API compatibility; will be removed in future.
}

CommandNode::~CommandNode() {
    DestroyChildren();

    // Detach from parent (parent still owns us via unique_ptr,
    // but if we're being destroyed, parent's erase already happened
    // or parent is also being destroyed).
    _parent = nullptr;
}

void CommandNode::DestroyChildren() {
    // Clear parent back-pointers before destruction to avoid
    // dangling references during child dtors.
    for (auto& child : _children) {
        if (child) {
            child->_parent = nullptr;
        }
    }
    // Destroying children in reverse order mirrors stack semantics
    // and is generally safer for interdependent siblings.
    while (!_children.empty()) {
        _children.pop_back();
    }
}

void CommandNode::DetachSubtreeSubscriptions(CommandNode* subtreeRoot) {
    if (subtreeRoot == nullptr) { return; }
    // Visit every node in the subtree rooted at subtreeRoot and call
    // DetachExternalSubscriptions with the OVERALL subtreeRoot (not the
    // current node). This ensures IsInSubtree checks are relative to the
    // entire detached subtree, preserving intra-subtree subscriptions.
    std::function<void(CommandNode*)> visit = [&](CommandNode* node) {
        for (auto& child : node->_children) {
            visit(child.get());
        }
        node->DetachExternalSubscriptions(subtreeRoot);
    };
    visit(subtreeRoot);
}

// --------------------------------------------------------------------------- //
// Identity
// --------------------------------------------------------------------------- //

void CommandNode::SetId(std::string id) {
    _id = std::move(id);
}

// --------------------------------------------------------------------------- //
// Tree Navigation
// --------------------------------------------------------------------------- //

void CommandNode::SetParent(CommandNode* parent) {
    if (_parent == parent) {
        return;
    }
    // Detach from old parent -- old parent releases ownership
    std::unique_ptr<CommandNode> self;
    if (_parent) {
        self = _parent->RemoveChild(this);
    }
    _parent = parent;
    // Attach to new parent -- new parent takes ownership
    if (_parent) {
        if (self) {
            _parent->AddChild(std::move(self));
        }
    }
    // If no new parent, self goes out of scope and destroys this -- caller beware!
    // In practice, caller should use AddChild/RemoveChild directly.
}

auto CommandNode::AddChild(std::unique_ptr<CommandNode> child) -> CommandNode* {
    if (!child) {
        return nullptr;
    }
    auto* raw = child.get();
    raw->_parent = this;
    _children.push_back(std::move(child));
    OnChildAdded(raw);
    return raw;
}

auto CommandNode::RemoveChild(CommandNode* child) -> std::unique_ptr<CommandNode> {
    auto it = std::ranges::find_if(_children,
        [child](const auto& p) { return p.get() == child; });
    if (it == _children.end()) {
        return nullptr;
    }

    // Layer 3: Before detaching, clean up cross-boundary subscriptions.
    // For every node in the subtree being removed, remove subscriptions
    // whose publisher is outside the subtree (and vice versa).
    DetachSubtreeSubscriptions(child);

    auto owned = std::move(*it);
    _children.erase(it);
    owned->_parent = nullptr;
    OnChildRemoved(owned.get());
    return owned;
}

// --------------------------------------------------------------------------- //
// Notification Protocol
// --------------------------------------------------------------------------- //

void CommandNode::SendNotification(CommandNode* target, Notification& notif) {
    if (!target) {
        return;
    }
    // Deliver to target; propagate up parent chain if requested.
    // At each node:
    //   1. DispatchCallbacks(notif, sender) -- fire matching (publisher, type) callbacks
    //   2. AnalyseNotification(sender, notif) -- virtual handler decision
    // Callbacks are stored on the SUBSCRIBER (current node), filtered by
    // (publisher=sender, notifType). This is 3DEXPERIENCE AddAnalyseNotificationCB semantics.
    auto* sender = this;
    auto* current = target;
    while (current) {
        current->DispatchCallbacks(notif, sender);
        auto result = current->AnalyseNotification(sender, notif);
        if (result == PropagationMode::DontTransmitToParent) {
            break;
        }
        current = current->_parent;
    }
}

void CommandNode::SendNotificationQueued(CommandNode* target,
                                         std::shared_ptr<Notification> notif,
                                         std::weak_ptr<void> guard)
{
    if (!target || !notif) { return; }

    // Stamp the notification with the sender's current state generation
    notif->SetSourceGeneration(_stateGeneration);

    auto* queue = GetNotificationQueue();
    if (queue == nullptr) {
        // No queue wired — fall back to sync dispatch
        SendNotification(target, *notif);
        return;
    }
    queue->Enqueue(this, target, std::move(notif), std::move(guard));
}

auto CommandNode::GetNotificationQueue() const -> NotificationQueue*
{
    // Default: walk up to parent. Shell overrides to return the queue directly.
    if (_parent != nullptr) {
        return _parent->GetNotificationQueue();
    }
    return nullptr;
}

auto CommandNode::AnalyseNotification(CommandNode* /*sender*/,
                                      Notification& /*notif*/) -> PropagationMode {
    // Default: cannot handle, propagate to parent
    return PropagationMode::TransmitToParent;
}

// --------------------------------------------------------------------------- //
// Lifecycle
// --------------------------------------------------------------------------- //

auto CommandNode::Activate(CommandNode* /*sender*/, Notification& /*notif*/) -> StatusChangeRC {
    return StatusChangeRC::Completed;
}

auto CommandNode::Deactivate(CommandNode* /*sender*/, Notification& /*notif*/) -> StatusChangeRC {
    return StatusChangeRC::Completed;
}

auto CommandNode::Cancel(CommandNode* /*sender*/, Notification& /*notif*/) -> StatusChangeRC {
    return StatusChangeRC::Completed;
}

void CommandNode::BeginCommand() {}

void CommandNode::EndCommand() {}

void CommandNode::Reset() {}

void CommandNode::RequestDelayedDestruction() {
    _destructionRequested = true;
}

// --------------------------------------------------------------------------- //
// Tree Hooks
// --------------------------------------------------------------------------- //

void CommandNode::OnChildAdded(CommandNode* /*child*/) {}

void CommandNode::OnChildRemoved(CommandNode* /*child*/) {}

// --------------------------------------------------------------------------- //
// Widget Event Bridge
// --------------------------------------------------------------------------- //

void CommandNode::ForwardToParent(Notification& notif) {
    if (_parent) {
        SendNotification(_parent, notif);
    }
}

} // namespace matcha
