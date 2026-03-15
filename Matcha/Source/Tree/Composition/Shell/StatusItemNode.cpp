#include "Matcha/Tree/Composition/Shell/StatusItemNode.h"

#include "Matcha/Widgets/Controls/NyanLabel.h"
#include "Matcha/Widgets/Controls/NyanProgressBar.h"

#include <QString>
#include <QWidget>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(StatusItemNode, UiNode)

StatusItemNode::StatusItemNode(std::string id, StatusItemKind kind, QWidget* widget)
    : UiNode(std::move(id), NodeType::StatusBarItem)
    , _kind(kind)
    , _widget(widget)
{
}

StatusItemNode::~StatusItemNode() = default;

auto StatusItemNode::Widget() -> QWidget*
{
    return _widget;
}

// -- Label API --

void StatusItemNode::SetText(std::string_view text)
{
    _text = std::string(text);
    if (_kind == StatusItemKind::Label) {
        auto* label = qobject_cast<gui::NyanLabel*>(_widget);
        if (label != nullptr) {
            label->setText(QString::fromUtf8(_text.data(),
                                             static_cast<int>(_text.size())));
        }
    }
}

auto StatusItemNode::Text() const -> std::string_view
{
    return _text;
}

// -- Progress API --

void StatusItemNode::SetValue(int percent)
{
    if (_kind == StatusItemKind::Progress) {
        auto* bar = qobject_cast<gui::NyanProgressBar*>(_widget);
        if (bar != nullptr) {
            bar->setValue(percent);
        }
    }
}

auto StatusItemNode::Value() const -> int
{
    if (_kind == StatusItemKind::Progress) {
        auto* bar = qobject_cast<gui::NyanProgressBar*>(_widget);
        if (bar != nullptr) {
            return bar->value();
        }
    }
    return -1;
}

} // namespace matcha::fw
