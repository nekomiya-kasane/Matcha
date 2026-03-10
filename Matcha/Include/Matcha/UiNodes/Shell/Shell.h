#pragma once

/**
 * @file Shell.h
 * @brief Root UiNode of the framework -- pure tree root, zero Qt members.
 *
 * Shell is the UiNode tree root. Business layer obtains a Shell reference
 * (via Application::GetShell()) and uses it to access child WindowNodes,
 * the document manager, and container node accessors.
 *
 * Shell does NOT own QApplication or QMainWindow -- those responsibilities
 * belong to Application and WindowNode respectively.
 *
 * @note One Shell per Application. Created internally by Application.
 * @see docs/02_Architecture_Design.md section 2.5.4
 */

#include "Matcha/Foundation/Types.h"
#include "Matcha/UiNodes/Core/UiNode.h"

#include <vector>

namespace matcha::gui {
class UpdateGuard;
class IThemeService;
} // namespace matcha::gui

namespace matcha::fw {

class Application;
class WindowNode;
class MenuNode;
class ActionBarNode;
class StatusBarNode;
class DocumentArea;

/**
 * @brief Root UiNode -- pure tree root with no Qt dependencies.
 *
 * WindowNode children represent top-level windows.
 * Created by Application; do not construct directly.
 */
class MATCHA_EXPORT Shell : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    Shell();
    ~Shell() override;

    Shell(const Shell&) = delete;
    auto operator=(const Shell&) -> Shell& = delete;
    Shell(Shell&&) = delete;
    auto operator=(Shell&&) -> Shell& = delete;

    // -- Window access --

    /// @brief Get the main WindowNode. Returns nullptr if not yet created.
    [[nodiscard]] auto MainWindow() const -> WindowNode*;

    /// @brief Alias for MainWindow() (legacy compatibility).
    [[nodiscard]] auto FindMainWindow() const -> WindowNode* { return MainWindow(); }

    /// @brief Get all WindowNode children (Main + Floating + Detached).
    [[nodiscard]] auto Windows() const -> std::vector<observer_ptr<WindowNode>>;

    // -- Container access (convenience, delegates to MainWindow) --

    /// @brief Get the main ActionBar node.
    [[nodiscard]] auto GetActionBar() -> observer_ptr<ActionBarNode>;

    /// @brief Get the main StatusBar node.
    [[nodiscard]] auto GetStatusBar() -> observer_ptr<StatusBarNode>;

    /// @brief Get the main DocumentArea node (via WorkspaceFrame).
    [[nodiscard]] auto GetDocumentArea() -> observer_ptr<DocumentArea>;

    // -- Application back-pointer --

    /// @brief Get the owning Application. Valid after Initialize().
    [[nodiscard]] auto GetApplication() const -> Application*;

    // -- Context Menu --

    /// @brief Get the persistent context menu (MenuNode). Valid after Initialize().
    [[nodiscard]] auto GetContextMenu() -> MenuNode*;

    // -- Notification Queue --

    /// @brief Override: return the application-level notification queue directly.
    [[nodiscard]] auto GetNotificationQueue() const -> matcha::NotificationQueue* override;

    // -- Freeze updates (delegates to Main WindowNode) --

    /// @brief Freeze main window painting. Returns RAII guard.
    [[nodiscard]] auto FreezeUpdates() -> gui::UpdateGuard;

private:
    Application* _application = nullptr;            ///< Back-pointer (Application owns Shell)
    MenuNode* _contextMenu = nullptr;              ///< Owned as child node
    friend class Application;                       ///< Wires _application + _contextMenu
};

} // namespace matcha::fw
