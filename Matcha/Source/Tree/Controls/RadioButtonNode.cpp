#include "Matcha/Tree/Controls/RadioButtonNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanRadioButton.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(RadioButtonNode, WidgetNode)

RadioButtonNode::RadioButtonNode(std::string id)
    : WidgetNode(std::move(id), NodeType::RadioButton)
{
}

RadioButtonNode::~RadioButtonNode() = default;

void RadioButtonNode::SetChecked(bool checked)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanRadioButton*>(_widget)) {
        w->setChecked(checked);
    }
}

auto RadioButtonNode::IsChecked() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanRadioButton*>(_widget)) {
        return w->isChecked();
    }
    return false;
}

void RadioButtonNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanRadioButton*>(_widget)) {
        w->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto RadioButtonNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanRadioButton*>(_widget)) {
        return w->text().toStdString();
    }
    return {};
}

auto RadioButtonNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanRadioButton(parent);
    QObject::connect(w, &QRadioButton::toggled, w, [this](bool checked) {
        Toggled notif(checked);
        SendNotification(this, notif);
    });
    QObject::connect(w, &QRadioButton::clicked, w, [this]() {
        Clicked notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
