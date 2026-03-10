#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/CApi/NyanCApi.h>

#ifndef MATCHA_TEST_PALETTE_DIR
#define MATCHA_TEST_PALETTE_DIR ""
#endif

// ============================================================================
// App Lifecycle (no Initialize — avoids QApplication conflicts)
// ============================================================================

TEST_CASE("CApi: NyanApp_Create and Destroy round-trip") {
    NyanAppHandle app = nullptr;
    auto err = NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    CHECK(err == NYAN_OK);
    CHECK(app != nullptr);

    err = NyanApp_Destroy(&app);
    CHECK(err == NYAN_OK);
    CHECK(app == nullptr);
}

TEST_CASE("CApi: NyanApp_Create with null outApp") {
    auto err = NyanApp_Create(MATCHA_TEST_PALETTE_DIR, nullptr);
    CHECK(err == NYAN_ERR_NULL_HANDLE);
}

TEST_CASE("CApi: NyanApp_Destroy null handle is no-op") {
    NyanAppHandle app = nullptr;
    auto err = NyanApp_Destroy(&app);
    CHECK(err == NYAN_OK);
}

TEST_CASE("CApi: NyanApp_Destroy with null pointer-to-handle") {
    auto err = NyanApp_Destroy(nullptr);
    CHECK(err == NYAN_ERR_NULL_HANDLE);
}

TEST_CASE("CApi: NyanApp_Tick on null handle") {
    auto err = NyanApp_Tick(nullptr);
    CHECK(err == NYAN_ERR_NULL_HANDLE);
}

TEST_CASE("CApi: NyanApp_ProcessEvents on null handle") {
    auto err = NyanApp_ProcessEvents(nullptr);
    CHECK(err == NYAN_ERR_NULL_HANDLE);
}

TEST_CASE("CApi: NyanApp_ShouldClose on null handle") {
    int result = 0;
    auto err = NyanApp_ShouldClose(nullptr, &result);
    CHECK(err == NYAN_ERR_NULL_HANDLE);
}

TEST_CASE("CApi: NyanApp_ShouldClose with null out-param") {
    NyanAppHandle app = nullptr;
    NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(app != nullptr);

    auto err = NyanApp_ShouldClose(app, nullptr);
    CHECK(err == NYAN_ERR_INVALID_ARGUMENT);

    NyanApp_Destroy(&app);
}

TEST_CASE("CApi: NyanApp_GetShell before Initialize returns NOT_INITIALIZED") {
    NyanAppHandle app = nullptr;
    NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(app != nullptr);

    NyanShellHandle shell = nullptr;
    auto err = NyanApp_GetShell(app, &shell);
    CHECK(err == NYAN_ERR_NOT_INITIALIZED);
    CHECK(shell == nullptr);

    NyanApp_Destroy(&app);
}

TEST_CASE("CApi: NyanApp_GetShell with null out-param") {
    NyanAppHandle app = nullptr;
    NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(app != nullptr);

    auto err = NyanApp_GetShell(app, nullptr);
    CHECK(err == NYAN_ERR_INVALID_ARGUMENT);

    NyanApp_Destroy(&app);
}

// Full lifecycle + Shell-dependent tests are in Integration/CApiIntegrationTest.cpp
// (unit test process has no QApplication — Initialize would crash on teardown).

// ============================================================================
// Theme (no Initialize needed)
// ============================================================================

TEST_CASE("CApi: NyanTheme_CurrentName before Initialize returns NOT_INITIALIZED") {
    NyanAppHandle app = nullptr;
    NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(app != nullptr);

    char buf[64] = {};
    auto err = NyanTheme_CurrentName(app, buf, sizeof(buf));
    CHECK(err == NYAN_ERR_NOT_INITIALIZED);

    NyanApp_Destroy(&app);
}

TEST_CASE("CApi: NyanTheme_SetTheme before Initialize returns NOT_INITIALIZED") {
    NyanAppHandle app = nullptr;
    NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(app != nullptr);

    auto err = NyanTheme_SetTheme(app, "Light");
    CHECK(err == NYAN_ERR_NOT_INITIALIZED);

    NyanApp_Destroy(&app);
}

TEST_CASE("CApi: NyanTheme on null handle") {
    CHECK(NyanTheme_SetTheme(nullptr, "Light") == NYAN_ERR_NULL_HANDLE);
    char buf[64] = {};
    CHECK(NyanTheme_CurrentName(nullptr, buf, sizeof(buf)) == NYAN_ERR_NULL_HANDLE);
}

// ============================================================================
// Memory Management
// ============================================================================

TEST_CASE("CApi: NyanString_Free with null is no-op") {
    NyanString_Free(nullptr);
    CHECK(true);
}

TEST_CASE("CApi: NyanArray_Free with null is no-op") {
    NyanArray_Free(nullptr, 0);
    CHECK(true);
}

// ============================================================================
// ActionBar / StatusBar null-handle tests (no Shell needed)
// ============================================================================

TEST_CASE("CApi: NyanActionBar on null shell") {
    int count = 0;
    CHECK(NyanActionBar_TabCount(nullptr, &count) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanActionBar_SwitchTab(nullptr, "x") == NYAN_ERR_NULL_HANDLE);
    int exists = 0;
    CHECK(NyanActionBar_HasTab(nullptr, "x", &exists) == NYAN_ERR_NULL_HANDLE);
    const char* tabId = nullptr;
    CHECK(NyanActionBar_CurrentTabId(nullptr, &tabId) == NYAN_ERR_NULL_HANDLE);
}

TEST_CASE("CApi: NyanStatusBar on null shell") {
    CHECK(NyanStatusBar_AddLabel(nullptr, "x", "y", 0) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanStatusBar_AddProgress(nullptr, "x", 0) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanStatusBar_RemoveItem(nullptr, "x") == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanStatusBar_SetItemText(nullptr, "x", "y") == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanStatusBar_SetItemProgress(nullptr, "x", 50) == NYAN_ERR_NULL_HANDLE);
}
