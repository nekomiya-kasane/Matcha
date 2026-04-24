#pragma once

/**
 * @file Notification.h
 * @brief Typed notification base class for the command tree communication protocol.
 *
 * Matcha equivalent of CATNotification. Notifications are typed event containers
 * that carry information between CommandNode instances in the command tree.
 *
 * Key differences from CATNotification:
 *  - No COM-style AddRef/Release (RAII via unique_ptr/shared_ptr at call sites)
 *  - RTTI via className() virtual + IsA<T>() template, no CATDeclareClass macro
 *  - No global linked-list auto-delete (caller owns lifetime)
 */

#include "Matcha/Core/Macros.h"
#include <cstdint>
#include <cassert>
#include <string_view>

namespace matcha {

/**
 * @brief Base class for all notifications in the command tree.
 *
 * Notifications convey information from one CommandNode to another via the
 * Send/Receive communication protocol. Derive from this class to create
 * domain-specific notifications carrying typed payloads.
 *
 * @par Lifetime — Dual Ownership Model
 * - **Sync dispatch** (default): Notification is created on the stack and
 *   passed by reference to SendNotification(). The framework does NOT take
 *   ownership. Callers must ensure the notification outlives the dispatch.
 * - **Queued dispatch**: Notification is created as shared_ptr and passed to
 *   SendNotificationQueued(). The queue takes shared ownership; the
 *   notification survives until all subscribers have processed it.
 *
 * All Notification payload fields must be **self-contained value types**.
 * Never store raw pointers or references to transient objects — use StrongId
 * identifiers instead. This ensures safety in both sync and async paths.
 *
 * @par Accept/Reject (sync-only)
 * SetAccepted()/IsAccepted() are meaningful only for sync dispatch (e.g. DnD
 * events where the Qt event must be accepted/ignored immediately). For queued
 * dispatch, accept/reject is a no-op — the sender has already continued.
 */
class MATCHA_EXPORT Notification {
public:
    Notification() = default;
    virtual ~Notification();

    // Non-copyable, movable
    Notification(const Notification&) = delete;
    auto operator=(const Notification&) -> Notification& = delete;
    Notification(Notification&&) noexcept = default;
    auto operator=(Notification&&) noexcept -> Notification& = default;

    /**
     * @brief Return the class name of this notification.
     *
     * Override in derived classes. Used for RTTI filtering in callback dispatch.
     * Equivalent to CATNotification::GetNotificationName().
     */
    [[nodiscard]] virtual auto ClassName() const -> std::string_view {
        return "Notification";
    }

    /**
     * @brief Check if this notification is of a specific derived type.
     * @tparam T The target notification type to check against.
     * @return true if this object is an instance of T or a subclass of T.
     */
    template <typename T>
        requires std::derived_from<T, Notification>
    [[nodiscard]] auto IsA() const -> bool {
        return dynamic_cast<const T*>(this) != nullptr;
    }

    /**
     * @brief Downcast to a specific notification type.
     * @tparam T The target notification type.
     * @return Pointer to T if the cast succeeds, nullptr otherwise.
     */
    template <typename T>
        requires std::derived_from<T, Notification>
    [[nodiscard]] auto As() -> T* {
        return dynamic_cast<T*>(this);
    }

    /** @brief Const overload of As(). */
    template <typename T>
        requires std::derived_from<T, Notification>
    [[nodiscard]] auto As() const -> const T* {
        return dynamic_cast<const T*>(this);
    }

    /**
     * @brief Unchecked downcast — caller MUST guarantee the runtime type.
     *
     * Used by framework internals (e.g. AddAnalyseNotificationCB) where
     * ClassName() filtering has already verified the notification type.
     * In debug builds, a dynamic_cast assertion catches any mismatch.
     *
     * @tparam T The target notification type.
     * @return Pointer to T (never nullptr if precondition holds).
     */
    template <typename T>
        requires std::derived_from<T, Notification>
    [[nodiscard]] auto UnsafeAs() -> T* {
        assert(dynamic_cast<T*>(this) != nullptr && "UnsafeAs: ClassName filter mismatch");
        return static_cast<T*>(this);
    }

    // -- Accept / Reject (sync dispatch only) --

    /** @brief Mark this notification as accepted (e.g. DnD event consumed). */
    void SetAccepted(bool accepted) { _accepted = accepted; }

    /** @brief Check whether a subscriber has accepted this notification. */
    [[nodiscard]] auto IsAccepted() const -> bool { return _accepted; }

    // -- Source Generation (stale-event detection) --

    /**
     * @brief Return the sender's state generation at the time this notification
     *        was created / enqueued.
     *
     * For sync dispatch this is 0 (unused). For queued dispatch the framework
     * stamps this automatically in SendNotificationQueued(). Subscribers can
     * compare with `sender.Generation()` to detect stale events:
     *
     * @code
     *   if (notif.SourceGeneration() != sender.Generation()) {
     *       return; // stale — sender state has changed since enqueue
     *   }
     * @endcode
     */
    [[nodiscard]] auto SourceGeneration() const -> uint64_t { return _sourceGeneration; }

    /** @brief Set the source generation (called by framework). */
    void SetSourceGeneration(uint64_t gen) { _sourceGeneration = gen; }

    // -- Queue Coalescing (reserved for future use) --

    /**
     * @brief Whether this notification can be merged with a subsequent one
     *        of the same type in the notification queue.
     *
     * Override in derived classes to return true if two consecutive instances
     * of the same notification type can be merged (e.g. DragMoved). The queue
     * will keep only the latest instance.
     *
     * @note Not yet implemented in the queue — reserved for future optimization.
     */
    [[nodiscard]] virtual auto CanCoalesce() const -> bool { return false; }

private:
    bool _accepted = false;
    uint64_t _sourceGeneration = 0;
};

} // namespace matcha
