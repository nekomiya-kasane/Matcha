/**
 * @file NyanCApi.h
 * @brief Stable C ABI for the Matcha/Nyan GUI framework.
 *
 * Flat C API covering: App lifecycle, Document management, ActionBar,
 * StatusBar, Theme, Signal callbacks, Viewport, Handle invalidation,
 * and Memory management.
 *
 * Naming convention:
 *   Functions:  Nyan<Module>_<Method>  (PascalCase)
 *   Parameters: camelCase
 *   Enums/Macros: SCREAMING_CASE
 *
 * All functions return NyanErrorCode. Output values via out-parameters.
 * Ownership: [transfer] = caller owns, [borrow] = framework owns.
 * _Take/_Release suffix for ownership transfer functions.
 *
 * @see docs/05_Greenfield_Plan.md Phase 7
 */

#ifndef NYAN_CAPI_H
#define NYAN_CAPI_H

/* ---- Platform export macro ---- */

#if defined(MATCHA_LIB_EXPORT)
    #if defined(_MSC_VER)
        #define NYAN_API __declspec(dllexport)
    #elif defined(__GNUC__) || defined(__clang__)
        #define NYAN_API __attribute__((visibility("default")))
    #else
        #define NYAN_API
    #endif
#else
    #if defined(_MSC_VER)
        #define NYAN_API __declspec(dllimport)
    #else
        #define NYAN_API
    #endif
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Section 1: Error Codes
 * ========================================================================= */

typedef enum NyanErrorCode {
    NYAN_OK                    = 0,
    NYAN_ERR_NOT_FOUND         = 1,
    NYAN_ERR_ALREADY_EXISTS    = 2,
    NYAN_ERR_INVALID_ARGUMENT  = 3,
    NYAN_ERR_STALE_HANDLE      = 4,
    NYAN_ERR_PLUGIN_LOAD_FAILED = 5,
    NYAN_ERR_NULL_HANDLE       = 6,
    NYAN_ERR_NOT_INITIALIZED   = 7,
    NYAN_ERR_BUFFER_TOO_SMALL  = 8,
    NYAN_ERR_INTERNAL          = 99
} NyanErrorCode;

/* =========================================================================
 * Section 2: Opaque Handle Types
 * ========================================================================= */

typedef struct NyanAppHandle_T*        NyanAppHandle;
typedef struct NyanShellHandle_T*      NyanShellHandle;
typedef struct NyanDocManagerHandle_T* NyanDocManagerHandle;

/* =========================================================================
 * Section 3: Memory Management
 * ========================================================================= */

/**
 * @brief Free a string returned by the C ABI. [transfer-back]
 * @param str String to free. Null is a no-op.
 */
NYAN_API void NyanString_Free(const char* str);

/**
 * @brief Free an array returned by the C ABI. [transfer-back]
 * @param arr Array pointer to free. Null is a no-op.
 * @param count Number of elements (unused, reserved for future validation).
 */
NYAN_API void NyanArray_Free(void* arr, int count);

/* =========================================================================
 * Section 4: App Lifecycle
 * ========================================================================= */

/**
 * @brief Create a new Application instance. [transfer]
 * @param paletteDir Path to theme palette directory (UTF-8). [borrow]
 * @param outApp Receives the new application handle.
 * @return NYAN_OK on success.
 */
NYAN_API NyanErrorCode NyanApp_Create(const char* paletteDir,
                                      NyanAppHandle* outApp);

/**
 * @brief Destroy an Application instance. Idempotent (ADR-019).
 *
 * Null handle -> NYAN_OK (no-op).
 * Already-destroyed handle -> NYAN_ERR_STALE_HANDLE.
 *
 * @param app Application handle. Set to null after destroy.
 * @return NYAN_OK or NYAN_ERR_STALE_HANDLE.
 */
NYAN_API NyanErrorCode NyanApp_Destroy(NyanAppHandle* app);

/**
 * @brief Initialize the application (creates QApplication + main window).
 * @param app Application handle. [borrow]
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return NYAN_OK on success.
 */
NYAN_API NyanErrorCode NyanApp_Initialize(NyanAppHandle app,
                                          int argc, char** argv);

/**
 * @brief Process pending Qt events + flush dirty viewports.
 * @param app Application handle. [borrow]
 */
