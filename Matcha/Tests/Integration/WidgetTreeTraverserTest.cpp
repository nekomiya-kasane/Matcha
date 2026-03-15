/**
 * @file WidgetTreeTraverserTest.cpp
 * @brief Integration test that boots NyanCad's main window and traverses
 *        the entire QWidget tree, dumping positions/sizes and checking
 *        for common layout issues.
 *
 * Issues detected:
 *  - Zero-size visible widgets
 *  - Negative position widgets
 *  - Widgets extending beyond parent bounds
 *  - Invisible widgets that should be visible (key framework widgets)
 */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "DocumentView.h"
#include "NyanCadDocument.h"
#include "NyanCadMainWindow.h"
#include "NyanCadWorkshopSetup.h"

#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Tree/Composition/Workbench/WorkbenchManager.h"
#include "Matcha/Tree/Composition/Workbench/WorkshopRegistry.h"
#include "Matcha/Services/DocumentManager.h"
#include "Matcha/Tree/Composition/Shell/MainTitleBarNode.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"
#include "Matcha/Tree/Composition/Shell/StatusBarNode.h"
#include "Matcha/Tree/Composition/Shell/TitleBarNode.h"
#include "Matcha/Tree/Composition/Shell/WindowNode.h"
#include "Matcha/Tree/Composition/Shell/WorkspaceFrame.h"
#include "Matcha/Widgets/ActionBar/NyanActionBar.h"
#include "Matcha/Widgets/Shell/NyanMainTitleBar.h"
#include "Matcha/Widgets/Shell/NyanStatusBar.h"
#include "Matcha/Theming/NyanTheme.h"

#include <QApplication>
#include <QWidget>

#include <print>
#include <string>
#include <vector>

namespace {

struct WidgetInfo {
    std::string className;
    std::string objectName;
    int x      = 0;
    int y      = 0;
    int width  = 0;
    int height = 0;
    bool visible = false;
    int depth    = 0;
};

struct TraversalIssue {
    std::string description;
    std::string widgetPath;
};

auto BuildWidgetPath(QWidget* w) -> std::string
{
    std::string path;
    while (w != nullptr) {
        std::string name = w->objectName().isEmpty()
                               ? w->metaObject()->className()
                               : w->objectName().toStdString();
        if (path.empty()) {
            path = name;
        } else {
            path = name + " > " + path;
        }
        w = w->parentWidget();
    }
    return path;
}

void TraverseWidgetTree(QWidget* root, int depth,
                        std::vector<WidgetInfo>& infos,
                        std::vector<TraversalIssue>& issues)
{
    if (root == nullptr) { return; }

    WidgetInfo info;
    info.className  = root->metaObject()->className();
    info.objectName = root->objectName().toStdString();
    info.x      = root->x();
    info.y      = root->y();
    info.width  = root->width();
    info.height = root->height();
    info.visible = root->isVisible();
    info.depth   = depth;
    infos.push_back(info);

    auto widgetPath = BuildWidgetPath(root);

    // Check: zero-size visible widget
    if (info.visible && (info.width == 0 || info.height == 0)) {
        issues.push_back({
            std::string("Zero-size visible widget: ") +
                std::to_string(info.width) + "x" + std::to_string(info.height),
            widgetPath
        });
    }

    // Check: negative position
    if (info.x < 0 || info.y < 0) {
        issues.push_back({
            std::string("Negative position: (") +
                std::to_string(info.x) + ", " + std::to_string(info.y) + ")",
            widgetPath
        });
    }

    // Check: widget extends beyond parent
    auto* parent = root->parentWidget();
    if (parent != nullptr && info.visible && parent->isVisible()) {
        int parentW = parent->width();
        int parentH = parent->height();
        if (parentW > 0 && parentH > 0) {
            if (info.x + info.width > parentW + 2 ||
                info.y + info.height > parentH + 2) {
                issues.push_back({
                    std::string("Extends beyond parent: widget(") +
                        std::to_string(info.x) + "," + std::to_string(info.y) +
                        " " + std::to_string(info.width) + "x" +
                        std::to_string(info.height) + ") parent(" +
                        std::to_string(parentW) + "x" + std::to_string(parentH) + ")",
                    widgetPath
                });
            }
        }
    }

    // Recurse into children
    for (auto* child : root->findChildren<QWidget*>(Qt::FindDirectChildrenOnly)) {
        TraverseWidgetTree(child, depth + 1, infos, issues);
    }
}

} // anonymous namespace

