#pragma once

/**
 * @file CommandNode.h
 * @brief Command tree node -- parent/child hierarchy with notification propagation.
 *
 * Matcha equivalent of CATCommand. Provides:
 *  - Parent-child command tree (notifications propagate up to parent)
 *  - Send/Receive notification protocol
 *  - Lifecycle callbacks: Activate / Deactivate / Cancel
 *  - Command start mode (Shared / Exclusive / Undefined)
 *  - Typed callback registration for notification analysis
 *  - Widget Event Bridge pattern for business-layer wrappers
 *
 * @par Widget Event Bridge -- Business-Layer Wrapper Protocol
 * Matcha is a **pure UI framework** -- it provides widgets, layout, and the
 * command tree transport, but contains **zero business logic**.
 *
 * Business-layer Wrapper classes (e.g., COCALineEdit, COCAMeshPanel) derive
 * from CommandNode and wrap one or more Qt widgets. The Wrapper is responsible
 * for:
 *  1. **Connecting** to all business-relevant Qt signals on the wrapped widget
 *     (e.g., QLineEdit::textChanged, QPushButton::clicked)
 *  2. **Converting** each Qt signal into a typed Notification subclass that
 *     carries the signal payload (e.g., TextChangedNotification{newText})
 *  3. **Dispatching** the Notification via SendNotification() so it propagates
 *     up the command tree to other business modules that have subscribed
 *
 * This architecture ensures:
 *  - The UI framework never imports business headers
 *  - Business modules communicate exclusively via typed Notifications
 *  - A Wrapper is the single point of truth for "what events does this widget
 *    expose to the application layer"
 *  - Replacing a widget implementation requires changing only the Wrapper,
 *    not any consuming business module
 *
 * @par Example (business layer, NOT inside Matcha)
 * @code
 *   class COCALineEdit : public matcha::CommandNode {
 *       MATCHA_DECLARE_CLASS
 *   public:
 *       COCALineEdit(CommandNode* parent, NyanLineEdit* widget)
 *           : CommandNode(parent, "COCALineEdit")
 *           , _widget(widget) {
 *           // Bridge Qt signal -> Notification -> command tree
 *           QObject::connect(_widget, &QLineEdit::textChanged,
 *               [this](const QString& text) {
 *                   TextChangedNotification notif{text.toStdString()};
 *                   SendNotification(Parent(), notif);
 *               });
 *       }
 *   private:
 *       NyanLineEdit* _widget;
 *   };
 * @endcode
 */

#include "Matcha/Event/EventNode.h"

#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace matcha {

// --------------------------------------------------------------------------- //
// Command start mode
// --------------------------------------------------------------------------- //

/**
 * @brief Command start mode -- determines how the command interacts with the
 *        command selector (focus management).
 *
 * Equivalent to CATCommandMode in 3DEXPERIENCE.
 *
 * - Shared:    Command can be deactivated and reactivated (pushed to stack).
 * - Exclusive: Previous active command is deleted when this one starts.
 * - Undefined: Command is invisible to the command selector; runs to completion.
 */
enum class CommandMode : uint8_t {
    Shared    = 1,
    Exclusive = 0,
    Undefined = 3,
};

// --------------------------------------------------------------------------- //
// Notification propagation mode
// --------------------------------------------------------------------------- //

/**
 * @brief Return value from AnalyseNotification -- controls notification propagation.
 *
 * Equivalent to CATNotifPropagationMode.
 */
enum class PropagationMode : uint8_t {
    TransmitToParent     = 0, ///< This node cannot handle the notification; propagate to parent.
    DontTransmitToParent = 1, ///< This node handled the notification; stop propagation.
};

// --------------------------------------------------------------------------- //
// Dispatch mode
// --------------------------------------------------------------------------- //

/**
 * @brief Controls whether a notification is dispatched synchronously or queued.
 *
 * - Sync:   Immediate tree propagation. Supports accept/reject. Default.
 * - Queued: Enqueued in the Application's NotificationQueue, dispatched on
 *           the next Tick(). Does NOT support accept/reject (sender has
 *           already continued). Notification must be heap-allocated
 *           (shared_ptr). Target liveness is checked at flush time via
 *           EventNode::AliveToken().
 */
enum class DispatchMode : uint8_t {
    Sync   = 0,
    Queued = 1,
};