NYAN_API NyanErrorCode NyanApp_Tick(NyanAppHandle app);

/**
 * @brief Process pending Qt events only.
 * @param app Application handle. [borrow]
 */
NYAN_API NyanErrorCode NyanApp_ProcessEvents(NyanAppHandle app);

/**
 * @brief Check if the main window close event has been received.
 * @param app Application handle. [borrow]
 * @param outShouldClose Receives 1 if should close, 0 otherwise.
 */
NYAN_API NyanErrorCode NyanApp_ShouldClose(NyanAppHandle app,
                                           int* outShouldClose);

/**
 * @brief Shut down the application (destroy windows + QApplication).
 * @param app Application handle. [borrow]
 */
NYAN_API NyanErrorCode NyanApp_Shutdown(NyanAppHandle app);

/**
 * @brief Get the Shell handle from an Application.
 * @param app Application handle. [borrow]
 * @param outShell Receives the shell handle. [borrow]
 */
NYAN_API NyanErrorCode NyanApp_GetShell(NyanAppHandle app,
                                        NyanShellHandle* outShell);

/* =========================================================================
 * Section 5: Document Management
 * ========================================================================= */

/**
 * @brief Create a new document.
 * @param app Application handle. [borrow]
 * @param name Document name (UTF-8). [borrow]
 * @param outDocId Receives the new document ID.
 */
NYAN_API NyanErrorCode NyanDoc_Create(NyanAppHandle app,
                                      const char* name,
                                      uint64_t* outDocId);

/**
 * @brief Close a document (closes all its pages).
 * @param app Application handle. [borrow]
 * @param docId Document to close.
 */
NYAN_API NyanErrorCode NyanDoc_Close(NyanAppHandle app,
                                     uint64_t docId);

/**
 * @brief Switch the active document.
 * @param app Application handle. [borrow]
 * @param docId Document to make active.
 */
NYAN_API NyanErrorCode NyanDoc_SwitchTo(NyanAppHandle app,
                                        uint64_t docId);

/**
 * @brief Get the currently active document ID.
 * @param app Application handle. [borrow]
 * @param outDocId Receives the active document ID.
 * @return NYAN_ERR_NOT_FOUND if no document is active.
 */
NYAN_API NyanErrorCode NyanDoc_ActiveId(NyanAppHandle app,
                                        uint64_t* outDocId);

/**
 * @brief Get the number of open documents.
 * @param app Application handle. [borrow]
 * @param outCount Receives the document count.
 */
NYAN_API NyanErrorCode NyanDoc_Count(NyanAppHandle app,
                                     int* outCount);

/**
 * @brief Get all open document IDs. [transfer]
 * @param app Application handle. [borrow]
 * @param outIds Receives a caller-owned array of document IDs.
 *               Free with NyanArray_Free(outIds, outCount).
 * @param outCount Receives the number of IDs.
 */
NYAN_API NyanErrorCode NyanDoc_AllIds(NyanAppHandle app,
                                      uint64_t** outIds,
                                      int* outCount);

/**
 * @brief Create an additional DocumentPage for an existing document.
 * @param app Application handle. [borrow]
 * @param docId Document to create a page for.
 * @param outPageId Receives the new page ID.
 */
NYAN_API NyanErrorCode NyanDoc_CreatePage(NyanAppHandle app,
                                          uint64_t docId,
                                          uint64_t* outPageId);

/**
 * @brief Close a specific DocumentPage.
 * @param app Application handle. [borrow]
 * @param pageId Page to close.
 */
NYAN_API NyanErrorCode NyanDoc_ClosePage(NyanAppHandle app,
                                         uint64_t pageId);

/* =========================================================================
 * Section 6: ActionBar
 * ========================================================================= */

/**
 * @brief Get the number of ActionBar tabs.
 * @param shell Shell handle. [borrow]
 * @param outCount Receives the tab count.
 */
NYAN_API NyanErrorCode NyanActionBar_TabCount(NyanShellHandle shell,
                                              int* outCount);

/**
 * @brief Switch to a tab by string ID.
 * @param shell Shell handle. [borrow]
 * @param tabId Tab identifier (UTF-8). [borrow]
 */
NYAN_API NyanErrorCode NyanActionBar_SwitchTab(NyanShellHandle shell,
                                               const char* tabId);

