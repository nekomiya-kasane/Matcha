#include "Matcha/Tree/Composition/ActionBar/ActionButtonNode.h"

#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Widgets/Controls/NyanToolButton.h"
#include "Matcha/Theming/IThemeService.h"

#include <QIcon>
#include <QString>
#include <QWidget>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ActionButtonNode, UiNode)

ActionButtonNode::ActionButtonNode(std::string id, gui::NyanToolButton* button)
    : UiNode(std::move(id), NodeType::ActionButton)
    , _button(button)
{
    if (_button != nullptr) {
        QObject::connect(_button, &gui::NyanToolButton::clicked,
            [this](bool checked) {
                Notification::Clicked notif(checked);
                SendNotification(this, notif);
            });
    }
}

ActionButtonNode::~ActionButtonNode() = default;

auto ActionButtonNode::Button() -> gui::NyanToolButton*
{
    return _button;
}

auto ActionButtonNode::Widget() -> QWidget*
{
    return _button;
}

void ActionButtonNode::SetEnabled(bool enabled)
{
    if (_button != nullptr) {
        _button->setEnabled(enabled);
    }
}

auto ActionButtonNode::IsEnabled() const -> bool
{
    return _button != nullptr && _button->isEnabled();
}

void ActionButtonNode::SetCheckable(bool checkable)
{
    if (_button != nullptr) {
        _button->setCheckable(checkable);
    }
}

auto ActionButtonNode::IsCheckable() const -> bool
{
    return _button != nullptr && _button->isCheckable();
}

void ActionButtonNode::SetChecked(bool checked)
{
    if (_button != nullptr) {
        _button->setChecked(checked);
    }
}

auto ActionButtonNode::IsChecked() const -> bool
{
    return _button != nullptr && _button->isChecked();
}

void ActionButtonNode::SetText(std::string_view text)
{
    if (_button != nullptr) {
        _button->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

void ActionButtonNode::SetToolTip(std::string_view tip)
{
    if (_button != nullptr) {
        _button->setToolTip(QString::fromUtf8(tip.data(), static_cast<int>(tip.size())));
    }
}

void ActionButtonNode::SetIcon(std::string_view iconId, fw::IconToken size)
{
    if (_button == nullptr || !gui::HasThemeService()) {
        return;
    }
    if (iconId.empty()) {
        _button->setIcon(QIcon());
        return;
    }
    const int sizePx = static_cast<int>(size);
    const QColor fg = gui::GetThemeService().Color(gui::ColorToken::colorText);
    const QPixmap pm = gui::GetThemeService().ResolveIcon(fw::IconId(iconId), size, fg);
    if (!pm.isNull()) {
        _button->setIcon(QIcon(pm));
        _button->setIconSize(QSize(sizePx, sizePx));
    }
}

auto ActionButtonNode::Index() const -> int
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

} // namespace matcha::fw
