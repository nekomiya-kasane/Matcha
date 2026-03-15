/**
 * @file WorkspaceFrame.cpp
 * @brief Implementation of WorkspaceFrame UiNode.
 */

#include <Matcha/Tree/Composition/Shell/WorkspaceFrame.h>

#include <Matcha/Tree/Composition/ActionBar/ActionBarNode.h>
#include <Matcha/Tree/Composition/Shell/ControlBar.h>
#include <Matcha/Tree/Composition/Menu/DialogNode.h>
#include <Matcha/Tree/Composition/Document/DocumentArea.h>
#include <Matcha/Widgets/Menu/NyanDialog.h>

namespace matcha::fw {

WorkspaceFrame::WorkspaceFrame(std::string name, UiNode* parentHint)
    : UiNode("workspace-frame", NodeType::WorkspaceFrame, std::move(name))
    , _container(parentHint ? parentHint->Widget() : nullptr)
{
}

WorkspaceFrame::~WorkspaceFrame() = default;

auto WorkspaceFrame::Widget() -> QWidget*
{
    return _container;
}

void WorkspaceFrame::ShowDialog(std::unique_ptr<DialogNode> dialog)
{
    if (!dialog || _container == nullptr) { return; }
    auto* dlg = dialog->Dialog();
    dlg->SetEmbedded(_container);
    dlg->ShowModeless();
    AddNode(std::move(dialog));
}

void WorkspaceFrame::CloseDialog(DialogNode* dialog)
{
    if (dialog == nullptr) { return; }
    dialog->Close();
    RemoveNode(dialog);
}

auto WorkspaceFrame::GetDocumentArea() -> observer_ptr<DocumentArea>
{
    for (auto* child : ChildrenOfType(NodeType::DocumentArea)) {
        return observer_ptr<DocumentArea>(static_cast<DocumentArea*>(child));
    }
    return observer_ptr<DocumentArea>(nullptr);
}

auto WorkspaceFrame::GetActionBar() -> observer_ptr<ActionBarNode>
{
    for (auto* child : ChildrenOfType(NodeType::ActionBar)) {
        return observer_ptr<ActionBarNode>(static_cast<ActionBarNode*>(child));
    }
    return observer_ptr<ActionBarNode>(nullptr);
}

auto WorkspaceFrame::GetControlBar() -> observer_ptr<ControlBar>
{
    for (auto* child : ChildrenOfType(NodeType::ControlBar)) {
        return observer_ptr<ControlBar>(static_cast<ControlBar*>(child));
    }
    return observer_ptr<ControlBar>(nullptr);
}

} // namespace matcha::fw