/**
 * @brief Check if a tab exists.
 * @param shell Shell handle. [borrow]
 * @param tabId Tab identifier (UTF-8). [borrow]
 * @param outExists Receives 1 if tab exists, 0 otherwise.
 */
NYAN_API NyanErrorCode NyanActionBar_HasTab(NyanShellHandle shell,
                                            const char* tabId,
                                            int* outExists);

/**
 * @brief Get the current (active) tab ID. [transfer]
 * @param shell Shell handle. [borrow]
 * @param outTabId Receives a caller-owned string. Free with NyanString_Free.
 */
NYAN_API NyanErrorCode NyanActionBar_CurrentTabId(NyanShellHandle shell,
                                                  const char** outTabId);

/* =========================================================================
 * Section 7: StatusBar (item-based)
 * ========================================================================= */

/**
 * @brief Add a label item to the StatusBar.
 * @param shell Shell handle. [borrow]
 * @param id Unique item id (UTF-8). [borrow]
 * @param text Initial text (UTF-8). [borrow]
 * @param side 0 = Left, 1 = Right.
 */
NYAN_API NyanErrorCode NyanStatusBar_AddLabel(NyanShellHandle shell,
                                              const char* id,
                                              const char* text,
                                              int side);

/**
 * @brief Add a progress item to the StatusBar.
 * @param shell Shell handle. [borrow]
 * @param id Unique item id (UTF-8). [borrow]
 * @param side 0 = Left, 1 = Right.
 */
NYAN_API NyanErrorCode NyanStatusBar_AddProgress(NyanShellHandle shell,
                                                 const char* id,
                                                 int side);

/**
 * @brief Remove a StatusBar item by id.
 * @param shell Shell handle. [borrow]
 * @param id Item id (UTF-8). [borrow]
 */
NYAN_API NyanErrorCode NyanStatusBar_RemoveItem(NyanShellHandle shell,
                                                const char* id);

/**
 * @brief Set text on a StatusBar label item.
 * @param shell Shell handle. [borrow]
 * @param id Item id (UTF-8). [borrow]
 * @param text New text (UTF-8). [borrow]
 */
NYAN_API NyanErrorCode NyanStatusBar_SetItemText(NyanShellHandle shell,
                                                 const char* id,
                                                 const char* text);

/**
 * @brief Set value on a StatusBar progress item.
 * @param shell Shell handle. [borrow]
 * @param id Item id (UTF-8). [borrow]
 * @param percent Progress percentage (0-100).
 */
NYAN_API NyanErrorCode NyanStatusBar_SetItemProgress(NyanShellHandle shell,
                                                     const char* id,
                                                     int percent);

/* =========================================================================
 * Section 8: Theme
 * ========================================================================= */

/**
 * @brief Set the active theme by name.
 * @param app Application handle. [borrow]
 * @param themeName Theme name (UTF-8), e.g. "Light", "Dark", or a registered custom name.
 */
NYAN_API NyanErrorCode NyanTheme_SetTheme(NyanAppHandle app,
                                          const char* themeName);

/**
 * @brief Get the current theme name.
 * @param app Application handle. [borrow]
 * @param outBuf   Buffer to receive the theme name (UTF-8, null-terminated).
 * @param bufSize  Size of the buffer in bytes.
 * @return NYAN_OK on success, NYAN_ERR_BUFFER_TOO_SMALL if buffer is too small.
 */
NYAN_API NyanErrorCode NyanTheme_CurrentName(NyanAppHandle app,
                                             char* outBuf,
                                             int bufSize);

/**
 * @brief Register a custom theme.
 * @param app      Application handle. [borrow]
 * @param name     Unique theme name (UTF-8). [borrow]
 * @param jsonPath Absolute path to palette JSON file (UTF-8). [borrow]
 * @param isDark   Non-zero if the theme belongs to the Dark family.
 */
NYAN_API NyanErrorCode NyanTheme_Register(NyanAppHandle app,
                                          const char* name,
                                          const char* jsonPath,
                                          int isDark);

/* =========================================================================
 * Section 9: Notification Listener
 * ========================================================================= */

/**
 * @brief Opaque handle to a Notification received by a listener callback.
 *
 * Valid only within the callback invocation. Do not store.
 */
typedef const void* NyanNotificationHandle;

