#include "Matcha/CApi/NyanCApi.h"

#include "Matcha/Core/ErrorCode.h"
#include "Matcha/Core/StrongId.h"
#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Event/CommandNode.h"
#include "Matcha/Services/IDocumentManager.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"
#include "Matcha/Tree/Composition/Shell/StatusBarNode.h"
#include "Matcha/Tree/Composition/Shell/StatusItemNode.h"
#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Widgets/Shell/NyanStatusBar.h"
#include "Matcha/Tree/Composition/Document/Viewport.h"
#include "Matcha/Theming/IThemeService.h"
#include "Matcha/Theming/NyanTheme.h"

#include <QApplication>

#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================================
// Internal state -- hidden behind the opaque handles
// ============================================================================

namespace {

struct AppState {
    std::string paletteDir;
    int storedArgc = 0; // QApplication needs stable argc reference
    std::unique_ptr<matcha::gui::NyanTheme> theme;
    std::unique_ptr<matcha::fw::Application> app;
    uint32_t generation = 1;

    // Handle invalidation (ADR-019)
    NyanInvalidationCallback invalidationCb = nullptr;
    void* invalidationUserData = nullptr;
};

auto ToApp(NyanAppHandle handle) -> AppState*
{
    return reinterpret_cast<AppState*>(handle); // NOLINT
}

auto FromApp(AppState* state) -> NyanAppHandle
{
    return reinterpret_cast<NyanAppHandle>(state); // NOLINT
}

auto ToShell(NyanShellHandle handle) -> matcha::fw::Shell*
{
    return reinterpret_cast<matcha::fw::Shell*>(handle); // NOLINT
}

auto FromShell(matcha::fw::Shell* shell) -> NyanShellHandle
{
    return reinterpret_cast<NyanShellHandle>(shell); // NOLINT
}

auto MapErrorCode(matcha::fw::ErrorCode code) -> NyanErrorCode
{
    switch (code) {
        case matcha::fw::ErrorCode::Ok:               return NYAN_OK;
        case matcha::fw::ErrorCode::NotFound:          return NYAN_ERR_NOT_FOUND;
        case matcha::fw::ErrorCode::AlreadyExists:     return NYAN_ERR_ALREADY_EXISTS;
        case matcha::fw::ErrorCode::InvalidArgument:   return NYAN_ERR_INVALID_ARGUMENT;
        case matcha::fw::ErrorCode::StaleHandle:       return NYAN_ERR_STALE_HANDLE;
        case matcha::fw::ErrorCode::PluginLoadFailed:  return NYAN_ERR_PLUGIN_LOAD_FAILED;
        case matcha::fw::ErrorCode::Timeout:           return NYAN_ERR_INTERNAL;
        case matcha::fw::ErrorCode::AccessDenied:      return NYAN_ERR_INTERNAL;
        case matcha::fw::ErrorCode::Cancelled:         return NYAN_ERR_INTERNAL;
    }
    return NYAN_ERR_INTERNAL;
}

auto DupString(const std::string& s) -> char*
{
    auto* buf = new char[s.size() + 1]; // NOLINT
    std::memcpy(buf, s.c_str(), s.size() + 1);
    return buf;
}

} // anonymous namespace

// ============================================================================
// Section 3: Memory Management
// ============================================================================

