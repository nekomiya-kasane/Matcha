#include "Matcha/Tree/Composition/Menu/DialogNode.h"

#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Widgets/Menu/NyanDialog.h"

#include <QString>
#include <QWidget>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(DialogNode, UiNode)

DialogNode::DialogNode(std::string id, UiNode* parentHint)
    : UiNode(std::move(id), NodeType::Dialog)
    , _dialog(new gui::NyanDialog(parentHint ? parentHint->Widget() : nullptr))
{
    SetFocusScope(true);

    QObject::connect(_dialog, &gui::NyanDialog::Closed,
        [this](gui::DialogResult result) {
            DialogClosed notif(static_cast<uint8_t>(result));
            SendNotification(this, notif);
        });
}

DialogNode::~DialogNode() = default;

auto DialogNode::Dialog() -> gui::NyanDialog*
{
    return _dialog;
}

auto DialogNode::Widget() -> QWidget*
{
    return _dialog;
}

void DialogNode::SetTitle(std::string_view title)
{
    _dialog->SetTitle(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
}

auto DialogNode::Title() const -> std::string
{
    return _dialog->Title().toStdString();
}

void DialogNode::SetContent(QWidget* content)
{
    _dialog->SetContent(content);
}

void DialogNode::SetContentNode(std::unique_ptr<UiNode> contentNode)
{
    if (contentNode != nullptr) {
        auto* widget = contentNode->Widget();
        AddNode(std::move(contentNode));
        _dialog->SetContent(widget);
    }
}

void DialogNode::SetModality(gui::DialogModality modality)
{
    _dialog->SetDialogModality(modality);
}

auto DialogNode::Modality() const -> gui::DialogModality
{
    return _dialog->Modality();
}

auto DialogNode::ShowModal() -> gui::DialogResult
{
    return _dialog->ShowModal();
}

void DialogNode::ShowSemiModal()
{
    // SemiModal: blocks parent + siblings but allows viewport interaction.
    // Implemented via Qt::WindowModal which blocks the parent window only.
    _dialog->setWindowModality(Qt::WindowModal);
    _dialog->show();
}

void DialogNode::ShowModeless()
{
    _dialog->ShowModeless();
}

void DialogNode::SetWidth(DialogWidth width)
{
    _dialog->setFixedWidth(static_cast<int>(width));
}

void DialogNode::Close()
{
    _dialog->close();
}

} // namespace matcha::fw
