#pragma once

#include "WidgetTestFixture.h"

#include <QApplication>
#include <QTest>
#include <QWidget>

extern matcha::test::WidgetTestFixture* gFixture;

class WidgetTestFixtureTest : public QObject {
    Q_OBJECT

private slots: // NOLINT(readability-redundant-access-specifiers)
    void appExists()
    {
        QVERIFY(gFixture != nullptr);
        QVERIFY(gFixture->App() != nullptr);
        QVERIFY(QApplication::instance() != nullptr);
    }

    void createWidgetReturnsValid()
    {
        auto* widget = gFixture->CreateWidget();
        QVERIFY(widget != nullptr);
        QVERIFY(qobject_cast<QWidget*>(widget) != nullptr);
    }

    void animationOverrideIsZero()
    {
        auto duration = gFixture->App()->property("animationDuration");
        QVERIFY(duration.isValid());
        QCOMPARE(duration.toInt(), 0);
    }
};
