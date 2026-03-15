#include "Matcha/Tree/Controls/ToggleSwitchNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanToggleSwitch.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ToggleSwitchNode, WidgetNode)

ToggleSwitchNode::ToggleSwitchNode(std::string id)
    : WidgetNode(std::move(id), NodeType::ToggleSwitch)
{
}

ToggleSwitchNode::~ToggleSwitchNode() = default;

void ToggleSwitchNode::SetChecked(bool checked)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanToggleSwitch*>(_widget)) {
        w->SetChecked(checked);
    }
}

auto ToggleSwitchNode::IsChecked() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanToggleSwitch*>(_widget)) {
        return w->IsChecked();
    }
    return false;
}

auto ToggleSwitchNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanToggleSwitch(parent);
    QObject::connect(w, &gui::NyanToggleSwitch::Toggled, w, [this](bool checked) {
        Toggled notif(checked);
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
