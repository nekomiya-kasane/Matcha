#include "Matcha/UiNodes/Controls/MessageNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanMessage.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(MessageNode, WidgetNode)

MessageNode::MessageNode(std::string id)
    : WidgetNode(std::move(id), NodeType::Message)
{
}

MessageNode::~MessageNode() = default;

void MessageNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanMessage*>(_widget)) {
        w->SetText(text);
    }
}

auto MessageNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanMessage*>(_widget)) {
        return w->Text();
    }
    return {};
}

void MessageNode::SetType(uint8_t type)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanMessage*>(_widget)) {
        w->SetType(static_cast<gui::MessageType>(type));
    }
}

auto MessageNode::Type() const -> uint8_t
{
    if (auto* w = qobject_cast<gui::NyanMessage*>(_widget)) {
        return static_cast<uint8_t>(w->Type());
    }
    return 0;
}

void MessageNode::SetClosable(bool closable)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanMessage*>(_widget)) {
        w->SetClosable(closable);
    }
}

auto MessageNode::IsClosable() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanMessage*>(_widget)) {
        return w->IsClosable();
    }
    return false;
}

void MessageNode::SetAction(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanMessage*>(_widget)) {
        w->SetAction(text, [this]() {
            ActionClicked notif;
            SendNotification(this, notif);
        });
    }
}

auto MessageNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanMessage(parent);
    QObject::connect(w, &gui::NyanMessage::Closed, w, [this]() {
        CloseRequested notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