/**
 * @brief Notification listener callback signature.
 *
 * @param className  Notification class name (e.g. "DocumentCreated"). Null-terminated.
 * @param notif      Opaque handle for querying notification fields.
 * @param userData   Opaque pointer provided at registration.
 *
 * @note The notif handle is only valid during the callback invocation.
 */
typedef void (*NyanNotificationCallback)(const char* className,
                                         NyanNotificationHandle notif,
                                         void* userData);

/**
 * @brief Register a Notification listener on the command tree.
 *
 * The listener is installed as a parent of the target node (Shell by default),
 * receiving all Notifications that propagate upward through the tree.
 *
 * @param shell      Shell handle. [borrow]
 * @param callback   Listener function.
 * @param userData   Opaque pointer passed to each callback invocation.
 * @param outId      Receives the listener ID for later unregistration.
 * @return NYAN_OK on success.
 */
NYAN_API NyanErrorCode NyanNotification_Register(
    NyanShellHandle shell,
    NyanNotificationCallback callback,
    void* userData,
    uint64_t* outId);

/**
 * @brief Unregister a previously registered Notification listener.
 *
 * @param shell  Shell handle. [borrow]
 * @param id     Listener ID returned by NyanNotification_Register.
 * @return NYAN_OK on success, NYAN_ERR_NOT_FOUND if id is invalid.
 */
NYAN_API NyanErrorCode NyanNotification_Unregister(
    NyanShellHandle shell,
    uint64_t id);

/**
 * @brief Query an integer field from a Notification handle.
 *
 * Field names are Notification-type-specific:
 * - DocumentCreated/Closed/Switched: "docId"
 * - DocumentClosing: "docId", "cancelled" (read), use NyanNotification_SetCancel to veto
 * - PageCreated/Removed/Switched: "pageId"
 * - ActiveVpChanged/VpMoved/VpRemoved/VpMaximized/VpRestored: "viewportId"
 * - VpCreated: "newId", "splitFromId", "direction"
 * - VpSwapped: "viewportIdA", "viewportIdB"
 *
 * @param notif      Notification handle from callback.
 * @param fieldName  Field name to query.
 * @param outValue   Receives the field value.
 * @return NYAN_OK on success, NYAN_ERR_NOT_FOUND if field not available.
 */
NYAN_API NyanErrorCode NyanNotification_GetInt(
    NyanNotificationHandle notif,
    const char* fieldName,
    int64_t* outValue);

/**
 * @brief Cancel a vetoable Notification (e.g. DocumentClosing).
 *
 * @param notif  Notification handle from callback. Must be a vetoable type.
 * @return NYAN_OK on success, NYAN_ERR_INVALID_ARGUMENT if not vetoable.
 */
NYAN_API NyanErrorCode NyanNotification_SetCancel(
    NyanNotificationHandle notif);

/* =========================================================================
 * Section 10: Viewport
 * ========================================================================= */

/**
 * @brief Get the number of viewports in the active document page.
 * @param shell Shell handle. [borrow]
 * @param outCount Receives the viewport count.
 */
NYAN_API NyanErrorCode NyanViewport_Count(NyanShellHandle shell,
                                          int* outCount);

/**
 * @brief Request a frame render for a specific viewport.
 * @param shell Shell handle. [borrow]
 * @param viewportIndex 0-based viewport index.
 */
NYAN_API NyanErrorCode NyanViewport_RequestFrame(NyanShellHandle shell,
                                                 int viewportIndex);

/* =========================================================================
 * Section 11: Handle Invalidation (ADR-019)
 * ========================================================================= */

/**
 * @brief Callback type for handle invalidation.
 * @param handleId The ID of the invalidated handle.
 * @param userData Opaque pointer from registration.
 */
typedef void (*NyanInvalidationCallback)(uint64_t handleId, void* userData);

/**
 * @brief Register a callback for handle invalidation events.
 *
 * Called when a widget handle becomes invalid (e.g., parent destroyed
 * by Qt object tree). Used by GC language bindings to mark wrappers
 * as phantom, preventing finalizer segfaults.
 *
 * @param app Application handle. [borrow]
 * @param callback Invalidation callback function.
 * @param userData Opaque pointer passed to callback.
 */
NYAN_API NyanErrorCode NyanHandle_OnInvalidated(
    NyanAppHandle app,
    NyanInvalidationCallback callback,
    void* userData);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NYAN_CAPI_H */