extern "C" {

NYAN_API void NyanString_Free(const char* str)
{
    delete[] str; // NOLINT
}

NYAN_API void NyanArray_Free(void* arr, int /*count*/)
{
    auto* p = static_cast<uint64_t*>(arr);
    delete[] p; // NOLINT
}

// ============================================================================
// Section 4: App Lifecycle
// ============================================================================

NYAN_API NyanErrorCode NyanApp_Create(const char* paletteDir,
                                      NyanAppHandle* outApp)
{
    if (outApp == nullptr) { return NYAN_ERR_NULL_HANDLE; }

    try {
        auto* state = new AppState(); // NOLINT
        state->paletteDir = (paletteDir != nullptr) ? paletteDir : "";
        *outApp = FromApp(state);
        return NYAN_OK;
    } catch (...) {
        *outApp = nullptr;
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanApp_Destroy(NyanAppHandle* app)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (*app == nullptr) { return NYAN_OK; } // Idempotent: null -> no-op

    auto* state = ToApp(*app);
    if (state->generation == 0) {
        return NYAN_ERR_STALE_HANDLE; // Already destroyed
    }

    state->generation = 0; // Mark as destroyed
    delete state; // NOLINT
    *app = nullptr;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanApp_Initialize(NyanAppHandle app,
                                          int argc, char** argv)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }

    try {
        // Ensure QApplication exists before creating QObject-derived types.
        if (QApplication::instance() == nullptr) {
            state->storedArgc = argc;
            new QApplication(state->storedArgc, argv); // NOLINT(cppcoreguidelines-owning-memory)
        }
        // Lazily create Theme + Application.
        if (!state->theme) {
            state->theme = std::make_unique<matcha::gui::NyanTheme>(
                QString::fromUtf8(state->paletteDir.c_str()));
            state->theme->SetTheme(matcha::gui::kThemeLight);
            state->theme->RegisterIconDirectory(
                matcha::fw::kMatchaIconPrefix,
                QString::fromUtf8(state->paletteDir.c_str()) + QStringLiteral("/../Icons"));
        }
        if (!state->app) {
            matcha::gui::SetThemeService(state->theme.get());
            state->app = std::make_unique<matcha::fw::Application>();
        }
        // Application::Initialize will see QApplication exists and skip creating one.
        state->app->Initialize(argc, argv);
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanApp_Tick(NyanAppHandle app)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }

    try {
        state->app->Tick();
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanApp_ProcessEvents(NyanAppHandle app)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }

    try {
        state->app->ProcessEvents();
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanApp_ShouldClose(NyanAppHandle app,
                                           int* outShouldClose)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outShouldClose == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }

    try {
        *outShouldClose = state->app->ShouldClose() ? 1 : 0;
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanApp_Shutdown(NyanAppHandle app)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app) { return NYAN_OK; } // Not initialized, nothing to shut down

    try {
        state->app->Shutdown();
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanApp_GetShell(NyanAppHandle app,
                                        NyanShellHandle* outShell)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outShell == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app || !state->app->IsInitialized()) { return NYAN_ERR_NOT_INITIALIZED; }

    try {
        *outShell = FromShell(&state->app->GetShell());
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

// ============================================================================
// Section 5: Document Management
// ============================================================================

NYAN_API NyanErrorCode NyanDoc_Create(NyanAppHandle app,
                                      const char* name,
                                      uint64_t* outDocId)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (name == nullptr || outDocId == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    auto result = docMgr->CreateDocument(name);
    if (!result.has_value()) {
        return MapErrorCode(result.error());
    }
    *outDocId = result.value().value;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanDoc_Close(NyanAppHandle app,
                                     uint64_t docId)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    auto result = docMgr->CloseDocument(matcha::fw::DocumentId::From(docId));
    if (!result.has_value()) {
        return MapErrorCode(result.error());
    }
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanDoc_SwitchTo(NyanAppHandle app,
                                        uint64_t docId)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    auto result = docMgr->SwitchTo(matcha::fw::DocumentId::From(docId));
    if (!result.has_value()) {
        return MapErrorCode(result.error());
    }
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanDoc_ActiveId(NyanAppHandle app,
                                        uint64_t* outDocId)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outDocId == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    auto active = docMgr->ActiveDocument();
    if (!active.has_value()) {
        return NYAN_ERR_NOT_FOUND;
    }
    *outDocId = active.value().value;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanDoc_Count(NyanAppHandle app,
                                     int* outCount)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outCount == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    *outCount = static_cast<int>(docMgr->DocumentCount());
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanDoc_AllIds(NyanAppHandle app,
                                      uint64_t** outIds,
                                      int* outCount)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outIds == nullptr || outCount == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    auto docs = docMgr->AllDocuments();
    *outCount = static_cast<int>(docs.size());
    if (docs.empty()) {
        *outIds = nullptr;
        return NYAN_OK;
    }
    auto* arr = new uint64_t[docs.size()]; // NOLINT
    for (size_t i = 0; i < docs.size(); ++i) {
        arr[i] = docs[i].value;
    }
    *outIds = arr;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanDoc_CreatePage(NyanAppHandle app,
                                          uint64_t docId,
                                          uint64_t* outPageId)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outPageId == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    auto result = docMgr->CreateDocumentPage(matcha::fw::DocumentId::From(docId));
    if (!result.has_value()) {
        return MapErrorCode(result.error());
    }
    *outPageId = result.value().value;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanDoc_ClosePage(NyanAppHandle app,
                                         uint64_t pageId)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }

    auto* state = ToApp(app);
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }
    auto docMgr = state->app->GetDocumentManager();
    if (!docMgr) { return NYAN_ERR_NOT_FOUND; }

    auto result = docMgr->CloseDocumentPage(matcha::fw::PageId::From(pageId));
    if (!result.has_value()) {
        return MapErrorCode(result.error());
    }
    return NYAN_OK;
}

// ============================================================================
// Section 6: ActionBar
// ============================================================================

NYAN_API NyanErrorCode NyanActionBar_TabCount(NyanShellHandle shell,
                                              int* outCount)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outCount == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto ab = sh->GetActionBar();
    if (!ab) { return NYAN_ERR_NOT_FOUND; }

    *outCount = ab->TabCount();
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanActionBar_SwitchTab(NyanShellHandle shell,
                                               const char* tabId)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (tabId == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto ab = sh->GetActionBar();
    if (!ab) { return NYAN_ERR_NOT_FOUND; }

    ab->SwitchTab(tabId);
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanActionBar_HasTab(NyanShellHandle shell,
                                            const char* tabId,
                                            int* outExists)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (tabId == nullptr || outExists == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto ab = sh->GetActionBar();
    if (!ab) { return NYAN_ERR_NOT_FOUND; }

    *outExists = ab->HasTab(tabId) ? 1 : 0;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanActionBar_CurrentTabId(NyanShellHandle shell,
                                                  const char** outTabId)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outTabId == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto ab = sh->GetActionBar();
    if (!ab) { return NYAN_ERR_NOT_FOUND; }

    auto id = ab->CurrentTabId();
    *outTabId = DupString(id);
    return NYAN_OK;
}

// ============================================================================
// Section 7: StatusBar (item-based)
// ============================================================================

NYAN_API NyanErrorCode NyanStatusBar_AddLabel(NyanShellHandle shell,
                                              const char* id,
                                              const char* text,
                                              int side)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (id == nullptr || text == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto sb = sh->GetStatusBar();
    if (!sb) { return NYAN_ERR_NOT_FOUND; }

    auto s = (side == 0) ? matcha::gui::StatusBarSide::Left
                         : matcha::gui::StatusBarSide::Right;
    auto* item = sb->AddLabel(id, text, s);
    return (item != nullptr) ? NYAN_OK : NYAN_ERR_INVALID_ARGUMENT;
}

NYAN_API NyanErrorCode NyanStatusBar_AddProgress(NyanShellHandle shell,
                                                 const char* id,
                                                 int side)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (id == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto sb = sh->GetStatusBar();
    if (!sb) { return NYAN_ERR_NOT_FOUND; }

    auto s = (side == 0) ? matcha::gui::StatusBarSide::Left
                         : matcha::gui::StatusBarSide::Right;
    auto* item = sb->AddProgress(id, s);
    return (item != nullptr) ? NYAN_OK : NYAN_ERR_INVALID_ARGUMENT;
}

NYAN_API NyanErrorCode NyanStatusBar_RemoveItem(NyanShellHandle shell,
                                                const char* id)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (id == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto sb = sh->GetStatusBar();
    if (!sb) { return NYAN_ERR_NOT_FOUND; }

    return sb->RemoveItem(id) ? NYAN_OK : NYAN_ERR_NOT_FOUND;
}

NYAN_API NyanErrorCode NyanStatusBar_SetItemText(NyanShellHandle shell,
                                                 const char* id,
                                                 const char* text)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (id == nullptr || text == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto sb = sh->GetStatusBar();
    if (!sb) { return NYAN_ERR_NOT_FOUND; }

    auto* item = sb->FindItem(id);
    if (item == nullptr) { return NYAN_ERR_NOT_FOUND; }

    item->SetText(text);
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanStatusBar_SetItemProgress(NyanShellHandle shell,
                                                     const char* id,
                                                     int percent)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (id == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto sb = sh->GetStatusBar();
    if (!sb) { return NYAN_ERR_NOT_FOUND; }

    auto* item = sb->FindItem(id);
    if (item == nullptr) { return NYAN_ERR_NOT_FOUND; }

    item->SetValue(percent);
    return NYAN_OK;
}

// ============================================================================
// Section 8: Theme
// ============================================================================

NYAN_API NyanErrorCode NyanTheme_SetTheme(NyanAppHandle app,
                                          const char* themeName)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (themeName == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }

    try {
        state->app->Theme().SetTheme(QString::fromUtf8(themeName));
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanTheme_CurrentName(NyanAppHandle app,
                                             char* outBuf,
                                             int bufSize)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outBuf == nullptr || bufSize <= 0) { return NYAN_ERR_INVALID_ARGUMENT; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }

    try {
        const QByteArray utf8 = state->app->Theme().CurrentTheme().toUtf8();
        if (utf8.size() + 1 > bufSize) { return NYAN_ERR_BUFFER_TOO_SMALL; }
        std::memcpy(outBuf, utf8.constData(), static_cast<size_t>(utf8.size()) + 1);
        return NYAN_OK;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

NYAN_API NyanErrorCode NyanTheme_Register(NyanAppHandle app,
                                          const char* name,
                                          const char* jsonPath,
                                          int isDark)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (name == nullptr || jsonPath == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }
    if (!state->app) { return NYAN_ERR_NOT_INITIALIZED; }

    try {
        const auto mode = (isDark != 0)
            ? matcha::gui::ThemeMode::Dark
            : matcha::gui::ThemeMode::Light;
        const bool ok = state->app->Theme().RegisterTheme(
            QString::fromUtf8(name), QString::fromUtf8(jsonPath), mode);
        return ok ? NYAN_OK : NYAN_ERR_INVALID_ARGUMENT;
    } catch (...) {
        return NYAN_ERR_INTERNAL;
    }
}

} // extern "C"

// ============================================================================
// Section 9: Notification Listener (C++ helpers)
// ============================================================================

namespace {

struct NotifListenerEntry {
    matcha::CallbackId cbId;
    matcha::EventNode* publisher;
};

struct NotifListenerRegistry {
    std::unordered_map<uint64_t, NotifListenerEntry> entries;
    uint64_t nextId = 1;
};

auto GetListenerRegistry() -> NotifListenerRegistry& {
    static NotifListenerRegistry reg;
    return reg;
}

/// @brief Query a single int64 field from a Notification by field name.
auto QueryNotifField(matcha::Notification& notif,
                     const char* fieldName,
                     int64_t* outValue) -> NyanErrorCode
{
    using namespace matcha::fw;
    std::string_view f(fieldName);

    // DocumentCreated
    if (auto* n = notif.As<DocumentCreated>()) {
        if (f == "docId") { *outValue = static_cast<int64_t>(n->DocId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // DocumentSwitched
    if (auto* n = notif.As<DocumentSwitched>()) {
        if (f == "docId") { *outValue = static_cast<int64_t>(n->DocId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // DocumentClosing
    if (auto* n = notif.As<DocumentClosing>()) {
        if (f == "docId") { *outValue = static_cast<int64_t>(n->DocId().value); return NYAN_OK; }
        if (f == "cancelled") { *outValue = n->IsCancelled() ? 1 : 0; return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // DocumentClosed
    if (auto* n = notif.As<DocumentClosed>()) {
        if (f == "docId") { *outValue = static_cast<int64_t>(n->DocId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // PageCreated
    if (auto* n = notif.As<PageCreated>()) {
        if (f == "pageId") { *outValue = static_cast<int64_t>(n->GetPageId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // PageRemoved
    if (auto* n = notif.As<PageRemoved>()) {
        if (f == "pageId") { *outValue = static_cast<int64_t>(n->GetPageId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // PageSwitched
    if (auto* n = notif.As<PageSwitched>()) {
        if (f == "pageId") { *outValue = static_cast<int64_t>(n->GetPageId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // ActiveVpChanged
    if (auto* n = notif.As<ActiveVpChanged>()) {
        if (f == "viewportId") { *outValue = static_cast<int64_t>(n->GetViewportId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // VpCreated
    if (auto* n = notif.As<VpCreated>()) {
        if (f == "newId") { *outValue = static_cast<int64_t>(n->NewId().value); return NYAN_OK; }
        if (f == "splitFromId") { *outValue = static_cast<int64_t>(n->SplitFrom().value); return NYAN_OK; }
        if (f == "direction") { *outValue = static_cast<int64_t>(n->Direction()); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // VpRemoved
    if (auto* n = notif.As<VpRemoved>()) {
        if (f == "viewportId") { *outValue = static_cast<int64_t>(n->RemovedId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // VpSwapped
    if (auto* n = notif.As<VpSwapped>()) {
        if (f == "viewportIdA") { *outValue = static_cast<int64_t>(n->IdA().value); return NYAN_OK; }
        if (f == "viewportIdB") { *outValue = static_cast<int64_t>(n->IdB().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // VpMoved
    if (auto* n = notif.As<VpMoved>()) {
        if (f == "viewportId") { *outValue = static_cast<int64_t>(n->GetViewportId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // VpMaximized
    if (auto* n = notif.As<VpMaximized>()) {
        if (f == "viewportId") { *outValue = static_cast<int64_t>(n->GetViewportId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }
    // VpRestored
    if (auto* n = notif.As<VpRestored>()) {
        if (f == "viewportId") { *outValue = static_cast<int64_t>(n->GetViewportId().value); return NYAN_OK; }
        return NYAN_ERR_NOT_FOUND;
    }

    return NYAN_ERR_NOT_FOUND;
}

} // anonymous namespace

extern "C" {

NYAN_API NyanErrorCode NyanNotification_Register(
    NyanShellHandle shell,
    NyanNotificationCallback callback,
    void* userData,
    uint64_t* outId)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (callback == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }
    if (outId == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    auto& reg = GetListenerRegistry();
    auto listenerId = reg.nextId++;

    // Subscribe on Shell (subscriber) for all notifications from any sender.
    // publisher=nullptr (wildcard), notifType="*" (wildcard).
    auto cbId = sh->Subscribe(nullptr, "*",
        [callback, userData](matcha::EventNode& /*sender*/, matcha::Notification& notif) {
            auto name = notif.ClassName();
            std::string nameStr(name);
            callback(nameStr.c_str(),
                     static_cast<NyanNotificationHandle>(&notif),
                     userData);
        });

    reg.entries.emplace(listenerId, NotifListenerEntry{cbId, sh});
    *outId = listenerId;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanNotification_Unregister(
    NyanShellHandle shell,
    uint64_t id)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }

    auto& reg = GetListenerRegistry();
    auto it = reg.entries.find(id);
    if (it == reg.entries.end()) { return NYAN_ERR_NOT_FOUND; }

    // Unsubscribe from the publisher
    it->second.publisher->Unsubscribe(it->second.cbId);
    reg.entries.erase(it);
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanNotification_GetInt(
    NyanNotificationHandle notif,
    const char* fieldName,
    int64_t* outValue)
{
    if (notif == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (fieldName == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }
    if (outValue == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* n = const_cast<matcha::Notification*>(
        static_cast<const matcha::Notification*>(notif));
    return QueryNotifField(*n, fieldName, outValue);
}

NYAN_API NyanErrorCode NyanNotification_SetCancel(
    NyanNotificationHandle notif)
{
    if (notif == nullptr) { return NYAN_ERR_NULL_HANDLE; }

    auto* n = const_cast<matcha::Notification*>(
        static_cast<const matcha::Notification*>(notif));
    auto* closing = n->As<matcha::fw::DocumentClosing>();
    if (closing == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }
    closing->SetCancel(true);
    return NYAN_OK;
}

// ============================================================================
// Section 10: Viewport
// ============================================================================

NYAN_API NyanErrorCode NyanViewport_Count(NyanShellHandle shell,
                                          int* outCount)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (outCount == nullptr) { return NYAN_ERR_INVALID_ARGUMENT; }

    // Count all Viewport descendants across all windows
    auto* sh = ToShell(shell);
    int count = 0;
    for (size_t i = 0; i < sh->NodeCount(); ++i) {
        auto* win = sh->NodeAt(i);
        if (win == nullptr) { continue; }
        for (auto* node : win->DescendantsOfType(matcha::fw::NodeType::Viewport)) {
            (void)node;
            ++count;
        }
    }
    *outCount = count;
    return NYAN_OK;
}

NYAN_API NyanErrorCode NyanViewport_RequestFrame(NyanShellHandle shell,
                                                 int viewportIndex)
{
    if (shell == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    if (viewportIndex < 0) { return NYAN_ERR_INVALID_ARGUMENT; }

    auto* sh = ToShell(shell);
    int idx = 0;
    for (size_t i = 0; i < sh->NodeCount(); ++i) {
        auto* win = sh->NodeAt(i);
        if (win == nullptr) { continue; }
        for (auto* node : win->DescendantsOfType(matcha::fw::NodeType::Viewport)) {
            if (idx == viewportIndex) {
                auto* vp = dynamic_cast<matcha::fw::Viewport*>(node);
                if (vp != nullptr) { vp->RequestFrame(); }
                return NYAN_OK;
            }
            ++idx;
        }
    }
    return NYAN_ERR_NOT_FOUND;
}

// ============================================================================
// Section 11: Handle Invalidation (ADR-019)
// ============================================================================

NYAN_API NyanErrorCode NyanHandle_OnInvalidated(
    NyanAppHandle app,
    NyanInvalidationCallback callback,
    void* userData)
{
    if (app == nullptr) { return NYAN_ERR_NULL_HANDLE; }
    auto* state = ToApp(app);
    if (state->generation == 0) { return NYAN_ERR_STALE_HANDLE; }

    state->invalidationCb = callback;
    state->invalidationUserData = userData;
    return NYAN_OK;
}

} // extern "C"
