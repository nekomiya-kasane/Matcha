/**
 * @file ToastStackNode.cpp
 * @brief ToastStackNode bridges NotificationStackManager ↔ NyanNotificationManager.
 */

#include "Matcha/Tree/Controls/ToastStackNode.h"

#include "Matcha/Widgets/Controls/NyanNotification.h"

#include <QString>
#include <QTimer>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ToastStackNode, UiNode)

ToastStackNode::ToastStackNode(std::string id)
    : UiNode(std::move(id), NodeType::Container)
{
    _dataMgr.SetMaxVisible(3);
}

ToastStackNode::~ToastStackNode()
{
    if (_tickTimer != nullptr) {
        _tickTimer->stop();
    }
    delete _mgr;
}

// ============================================================================
// Configuration
// ============================================================================

void ToastStackNode::SetAnchorWidget(QWidget* parent)
{
    _anchor = parent;
}

void ToastStackNode::SetMaxVisible(int max)
{
    _dataMgr.SetMaxVisible(max);
}

// ============================================================================
// Push / Dismiss
// ============================================================================

void ToastStackNode::EnsureManager()
{
    if (_mgr != nullptr || _anchor == nullptr) {
        return;
    }
    _mgr = new gui::NyanNotificationManager(_anchor);

    // Start tick timer to drive NSM auto-dismiss (16ms ≈ 60fps)
    _tickTimer = new QTimer(_anchor);
    QObject::connect(_tickTimer, &QTimer::timeout, _anchor, [this]() {
        _dataMgr.Tick(std::chrono::milliseconds{50});
    });
    _tickTimer->start(50);

    // When NSM dismisses a notification, mirror to NyanNotificationManager
    _dataMgr.OnDismissed([](NotificationId /*id*/) {
        // NyanNotificationManager handles its own dismiss animation.
        // The NSM callback is for bookkeeping only; no extra action needed
        // because the widget-layer dismiss is triggered by the timer/close button.
    });
}

namespace {

auto PriorityToType(NotificationPriority p) -> gui::NotificationType
{
    switch (p) {
    case NotificationPriority::Low:
    case NotificationPriority::Normal: return gui::NotificationType::Info;
    case NotificationPriority::High:   return gui::NotificationType::Warning;
    case NotificationPriority::Urgent: return gui::NotificationType::Error;
    }
    return gui::NotificationType::Info;
}

} // namespace

auto ToastStackNode::Push(StackNotification notif) -> NotificationId
{
    EnsureManager();

    // Push into data layer for bookkeeping
    auto id = _dataMgr.Push(notif);

    // Push into widget layer for actual rendering
    if (_mgr != nullptr) {
        auto displayText = QString::fromStdString(notif.title);
        if (!notif.message.empty()) {
            displayText += ": " + QString::fromStdString(notif.message);
        }

        auto type = PriorityToType(notif.priority);

        if (!notif.actionLabel.empty()) {
            auto cb = notif.actionCallback; // copy for lambda
            _mgr->ShowWithAction(
                displayText,
                QString::fromStdString(notif.actionLabel),
                std::move(cb),
                type,
                notif.duration);
        } else {
            _mgr->Show(displayText, type, notif.duration);
        }
    }

    return id;
}

auto ToastStackNode::PushText(std::string_view message,
                               NotificationPriority priority,
                               std::chrono::milliseconds duration) -> NotificationId
{
    StackNotification notif;
    notif.priority = priority;
    notif.message  = std::string(message);
    notif.duration = duration;
    return Push(std::move(notif));
}

auto ToastStackNode::Dismiss(NotificationId id) -> bool
{
    return _dataMgr.Dismiss(id);
}

void ToastStackNode::DismissAll()
{
    _dataMgr.DismissAll();
    if (_mgr != nullptr) {
        _mgr->DismissAll();
    }
}

// ============================================================================
// Query
// ============================================================================

auto ToastStackNode::VisibleCount() const -> int
{
    if (_mgr != nullptr) {
        return _mgr->VisibleCount();
    }
    return _dataMgr.VisibleCount();
}

auto ToastStackNode::TotalCount() const -> int
{
    return _dataMgr.TotalCount();
}

} // namespace matcha::fw
