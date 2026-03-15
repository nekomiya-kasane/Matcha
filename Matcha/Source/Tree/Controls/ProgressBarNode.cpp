#include "Matcha/Tree/Controls/ProgressBarNode.h"

#include "Matcha/Widgets/Controls/NyanProgressBar.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ProgressBarNode, WidgetNode)

ProgressBarNode::ProgressBarNode(std::string id)
    : WidgetNode(std::move(id), NodeType::ProgressBar)
{
}

ProgressBarNode::~ProgressBarNode() = default;

void ProgressBarNode::SetValue(int value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressBar*>(_widget)) {
        w->setValue(value);
    }
}

auto ProgressBarNode::Value() const -> int
{
    if (auto* w = qobject_cast<gui::NyanProgressBar*>(_widget)) {
        return w->value();
    }
    return 0;
}

void ProgressBarNode::SetRange(int minimum, int maximum)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressBar*>(_widget)) {
        w->setRange(minimum, maximum);
    }
}

auto ProgressBarNode::Minimum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanProgressBar*>(_widget)) {
        return w->minimum();
    }
    return 0;
}

auto ProgressBarNode::Maximum() const -> int
{
    if (auto* w = qobject_cast<gui::NyanProgressBar*>(_widget)) {
        return w->maximum();
    }
    return 100;
}

void ProgressBarNode::SetIndeterminate(bool indeterminate)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanProgressBar*>(_widget)) {
        if (indeterminate) {
            w->setRange(0, 0);
        } else {
            w->setRange(0, 100);
        }
    }
}

auto ProgressBarNode::IsIndeterminate() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanProgressBar*>(_widget)) {
        return w->minimum() == 0 && w->maximum() == 0;
    }
    return false;
}

auto ProgressBarNode::CreateWidget(QWidget* parent) -> QWidget*
{
    return new gui::NyanProgressBar(parent);
}

} // namespace matcha::fw
