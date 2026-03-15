#include "Matcha/Tree/Controls/SliderNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanSlider.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(SliderNode, WidgetNode)

SliderNode::SliderNode(std::string id)
    : WidgetNode(std::move(id), NodeType::Slider)
{
}

SliderNode::~SliderNode() = default;

void SliderNode::SetValue(int value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSlider*>(_widget)) {
        w->setValue(value);
    }
}

auto SliderNode::Value() const -> int
{
    if (auto* w = qobject_cast<gui::NyanSlider*>(_widget)) {
        return w->value();
    }
    return 0;
}

void SliderNode::SetRange(int minimum, int maximum)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSlider*>(_widget)) {
        w->SetRange(minimum, maximum);
    }
}

auto SliderNode::Minimum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanSlider*>(_widget)) {
        return w->minimum();
    }
    return 0;
}

auto SliderNode::Maximum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanSlider*>(_widget)) {
        return w->maximum();
    }
    return 99;
}

void SliderNode::SetStep(int step)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSlider*>(_widget)) {
        w->SetStep(step);
    }
}

auto SliderNode::Step() const -> int
{
    if (auto* w = qobject_cast<gui::NyanSlider*>(_widget)) {
        return w->singleStep();
    }
    return 1;
}

auto SliderNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanSlider(parent);
    QObject::connect(w, &QSlider::valueChanged, w, [this](int value) {
        IntValueChanged notif(value);
        SendNotification(this, notif);
    });
    QObject::connect(w, &QSlider::sliderPressed, w, [this]() {
        SliderPressed notif;
        SendNotification(this, notif);
    });
    QObject::connect(w, &QSlider::sliderReleased, w, [this]() {
        SliderReleased notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
