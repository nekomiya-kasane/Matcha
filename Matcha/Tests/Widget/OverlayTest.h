#pragma once

#include <Matcha/Widgets/Shell/ViewportWidget.h>

#include <QTest>

class OverlayTest : public QObject {
    Q_OBJECT

private slots:
    void overlayWidget_existsAfterConstruction()
    {
        matcha::fw::ViewportWidget widget;
        QVERIFY(widget.OverlayWidget() != nullptr);
    }

    void overlayWidget_isChildOfViewportWidget()
    {
        matcha::fw::ViewportWidget widget;
        auto* overlay = widget.OverlayWidget();
        QCOMPARE(overlay->parentWidget(), &widget);
    }

    void overlayWidget_isTransparentForMouseEvents()
    {
        matcha::fw::ViewportWidget widget;
        auto* overlay = widget.OverlayWidget();
        QVERIFY(overlay->testAttribute(Qt::WA_TransparentForMouseEvents));
    }

    void overlayWidget_matchesParentGeometry()
    {
        matcha::fw::ViewportWidget widget;
        widget.show();
        widget.resize(300, 200);
        QVERIFY(QTest::qWaitForWindowExposed(&widget));

        auto* overlay = widget.OverlayWidget();
        QCOMPARE(overlay->width(), widget.width());
        QCOMPARE(overlay->height(), widget.height());
    }
};
