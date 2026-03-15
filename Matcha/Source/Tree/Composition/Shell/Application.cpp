#include "Matcha/Tree/Composition/Shell/Application.h"

#include "Matcha/Interaction/Focus/FocusManager.h"
#include "Matcha/Interaction/Focus/MnemonicManager.h"
#include "Matcha/Interaction/Focus/MnemonicState.h"
#include "Matcha/Event/NotificationQueue.h"
#include "Matcha/Tree/Composition/Workbench/WorkbenchManager.h"
#include "Matcha/Tree/Composition/Workbench/WorkshopRegistry.h"
#include "Matcha/Theming/Token/TokenRegistryGlobal.h"
#include "Matcha/Services/DocumentManager.h"
#include "Matcha/Services/IDocumentManager.h"
#include "Matcha/Tree/Composition/Menu/MenuNode.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"
#include "Matcha/Animation/IAnimationService.h"
#include "Matcha/Theming/IThemeService.h"
#include "Animation/AnimationService.h"
#include "Matcha/Tree/Composition/Document/Viewport.h"
#include "Matcha/Tree/Composition/Shell/WindowNode.h"
#include "Matcha/Widgets/Shell/ViewportWidget.h"

#include <QApplication>
#include <QStyleFactory>

#include <cstdlib>

namespace matcha::fw {

// ---------------------------------------------------------------------------
// Application::Impl
// ---------------------------------------------------------------------------

struct Application::Impl {
    std::unique_ptr<Shell> shell;
    std::unique_ptr<matcha::NotificationQueue> notificationQueue;
    std::unique_ptr<DocumentManager> docMgr;
    std::unique_ptr<gui::AnimationService> animationService;
    std::unique_ptr<FocusManager> focusManager;
    std::unique_ptr<WorkshopRegistry> workshopRegistry;
    std::unique_ptr<WorkbenchManager> workbenchManager;
    std::unique_ptr<MnemonicManager> mnemonicManager;
    std::unique_ptr<gui::MnemonicState> mnemonicState;
    bool initialized = false;
    uint32_t nextWindowId = 2;  // 1 is reserved for Main
};

// ---------------------------------------------------------------------------
// Application
// ---------------------------------------------------------------------------

Application::Application()
    : _impl(std::make_unique<Impl>())
{
}

Application::~Application()
{
    Shutdown();
}

void Application::Initialize(int argc, char* argv[])
{
    if (_impl->initialized) {
        return;
    }

    // QApplication must exist before any QWidget creation.
    // If the caller already created one (e.g. test harness), skip.
    if (QApplication::instance() == nullptr) {
        // QApplication stores argc by reference -- must stay alive.
        // We rely on the caller's main() stack keeping argc/argv valid.
        new QApplication(argc, argv);  // NOLINT(cppcoreguidelines-owning-memory)
    }

    // Use Fusion as the base style — provides a clean, cross-platform neutral
    // foundation. All visual customization comes from Design Token stylesheets.
    if (auto* fusion = QStyleFactory::create(QStringLiteral("Fusion"))) {
        QApplication::setStyle(fusion);
    }

    // Create notification queue (before Shell, so it's available during tree construction)
    _impl->notificationQueue = std::make_unique<matcha::NotificationQueue>();

    // Create Shell (UiNode root)
    _impl->shell = std::make_unique<Shell>();

    // Wire Shell's back-pointer to this Application (friend access)
    _impl->shell->_application = this;

    // Create Main WindowNode as Shell's child
    auto mainWindow = std::make_unique<WindowNode>(
        "main_window", WindowId::From(1), WindowKind::Main);
    mainWindow->BuildWindow(nullptr);
    _impl->shell->AddNode(std::move(mainWindow));

    // Create persistent context menu as Shell's child
    auto ctxMenu = std::make_unique<MenuNode>("context_menu");
    _impl->shell->_contextMenu = ctxMenu.get();
    _impl->shell->AddNode(std::move(ctxMenu));

    // Create DocumentManager as Shell's child (for notification propagation)
    _impl->docMgr = std::make_unique<DocumentManager>();
    _impl->docMgr->SetParent(_impl->shell.get());

    // Wire fw-layer global token registry (so ContainerNode etc. can resolve tokens)
    SetGlobalTokenRegistry(&gui::GetThemeService());

    // Create Animation Engine (depends on IThemeService for duration/easing resolution)
    _impl->animationService = std::make_unique<gui::AnimationService>(
        gui::GetThemeService());
    gui::SetAnimationService(_impl->animationService.get());

    // Create FocusManager (centralized focus tracking + cross-region flow)
    _impl->focusManager = std::make_unique<FocusManager>();
    SetFocusManager(_impl->focusManager.get());

    // Create WorkshopRegistry + WorkbenchManager (Workshop/Workbench architecture)
    _impl->workshopRegistry = std::make_unique<WorkshopRegistry>();
    _impl->workbenchManager = std::make_unique<WorkbenchManager>(
        *_impl->shell, *_impl->workshopRegistry);

    // Create MnemonicManager (UiNode-layer mnemonic dispatch)
    _impl->mnemonicManager = std::make_unique<MnemonicManager>();
    SetMnemonicManager(_impl->mnemonicManager.get());

    // Create MnemonicState (Widget-layer mnemonic visibility + rendering)
    _impl->mnemonicState = std::make_unique<gui::MnemonicState>();
    gui::SetMnemonicState(_impl->mnemonicState.get());

    // Respect OS "Always underline access keys" setting (Windows SPI_GETKEYBOARDCUES)
    if (gui::MnemonicState::QueryOsKeyboardCues()) {
        _impl->mnemonicState->SetAlwaysShow(true);
    }

    _impl->initialized = true;

    // Log viewport backend env var (informational only)
#ifdef _WIN32
    char* backend = nullptr;
    size_t len = 0;
    if (_dupenv_s(&backend, &len, "MATCHA_VIEWPORT_BACKEND") == 0 && backend != nullptr) {
        // Future: use this to select QRhiWidget vs native handle path
        free(backend); // NOLINT(cppcoreguidelines-no-malloc)
    }
#else
    if (const char* backend = std::getenv("MATCHA_VIEWPORT_BACKEND")) { // NOLINT(concurrency-mt-unsafe)
        (void)backend;
    }
#endif
}

auto Application::IsInitialized() const -> bool
{
    return _impl->initialized;
}

void Application::ProcessEvents()
{
    if (QApplication::instance() != nullptr) {
        QApplication::processEvents();
    }
}

void Application::Tick()
{
    FlushNotificationQueue();
    ProcessEvents();
    FlushDirtyViewports();
}

void Application::FlushDirtyViewports()
{
    if (!_impl->initialized || _impl->shell == nullptr) { return; }

    // Traverse all WindowNodes -> descendants of type Viewport
    for (size_t i = 0; i < _impl->shell->NodeCount(); ++i) {
        auto* win = _impl->shell->NodeAt(i);
        if (win == nullptr) { continue; }
        for (auto* node : win->DescendantsOfType(NodeType::Viewport)) {
            auto* vp = dynamic_cast<Viewport*>(node);
            if (vp == nullptr || !vp->IsDirty()) { continue; }

            auto* widget = vp->GetWidget();
            if (widget != nullptr) {
                widget->RenderFrame();
            }
            vp->ClearDirty();
        }
    }
}

auto Application::ShouldClose() const -> bool
{
    if (!_impl->initialized || _impl->shell == nullptr) {
        return true;
    }
    // Check if main WindowNode's close was requested
    auto* mainWin = _impl->shell->FindMainWindow();
    return mainWin == nullptr || mainWin->IsCloseRequested();
}

auto Application::HasDirtyViewports() const -> bool
{
    if (!_impl->initialized || _impl->shell == nullptr) { return false; }

    for (size_t i = 0; i < _impl->shell->NodeCount(); ++i) {
        auto* win = _impl->shell->NodeAt(i);
        if (win == nullptr) { continue; }
        for (auto* node : win->DescendantsOfType(NodeType::Viewport)) {
            auto* vp = dynamic_cast<Viewport*>(node);
            if (vp != nullptr && vp->IsDirty()) { return true; }
        }
    }
    return false;
}

void Application::Shutdown()
{
    if (!_impl->initialized) {
        return;
    }

    // Clear queued notifications before destroying the tree
    if (_impl->notificationQueue) {
        _impl->notificationQueue->Clear();
    }

    // Clear fw-layer global token registry
    SetGlobalTokenRegistry(nullptr);

    // Clear global focus manager pointer before destroying the manager
    SetFocusManager(nullptr);

    // Clear global animation service pointer before destroying the service
    gui::SetAnimationService(nullptr);

    // Clear global mnemonic pointers before destroying
    gui::SetMnemonicState(nullptr);
    _impl->mnemonicState.reset();
    SetMnemonicManager(nullptr);
    _impl->mnemonicManager.reset();

    // Destroy WorkbenchManager before Shell (it references Shell & Registry)
    _impl->workbenchManager.reset();
    _impl->workshopRegistry.reset();

    // Destroy Shell (and all its children including WindowNodes)
    _impl->shell.reset();
    _impl->notificationQueue.reset();
    _impl->initialized = false;
}

auto Application::GetShell() -> Shell&
{
    return *_impl->shell;
}

auto Application::Theme() -> gui::IThemeService&
{
    return gui::GetThemeService();
}

auto Application::Tokens() -> ITokenRegistry&
{
    return gui::GetThemeService();
}

auto Application::MainWindow() -> WindowNode&
{
    auto* win = _impl->shell->FindMainWindow();
    return *win;
}

auto Application::CreateWindow(WindowKind kind, int width, int height) -> WindowNode&
{
    auto id = WindowId::From(_impl->nextWindowId++);
    auto name = std::string("window_") + std::to_string(id.value);
    auto node = std::make_unique<WindowNode>(name, id, kind);

    // Floating windows are parented to the main window's QWidget
    QWidget* parent = nullptr;
    if (kind == WindowKind::Floating) {
        auto* mainWin = _impl->shell->FindMainWindow();
        if (mainWin != nullptr) {
            parent = mainWin->Widget();
        }
    }
    node->BuildWindow(parent);
    node->Resize(width, height);

    auto& ref = *node;
    _impl->shell->AddNode(std::move(node));
    return ref;
}

void Application::DestroyWindow(WindowId id)
{
    // Find the WindowNode first, then remove it
    WindowNode* target = nullptr;
    for (size_t i = 0; i < _impl->shell->NodeCount(); ++i) {
        auto* win = dynamic_cast<WindowNode*>(_impl->shell->NodeAt(i));
        if (win != nullptr && win->Id() == id && win->Kind() != WindowKind::Main) {
            target = win;
            break;
        }
    }
    if (target != nullptr) {
        target->Close();
        _impl->shell->RemoveNode(target);
    }
}

auto Application::FindWindow(WindowId id) -> WindowNode*
{
    for (size_t i = 0; i < _impl->shell->NodeCount(); ++i) {
        auto* win = dynamic_cast<WindowNode*>(_impl->shell->NodeAt(i));
        if (win != nullptr && win->Id() == id) {
            return win;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Document Manager
// ---------------------------------------------------------------------------

auto Application::GetDocumentManager() -> observer_ptr<IDocumentManager>
{
    return observer_ptr<IDocumentManager>{_impl->docMgr.get()};
}

auto Application::GetDocumentManagerImpl() -> DocumentManager*
{
    return _impl->docMgr.get();
}

// ---------------------------------------------------------------------------
// Context Menu
// ---------------------------------------------------------------------------

auto Application::GetContextMenu() -> MenuNode*
{
    return _impl->shell ? _impl->shell->GetContextMenu() : nullptr;
}

// ---------------------------------------------------------------------------
// Workshop/Workbench architecture
// ---------------------------------------------------------------------------

auto Application::GetWorkshopRegistry() -> WorkshopRegistry*
{
    return _impl->workshopRegistry.get();
}

auto Application::GetWorkbenchManager() -> WorkbenchManager*
{
    return _impl->workbenchManager.get();
}

// ---------------------------------------------------------------------------
// Notification Queue
// ---------------------------------------------------------------------------

void Application::FlushNotificationQueue()
{
    if (_impl->notificationQueue) {
        _impl->notificationQueue->FlushPending();
    }
}

auto Application::GetNotificationQueue() -> matcha::NotificationQueue*
{
    return _impl->notificationQueue.get();
}

} // namespace matcha::fw
