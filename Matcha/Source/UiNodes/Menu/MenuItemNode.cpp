#include "Matcha/UiNodes/Menu/MenuItemNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Menu/NyanMenuItem.h"

#include <QString>
#include <QWidget>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(MenuItemNode, UiNode)

MenuItemNode::MenuItemNode(std::string id)
    : UiNode(std::move(id), NodeType::MenuItem)
{
}

MenuItemNode::~MenuItemNode() = default;

void MenuItemNode::SetText(std::string_view text)
{
    _text = std::string(text);
    if (_item) {
        _item->SetText(QString::fromUtf8(_text.data(), static_cast<int>(_text.size())));
    }
}

auto MenuItemNode::Text() const -> std::string
{
    return _text;
}

void MenuItemNode::SetEnabled(bool enabled)
{
    _enabled = enabled;
    if (_item) {
        _item->setEnabled(enabled);
    }
}

auto MenuItemNode::IsEnabled() const -> bool
{
    return _enabled;
}

void MenuItemNode::Bind(gui::NyanMenuItem* item)
{
    _item = item;
    if (_item) {
        // Apply cached state
        if (!_text.empty()) {
            _item->SetText(QString::fromUtf8(_text.data(), static_cast<int>(_text.size())));
        }
        _item->setEnabled(_enabled);

        // Connect triggered signal
        QObject::connect(_item, &gui::NyanMenuItem::Triggered, _item, [this]() {
            Activated notif;
            SendNotification(this, notif);
        });
    }
}

auto MenuItemNode::Widget() -> QWidget*
{
    return _item;
}

} // namespace matcha::fw
