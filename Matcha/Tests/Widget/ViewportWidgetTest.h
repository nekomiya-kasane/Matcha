#pragma once

#include "MockRenderer.h"
#include "WidgetTestFixture.h"

#include <Matcha/Widgets/Shell/ViewportWidget.h>

#include <QTest>

class ViewportWidgetTest : public QObject {
    Q_OBJECT

private slots:
    void setRenderer_callsOnAttach()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;
        widget.resize(200, 100);

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(1));
        QCOMPARE(renderer.attachCount, 1);
        QCOMPARE(renderer.lastVpId, matcha::fw::ViewportId::From(1));
    }

    void removeRenderer_callsOnDetach()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(2));
        widget.RemoveRenderer();
        QCOMPARE(renderer.detachCount, 1);
    }

    void renderFrame_callsOnRenderFrame()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(3));
        widget.RenderFrame();
        QCOMPARE(renderer.renderCount, 1);
    }

    void renderFrame_skipsWhenNotReady()
    {
        matcha::test::MockRenderer renderer;
        renderer.ready = false;
        matcha::fw::ViewportWidget widget;

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(4));
        widget.RenderFrame();
        QCOMPARE(renderer.renderCount, 0);
    }

    void nativeHandle_returnsNonNull()
    {
        matcha::fw::ViewportWidget widget;
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));

        QVERIFY(widget.NativeHandle() != nullptr);
    }

    void doubleSetRenderer_detachesFirst()
    {
        matcha::test::MockRenderer r1;
        matcha::test::MockRenderer r2;
        matcha::fw::ViewportWidget widget;

        widget.SetRenderer(&r1, matcha::fw::ViewportId::From(5));
        widget.SetRenderer(&r2, matcha::fw::ViewportId::From(5));
        QCOMPARE(r1.detachCount, 1);
        QCOMPARE(r2.attachCount, 1);
    }

    void showEvent_forwardsVisibility()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(6));
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));
        QVERIFY(renderer.visCount >= 1);
        QCOMPARE(renderer.lastVisible, true);
    }

    void hideEvent_forwardsVisibility()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(7));
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));
        int prevCount = renderer.visCount;

        widget.hide();
        QVERIFY(renderer.visCount > prevCount);
        QCOMPARE(renderer.lastVisible, false);
    }

    void resizeEvent_forwardsOnResize()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(8));
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));

        int prevResizeCount = renderer.resizeCount;
        widget.resize(400, 300);
        QTest::qWait(50);
        QVERIFY(renderer.resizeCount > prevResizeCount);
    }

    void mousePressEvent_forwardsInputEvent()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;
        widget.resize(200, 200);

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(9));
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));

        QTest::mousePress(&widget, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
        QVERIFY(renderer.inputCount >= 1);
        QVERIFY(!renderer.receivedEvents.empty());
        QCOMPARE(renderer.receivedEvents.back().type, matcha::fw::InputEventType::MousePress);
    }

    void keyPressEvent_forwardsInputEvent()
    {
        matcha::test::MockRenderer renderer;
        matcha::fw::ViewportWidget widget;
        widget.resize(200, 200);

        widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(10));
        widget.show();
        QVERIFY(QTest::qWaitForWindowExposed(&widget));
        widget.setFocus();

        QTest::keyPress(&widget, Qt::Key_A);
        QVERIFY(renderer.inputCount >= 1);
        bool foundKeyPress = false;
        for (const auto& evt : renderer.receivedEvents) {
            if (evt.type == matcha::fw::InputEventType::KeyPress) {
                foundKeyPress = true;
                break;
            }
        }
        QVERIFY(foundKeyPress);
    }

    void destruction_detachesRenderer()
    {
        matcha::test::MockRenderer renderer;
        {
            matcha::fw::ViewportWidget widget;
            widget.SetRenderer(&renderer, matcha::fw::ViewportId::From(11));
        }
        QCOMPARE(renderer.detachCount, 1);
    }
};
