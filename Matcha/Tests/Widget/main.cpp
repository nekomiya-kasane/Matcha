#include "ApplicationWidgetTest.h"
#include "ContainerNodeTest.h"
#include "MenuBarTest.h"
#include "ContainerWidgetTest.h"
#include "CoreWidgetTest.h"
#include "DialogNodeTest.h"
#include "OverlayTest.h"
#include "ShellWindowTest.h"
#include "ThemeTest.h"
#include "UiNodeWidgetTest.h"
#include "ViewportFocusTest.h"
#include "ViewportFrameTest.h"
#include "ViewportGroupWidgetTest.h"
#include "ViewportWidgetTest.h"
#include "WidgetTestFixtureTest.h"

#include <QTest>

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
matcha::test::WidgetTestFixture* gFixture = nullptr;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

auto main(int argc, char* argv[]) -> int {
    matcha::test::WidgetTestFixture fixture;
    gFixture = &fixture;

    int status = 0;

    WidgetTestFixtureTest fixtureTest;
    status |= QTest::qExec(&fixtureTest, argc, argv);

    ThemeTest themeTest;
    status |= QTest::qExec(&themeTest, argc, argv);

    CoreWidgetTest coreWidgetTest;
    status |= QTest::qExec(&coreWidgetTest, argc, argv);

    ContainerWidgetTest containerWidgetTest;
    status |= QTest::qExec(&containerWidgetTest, argc, argv);

    ApplicationWidgetTest applicationWidgetTest;
    status |= QTest::qExec(&applicationWidgetTest, argc, argv);

    UiNodeWidgetTest uiNodeWidgetTest;
    status |= QTest::qExec(&uiNodeWidgetTest, argc, argv);

    ShellWindowTest shellWindowTest;
    status |= QTest::qExec(&shellWindowTest, argc, argv);

    DialogNodeTest dialogNodeTest;
    status |= QTest::qExec(&dialogNodeTest, argc, argv);

    ContainerNodeTest containerNodeTest;
    status |= QTest::qExec(&containerNodeTest, argc, argv);

    ViewportWidgetTest viewportWidgetTest;
    status |= QTest::qExec(&viewportWidgetTest, argc, argv);

    OverlayTest overlayTest;
    status |= QTest::qExec(&overlayTest, argc, argv);

    ViewportFocusTest viewportFocusTest;
    status |= QTest::qExec(&viewportFocusTest, argc, argv);

    MenuBarTest menuBarTest;
    status |= QTest::qExec(&menuBarTest, argc, argv);

    ViewportGroupWidgetTest vpGroupWidgetTest;
    status |= QTest::qExec(&vpGroupWidgetTest, argc, argv);

    ViewportFrameTest vpFrameTest;
    status |= QTest::qExec(&vpFrameTest, argc, argv);

    return status;
}

#include "moc_ApplicationWidgetTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_ContainerNodeTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_ContainerWidgetTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_CoreWidgetTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_DialogNodeTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_ShellWindowTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_WidgetTestFixtureTest.cpp"
#include "moc_ThemeTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_UiNodeWidgetTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_ViewportWidgetTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_OverlayTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_ViewportFocusTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_ViewportFrameTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_ViewportGroupWidgetTest.cpp" // NOLINT(bugprone-suspicious-include)
#include "moc_MenuBarTest.cpp" // NOLINT(bugprone-suspicious-include)