// Forward declaration for queued dispatch
class NotificationQueue;

// --------------------------------------------------------------------------- //
// Command lifecycle status
// --------------------------------------------------------------------------- //

/**
 * @brief Return value from Activate / Deactivate / Cancel lifecycle methods.
 *
 * Equivalent to CATStatusChangeRC.
 */
enum class StatusChangeRC : uint8_t {
    Completed = 0,
    Aborted   = 1,
};

// --------------------------------------------------------------------------- //
// CommandNode
// --------------------------------------------------------------------------- //

/**
 * @brief Command tree node -- the core building block of the Matcha command hierarchy.
 *
 * Equivalent to CATCommand in the 3DEXPERIENCE architecture.
 * All existing commands form a tree structure. Each command has a parent
 * and possibly children. Notifications propagate up the tree until a command
 * handles them (returns DontTransmitToParent from AnalyseNotification).
 *
 * @par Tree Structure
 * The parent is set at construction time (or via SetParent()). Children are
 * tracked as non-owning observer pointers -- the CommandNode does NOT own its
 * children. Ownership is managed externally (e.g., by the application layer
 * using unique_ptr or by the framework's UiNode tree).
 *
 * @par Notification Protocol (Send/Receive)
 * 1. A CommandNode calls SendNotification(target, notif) to send a notification
 * 2. At each node on the propagation path:
 *    a. DispatchCallbacks() fires all EventNode pub/sub subscribers (observers)
 *    b. AnalyseNotification() runs the virtual handler (returns TransmitToParent
 *       or DontTransmitToParent)
 * 3. If TransmitToParent, the notification is forwarded to target's parent
 * 4. This continues until a node handles it or the root is reached
 *
 * @par Callback Registration
 * External code can observe notifications passing through any CommandNode by
 * calling Subscribe() (inherited from EventNode) or the convenience method
 * AddAnalyseNotificationCB<T>(publisher, callback). This avoids the need to
 * subclass CommandNode or insert spy nodes into the parent chain.
 *
 * @par Lifecycle (Command Selector Protocol)
 * The command selector (framework-level) calls:
 *  - Activate()    -- give focus to this command
 *  - Deactivate()  -- temporarily withdraw focus
 *  - Cancel()      -- permanently withdraw focus (command should self-destruct)
 */
class MATCHA_EXPORT CommandNode : public EventNode {
    MATCHA_DECLARE_CLASS
public:
    /**
     * @brief Construct a command node.
     * @param parent  Parent command in the tree (nullptr for root).
     * @param id      Unique identifier for this command.
     * @param mode    Start mode (default: Undefined).
     */
    explicit CommandNode(CommandNode* parent = nullptr,
                         std::string id = {},
                         CommandMode mode = CommandMode::Undefined);

    ~CommandNode() override;

    // Non-copyable, non-movable
    CommandNode(const CommandNode&) = delete;
    auto operator=(const CommandNode&) -> CommandNode& = delete;
    CommandNode(CommandNode&&) = delete;
    auto operator=(CommandNode&&) -> CommandNode& = delete;

    // -- Identity -------------------------------------------------------------

    /** @brief Return the command identifier. Equivalent to CATCommand::GetName(). */
    [[nodiscard]] auto Id() const -> std::string_view { return _id; }

    /** @brief Set the command identifier. Equivalent to CATCommand::SetName(). */
    void SetId(std::string id);

    /** @brief Return the command start mode. */
    [[nodiscard]] auto StartMode() const -> CommandMode { return _mode; }

    // -- Tree Navigation ------------------------------------------------------

    /**
     * @brief Return the parent command.
     *
     * Equivalent to CATCommand::GetFather().
     */
    [[nodiscard]] auto Parent() const -> CommandNode* { return _parent; }

    /**
     * @brief Set the parent command.
     *
     * Removes this node from the old parent's children list and adds to the
     * new parent. Equivalent to CATCommand::SetFather().
     *
     * @param parent New parent (nullptr to detach).
     */
    void SetParent(CommandNode* parent);

    /**
     * @brief Return the list of child commands (read-only view).
     */
    [[nodiscard]] auto Children() const
        -> std::span<const std::unique_ptr<CommandNode>> { return _children; }

    /** @brief Return the number of child commands. */
    [[nodiscard]] auto ChildCount() const -> size_t { return _children.size(); }