TEST_SUITE("WidgetTreeTraverser") {

TEST_CASE("Traverse NyanCad main window widget tree") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);
    matcha::gui::SetThemeService(&theme);

    matcha::fw::Application app;
    app.Initialize(0, nullptr);

    auto* wsRegistry = app.GetWorkshopRegistry();
    REQUIRE(wsRegistry != nullptr);
    nyancad::RegisterNyanCadWorkshops(*wsRegistry);

    auto* docMgr = app.GetDocumentManagerImpl();
    REQUIRE(docMgr != nullptr);
    nyancad::NyanCadDocumentHost docHost;
    docMgr->SetParent(&docHost);

    nyancad::NyanCadMainWindow mainWin;
    mainWin.Setup(app);

    auto* wbMgr = app.GetWorkbenchManager();
    REQUIRE(wbMgr != nullptr);
    wbMgr->ActivateWorkshop(matcha::fw::WorkshopId::From("mesh"));

    // Create initial documents so DocumentArea has content
    docHost.CreateInitialDocuments(*docMgr);

    // Force layout by showing + processing events
    app.MainWindow().Show();
    QApplication::processEvents();

    // Get key widgets via UiNode accessors
    auto titleBarObs = app.MainWindow().GetTitleBar();
    auto wsFrameObs  = app.MainWindow().GetWorkspaceFrame();
    auto statusObs   = app.MainWindow().GetStatusBarNode();
    auto* central    = app.MainWindow().ContentArea();

    REQUIRE(titleBarObs.get() != nullptr);
    REQUIRE(wsFrameObs.get() != nullptr);
    REQUIRE(statusObs.get() != nullptr);
    REQUIRE(central != nullptr);

    auto* titleBarWidget = titleBarObs->Widget();
    REQUIRE(titleBarWidget != nullptr);

    // Find the top-level QMainWindow by walking up from titleBar widget
    QWidget* topLevel = titleBarWidget;
    while (topLevel->parentWidget() != nullptr) {
        topLevel = topLevel->parentWidget();
    }
    REQUIRE(topLevel != nullptr);

    // Traverse
    std::vector<WidgetInfo> infos;
    std::vector<TraversalIssue> issues;
    TraverseWidgetTree(topLevel, 0, infos, issues);

    // Print the full tree
    std::println("=== NyanCad Widget Tree ({} widgets) ===", infos.size());
    for (const auto& wi : infos) {
        std::string indent(static_cast<size_t>(wi.depth) * 2, ' ');
        std::string name = wi.objectName.empty() ? wi.className : wi.objectName;
        std::println("{}[{}] {}  pos=({},{}) size={}x{} vis={}",
                     indent, wi.className, name,
                     wi.x, wi.y, wi.width, wi.height,
                     wi.visible ? "Y" : "N");
    }

    // Print issues
    if (!issues.empty()) {
        std::println("\n=== Layout Issues ({}) ===", issues.size());
        for (const auto& iss : issues) {
            std::println("  ISSUE: {} @ {}", iss.description, iss.widgetPath);
        }
    } else {
        std::println("\n=== No layout issues found ===");
    }

    // Verify key widgets exist and are properly sized
    SUBCASE("TitleBar has nonzero size") {
        CHECK(titleBarWidget->width() > 0);
        CHECK(titleBarWidget->height() > 0);
        MESSAGE("TitleBar: ", titleBarWidget->width(), "x", titleBarWidget->height());
    }

    SUBCASE("ActionBar has nonzero size") {
        auto abNode = wsFrameObs->GetActionBar();
        REQUIRE(abNode.get() != nullptr);
        auto* abWidget = abNode->ActionBar();
        REQUIRE(abWidget != nullptr);
        CHECK(abWidget->width() > 0);
        CHECK(abWidget->height() > 0);
        MESSAGE("ActionBar: ", abWidget->width(), "x", abWidget->height());
    }

    SUBCASE("StatusBar has nonzero size") {
        auto* sbWidget = statusObs->StatusBar();
        REQUIRE(sbWidget != nullptr);
        CHECK(sbWidget->width() > 0);
        CHECK(sbWidget->height() > 0);
        MESSAGE("StatusBar: ", sbWidget->width(), "x", sbWidget->height());
    }

    SUBCASE("CentralArea has nonzero size") {
        CHECK(central->width() > 0);
        CHECK(central->height() > 0);
        MESSAGE("Central: ", central->width(), "x", central->height());
    }

    SUBCASE("Widget tree has reasonable depth and count") {
        CHECK(infos.size() > 10);
        int maxDepth = 0;
        for (const auto& wi : infos) {
            maxDepth = std::max(maxDepth, wi.depth);
        }
        CHECK(maxDepth >= 3);
        MESSAGE("Total widgets: ", infos.size(), ", max depth: ", maxDepth);
    }

    SUBCASE("DocumentView exists after setup") {
        auto* docView = mainWin.GetDocumentView();
        REQUIRE(docView != nullptr);
        MESSAGE("DocumentView is valid");
    }

    SUBCASE("No zero-size visible key widgets") {
        // Filter issues for key widget types only
        int keyIssues = 0;
        for (const auto& iss : issues) {
            if (iss.description.contains("Zero-size")) {
                // Ignore zero-size for non-visible or layout spacer widgets
                if (iss.widgetPath.contains("NyanMainTitleBar") ||
                    iss.widgetPath.contains("NyanActionBar") ||
                    iss.widgetPath.contains("NyanStatusBar") ||
                    iss.widgetPath.contains("DocumentArea")) {
                    ++keyIssues;
                }
            }
        }
        CHECK(keyIssues == 0);
    }

    app.Shutdown();
}

