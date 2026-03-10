#pragma once

/**
 * @file Application.h
 * @brief Application — non-UiNode class managing QApplication lifecycle + global services.
 *
 * Application is the entry point for the Matcha framework. It owns:
 *  - QApplication lifecycle (Initialize / Shutdown)
 *  - Main loop integration (ProcessEvents / Tick / FlushDirtyViewports)
 *  - Shell (UiNode root)
 *  - Multi-window management (CreateWindow / DestroyWindow)
 *  - Theme service reference
 *
 * Application is NOT a UiNode — it sits above the UiNode tree.
 * Zero Qt types in public header (Pimpl pattern).
 *
 * @note One Application per process.
 * @see docs/02_Architecture_Design.md section 2.5.4
 */

#include "Matcha/Foundation/Macros.h"
#include "Matcha/Foundation/StrongId.h"
#include "Matcha/Foundation/Types.h"

#include <memory>

namespace matcha { class NotificationQueue; }
namespace matcha::fw { class ITokenRegistry; }
namespace matcha::gui { class IThemeService; }

namespace matcha::fw {

class Shell;
class WindowNode;
class MenuNode;
class IDocumentManager;
class DocumentManager;
class WorkshopRegistry;
class WorkbenchManager;
enum class WindowKind : uint8_t;

/**
 * @brief Application — manages QApplication lifecycle and global services.
 *
 * Not a UiNode. Owns Shell (UiNode root) and WindowNode instances.
 * Business layer creates one Application, calls Initialize(), runs main loop
 * via Tick(), and calls Shutdown() before exit.
 */
class MATCHA_EXPORT Application {
public:
    /// @brief Construct the Application.
    Application();
    ~Application();

    Application(const Application&) = delete;
    auto operator=(const Application&) -> Application& = delete;
    Application(Application&&) = delete;
    auto operator=(Application&&) -> Application& = delete;

    // -- Initialization --

    /// @brief Create QApplication + build main WindowNode. Must be called once from main thread.
    void Initialize(int argc, char* argv[]);

    /// @brief Whether Initialize() has been called.
    [[nodiscard]] auto IsInitialized() const -> bool;

    // -- Main loop integration (framework does NOT own event loop) --

    /// @brief Process pending Qt events (input, paint, layout, timer).
    void ProcessEvents();

    /// @brief ProcessEvents() + FlushDirtyViewports(). Recommended single entry point.
    void Tick();

    /// @brief Render all dirty viewports synchronously. (Stub until Phase 5.)
    void FlushDirtyViewports();

    /// @brief Check if the main window close event has been received.
    [[nodiscard]] auto ShouldClose() const -> bool;

    /// @brief Check if any viewport is dirty. (Stub until Phase 5.)
    [[nodiscard]] auto HasDirtyViewports() const -> bool;

    /// @brief Destroy all windows and QApplication. Must be called from main thread.
    void Shutdown();

    // -- Global services --

    /// @brief Get the Shell (UiNode root).
    [[nodiscard]] auto GetShell() -> Shell&;

    /// @brief Get the theme service (gui-layer, Qt-dependent queries).
    [[nodiscard]] auto Theme() -> gui::IThemeService&;

    /// @brief Get the token registry (fw-layer, Qt-free queries).
    /// Returns the same underlying NyanTheme object as Theme().
    [[nodiscard]] auto Tokens() -> ITokenRegistry&;

    // -- Multi-window management --

    /// @brief Get the main WindowNode.
    [[nodiscard]] auto MainWindow() -> WindowNode&;

    /// @brief Create a new floating/detached window.
    /// @param kind Must be Floating or Detached (not Main).
    /// @param width Initial width.
    /// @param height Initial height.
    /// @return Reference to the newly created WindowNode.
    auto CreateWindow(WindowKind kind, int width, int height) -> WindowNode&;

    /// @brief Destroy a window by its WindowId.
    void DestroyWindow(WindowId id);

    /// @brief Find a window by its WindowId. Returns nullptr if not found.
    [[nodiscard]] auto FindWindow(WindowId id) -> WindowNode*;

    // -- Document Manager (application-owned service) --

    /// @brief Get the document manager (interface). Returns nullptr if not yet created.
    [[nodiscard]] auto GetDocumentManager() -> observer_ptr<IDocumentManager>;

    /// @brief Get the concrete document manager. Returns nullptr if not yet created.
    [[nodiscard]] auto GetDocumentManagerImpl() -> DocumentManager*;

    // -- Context Menu --

    /// @brief Get the persistent context menu (MenuNode). Valid after Initialize().
    [[nodiscard]] auto GetContextMenu() -> MenuNode*;

    // -- Workshop/Workbench architecture --

    /// @brief Get the workshop registry (valid after Initialize).
    [[nodiscard]] auto GetWorkshopRegistry() -> WorkshopRegistry*;

    /// @brief Get the workbench manager (valid after Initialize).
    [[nodiscard]] auto GetWorkbenchManager() -> WorkbenchManager*;

    // -- Notification Queue --

    /// @brief Dispatch all queued notifications. Called automatically by Tick().
    void FlushNotificationQueue();

    /// @brief Get the notification queue (valid after Initialize).
    [[nodiscard]] auto GetNotificationQueue() -> matcha::NotificationQueue*;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace matcha::fw
