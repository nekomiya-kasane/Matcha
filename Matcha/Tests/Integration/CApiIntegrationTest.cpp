#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/CApi/NyanCApi.h>

#include <cstring>

#ifndef MATCHA_TEST_PALETTE_DIR
#define MATCHA_TEST_PALETTE_DIR ""
#endif

// ============================================================================
// Full C ABI lifecycle (requires QApplication from integration test main)
// ============================================================================

TEST_CASE("CApi Integration: full lifecycle round-trip") {
    NyanAppHandle app = nullptr;
    auto err = NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(err == NYAN_OK);
    REQUIRE(app != nullptr);

    int argc = 0;
    err = NyanApp_Initialize(app, argc, nullptr);
    CHECK(err == NYAN_OK);

    // Shell access
    NyanShellHandle shell = nullptr;
    err = NyanApp_GetShell(app, &shell);
    CHECK(err == NYAN_OK);
    CHECK(shell != nullptr);

    // Tick / ProcessEvents
    CHECK(NyanApp_Tick(app) == NYAN_OK);
    CHECK(NyanApp_ProcessEvents(app) == NYAN_OK);

    // ShouldClose
    int shouldClose = -1;
    CHECK(NyanApp_ShouldClose(app, &shouldClose) == NYAN_OK);
    CHECK(shouldClose == 0);

    // ActionBar
    int tabCount = -1;
    CHECK(NyanActionBar_TabCount(shell, &tabCount) == NYAN_OK);
    CHECK(tabCount >= 0);

    // StatusBar (item-based)
    CHECK(NyanStatusBar_AddLabel(shell, "msg", "Hello C ABI", 0) == NYAN_OK);
    CHECK(NyanStatusBar_AddProgress(shell, "prog", 1) == NYAN_OK);
    CHECK(NyanStatusBar_SetItemText(shell, "msg", "Updated") == NYAN_OK);
    CHECK(NyanStatusBar_SetItemProgress(shell, "prog", 42) == NYAN_OK);
    CHECK(NyanStatusBar_RemoveItem(shell, "prog") == NYAN_OK);

    // Theme (string-based)
    char themeBuf[64] = {};
    CHECK(NyanTheme_CurrentName(app, themeBuf, sizeof(themeBuf)) == NYAN_OK);
    CHECK(std::strcmp(themeBuf, "Light") == 0);

    CHECK(NyanTheme_SetTheme(app, "Dark") == NYAN_OK);
    NyanTheme_CurrentName(app, themeBuf, sizeof(themeBuf));
    CHECK(std::strcmp(themeBuf, "Dark") == 0);

    CHECK(NyanTheme_SetTheme(app, "Light") == NYAN_OK);

    // Theme null
    CHECK(NyanTheme_SetTheme(app, nullptr) == NYAN_ERR_INVALID_ARGUMENT);

    // Doc null out-params (doc functions now take NyanAppHandle)
    CHECK(NyanDoc_Create(app, "test", nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanDoc_Create(app, nullptr, nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanDoc_ActiveId(app, nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanDoc_Count(app, nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanDoc_AllIds(app, nullptr, nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanDoc_CreatePage(app, 1, nullptr) == NYAN_ERR_INVALID_ARGUMENT);

    // Notification null callback
    uint64_t notifId = 0;
    CHECK(NyanNotification_Register(shell, nullptr, nullptr, &notifId) == NYAN_ERR_INVALID_ARGUMENT);

    // Viewport
    int vpCount = -1;
    CHECK(NyanViewport_Count(shell, &vpCount) == NYAN_OK);
    CHECK(vpCount >= 0);
    CHECK(NyanViewport_RequestFrame(shell, 9999) == NYAN_ERR_NOT_FOUND);
    CHECK(NyanViewport_RequestFrame(shell, -1) == NYAN_ERR_INVALID_ARGUMENT);

    // ActionBar null params
    CHECK(NyanActionBar_TabCount(shell, nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanActionBar_SwitchTab(shell, nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanActionBar_HasTab(shell, nullptr, nullptr) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanActionBar_CurrentTabId(shell, nullptr) == NYAN_ERR_INVALID_ARGUMENT);

    // StatusBar null params
    CHECK(NyanStatusBar_AddLabel(shell, nullptr, "x", 0) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanStatusBar_AddLabel(shell, "x", nullptr, 0) == NYAN_ERR_INVALID_ARGUMENT);
    CHECK(NyanStatusBar_RemoveItem(shell, nullptr) == NYAN_ERR_INVALID_ARGUMENT);

    // Shutdown + Destroy
    CHECK(NyanApp_Shutdown(app) == NYAN_OK);
    CHECK(NyanApp_Destroy(&app) == NYAN_OK);
    CHECK(app == nullptr);
}
