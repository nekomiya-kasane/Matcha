#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Tree/Composition/ActionBar/ActionBarNode.h>
#include <Matcha/Tree/Composition/Shell/StatusBarNode.h>
#include <Matcha/Tree/Composition/Shell/StatusItemNode.h>
#include <Matcha/Theming/IThemeService.h>
#include <Matcha/Widgets/ActionBar/NyanActionBar.h>
#include <Matcha/Widgets/Shell/NyanStatusBar.h>
#include <Matcha/Theming/NyanTheme.h>

#include <QTest>

extern matcha::test::WidgetTestFixture* gFixture;

class ContainerNodeTest : public QObject {
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

    // -- StatusBarNode --

    void statusBarNode_construct()
    {
        matcha::fw::StatusBarNode sb(nullptr);
        QVERIFY(sb.StatusBar() != nullptr);
        QVERIFY(sb.Widget() != nullptr);
        QCOMPARE(sb.Type(), matcha::fw::NodeType::StatusBar);
        QCOMPARE(sb.ItemCount(), 0);
    }

    void statusBarNode_addLabel()
    {
        matcha::fw::StatusBarNode sb(nullptr);
        auto* item = sb.AddLabel("msg", "Hello", matcha::gui::StatusBarSide::Left);
        QVERIFY(item != nullptr);
        QCOMPARE(sb.ItemCount(), 1);
        QCOMPARE(item->Text(), std::string_view("Hello"));
    }

    void statusBarNode_addProgress()
    {
        matcha::fw::StatusBarNode sb(nullptr);
        auto* item = sb.AddProgress("prog", matcha::gui::StatusBarSide::Right);
        QVERIFY(item != nullptr);
        QCOMPARE(sb.ItemCount(), 1);
        item->SetValue(50);
        QCOMPARE(item->Value(), 50);
    }

    void statusBarNode_removeItem()
    {
        matcha::fw::StatusBarNode sb(nullptr);
        sb.AddLabel("msg", "Hello", matcha::gui::StatusBarSide::Left);
        QCOMPARE(sb.ItemCount(), 1);
        QVERIFY(sb.RemoveItem("msg"));
        QCOMPARE(sb.ItemCount(), 0);
        QVERIFY(!sb.RemoveItem("msg"));
    }

    void statusBarNode_duplicateId()
    {
        matcha::fw::StatusBarNode sb(nullptr);
        auto* first = sb.AddLabel("msg", "A", matcha::gui::StatusBarSide::Left);
        auto* dup   = sb.AddLabel("msg", "B", matcha::gui::StatusBarSide::Right);
        QVERIFY(first != nullptr);
        QVERIFY(dup == nullptr);
        QCOMPARE(sb.ItemCount(), 1);
    }

    // -- ActionBarNode --

    void actionBarNode_construct()
    {
        matcha::fw::ActionBarNode ab(nullptr);
        QVERIFY(ab.ActionBar() != nullptr);
        QVERIFY(ab.Widget() != nullptr);
        QCOMPARE(ab.Type(), matcha::fw::NodeType::ActionBar);
    }

    void actionBarNode_dockSide()
    {
        matcha::fw::ActionBarNode ab(nullptr);
        ab.SetDockSide(matcha::gui::DockSide::Right);
        QCOMPARE(ab.GetDockSide(), matcha::gui::DockSide::Right);
    }

    void actionBarNode_collapsed()
    {
        matcha::fw::ActionBarNode ab(nullptr);
        QVERIFY(!ab.IsCollapsed());
        ab.SetCollapsed(true);
        QVERIFY(ab.IsCollapsed());
        ab.SetCollapsed(false);
        QVERIFY(!ab.IsCollapsed());
    }

    void actionBarNode_docked()
    {
        matcha::fw::ActionBarNode ab(nullptr);
        QVERIFY(!ab.IsDocked());
        ab.SetDocked(true);
        QVERIFY(ab.IsDocked());
        ab.SetDocked(false);
        QVERIFY(!ab.IsDocked());
    }

};
