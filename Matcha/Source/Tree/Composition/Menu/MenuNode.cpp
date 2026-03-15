#include "Matcha/Tree/Composition/Menu/MenuNode.h"
#include "Matcha/Tree/Composition/Menu/MenuItemNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Menu/NyanMenu.h"
#include "Matcha/Widgets/Menu/NyanMenuCheckItem.h"
#include "Matcha/Widgets/Menu/NyanMenuItem.h"

#include <QObject>
#include <QPoint>
#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(MenuNode, UiNode)

MenuNode::MenuNode(std::string id)
    : UiNode(std::move(id), NodeType::Menu)
    , _menu(new gui::NyanMenu())
{
    // Connect the widget-level signal to UiNode-level routing.
    // When mouse exits this menu's rect, route through the UiNode tree.
    QObject::connect(_menu, &gui::NyanMenu::MouseExitedToward,
                     _menu, [this](QPoint globalPos) {
        // Walk up the UiNode tree to find our parent MenuNode
        auto* parentNode = dynamic_cast<MenuNode*>(ParentNode());
        if (parentNode) {
            parentNode->OnChildSubmenuMouseExited(globalPos);
        }
    });

    // Forward ItemTriggered from the widget to UiNode notification
    QObject::connect(_menu, &gui::NyanMenu::ItemTriggered,
                     _menu, [this](gui::NyanMenuItem* /*item*/) {
        Activated notif;
        SendNotification(this, notif);
    });
}

MenuNode::~MenuNode()
{
    if (_ownsMenu) {
        delete _menu;
    }
}

void MenuNode::BindMenu(gui::NyanMenu* menu)
{
    if (_ownsMenu) {
        delete _menu;
    }
    _menu = menu;
    _ownsMenu = false;

    // Re-connect signals on the new widget
    QObject::connect(_menu, &gui::NyanMenu::MouseExitedToward,
                     _menu, [this](QPoint globalPos) {
        auto* parentNode = dynamic_cast<MenuNode*>(ParentNode());
        if (parentNode) {
            parentNode->OnChildSubmenuMouseExited(globalPos);
        }
    });

    QObject::connect(_menu, &gui::NyanMenu::ItemTriggered,
                     _menu, [this](gui::NyanMenuItem* /*item*/) {
        Activated notif;
        SendNotification(this, notif);
    });
}

auto MenuNode::AddItem(std::string_view text) -> MenuItemNode*
{
    auto node = std::make_unique<MenuItemNode>(std::string(text));
    node->SetText(text);

    // Create the widget-level item
    auto* widgetItem = _menu->AddItem(
        QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    node->Bind(widgetItem);

    auto* raw = node.get();
    AddNode(std::move(node));
    return raw;
}

void MenuNode::AddSeparator()
{
    _menu->AddSeparator();
}

auto MenuNode::AddCheckItem(std::string_view text, bool checked)
    -> gui::NyanMenuCheckItem*
{
    return _menu->AddCheckItem(
        QString::fromUtf8(text.data(), static_cast<int>(text.size())),
        checked);
}

auto MenuNode::AddSubmenu(std::string_view text) -> MenuNode*
{
    // Create the widget-level submenu first (NyanMenu::AddSubmenu creates
    // both the NyanMenuItem trigger and the child NyanMenu).
    auto* widgetSubmenu = _menu->AddSubmenu(
        QString::fromUtf8(text.data(), static_cast<int>(text.size())));

    // Create the UiNode and bind it to the already-created widget
    auto node = std::make_unique<MenuNode>(std::string(text));
    // The widget is owned by the parent NyanMenu, so don't let MenuNode delete it
    node->BindMenu(widgetSubmenu);

    auto* raw = node.get();
    AddNode(std::move(node));
    return raw;
}

auto MenuNode::InstallNode(std::unique_ptr<UiNode> node) -> UiNode*
{
    if (!node) {
        return nullptr;
    }

    auto* raw = node.get();

    // If the node has a widget, add it to the menu layout
    if (auto* widget = raw->Widget()) {
        _menu->AddWidget(widget);
    }

    AddNode(std::move(node));
    return raw;
}

void MenuNode::ClearAll()
{
    // Remove all UiNode children first (detach subscriptions etc.)
    while (NodeCount() > 0) {
        RemoveNode(NodeAt(0));
    }
    // Then clear the widget layout
    _menu->Clear();
}

void MenuNode::Popup(const QPoint& globalPos)
{
    _menu->Popup(globalPos);
}

void MenuNode::Close()
{
    _menu->Close();
}

auto MenuNode::IsOpen() const -> bool
{
    return _menu && _menu->IsOpen();
}

auto MenuNode::Menu() const -> gui::NyanMenu*
{
    return _menu;
}

auto MenuNode::Widget() -> QWidget*
{
    return _menu;
}

void MenuNode::OnChildSubmenuMouseExited(const QPoint& globalPos)
{
    // Check if the global position is within our own menu's area
    QPoint localPos = _menu->mapFromGlobal(globalPos);
    if (_menu->rect().contains(localPos)) {
        // Mouse is over our menu -- handle the hover
        _menu->HandleExternalMouseMove(globalPos);
        return;
    }

    // Not over us either -- propagate upward through the UiNode tree
    auto* parentNode = dynamic_cast<MenuNode*>(ParentNode());
    if (parentNode) {
        parentNode->OnChildSubmenuMouseExited(globalPos);
    }
}

} // namespace matcha::fw
