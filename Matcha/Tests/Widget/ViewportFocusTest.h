#pragma once

#include <Matcha/UiNodes/Document/Viewport.h>

#include <QTest>

class ViewportFocusTest : public QObject {
    Q_OBJECT

private slots:
    void isFocused_falseBeforeCreateWidget()
    {
        matcha::fw::Viewport vp("vp1", matcha::fw::ViewportId::From(1));
        QCOMPARE(vp.IsFocused(), false);
    }

    void viewportId_matchesConstruction()
    {
        matcha::fw::Viewport vp("vp1", matcha::fw::ViewportId::From(2));
        QCOMPARE(vp.GetViewportId().value, uint32_t(2));
    }

    void dirtyFlag_requestAndClear()
    {
        matcha::fw::Viewport vp("vp1", matcha::fw::ViewportId::From(3));
        QCOMPARE(vp.IsDirty(), false);
        vp.RequestFrame();
        QCOMPARE(vp.IsDirty(), true);
        vp.ClearDirty();
        QCOMPARE(vp.IsDirty(), false);
    }
};
