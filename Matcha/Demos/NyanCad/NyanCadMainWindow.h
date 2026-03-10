#pragma once

/**
 * @file NyanCadMainWindow.h
 * @brief Configures Shell's MainWindow for NyanCad demo.
 */

#include "DocumentView.h"

#include "Matcha/Foundation/StrongId.h"

#include <memory>
#include <vector>

namespace matcha::fw {
class Application;
class DocumentManager;
class TabBarNode;
} // namespace matcha::fw

namespace nyancad {

/// @brief Holds the per-floating-window document state.
struct FloatingDocState {
    std::unique_ptr<DocumentView> docView;
};

/**
 * @brief Configures the main WindowNode: ActionBar tabs, StatusBar, central layout.
 *
 * This is NOT a QWidget. It operates on the Shell's MainWindow WindowNode
 * through the Application API.
 */
class NyanCadMainWindow {
public:
    NyanCadMainWindow() = default;
    ~NyanCadMainWindow();

    NyanCadMainWindow(const NyanCadMainWindow&) = delete;
    NyanCadMainWindow& operator=(const NyanCadMainWindow&) = delete;
    NyanCadMainWindow(NyanCadMainWindow&&) = delete;
    NyanCadMainWindow& operator=(NyanCadMainWindow&&) = delete;

    /**
     * @brief Set up the main window with ActionBar tabs, StatusBar, DocumentArea.
     * @param app Application reference (provides MainWindow, Theme, DocumentManager).
     */
    void Setup(matcha::fw::Application& app);

    /// @brief Get the main window DocumentView (valid after Setup).
    [[nodiscard]] auto GetDocumentView() -> DocumentView*;

    /// @brief Close and release all floating windows. Call before shutdown.
    void CloseFloatingWindows();

    /// @brief Tear down all business-layer objects that hold ScopedSubscriptions
    /// to UiNode tree nodes. MUST be called before Application::Shutdown()
    /// destroys the UiNode tree, otherwise ScopedSubscription destructors
    /// will access destroyed EventNodes.
    void Teardown();

private:
    void OnOpenDialog();
    void OnOpenFloatingWindow();

    void CreateFloatingWindowForPage(matcha::fw::PageId pageId, int globalX, int globalY);

    matcha::fw::Application* _app = nullptr;
    DocumentView* _docView = nullptr;
    std::vector<std::unique_ptr<FloatingDocState>> _floatingDocStates;
};

} // namespace nyancad
