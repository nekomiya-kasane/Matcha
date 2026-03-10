#include "Matcha/UiNodes/Controls/CheckBoxNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanCheckBox.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(CheckBoxNode, WidgetNode)

CheckBoxNode::CheckBoxNode(std::string id)
    : WidgetNode(std::move(id), NodeType::CheckBox)
{
}

CheckBoxNode::~CheckBoxNode() = default;

void CheckBoxNode::SetChecked(bool checked)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanCheckBox*>(_widget)) {
        w->setChecked(checked);
    }
}

auto CheckBoxNode::IsChecked() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanCheckBox*>(_widget)) {
        return w->isChecked();
    }
    return false;
}

void CheckBoxNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanCheckBox*>(_widget)) {
        w->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto CheckBoxNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanCheckBox*>(_widget)) {
        return w->text().toStdString();
    }
    return {};
}

auto CheckBoxNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanCheckBox(parent);
    QObject::connect(w, &QCheckBox::toggled, w, [this](bool checked) {
        Toggled notif(checked);
        SendNotification(this, notif);
    });
    QObject::connect(w, &QCheckBox::clicked, w, [this]() {
        Clicked notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
