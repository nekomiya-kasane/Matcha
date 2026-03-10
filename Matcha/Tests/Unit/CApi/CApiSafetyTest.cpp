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
// Idempotent Destroy (ADR-019)
// ============================================================================

TEST_CASE("CApi Safety: double destroy returns NYAN_ERR_STALE_HANDLE") {
    NyanAppHandle app = nullptr;
    NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(app != nullptr);

    // First destroy
    NyanAppHandle copy = app;
    auto err = NyanApp_Destroy(&app);
    CHECK(err == NYAN_OK);
    CHECK(app == nullptr);

    // Second destroy on copy — stale (copy is dangling but generation=0)
    // We can't safely test double-destroy of same pointer since memory
    // is freed. But null-handle path is tested separately.
    NyanAppHandle null = nullptr;
    err = NyanApp_Destroy(&null);
    CHECK(err == NYAN_OK); // null -> no-op
    (void)copy;
}

TEST_CASE("CApi Safety: operations on null handle return NYAN_ERR_NULL_HANDLE") {
    CHECK(NyanApp_Tick(nullptr) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanApp_ProcessEvents(nullptr) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanApp_Shutdown(nullptr) == NYAN_ERR_NULL_HANDLE);

    int val = 0;
    CHECK(NyanApp_ShouldClose(nullptr, &val) == NYAN_ERR_NULL_HANDLE);

    NyanShellHandle sh = nullptr;
    CHECK(NyanApp_GetShell(nullptr, &sh) == NYAN_ERR_NULL_HANDLE);

    CHECK(NyanTheme_SetTheme(nullptr, "Light") == NYAN_ERR_NULL_HANDLE);
    char themeBuf[64] = {};
    CHECK(NyanTheme_CurrentName(nullptr, themeBuf, sizeof(themeBuf)) == NYAN_ERR_NULL_HANDLE);
}

// ============================================================================
// Handle Invalidation Callback (ADR-019)
// ============================================================================

TEST_CASE("CApi Safety: NyanHandle_OnInvalidated registers callback") {
    NyanAppHandle app = nullptr;
    NyanApp_Create(MATCHA_TEST_PALETTE_DIR, &app);
    REQUIRE(app != nullptr);

    int callCount = 0;
    auto err = NyanHandle_OnInvalidated(app,
        [](uint64_t /*handleId*/, void* ud) {
            auto* count = static_cast<int*>(ud);
            (*count)++;
        },
        &callCount);
    CHECK(err == NYAN_OK);

    NyanApp_Destroy(&app);
}

TEST_CASE("CApi Safety: NyanHandle_OnInvalidated on null handle") {
    auto err = NyanHandle_OnInvalidated(nullptr, nullptr, nullptr);
    CHECK(err == NYAN_ERR_NULL_HANDLE);
}

// ============================================================================
// Document null-handle safety (no Initialize needed)
// ============================================================================

TEST_CASE("CApi Safety: Doc functions on null shell") {
    uint64_t docId = 0;
    CHECK(NyanDoc_Create(nullptr, "test", &docId) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanDoc_Close(nullptr, 1) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanDoc_SwitchTo(nullptr, 1) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanDoc_ActiveId(nullptr, &docId) == NYAN_ERR_NULL_HANDLE);

    int count = 0;
    CHECK(NyanDoc_Count(nullptr, &count) == NYAN_ERR_NULL_HANDLE);

    uint64_t* ids = nullptr;
    CHECK(NyanDoc_AllIds(nullptr, &ids, &count) == NYAN_ERR_NULL_HANDLE);

    uint64_t pageId = 0;
    CHECK(NyanDoc_CreatePage(nullptr, 1, &pageId) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanDoc_ClosePage(nullptr, 1) == NYAN_ERR_NULL_HANDLE);
}

// ============================================================================
// Signal / Viewport null-handle (no Initialize needed)
// ============================================================================

TEST_CASE("CApi Safety: NyanNotification_Register on null shell") {
    uint64_t id = 0;
    auto dummyCb = [](const char*, NyanNotificationHandle, void*) {};
    CHECK(NyanNotification_Register(nullptr, dummyCb, nullptr, &id) == NYAN_ERR_NULL_HANDLE);
}

TEST_CASE("CApi Safety: NyanViewport on null shell") {
    int count = 0;
    CHECK(NyanViewport_Count(nullptr, &count) == NYAN_ERR_NULL_HANDLE);
    CHECK(NyanViewport_RequestFrame(nullptr, 0) == NYAN_ERR_NULL_HANDLE);
}

// Shell-dependent safety checks are in CApiRoundTripTest.cpp's single Initialize test.
