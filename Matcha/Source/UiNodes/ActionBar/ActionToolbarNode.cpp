#include "Matcha/UiNodes/ActionBar/ActionToolbarNode.h"

#include "Matcha/UiNodes/ActionBar/ActionButtonNode.h"
#include "Matcha/Widgets/ActionBar/NyanActionToolbar.h"
#include "Matcha/Widgets/Controls/NyanToolButton.h"

#include <QString>
#include <QWidget>

#include <cassert>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ActionToolbarNode, UiNode)

ActionToolbarNode::ActionToolbarNode(std::string id, UiNode* parentHint)
    : UiNode(std::move(id), NodeType::ActionToolbar)
    , _toolbar(new gui::NyanActionToolbar(parentHint ? parentHint->Widget() : nullptr))
{
}

ActionToolbarNode::~ActionToolbarNode() = default;

auto ActionToolbarNode::Toolbar() -> gui::NyanActionToolbar*
{
    return _toolbar;
}

auto ActionToolbarNode::Widget() -> QWidget*
{
    return _toolbar;
}

auto ActionToolbarNode::ButtonCount() const -> int
{
    return _toolbar->ButtonCount();
}

auto ActionToolbarNode::Index() const -> int
{
    auto* parent = ParentNode();
    if (parent == nullptr) {
        return -1;
    }
    for (std::size_t i = 0; i < parent->ChildCount(); ++i) {
        if (parent->NodeAt(i) == this) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

auto ActionToolbarNode::AddButton(std::string id, std::string_view icon,
                                  std::string_view tooltip) -> ActionButtonNode*
{
    gui::ActionButtonInfo info;
    info.id = QString::fromUtf8(id.data(), static_cast<int>(id.size()));
    info.text = QString::fromUtf8(tooltip.data(), static_cast<int>(tooltip.size()));
    info.tooltip = info.text;
    if (!icon.empty()) {
        info.icon = QIcon(QString::fromUtf8(icon.data(), static_cast<int>(icon.size())));
    }

    int idx = _toolbar->AddButton(info);
    auto* btn = _toolbar->ButtonAt(idx);

    // Use UiNode::AddNode directly -- widget already added above, skip override
    auto buttonNode = std::make_unique<ActionButtonNode>(std::move(id), btn);
    return static_cast<ActionButtonNode*>(UiNode::AddNode(std::move(buttonNode)));
}

void ActionToolbarNode::RemoveButton(std::string_view buttonId)
{
    auto* node = FindById(buttonId);
    if (node != nullptr) {
        (void)RemoveNode(node);
    }
}

auto ActionToolbarNode::FindButton(std::string_view buttonId) -> ActionButtonNode*
{
    auto* node = FindById(buttonId);
    return dynamic_cast<ActionButtonNode*>(node);
}

auto ActionToolbarNode::AddNode(std::unique_ptr<UiNode> child) -> UiNode*
{
    auto* buttonNode = dynamic_cast<ActionButtonNode*>(child.get());
    assert(buttonNode != nullptr && "ActionToolbarNode only accepts ActionButtonNode children");
    auto* raw = UiNode::AddNode(std::move(child));
    // Declarative path: button not yet in widget-layer toolbar, add it
    if (buttonNode != nullptr && buttonNode->Button() != nullptr) {
        gui::ActionButtonInfo info;
        info.id = QString::fromStdString(std::string(buttonNode->Id()));
        info.text = buttonNode->Button()->text();
        info.tooltip = buttonNode->Button()->toolTip();
        info.icon = buttonNode->Button()->icon();
        (void)_toolbar->AddButton(info);
    }
    return raw;
}

auto ActionToolbarNode::RemoveNode(UiNode* child) -> std::unique_ptr<UiNode>
{
    auto* buttonNode = dynamic_cast<ActionButtonNode*>(child);
    if (buttonNode != nullptr && buttonNode->Button() != nullptr) {
        auto* btn = buttonNode->Button();
        for (int i = 0; i < _toolbar->ItemCount(); ++i) {
            if (_toolbar->ButtonAt(i) == btn) {
                _toolbar->RemoveItem(i);
                break;
            }
        }
    }
    return UiNode::RemoveNode(child);
}

} // namespace matcha::fw
