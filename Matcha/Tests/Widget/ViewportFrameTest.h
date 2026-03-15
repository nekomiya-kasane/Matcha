#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Core/Types.h>
#include <Matcha/Tree/Composition/Document/SplitTreeNode.h>
#include <Matcha/Tree/Composition/Document/Viewport.h>
#include <Matcha/Tree/Composition/Document/ViewportGroup.h>
#include <Matcha/Interaction/DropZoneOverlay.h>
#include <Matcha/Widgets/Shell/ViewportFrame.h>
#include <Matcha/Widgets/Shell/ViewportHeaderBar.h>
#include <Matcha/Widgets/Shell/ViewportWidget.h>

#include <QSignalSpy>
#include <QSplitter>
#include <QTest>

class ViewportFrameTest : public QObject {
    Q_OBJECT

private slots:

    // -- ViewportHeaderBar tests --

    void headerBar_defaultLabel()
    {
        auto vpId = matcha::fw::ViewportId::From(1);
        matcha::gui::ViewportHeaderBar header(vpId);
        QCOMPARE(header.Label(), QStringLiteral("Viewport"));
        QCOMPARE(header.GetViewportId(), vpId);
    }

    void headerBar_setLabel()
    {
        auto vpId = matcha::fw::ViewportId::From(2);
        matcha::gui::ViewportHeaderBar header(vpId);
        header.SetLabel(QStringLiteral("3D Perspective"));
        QCOMPARE(header.Label(), QStringLiteral("3D Perspective"));
    }

    void headerBar_fixedHeight()
    {
        auto vpId = matcha::fw::ViewportId::From(1);
        matcha::gui::ViewportHeaderBar header(vpId);
        header.resize(200, 100);
        // Fixed height should be 24
        QCOMPARE(header.maximumHeight(), 24);
        QCOMPARE(header.minimumHeight(), 24);
    }

    void headerBar_ghostMode()
    {
        auto vpId = matcha::fw::ViewportId::From(1);
        matcha::gui::ViewportHeaderBar header(vpId);
        header.SetGhostMode(true);
        // Just verify no crash; visual test
        header.repaint();
        header.SetGhostMode(false);
        header.repaint();
    }

    void headerBar_doubleClick_emitsMaximizeToggled()
    {
        auto vpId = matcha::fw::ViewportId::From(3);
        matcha::gui::ViewportHeaderBar header(vpId);
        header.resize(200, 24);
        header.show();
        QTest::qWait(10);

        QSignalSpy spy(&header, &matcha::gui::ViewportHeaderBar::maximizeToggled);
        QTest::mouseDClick(&header, Qt::LeftButton, Qt::NoModifier, QPoint(50, 12));
        QCOMPARE(spy.count(), 1);
    }

    // -- DropZoneOverlay tests --

    void dropZoneOverlay_defaultState()
    {
        matcha::gui::DropZoneOverlay overlay;
        QVERIFY(!overlay.HasActiveZone());
    }

    void dropZoneOverlay_setActiveZone()
    {
        matcha::gui::DropZoneOverlay overlay;
        overlay.SetActiveZone(matcha::fw::DropZone::Top);
        QVERIFY(overlay.HasActiveZone());
        QCOMPARE(overlay.ActiveZone(), matcha::fw::DropZone::Top);
    }

    void dropZoneOverlay_clearZone()
    {
        matcha::gui::DropZoneOverlay overlay;
        overlay.SetActiveZone(matcha::fw::DropZone::Left);
        QVERIFY(overlay.HasActiveZone());
        overlay.ClearZone();
        QVERIFY(!overlay.HasActiveZone());
    }

    void dropZoneOverlay_zoneAtPoint_center()
    {
        matcha::gui::DropZoneOverlay overlay;
        overlay.resize(100, 100);
        // Center of 100x100 should be Center zone
        auto zone = overlay.ZoneAtPoint(QPoint(50, 50));
        QCOMPARE(zone, matcha::fw::DropZone::Center);
    }

    void dropZoneOverlay_zoneAtPoint_edges()
    {
        matcha::gui::DropZoneOverlay overlay;
        overlay.resize(100, 100);

        // Top edge (y < 30%)
        QCOMPARE(overlay.ZoneAtPoint(QPoint(50, 10)), matcha::fw::DropZone::Top);
        // Bottom edge (y > 70%)
        QCOMPARE(overlay.ZoneAtPoint(QPoint(50, 90)), matcha::fw::DropZone::Bottom);
        // Left edge (x < 30%, y in middle)
        QCOMPARE(overlay.ZoneAtPoint(QPoint(10, 50)), matcha::fw::DropZone::Left);
        // Right edge (x > 70%, y in middle)
        QCOMPARE(overlay.ZoneAtPoint(QPoint(90, 50)), matcha::fw::DropZone::Right);
    }

    void dropZoneOverlay_paintDoesNotCrash()
    {
        matcha::gui::DropZoneOverlay overlay;
        overlay.resize(200, 200);
        overlay.show();
        overlay.SetActiveZone(matcha::fw::DropZone::Left);
        overlay.repaint();
        overlay.ClearZone();
        overlay.repaint();
    }

