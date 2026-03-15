#include "Matcha/Tree/Controls/NotificationNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanNotification.h"

#include <QPoint>
#include <QString>

#include <chrono>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(NotificationNode, WidgetNode)

NotificationNode::NotificationNode(std::string id)
    : WidgetNode(std::move(id), NodeType::Notification)
{
}

NotificationNode::~NotificationNode() = default;

void NotificationNode::SetMessage(std::string_view message)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        w->SetMessage(QString::fromUtf8(message.data(), static_cast<int>(message.size())));
    }
}

auto NotificationNode::Message() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        return w->Message().toStdString();
    }
    return {};
}

void NotificationNode::SetType(uint8_t type)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        w->SetType(static_cast<gui::NotificationType>(type));
    }
}

auto NotificationNode::Type() const -> uint8_t
{
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        return static_cast<uint8_t>(w->Type());
    }
    return 0;
}

void NotificationNode::SetDurationMs(int ms)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        w->SetDuration(std::chrono::milliseconds(ms));
    }
}

auto NotificationNode::DurationMs() const -> int
{
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        return static_cast<int>(w->Duration().count());
    }
    return 5000;
}

void NotificationNode::SetAction(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        w->SetAction(
            QString::fromUtf8(text.data(), static_cast<int>(text.size())),
            [this]() {
                ActionClicked notif;
                SendNotification(this, notif);
            });
    }
}

void NotificationNode::ClearAction()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        w->ClearAction();
    }
}

void NotificationNode::ShowAt(int globalX, int globalY)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        w->ShowAt(QPoint(globalX, globalY));
    }
}

void NotificationNode::Dismiss()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanNotification*>(_widget)) {
        w->Dismiss();
    }
}

auto NotificationNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanNotification(parent);
    QObject::connect(w, &gui::NyanNotification::Dismissed, w,
        [this]() {
            Dismissed notif;
            SendNotification(this, notif);
        });
    return w;
}

} // namespace matcha::fw
