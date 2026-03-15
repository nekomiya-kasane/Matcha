#include "Matcha/Tree/Controls/RangeSliderNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanRangeSlider.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(RangeSliderNode, WidgetNode)

RangeSliderNode::RangeSliderNode(std::string id)
    : WidgetNode(std::move(id), NodeType::RangeSlider)
{
}

RangeSliderNode::~RangeSliderNode() = default;

void RangeSliderNode::SetRange(int minimum, int maximum)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        w->SetRange(minimum, maximum);
    }
}

void RangeSliderNode::SetLow(int value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        w->SetLow(value);
    }
}

auto RangeSliderNode::Low() const -> int
{
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        return w->Low();
    }
    return 0;
}

void RangeSliderNode::SetHigh(int value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        w->SetHigh(value);
    }
}

auto RangeSliderNode::High() const -> int
{
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        return w->High();
    }
    return 100;
}

auto RangeSliderNode::Minimum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        return w->Minimum();
    }
    return 0;
}

auto RangeSliderNode::Maximum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        return w->Maximum();
    }
    return 100;
}

void RangeSliderNode::SetStep(int step)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        w->SetStep(step);
    }
}

auto RangeSliderNode::Step() const -> int
{
    if (auto* w = qobject_cast<gui::NyanRangeSlider*>(_widget)) {
        return w->Step();
    }
    return 1;
}

auto RangeSliderNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanRangeSlider(parent);
    QObject::connect(w, &gui::NyanRangeSlider::RangeChanged, w,
        [this](int low, int high) {
            RangeChanged notif(low, high);
            SendNotification(this, notif);
        });
    return w;
}

} // namespace matcha::fw
