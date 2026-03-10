/**
 * @file NyanCadDocumentTest.cpp
 * @brief Integration tests for NyanCadDocumentHost + DocumentManager lifecycle.
 */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/Services/DocumentManager.h"

// NyanCadDocument is in the demo, not the library -- include directly
#include "NyanCadDocument.h"

TEST_SUITE("NyanCadDocument") {

TEST_CASE("Create document allocates per-doc state") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost host;
    docMgr.SetParent(&host);

    auto result = host.CreateDocument(docMgr, "TestDoc");
    REQUIRE(result.has_value());

    CHECK(host.DocumentCount() == 1);
    auto* state = host.GetState(result.value());
    REQUIRE(state != nullptr);
    CHECK(state->name == "TestDoc");
    CHECK(state->modified == false);
    CHECK(state->entityCount == 0);
}

TEST_CASE("Create multiple documents") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost host;
    docMgr.SetParent(&host);

    auto r1 = host.CreateDocument(docMgr, "Doc_A");
    auto r2 = host.CreateDocument(docMgr, "Doc_B");
    auto r3 = host.CreateDocument(docMgr, "Doc_C");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());
    REQUIRE(r3.has_value());

    CHECK(host.DocumentCount() == 3);
    CHECK(host.GetState(r1.value()) != nullptr);
    CHECK(host.GetState(r2.value()) != nullptr);
    CHECK(host.GetState(r3.value()) != nullptr);
}

TEST_CASE("Close document cleans up per-doc state") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost host;
    docMgr.SetParent(&host);

    auto r1 = host.CreateDocument(docMgr, "ToClose");
    REQUIRE(r1.has_value());
    CHECK(host.DocumentCount() == 1);

    auto closeResult = docMgr.CloseDocument(r1.value());
    REQUIRE(closeResult.has_value());
    CHECK(host.DocumentCount() == 0);
    CHECK(host.GetState(r1.value()) == nullptr);
}

TEST_CASE("Switch document updates active state") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost host;
    docMgr.SetParent(&host);

    auto r1 = host.CreateDocument(docMgr, "First");
    auto r2 = host.CreateDocument(docMgr, "Second");
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());

    // Active should be the last created
    auto* active = host.ActiveState();
    REQUIRE(active != nullptr);

    // Switch to first
    auto switchResult = docMgr.SwitchTo(r1.value());
    REQUIRE(switchResult.has_value());
    active = host.ActiveState();
    REQUIRE(active != nullptr);
    CHECK(active->name == "First");
}

TEST_CASE("Modified flag round-trip") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost host;
    docMgr.SetParent(&host);

    auto r1 = host.CreateDocument(docMgr, "ModTest");
    REQUIRE(r1.has_value());

    auto* state = host.GetState(r1.value());
    REQUIRE(state != nullptr);
    CHECK(state->modified == false);

    state->modified = true;
    CHECK(state->modified == true);

    state->modified = false;
    CHECK(state->modified == false);
}

TEST_CASE("CreateInitialDocuments creates Bracket and Housing") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost host;
    docMgr.SetParent(&host);
    host.CreateInitialDocuments(docMgr);

    CHECK(host.DocumentCount() == 2);
}

} // TEST_SUITE
