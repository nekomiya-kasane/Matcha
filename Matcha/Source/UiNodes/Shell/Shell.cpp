#include "Matcha/UiNodes/Shell/Shell.h"

#include "Matcha/UiNodes/ActionBar/ActionBarNode.h"
#include "Matcha/UiNodes/Document/DocumentArea.h"
#include "Matcha/UiNodes/Menu/MenuNode.h"
#include "Matcha/UiNodes/Shell/Application.h"
#include "Matcha/UiNodes/Shell/StatusBarNode.h"
#include "Matcha/UiNodes/Shell/WindowNode.h"
#include "Matcha/UiNodes/Shell/WorkspaceFrame.h"
#include "Matcha/Widgets/Core/UpdateGuard.h"

namespace matcha::fw {

// ---------------------------------------------------------------------------
// Shell -- pure UiNode root (no Qt members)
// ---------------------------------------------------------------------------

MATCHA_IMPLEMENT_CLASS(Shell, UiNode)

Shell::Shell()
    : UiNode("shell", NodeType::Shell)
{
}

Shell::~Shell() = default;

auto Shell::MainWindow() const -> WindowNode*
{
    for (size_t i = 0; i < NodeCount(); ++i) {
        auto* node = NodeAt(i);
        if (node->Type() == NodeType::WindowNode) {
            auto* win = static_cast<WindowNode*>(node);
            if (win->Kind() == WindowKind::Main) {
                return win;
            }
        }
    }
    return nullptr;
}

auto Shell::Windows() const -> std::vector<observer_ptr<WindowNode>>
{
    std::vector<observer_ptr<WindowNode>> result;
    for (size_t i = 0; i < NodeCount(); ++i) {
        auto* node = NodeAt(i);
        if (node->Type() == NodeType::WindowNode) {
            result.push_back(observer_ptr<WindowNode>{static_cast<WindowNode*>(node)});
        }
    }
    return result;
}

auto Shell::GetActionBar() -> observer_ptr<ActionBarNode>
{
    auto* main = MainWindow();
    if (main == nullptr) return observer_ptr<ActionBarNode>{nullptr};
    // ActionBarNode is nested under WorkspaceFrame
    for (size_t i = 0; i < main->NodeCount(); ++i) {
        auto* child = main->NodeAt(i);
        if (child->Type() == NodeType::WorkspaceFrame) {
            auto* wsFrame = static_cast<WorkspaceFrame*>(child);
            return wsFrame->GetActionBar();
        }
    }
    return observer_ptr<ActionBarNode>{nullptr};
}

auto Shell::GetStatusBar() -> observer_ptr<StatusBarNode>
{
    auto* main = MainWindow();
    if (main == nullptr) return observer_ptr<StatusBarNode>{nullptr};
    for (size_t i = 0; i < main->NodeCount(); ++i) {
        auto* child = main->NodeAt(i);
        if (child->Type() == NodeType::StatusBar) {
            return observer_ptr<StatusBarNode>{static_cast<StatusBarNode*>(child)};
        }
    }
    return observer_ptr<StatusBarNode>{nullptr};
}

auto Shell::GetDocumentArea() -> observer_ptr<DocumentArea>
{
    auto* main = MainWindow();
    if (main == nullptr) return observer_ptr<DocumentArea>{nullptr};
    for (size_t i = 0; i < main->NodeCount(); ++i) {
        auto* child = main->NodeAt(i);
        if (child->Type() == NodeType::WorkspaceFrame) {
            auto* wsFrame = static_cast<WorkspaceFrame*>(child);
            return wsFrame->GetDocumentArea();
        }
    }
    return observer_ptr<DocumentArea>{nullptr};
}


auto Shell::GetApplication() const -> Application*
{
    return _application;
}

auto Shell::GetContextMenu() -> MenuNode*
{
    return _contextMenu;
}

auto Shell::GetNotificationQueue() const -> matcha::NotificationQueue*
{
    return _application ? _application->GetNotificationQueue() : nullptr;
}

auto Shell::FreezeUpdates() -> gui::UpdateGuard
{
    auto* mainWin = MainWindow();
    if (mainWin != nullptr) {
        return mainWin->FreezeUpdates();
    }
    return gui::UpdateGuard::Create(nullptr);
}

} // namespace matcha::fw
