#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Tree/Composition/Menu/DialogNode.h>
#include <Matcha/Theming/IThemeService.h>
#include <Matcha/Widgets/Menu/NyanDialog.h>
#include <Matcha/Theming/NyanTheme.h>

#include <QTest>

extern matcha::test::WidgetTestFixture* gFixture;

class DialogNodeTest : public QObject {
    Q_OBJECT

private:
    matcha::gui::NyanTheme _theme{QStringLiteral(MATCHA_TEST_PALETTE_DIR)};

    void initTheme()
    {
        _theme.SetTheme(matcha::gui::kThemeLight);
        _theme.SetAnimationOverride(0);
        matcha::gui::SetThemeService(&_theme);
    }

private slots:

    void initTestCase() { initTheme(); }

    void dialogNode_construct()
    {
        matcha::fw::DialogNode dlg("test-dlg", nullptr);
        QVERIFY(dlg.Dialog() != nullptr);
        QVERIFY(dlg.Widget() != nullptr);
        QCOMPARE(dlg.Type(), matcha::fw::NodeType::Dialog);
    }

    void dialogNode_setTitle()
    {
        matcha::fw::DialogNode dlg("test-dlg", nullptr);
        dlg.SetTitle("Hello");
        QCOMPARE(dlg.Dialog()->windowTitle(), QStringLiteral("Hello"));
    }

    void dialogNode_setWidth_small()
    {
        matcha::fw::DialogNode dlg("test-dlg", nullptr);
        dlg.SetWidth(matcha::fw::DialogWidth::Small);
        QCOMPARE(dlg.Widget()->minimumWidth(), 400);
        QCOMPARE(dlg.Widget()->maximumWidth(), 400);
    }

    void dialogNode_setWidth_medium()
    {
        matcha::fw::DialogNode dlg("test-dlg", nullptr);
        dlg.SetWidth(matcha::fw::DialogWidth::Medium);
        QCOMPARE(dlg.Widget()->minimumWidth(), 600);
        QCOMPARE(dlg.Widget()->maximumWidth(), 600);
    }

    void dialogNode_setWidth_large()
    {
        matcha::fw::DialogNode dlg("test-dlg", nullptr);
        dlg.SetWidth(matcha::fw::DialogWidth::Large);
        QCOMPARE(dlg.Widget()->minimumWidth(), 800);
        QCOMPARE(dlg.Widget()->maximumWidth(), 800);
    }

    void dialogNode_showModeless()
    {
        matcha::fw::DialogNode dlg("test-dlg", nullptr);
        dlg.ShowModeless();
        QVERIFY(dlg.Widget()->isVisible());
        dlg.Close();
    }
};