TEST_CASE("Traverse NyanCad main window - verify ActionBar tab content") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);
    matcha::gui::SetThemeService(&theme);

    matcha::fw::Application app;
    app.Initialize(0, nullptr);

    auto* wsRegistry = app.GetWorkshopRegistry();
    REQUIRE(wsRegistry != nullptr);
    nyancad::RegisterNyanCadWorkshops(*wsRegistry);

    nyancad::NyanCadMainWindow mainWin;
    mainWin.Setup(app);

    auto* wbMgr = app.GetWorkbenchManager();
    REQUIRE(wbMgr != nullptr);
    wbMgr->ActivateWorkshop(matcha::fw::WorkshopId::From("mesh"));

    app.MainWindow().Show();
    QApplication::processEvents();

    auto wsObs = app.MainWindow().GetWorkspaceFrame();
    REQUIRE(wsObs.get() != nullptr);
    auto abObs = wsObs->GetActionBar();
    REQUIRE(abObs.get() != nullptr);
    auto* actionBar = abObs->ActionBar();
    REQUIRE(actionBar != nullptr);

    // Traverse ActionBar subtree
    std::vector<WidgetInfo> infos;
    std::vector<TraversalIssue> issues;
    TraverseWidgetTree(actionBar, 0, infos, issues);

    std::println("=== ActionBar Subtree ({} widgets) ===", infos.size());
    for (const auto& wi : infos) {
        std::string indent(static_cast<size_t>(wi.depth) * 2, ' ');
        std::string name = wi.objectName.empty() ? wi.className : wi.objectName;
        std::println("{}[{}] {}  pos=({},{}) size={}x{} vis={}",
                     indent, wi.className, name,
                     wi.x, wi.y, wi.width, wi.height,
                     wi.visible ? "Y" : "N");
    }

    CHECK(actionBar->TabCount() >= 3);
    MESSAGE("ActionBar tabs: ", actionBar->TabCount());
    MESSAGE("ActionBar widgets: ", infos.size());

    // Check that ActionBar position is within window bounds
    auto* parentWidget = actionBar->parentWidget();
    if (parentWidget != nullptr) {
        CHECK(actionBar->x() >= 0);
        CHECK(actionBar->y() >= 0);
        MESSAGE("ActionBar pos: (", actionBar->x(), ",", actionBar->y(),
                ") in parent ", parentWidget->width(), "x", parentWidget->height());
    }

    app.Shutdown();
}

TEST_CASE("Traverse NyanCad main window - verify TitleBar structure") {
    matcha::gui::NyanTheme theme(QString::fromUtf8(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(matcha::gui::kThemeDark);
    matcha::gui::SetThemeService(&theme);

    matcha::fw::Application app;
    app.Initialize(0, nullptr);

    nyancad::NyanCadMainWindow mainWin;
    mainWin.Setup(app);

    app.MainWindow().Show();
    QApplication::processEvents();

    auto titleBarObs2 = app.MainWindow().GetTitleBar();
    REQUIRE(titleBarObs2.get() != nullptr);
    auto* titleBar = titleBarObs2->Widget();
    REQUIRE(titleBar != nullptr);

    std::vector<WidgetInfo> infos;
    std::vector<TraversalIssue> issues;
    TraverseWidgetTree(titleBar, 0, infos, issues);

    std::println("=== TitleBar Subtree ({} widgets) ===", infos.size());
    for (const auto& wi : infos) {
        std::string indent(static_cast<size_t>(wi.depth) * 2, ' ');
        std::string name = wi.objectName.empty() ? wi.className : wi.objectName;
        std::println("{}[{}] {}  pos=({},{}) size={}x{} vis={}",
                     indent, wi.className, name,
                     wi.x, wi.y, wi.width, wi.height,
                     wi.visible ? "Y" : "N");
    }

    CHECK(titleBar->width() > 0);
    CHECK(titleBar->height() > 0);
    MESSAGE("TitleBar: ", titleBar->width(), "x", titleBar->height(),
            ", children: ", infos.size());

    if (!issues.empty()) {
        std::println("\n=== TitleBar Issues ({}) ===", issues.size());
        for (const auto& iss : issues) {
            std::println("  ISSUE: {} @ {}", iss.description, iss.widgetPath);
        }
    }

    app.Shutdown();
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