    /**
     * @brief Add a child command. Takes ownership.
     * @return Non-owning pointer to the added child.
     */
    auto AddChild(std::unique_ptr<CommandNode> child) -> CommandNode*;

    /**
     * @brief Remove a child command. Returns ownership.
     * @return Ownership of the removed child, or nullptr if not found.
     */
    auto RemoveChild(CommandNode* child) -> std::unique_ptr<CommandNode>;

    // -- Notification Protocol ------------------------------------------------

    /**
     * @brief Send a notification synchronously to another command.
     *
     * The notification is delivered to the target's AnalyseNotification().
     * If the target returns TransmitToParent, the notification propagates
     * up the target's parent chain. Supports accept/reject.
     *
     * Equivalent to CATCommand::SendNotification().
     *
     * @param target  The command to receive the notification.
     * @param notif   The notification to send (must outlive the call).
     */
    void SendNotification(CommandNode* target, Notification& notif);

    /**
     * @brief Enqueue a notification for deferred dispatch on the next Tick().
     *
     * The notification is stored in the Application's NotificationQueue and
     * dispatched during Application::Tick(). The shared_ptr keeps the
     * notification alive until all subscribers have processed it.
     *
     * Accept/reject is NOT supported for queued notifications.
     * If the target node has been destroyed by flush time, the notification
     * is silently discarded. If @p guard is provided and has expired by flush
     * time, the notification is also discarded.
     *
     * @param target  The command to receive the notification.
     * @param notif   Shared ownership of the notification.
     * @param guard   Optional lifetime guard (e.g. an external object's AliveToken).
     */
    void SendNotificationQueued(CommandNode* target,
                                std::shared_ptr<Notification> notif,
                                std::weak_ptr<void> guard = {});

    /**
     * @brief Get the notification queue for queued dispatch.
     *
     * Default implementation walks up the parent chain. Shell overrides
     * this to return the application-level queue directly.
     * Returns nullptr if no queue is wired (queued dispatch falls back to sync).
     */
    [[nodiscard]] virtual auto GetNotificationQueue() const -> NotificationQueue*;

    /**
     * @brief Analyse a notification sent by another command.
     *
     * Override this method to handle specific notification types.
     * Return DontTransmitToParent to stop propagation, or
     * TransmitToParent to forward to the parent.
     *
     * Equivalent to CATCommand::AnalyseNotification().
     *
     * @param sender  The command that sent the notification.
     * @param notif   The notification to analyse.
     * @return Propagation decision.
     */
    [[nodiscard]] virtual auto AnalyseNotification(CommandNode* sender,
                                                   Notification& notif) -> PropagationMode;

    /**
     * @brief Register a typed notification callback on this node (subscriber).
     *
     * Equivalent to CATCommand::AddAnalyseNotificationCB(). The callback is
     * stored on *this* (the subscriber). It fires when a Notification of type T
     * from the specified publisher propagates to this node during SendNotification.
     *
     * @tparam T        The Notification subclass to listen for.
     * @param publisher The expected sender (nullptr = wildcard, any sender).
     * @param callback  Callback receiving the typed notification reference.
     * @return A CallbackId for later Unsubscribe(), or use ScopedSubscription.
     */
    template <typename T>
        requires std::derived_from<T, Notification>
    auto AddAnalyseNotificationCB(CommandNode* publisher,
                                   std::function<void(T&)> callback) -> CallbackId
    {
        return Subscribe(publisher, T{}.ClassName(),
            [cb = std::move(callback)](EventNode& /*sender*/, Notification& notif) {
                cb(*notif.UnsafeAs<T>());
            });
    }

    // -- Lifecycle (Command Selector Protocol) --------------------------------

    /**
     * @brief Activate this command (give focus).
     *
     * Called by the command selector. Override to perform activation logic.
     * Equivalent to CATCommand::Activate().
     *
     * @param sender  The command requesting activation.
     * @param notif   Associated notification (may be empty).
     * @return Lifecycle status.
     */
    virtual auto Activate(CommandNode* sender, Notification& notif) -> StatusChangeRC;

    /**
     * @brief Deactivate this command (temporarily withdraw focus).
     *
     * Called by the command selector. Override to perform deactivation logic.
     * Equivalent to CATCommand::Desactivate().
     *
     * @param sender  The command requesting deactivation.
     * @param notif   Associated notification (may be empty).
     * @return Lifecycle status.
     */
    virtual auto Deactivate(CommandNode* sender, Notification& notif) -> StatusChangeRC;