    // -- ViewportFrame tests --

    void viewportFrame_construction()
    {
        auto vpId = matcha::fw::ViewportId::From(1);
        matcha::fw::ViewportWidget vpWidget;

        matcha::gui::ViewportFrame frame(vpId, &vpWidget);
        QCOMPARE(frame.GetViewportId(), vpId);
        QVERIFY(frame.HeaderBar() != nullptr);
        QVERIFY(frame.Overlay() != nullptr);
        QCOMPARE(frame.InnerWidget(), &vpWidget);
    }

    void viewportFrame_setLabel()
    {
        auto vpId = matcha::fw::ViewportId::From(1);
        matcha::fw::ViewportWidget vpWidget;

        matcha::gui::ViewportFrame frame(vpId, &vpWidget);
        frame.SetLabel(QStringLiteral("Top View"));
        QCOMPARE(frame.HeaderBar()->Label(), QStringLiteral("Top View"));
    }

    void viewportFrame_showHideOverlay()
    {
        auto vpId = matcha::fw::ViewportId::From(1);
        matcha::fw::ViewportWidget vpWidget;

        matcha::gui::ViewportFrame frame(vpId, &vpWidget);
        frame.resize(400, 300);
        frame.show();
        QTest::qWait(10);

        frame.ShowOverlay();
        QVERIFY(!frame.Overlay()->isHidden());

        frame.HideOverlay();
        QVERIFY(frame.Overlay()->isHidden());
    }

    void viewportFrame_signalForwarding()
    {
        auto vpId = matcha::fw::ViewportId::From(5);
        matcha::fw::ViewportWidget vpWidget;

        matcha::gui::ViewportFrame frame(vpId, &vpWidget);
        frame.resize(400, 300);
        frame.show();
        QTest::qWait(10);

        QSignalSpy maxSpy(&frame, &matcha::gui::ViewportFrame::maximizeToggled);

        // Double-click on header bar -> should forward maximizeToggled
        auto* header = frame.HeaderBar();
        QTest::mouseDClick(header, Qt::LeftButton, Qt::NoModifier, QPoint(50, 12));
        QCOMPARE(maxSpy.count(), 1);
    }

    // -- Integration: ViewportGroup builds ViewportFrames --

    void viewportGroup_rebuild_producesFrames()
    {
        matcha::fw::ViewportGroup vg("vg");
        auto id1 = vg.AllViewportIds()[0];
        auto r = vg.SplitViewport(id1, matcha::fw::SplitDirection::Horizontal);
        QVERIFY(r.has_value());

        QWidget container;
        container.resize(800, 600);
        vg.RebuildWidgetTree(&container);

        auto* root = vg.RootWidget();
        QVERIFY(root != nullptr);

        // Root should be a splitter
        auto* splitter = qobject_cast<QSplitter*>(root);
        QVERIFY(splitter != nullptr);
        QCOMPARE(splitter->count(), 2);

        // Both children should be ViewportFrame
        auto* frame0 = qobject_cast<matcha::gui::ViewportFrame*>(splitter->widget(0));
        auto* frame1 = qobject_cast<matcha::gui::ViewportFrame*>(splitter->widget(1));
        QVERIFY(frame0 != nullptr);
        QVERIFY(frame1 != nullptr);
    }

    void viewportGroup_maximize_showsSingleFrame()
    {
        matcha::fw::ViewportGroup vg("vg");
        auto id1 = vg.AllViewportIds()[0];
        auto r = vg.SplitViewport(id1, matcha::fw::SplitDirection::Horizontal);
        QVERIFY(r.has_value());

        QWidget container;
        container.resize(800, 600);

        // Maximize id1
        QVERIFY(vg.MaximizeViewport(id1).has_value());
        vg.RebuildWidgetTree(&container);

        auto* root = vg.RootWidget();
        QVERIFY(root != nullptr);

        // Root should be a ViewportFrame (not a splitter)
        auto* frame = qobject_cast<matcha::gui::ViewportFrame*>(root);
        QVERIFY(frame != nullptr);
        QCOMPARE(frame->GetViewportId(), id1);
    }

    void viewportGroup_restoreAfterMaximize_restoresSplitter()
    {
        matcha::fw::ViewportGroup vg("vg");
        auto id1 = vg.AllViewportIds()[0];
        auto r = vg.SplitViewport(id1, matcha::fw::SplitDirection::Horizontal);
        QVERIFY(r.has_value());

        QWidget container;
        container.resize(800, 600);

        // Maximize
        QVERIFY(vg.MaximizeViewport(id1).has_value());
        vg.RebuildWidgetTree(&container);
        QVERIFY(qobject_cast<matcha::gui::ViewportFrame*>(vg.RootWidget()) != nullptr);

        // Restore
        vg.RestoreLayout();
        vg.RebuildWidgetTree();
        QVERIFY(qobject_cast<QSplitter*>(vg.RootWidget()) != nullptr);
    }
};
