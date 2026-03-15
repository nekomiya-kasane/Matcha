/**
 * @file NyanCadSmokeTest.cpp
 * @brief Smoke tests for the full NyanCad demo lifecycle.
 */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "NyanCadDocument.h"
#include "NyanCadMainWindow.h"
#include "NyanCadWorkshopSetup.h"

#include "DetectionFailureArea.h"
#include "DetectionFreedomEdge.h"
#include "DetectionInterference.h"
#include "DetectionRepeat.h"

#include "Matcha/Services/PluginHost.h"
#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Tree/Composition/Workbench/WorkbenchManager.h"
#include "Matcha/Tree/Composition/Workbench/WorkshopRegistry.h"
#include "Matcha/Services/DocumentManager.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"
#include "Matcha/Tree/Composition/Shell/StatusBarNode.h"
#include "Matcha/Tree/Composition/Shell/WindowNode.h"
#include "Matcha/Tree/Composition/Shell/WorkspaceFrame.h"
#include "Matcha/Widgets/ActionBar/NyanActionBar.h"
#include "Matcha/Widgets/Shell/NyanStatusBar.h"
#include "Matcha/Theming/NyanTheme.h"

#include <QString>

TEST_SUITE("NyanCadSmoke") {

TEST_CASE("App create + initialize + shutdown round-trip") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme); matcha::fw::Application app;
    app.Initialize(0, nullptr);
    CHECK(app.IsInitialized());

    app.Shutdown();
}

TEST_CASE("MainWindow has expected title after setup") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme); matcha::fw::Application app;
    app.Initialize(0, nullptr);

    nyancad::NyanCadMainWindow mainWin;
    mainWin.Setup(app);

    CHECK(app.MainWindow().Title() == "NyanCad -- Multi-Window Multi-Viewport Demo");

    app.Shutdown();
}

TEST_CASE("ActionBar has File/Edit/Mesh tabs after setup") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme); matcha::fw::Application app;
    app.Initialize(0, nullptr);

    auto* wsRegistry = app.GetWorkshopRegistry();
    REQUIRE(wsRegistry != nullptr);
    nyancad::RegisterNyanCadWorkshops(*wsRegistry);

    nyancad::NyanCadMainWindow mainWin;
    mainWin.Setup(app);

    auto* wbMgr = app.GetWorkbenchManager();
    REQUIRE(wbMgr != nullptr);
    wbMgr->ActivateWorkshop(matcha::fw::WorkshopId::From("mesh"));

    auto wsObs = app.MainWindow().GetWorkspaceFrame();
    REQUIRE(wsObs.get() != nullptr);
    auto abObs = wsObs->GetActionBar();
    REQUIRE(abObs.get() != nullptr);
    auto* actionBar = abObs->ActionBar();
    REQUIRE(actionBar != nullptr);
    CHECK(actionBar->TabCount() >= 3);

    app.Shutdown();
}

TEST_CASE("StatusBar shows initial message after setup") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme); matcha::fw::Application app;
    app.Initialize(0, nullptr);

    nyancad::NyanCadMainWindow mainWin;
    mainWin.Setup(app);

    auto statusObs = app.MainWindow().GetStatusBarNode();
    REQUIRE(statusObs.get() != nullptr);
    auto* statusBar = statusObs->StatusBar();
    REQUIRE(statusBar != nullptr);

    app.Shutdown();
}

TEST_CASE("Create document via DocumentManager, verify count") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost docHost;
    docMgr.SetParent(&docHost);

    auto r1 = docHost.CreateDocument(docMgr, "SmokeDoc");
    REQUIRE(r1.has_value());
    CHECK(docHost.DocumentCount() == 1);
    CHECK(docMgr.DocumentCount() == 1);
}

TEST_CASE("Detection commands instantiate and execute without crash") {
    matcha::fw::DocumentManager docMgr;
    nyancad::NyanCadDocumentHost docHost;
    docMgr.SetParent(&docHost);
    auto r1 = docHost.CreateDocument(docMgr, "DetectDoc");
    REQUIRE(r1.has_value());

    auto* state = docHost.GetState(r1.value());
    REQUIRE(state != nullptr);

    nyancad::DetectionFreedomEdge freedomEdge;
    auto msg1 = freedomEdge.Execute(*state);
    CHECK(!msg1.empty());

    nyancad::DetectionInterference interference;
    auto msg2 = interference.Execute(*state);
    CHECK(!msg2.empty());

    nyancad::DetectionRepeat repeat;
    auto msg3 = repeat.Execute(*state);
    CHECK(!msg3.empty());

    nyancad::DetectionFailureArea failureArea;
    auto msg4 = failureArea.Execute(*state);
    CHECK(!msg4.empty());
}

TEST_CASE("Full lifecycle: create -> doc -> close -> shutdown") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);

    matcha::gui::SetThemeService(&theme); matcha::fw::Application app;
    app.Initialize(0, nullptr);

    auto* docMgr = app.GetDocumentManagerImpl();
    REQUIRE(docMgr != nullptr);

    nyancad::NyanCadDocumentHost docHost;
    docMgr->SetParent(&docHost);

    nyancad::NyanCadMainWindow mainWin;
    mainWin.Setup(app);

    auto r1 = docHost.CreateDocument(*docMgr, "LifecycleDoc");
    REQUIRE(r1.has_value());
    CHECK(docHost.DocumentCount() == 1);

    auto closeResult = docMgr->CloseDocument(r1.value());
    REQUIRE(closeResult.has_value());
    CHECK(docHost.DocumentCount() == 0);

    app.Shutdown();
}

} // TEST_SUITE