    /**
     * @brief Cancel this command (permanently withdraw focus).
     *
     * Called by the command selector. The command should clean up and
     * request destruction. Equivalent to CATCommand::Cancel().
     *
     * @param sender  The command requesting cancellation.
     * @param notif   Associated notification (may be empty).
     * @return Lifecycle status.
     */
    virtual auto Cancel(CommandNode* sender, Notification& notif) -> StatusChangeRC;

    // -- Lifecycle Hooks ------------------------------------------------------

    /**
     * @brief Called after construction. Override to initialize data members.
     *
     * Equivalent to CATCommand::BeginCommand().
     */
    virtual void BeginCommand();

    /**
     * @brief Called before destruction. Override to release resources.
     *
     * Equivalent to CATCommand::EndCommand().
     */
    virtual void EndCommand();

    /**
     * @brief Reset the command to its initial state.
     *
     * Equivalent to CATCommand::Reset().
     */
    virtual void Reset();

    // -- State Generation ------------------------------------------------------

    /**
     * @brief Return the current state generation counter.
     *
     * Incremented by IncrementGeneration() whenever the node's logical state
     * changes. Queued notifications are stamped with the sender's generation
     * at enqueue time; subscribers can compare to detect stale events.
     */
    [[nodiscard]] auto Generation() const -> uint64_t { return _stateGeneration; }

    /**
     * @brief Increment the state generation counter.
     *
     * Call this when the node's logical state changes (e.g. mode transition,
     * data update). Any queued notification sent before this call will carry
     * an older generation number, allowing subscribers to discard stale events.
     */
    void IncrementGeneration() { ++_stateGeneration; }

    // -- Delayed Destruction ------------------------------------------------------

    /**
     * @brief Request that this command be destroyed at the next safe point.
     *
     * Equivalent to CATCommand::RequestDelayedDestruction().
     * Marks the command for deferred deletion. The actual deletion timing
     * depends on the framework's event loop integration.
     */
    virtual void RequestDelayedDestruction();

    /** @brief Check if delayed destruction has been requested. */
    [[nodiscard]] auto IsDestructionRequested() const -> bool { return _destructionRequested; }

protected:
    /**
     * @brief Called when a child is added to this command's children list.
     * @param child The child command that was added.
     */
    virtual void OnChildAdded(CommandNode* child);

    /**
     * @brief Called when a child is removed from this command's children list.
     * @param child The child command that was removed.
     */
    virtual void OnChildRemoved(CommandNode* child);

    /**
     * @brief Destroy all children NOW, before this node's destructor body ends.
     *
     * **Invariant**: UiNode subclasses that own a Qt top-level widget (e.g.
     * QMainWindow, frameless QWidget) MUST call DestroyChildren() in their
     * destructor BEFORE deleting the Qt widget. This guarantees that every
     * child UiNode's destructor runs while its associated QWidget is still
     * alive, preventing use-after-free from Qt's parent-child cascade.
     *
     * Typical pattern in a WindowNode-like destructor:
     * @code
     *     MyWindowNode::~MyWindowNode() {
     *         DestroyChildren();   // UiNode tree torn down first
     *         delete _qtWidget;    // Qt widget tree torn down second
     *     }
     * @endcode
     *
     * Safe to call multiple times (second call is a no-op).
     */
    void DestroyChildren();

    // -- Widget Event Bridge (for business-layer wrappers) --------------------

    /**
     * @brief Convenience: convert a Notification and send to parent.
     *
     * Business-layer Wrapper subclasses call this from their Qt signal
     * handlers to forward events up the command tree. Equivalent to:
     *   SendNotification(Parent(), notif);
     * but with a null-parent safety check.
     *
     * @param notif The notification to send to the parent command.
     */
    void ForwardToParent(Notification& notif);

private:
    /// @brief Recursively call DetachExternalSubscriptions on every node in the subtree.
    static void DetachSubtreeSubscriptions(CommandNode* subtreeRoot);

    std::string _id;
    CommandMode _mode;
    CommandNode* _parent = nullptr;
    std::vector<std::unique_ptr<CommandNode>> _children;
    bool _destructionRequested = false;
    uint64_t _stateGeneration = 0;
};

} // namespace matcha
