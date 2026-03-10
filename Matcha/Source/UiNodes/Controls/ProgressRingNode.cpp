#include "Matcha/UiNodes/Controls/ProgressRingNode.h"

#include "Matcha/Widgets/Controls/NyanProgressRing.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ProgressRingNode, WidgetNode)

ProgressRingNode::ProgressRingNode(std::string id)
    : WidgetNode(std::move(id), NodeType::ProgressRing)
{
}

ProgressRingNode::~ProgressRingNode() = default;

void ProgressRingNode::SetValue(int value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        w->SetValue(value);
    }
}

auto ProgressRingNode::Value() const -> int
{
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        return w->Value();
    }
    return 0;
}

void ProgressRingNode::SetRange(int minimum, int maximum)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        w->SetRange(minimum, maximum);
    }
}

void ProgressRingNode::SetIndeterminate(bool indeterminate)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        w->SetIndeterminate(indeterminate);
    }
}

auto ProgressRingNode::IsIndeterminate() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        return w->IsIndeterminate();
    }
    return false;
}

void ProgressRingNode::SetThickness(int thickness)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        w->SetThickness(thickness);
    }
}

auto ProgressRingNode::Thickness() const -> int
{
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        return w->Thickness();
    }
    return 4;
}

void ProgressRingNode::SetTextVisible(bool visible)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        w->SetTextVisible(visible);
    }
}

auto ProgressRingNode::IsTextVisible() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanProgressRing*>(_widget)) {
        return w->IsTextVisible();
    }
    return true;
}

auto ProgressRingNode::CreateWidget(QWidget* parent) -> QWidget*
{
    return new gui::NyanProgressRing(parent);
}

} // namespace matcha::fw
