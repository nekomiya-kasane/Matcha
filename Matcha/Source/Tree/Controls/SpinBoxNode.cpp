#include "Matcha/Tree/Controls/SpinBoxNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanSpinBox.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(SpinBoxNode, WidgetNode)

SpinBoxNode::SpinBoxNode(std::string id)
    : WidgetNode(std::move(id), NodeType::SpinBox)
{
}

SpinBoxNode::~SpinBoxNode() = default;

void SpinBoxNode::SetValue(int value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSpinBox*>(_widget)) {
        w->setValue(value);
    }
}

auto SpinBoxNode::Value() const -> int
{
    if (auto* w = qobject_cast<gui::NyanSpinBox*>(_widget)) {
        return w->value();
    }
    return 0;
}

void SpinBoxNode::SetRange(int min, int max)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSpinBox*>(_widget)) {
        w->SetRange(min, max);
    }
}

auto SpinBoxNode::Minimum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanSpinBox*>(_widget)) {
        return w->minimum();
    }
    return 0;
}

auto SpinBoxNode::Maximum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanSpinBox*>(_widget)) {
        return w->maximum();
    }
    return 99;
}

void SpinBoxNode::SetSuffix(std::string_view suffix)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSpinBox*>(_widget)) {
        w->setSuffix(QString::fromUtf8(suffix.data(), static_cast<int>(suffix.size())));
    }
}

auto SpinBoxNode::Suffix() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanSpinBox*>(_widget)) {
        return w->suffix().toStdString();
    }
    return {};
}

auto SpinBoxNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanSpinBox(parent);
    QObject::connect(w, &QSpinBox::valueChanged, w, [this](int value) {
        IntValueChanged notif(value);
        SendNotification(this, notif);
    });
    QObject::connect(w, &QAbstractSpinBox::editingFinished, w, [this]() {
        EditingFinished notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
