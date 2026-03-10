#include "Matcha/UiNodes/Controls/LabelNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanLabel.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(LabelNode, WidgetNode)

LabelNode::LabelNode(std::string id)
    : WidgetNode(std::move(id), NodeType::Label)
{
}

LabelNode::~LabelNode() = default;

void LabelNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLabel*>(_widget)) {
        w->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto LabelNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanLabel*>(_widget)) {
        return w->text().toStdString();
    }
    return {};
}

void LabelNode::SetRole(gui::LabelRole role)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLabel*>(_widget)) {
        w->SetRole(role);
    }
}

auto LabelNode::Role() const -> gui::LabelRole
{
    if (auto* w = qobject_cast<gui::NyanLabel*>(_widget)) {
        return w->Role();
    }
    return gui::LabelRole::Body;
}

auto LabelNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanLabel(parent);
    QObject::connect(w, &QLabel::linkActivated, w, [this](const QString& link) {
        LinkActivated notif(link.toStdString());
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
