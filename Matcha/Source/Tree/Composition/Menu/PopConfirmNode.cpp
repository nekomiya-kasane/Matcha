/**
 * @file PopConfirmNode.cpp
 * @brief Implementation of PopConfirmNode — UiNode wrapper for NyanPopConfirm.
 */

#include "Matcha/Tree/Composition/Menu/PopConfirmNode.h"
#include "Matcha/Widgets/Menu/NyanPopConfirm.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(PopConfirmNode, PopupNode)

PopConfirmNode::PopConfirmNode(std::string id)
    : PopupNode(std::move(id), NodeType::Popup, PopupBehavior::Dropdown)
{
}

PopConfirmNode::~PopConfirmNode() = default;

void PopConfirmNode::SetTitle(std::string_view title)
{
    if (_popConfirm != nullptr) {
        _popConfirm->SetTitle(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
    }
}

auto PopConfirmNode::Title() const -> std::string
{
    if (_popConfirm != nullptr) {
        return _popConfirm->Title().toStdString();
    }
    return {};
}

void PopConfirmNode::SetMessage(std::string_view message)
{
    if (_popConfirm != nullptr) {
        _popConfirm->SetMessage(QString::fromUtf8(message.data(), static_cast<int>(message.size())));
    }
}

auto PopConfirmNode::Message() const -> std::string
{
    if (_popConfirm != nullptr) {
        return _popConfirm->Message().toStdString();
    }
    return {};
}

void PopConfirmNode::SetState(gui::PopConfirmState state)
{
    if (_popConfirm != nullptr) {
        _popConfirm->SetState(state);
    }
}

auto PopConfirmNode::PopConfirm() -> gui::NyanPopConfirm*
{
    return _popConfirm;
}

auto PopConfirmNode::CreatePopupContent(QWidget* parent) -> QWidget*
{
    _popConfirm = new gui::NyanPopConfirm(parent);
    // Strip the dialog's own window flags — it lives inside PopupNode's container
    _popConfirm->setWindowFlags(Qt::Widget);
    return _popConfirm;
}

auto PopConfirmNode::PreferredSize() -> Size
{
    return {312, 180};
}

} // namespace matcha::fw
