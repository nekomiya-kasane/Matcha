#include "Matcha/Tree/Controls/BadgeNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanBadge.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(BadgeNode, WidgetNode)

BadgeNode::BadgeNode(std::string id)
    : WidgetNode(std::move(id), NodeType::Badge)
{
}

BadgeNode::~BadgeNode() = default;

void BadgeNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanBadge*>(_widget)) {
        w->SetText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto BadgeNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanBadge*>(_widget)) {
        return w->Text().toStdString();
    }
    return {};
}

void BadgeNode::SetVariant(uint8_t variant)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanBadge*>(_widget)) {
        w->SetVariant(static_cast<gui::BadgeVariant>(variant));
    }
}

auto BadgeNode::Variant() const -> uint8_t
{
    if (auto* w = qobject_cast<gui::NyanBadge*>(_widget)) {
        return static_cast<uint8_t>(w->Variant());
    }
    return 0;
}

void BadgeNode::SetClosable(bool closable)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanBadge*>(_widget)) {
        w->SetClosable(closable);
    }
}

auto BadgeNode::IsClosable() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanBadge*>(_widget)) {
        return w->IsClosable();
    }
    return false;
}

auto BadgeNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanBadge(parent);
    QObject::connect(w, &gui::NyanBadge::Closed, w, [this]() {
        CloseRequested notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
