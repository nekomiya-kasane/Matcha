#include "Matcha/Tree/Controls/DoubleSpinBoxNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanDoubleSpinBox.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(DoubleSpinBoxNode, WidgetNode)

DoubleSpinBoxNode::DoubleSpinBoxNode(std::string id)
    : WidgetNode(std::move(id), NodeType::DoubleSpinBox)
{
}

DoubleSpinBoxNode::~DoubleSpinBoxNode() = default;

void DoubleSpinBoxNode::SetValue(double value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        w->setValue(value);
    }
}

auto DoubleSpinBoxNode::Value() const -> double
{
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        return w->value();
    }
    return 0.0;
}

void DoubleSpinBoxNode::SetRange(double min, double max)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        w->SetRange(min, max);
    }
}

void DoubleSpinBoxNode::SetStep(double step)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        w->SetStep(step);
    }
}

void DoubleSpinBoxNode::SetPrecision(int decimals)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        w->SetPrecision(decimals);
    }
}

auto DoubleSpinBoxNode::Minimum() const -> double
{
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        return w->minimum();
    }
    return 0.0;
}

auto DoubleSpinBoxNode::Maximum() const -> double
{
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        return w->maximum();
    }
    return 99.99;
}

auto DoubleSpinBoxNode::Step() const -> double
{
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        return w->singleStep();
    }
    return 1.0;
}

auto DoubleSpinBoxNode::Precision() const -> int
{
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        return w->Precision();
    }
    return 2;
}

void DoubleSpinBoxNode::SetSuffix(std::string_view suffix)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        w->setSuffix(QString::fromUtf8(suffix.data(), static_cast<int>(suffix.size())));
    }
}

auto DoubleSpinBoxNode::Suffix() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanDoubleSpinBox*>(_widget)) {
        return w->suffix().toStdString();
    }
    return {};
}

auto DoubleSpinBoxNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanDoubleSpinBox(parent);
    QObject::connect(w, &QDoubleSpinBox::valueChanged, w, [this](double value) {
        DoubleValueChanged notif(value);
        SendNotification(this, notif);
    });
    QObject::connect(w, &QAbstractSpinBox::editingFinished, w, [this]() {
        EditingFinished notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
