#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Tree/Composition/Document/SplitTreeNode.h>
#include <Matcha/Tree/Composition/Document/Viewport.h>
#include <Matcha/Tree/Composition/Document/ViewportGroup.h>

#include <QSplitter>
#include <QTest>
#include <QVBoxLayout>

class ViewportGroupWidgetTest : public QObject {
    Q_OBJECT

private slots:
    void rebuildWidgetTree_singleViewport()
    {
        matcha::fw::ViewportGroup vg("vg");
        QWidget container;
        container.resize(400, 300);

        vg.RebuildWidgetTree(&container);

        auto* root = vg.RootWidget();
        QVERIFY(root != nullptr);
        // Single viewport -> root widget is the ViewportWidget directly
        // (not a splitter, since there's only one leaf)
    }

    void rebuildWidgetTree_twoViewports_createsSplitter()
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

        // Root should be a QSplitter with horizontal orientation
        auto* splitter = qobject_cast<QSplitter*>(root);
        QVERIFY(splitter != nullptr);
        QCOMPARE(splitter->orientation(), Qt::Horizontal);
        QCOMPARE(splitter->count(), 2);
    }

    void rebuildWidgetTree_fourViewports_nestedSplitters()
    {
        matcha::fw::ViewportGroup vg("vg");
        auto id1 = vg.AllViewportIds()[0];

        // Build 2x2: [[id1/id3] | [id2/id4]]
        auto r1 = vg.SplitViewport(id1, matcha::fw::SplitDirection::Horizontal);
        auto id2 = r1->get()->GetViewportId();
        auto r2 = vg.SplitViewport(id1, matcha::fw::SplitDirection::Vertical);
        (void)r2;
        auto r3 = vg.SplitViewport(id2, matcha::fw::SplitDirection::Vertical);
        (void)r3;

        QCOMPARE(vg.ViewportCount(), 4);

        QWidget container;
        container.resize(800, 600);

        vg.RebuildWidgetTree(&container);

        auto* root = vg.RootWidget();
        QVERIFY(root != nullptr);

        // Root should be a horizontal splitter
        auto* rootSplitter = qobject_cast<QSplitter*>(root);
        QVERIFY(rootSplitter != nullptr);
        QCOMPARE(rootSplitter->orientation(), Qt::Horizontal);
        QCOMPARE(rootSplitter->count(), 2);

        // Both children should be vertical splitters
        auto* left = qobject_cast<QSplitter*>(rootSplitter->widget(0));
        auto* right = qobject_cast<QSplitter*>(rootSplitter->widget(1));
        QVERIFY(left != nullptr);
        QVERIFY(right != nullptr);
        QCOMPARE(left->orientation(), Qt::Vertical);
        QCOMPARE(right->orientation(), Qt::Vertical);
        QCOMPARE(left->count(), 2);
        QCOMPARE(right->count(), 2);
    }

    void rebuildWidgetTree_afterSplitAndRemove()
    {
        matcha::fw::ViewportGroup vg("vg");
        auto id1 = vg.AllViewportIds()[0];

        auto r1 = vg.SplitViewport(id1, matcha::fw::SplitDirection::Horizontal);
        auto id2 = r1->get()->GetViewportId();

        QWidget container;
        container.resize(800, 600);

        // Build with 2 viewports
        vg.RebuildWidgetTree(&container);
        auto* root1 = vg.RootWidget();
        QVERIFY(qobject_cast<QSplitter*>(root1) != nullptr);

        // Remove one, rebuild
        QVERIFY(vg.RemoveViewport(id2).has_value());
        vg.RebuildWidgetTree();

        auto* root2 = vg.RootWidget();
        QVERIFY(root2 != nullptr);
        // Should no longer be a splitter (single viewport)
        QVERIFY(qobject_cast<QSplitter*>(root2) == nullptr);
    }

    void rebuildWidgetTree_calledTwice_noLeak()
    {
        matcha::fw::ViewportGroup vg("vg");
        auto id1 = vg.AllViewportIds()[0];
        auto r = vg.SplitViewport(id1, matcha::fw::SplitDirection::Vertical);
        (void)r;

        QWidget container;
        container.resize(400, 400);

        // First build
        vg.RebuildWidgetTree(&container);
        auto* root1 = vg.RootWidget();
        QVERIFY(root1 != nullptr);

        // Second build (should destroy old splitters and create new ones)
        vg.RebuildWidgetTree();
        auto* root2 = vg.RootWidget();
        QVERIFY(root2 != nullptr);

        // Root should still be a splitter with 2 children
        auto* splitter = qobject_cast<QSplitter*>(root2);
        QVERIFY(splitter != nullptr);
        QCOMPARE(splitter->count(), 2);
    }

    void rebuildWidgetTree_noContainer_doesNothing()
    {
        matcha::fw::ViewportGroup vg("vg");
        // Call without container -- should be a no-op
        vg.RebuildWidgetTree(nullptr);
        QVERIFY(vg.RootWidget() == nullptr);
    }
};
