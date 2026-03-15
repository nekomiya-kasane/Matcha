#include "Matcha/Tree/Controls/LineNode.h"

#include "Matcha/Widgets/Controls/NyanLine.h"
#include "Matcha/Theming/DesignTokens.h"

#include <Qt>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(LineNode, WidgetNode)

LineNode::LineNode(std::string id)
    : WidgetNode(std::move(id), NodeType::Line)
{
}

LineNode::~LineNode() = default;

void LineNode::SetOrientation(fw::Orientation orientation)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLine*>(_widget)) {
        const auto qtOri = (orientation == fw::Orientation::Vertical)
                               ? Qt::Vertical
                               : Qt::Horizontal;
        w->SetOrientation(qtOri);
    }
}

auto LineNode::Orientation() const -> fw::Orientation
{
    if (auto* w = qobject_cast<gui::NyanLine*>(_widget)) {
        return (w->Orientation() == Qt::Vertical)
                   ? fw::Orientation::Vertical
                   : fw::Orientation::Horizontal;
    }
    return fw::Orientation::Horizontal;
}

void LineNode::SetColorToken(uint16_t token)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLine*>(_widget)) {
        w->SetColor(static_cast<gui::ColorToken>(token));
    }
}

auto LineNode::ColorToken() const -> uint16_t
{
    if (auto* w = qobject_cast<gui::NyanLine*>(_widget)) {
        return static_cast<uint16_t>(w->Color());
    }
    return static_cast<uint16_t>(gui::ColorToken::Separator);
}

auto LineNode::CreateWidget(QWidget* parent) -> QWidget*
{
    return new gui::NyanLine(parent);
}

} // namespace matcha::fw
