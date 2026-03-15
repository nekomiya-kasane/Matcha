/**
 * @file NyanCadApp.cpp
 * @brief NyanCad demo application implementation.
 */

#include "NyanCadApp.h"

#include "MeshEditors/NyanFwSketchEditor.h"
#include "Diagnostics/NyanCadDevToolsWindow.h"
#include "MeshEditors/NyanFwSurfaceMeshEditor.h"
#include "MeshEditors/NyanFwTetMeshEditor.h"
#include "NyanCadDocument.h"
#include "NyanCadMainWindow.h"
#include "NyanCadWorkshopSetup.h"

#include "Matcha/Services/PluginHost.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Tree/Composition/Workbench/WorkbenchManager.h"
#include "Matcha/Tree/Composition/Workbench/WorkshopRegistry.h"
#include "Matcha/Services/DocumentManager.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"
#include "Matcha/Tree/Composition/Shell/WindowNode.h"
#include "Matcha/Theming/NyanTheme.h"

#include <QApplication>
#include <QString>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <print>

namespace nyancad {

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct NyanCadApp::Impl {
#ifdef _WIN32
    HANDLE singleInstanceMutex = nullptr;
#endif
    std::unique_ptr<matcha::gui::NyanTheme> theme;
    std::unique_ptr<matcha::fw::Application> app;
    NyanCadMainWindow mainWindow;
    NyanCadDocumentHost docHost;
    NyanFwSurfaceMeshEditor surfaceMeshEditor;
    NyanFwTetMeshEditor tetMeshEditor;
    NyanFwSketchEditor sketchEditor;
    matcha::fw::PluginHost pluginHost;
    std::unique_ptr<NyanCadDevToolsWindow> devTools;
};

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

NyanCadApp::NyanCadApp()
    : _impl(std::make_unique<Impl>())
{
}

NyanCadApp::~NyanCadApp() = default;

// ---------------------------------------------------------------------------
// Single-Instance Guard
// ---------------------------------------------------------------------------

auto NyanCadApp::AcquireSingleInstanceLock() -> bool
{
#ifdef _WIN32
    _impl->singleInstanceMutex = CreateMutexW(nullptr, TRUE, L"NyanCad_SingleInstance");
    if (_impl->singleInstanceMutex == nullptr) {
        return false;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(_impl->singleInstanceMutex);
        _impl->singleInstanceMutex = nullptr;
        return false;
    }
    return true;
#else
    // Linux flock deferred -- always succeeds for now
    return true;
#endif
}

void NyanCadApp::ReleaseSingleInstanceLock()
{
#ifdef _WIN32
    if (_impl->singleInstanceMutex != nullptr) {
        ReleaseMutex(_impl->singleInstanceMutex);
        CloseHandle(_impl->singleInstanceMutex);
        _impl->singleInstanceMutex = nullptr;
    }
#endif
}

// ---------------------------------------------------------------------------
// Business Step (stub)
// ---------------------------------------------------------------------------

void NyanCadApp::BusinessStep()
{
    // Stub -- no real CAD engine. In a real app this would call:
    // myCadEngine.Step();
}

// ---------------------------------------------------------------------------
// Run
// ---------------------------------------------------------------------------

auto NyanCadApp::Run(int argc, char** argv) -> int
{
    // 1. Single-instance guard
    if (!AcquireSingleInstanceLock()) {
        std::println(stderr, "NyanCad: another instance is already running.");
        return 1;
    }

    // 2. QApplication (must exist before any QObject)
    QApplication qtApp(argc, argv);

    // 3. Theme service (set global accessor before any widget creation)
    _impl->theme = std::make_unique<matcha::gui::NyanTheme>(
        QString::fromUtf8(MATCHA_PALETTE_DIR));
    _impl->theme->SetTheme(matcha::gui::kThemeLight);
    // Register built-in icons from Resources/Icons directory
    _impl->theme->RegisterIconDirectory(
        matcha::fw::kMatchaIconPrefix,
        QString::fromUtf8(MATCHA_PALETTE_DIR) + QStringLiteral("/../Icons"));
    matcha::gui::SetThemeService(_impl->theme.get());

    // 4. Application + Shell + main WindowNode
    _impl->app = std::make_unique<matcha::fw::Application>();
    _impl->app->Initialize(argc, argv);

    // 5. Wire document host into command tree
    // Notifications propagate UP: docMgr -> docHost
    auto* docMgr = _impl->app->GetDocumentManagerImpl();
    if (docMgr) { docMgr->SetParent(&_impl->docHost); }

    // 6. Register Workshop/Workbench descriptors
    auto* wsRegistry = _impl->app->GetWorkshopRegistry();
    if (wsRegistry) {
        RegisterNyanCadWorkshops(*wsRegistry);
    }

    // 7. Configure main window (DocumentView subscribes to docMgr Notifications)
    _impl->mainWindow.Setup(*_impl->app);

    // 8. Activate the Mesh workshop (materializes base tabs + default workbench)
    auto* wbMgr = _impl->app->GetWorkbenchManager();
    if (wbMgr) {
        wbMgr->ActivateWorkshop(matcha::fw::WorkshopId::From("mesh"));
    }

    // 8. Load plugins from directory
    auto& shell = _impl->app->GetShell();
    auto pluginResult = _impl->pluginHost.LoadPluginsFromDirectory(
        MATCHA_PLUGIN_DIR, shell);
    if (pluginResult.has_value()) {
        std::println("NyanCad: loaded {} plugins.", pluginResult.value().size());
    }

    // 9. Create initial demo documents (after UI is wired so tabs appear)
    if (docMgr) { _impl->docHost.CreateInitialDocuments(*docMgr); }

    // 9. DevTools window (Notification Log + UI Inspector)
    _impl->devTools = std::make_unique<NyanCadDevToolsWindow>();
    _impl->devTools->BuildWindow(_impl->app->MainWindow().Widget());
    _impl->devTools->Bind(*_impl->app);
    _impl->devTools->Show();

    // 10. Show main window
    _impl->app->MainWindow().Show();

    // 11. Business main loop (framework does NOT own event loop)
    while (!_impl->app->ShouldClose()) {
        _impl->app->Tick();
        BusinessStep();
    }

    // ===================================================================
    // Shutdown (see docs/05_Greenfield_Plan.md Appendix G)
    // ===================================================================

    // S1: Detach business observers
    //     Postcondition: all ScopedSubscriptions released.
    //     UiNode tree and Qt widgets are STILL ALIVE.
    _impl->mainWindow.CloseFloatingWindows();
    _impl->mainWindow.Teardown();
    _impl->devTools.reset();

    // S2: Stop services
    //     Postcondition: no external events can arrive.
    _impl->app->MainWindow().Close();
    _impl->app->MainWindow().Hide();
    _impl->pluginHost.StopAll();

    // S3: Framework teardown
    //     Postcondition: UiNode tree gone, Qt widgets gone.
    _impl->app->Shutdown();

    // S4: Platform cleanup
    ReleaseSingleInstanceLock();
    QApplication::processEvents();

    return 0;
}

} // namespace nyancad
