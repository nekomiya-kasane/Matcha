#pragma once

/**
 * @file ToastStackNode.h
 * @brief UiNode that bridges NotificationStackManager (data) ↔ NyanNotificationManager (widget).
 *
 * Owns a NyanNotificationManager attached to a parent widget (typically the main window).
 * Provides a Push() API that accepts StackNotification from the Foundation layer,
 * converts it to NyanNotification calls, and renders visible toasts.
 *
 * The Push() API intentionally mirrors NotificationStackManager for easy integration,
 * but internally delegates to NyanNotificationManager for actual widget rendering.
 */

#include "Matcha/Tree/UiNode.h"
#include "Matcha/Feedback/NotificationStackManager.h"

#include <chrono>
#include <string>
#include <string_view>

class QWidget;
class QTimer;

namespace matcha::gui {
class NyanNotificationManager;
} // namespace matcha::gui

namespace matcha::fw {

/**
 * @brief UiNode wrapper that manages a visual toast notification stack.
 *
 * Connects the Foundation-layer NotificationStackManager data model
 * to the Widget-layer NyanNotificationManager rendering system.
 */
class MATCHA_EXPORT ToastStackNode : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    explicit ToastStackNode(std::string id);
    ~ToastStackNode() override;

    ToastStackNode(const ToastStackNode&) = delete;
    auto operator=(const ToastStackNode&) -> ToastStackNode& = delete;
    ToastStackNode(ToastStackNode&&) = delete;
    auto operator=(ToastStackNode&&) -> ToastStackNode& = delete;

    // ====================================================================
    // Configuration
    // ====================================================================

    /// @brief Set the parent widget where toast popups will be anchored.
    /// Must be called before Push(). Typically the main window widget.
    void SetAnchorWidget(QWidget* parent);

    /// @brief Set maximum visible notifications (default 3).
    void SetMaxVisible(int max);

    // ====================================================================
    // Push / Dismiss
    // ====================================================================

    /// @brief Push a toast using StackNotification from NSM.
    auto Push(StackNotification notif) -> NotificationId;

    /// @brief Push a simple text toast.
    auto PushText(std::string_view message,
                  NotificationPriority priority = NotificationPriority::Normal,
                  std::chrono::milliseconds duration = std::chrono::milliseconds{5000})
        -> NotificationId;

    /// @brief Dismiss a specific toast by ID.
    auto Dismiss(NotificationId id) -> bool;

    /// @brief Dismiss all visible and queued toasts.
    void DismissAll();

    // ====================================================================
    // Query
    // ====================================================================

    [[nodiscard]] auto VisibleCount() const -> int;
    [[nodiscard]] auto TotalCount() const -> int;

private:
    void EnsureManager();

    QWidget*                        _anchor  = nullptr;
    gui::NyanNotificationManager*   _mgr     = nullptr;
    NotificationStackManager        _dataMgr;
    QTimer*                         _tickTimer = nullptr;
};

} // namespace matcha::fw
