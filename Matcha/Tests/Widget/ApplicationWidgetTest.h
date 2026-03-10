#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Widgets/ActionBar/NyanActionBar.h>
#include <Matcha/Widgets/ActionBar/NyanActionTab.h>
#include <Matcha/Widgets/ActionBar/NyanActionToolbar.h>
#include <Matcha/Widgets/Menu/NyanDialog.h>
#include <Matcha/Widgets/Menu/NyanDialogFootBar.h>
#include <Matcha/Widgets/Menu/NyanDialogTitleBar.h>
#include <Matcha/Widgets/Shell/NyanTabBar.h>
#include <Matcha/Widgets/Shell/NyanTabItem.h>
#include <Matcha/Widgets/Shell/NyanMainTitleBar.h>
#include <Matcha/Widgets/Menu/NyanMenu.h>
#include <Matcha/Widgets/Menu/NyanMenuBar.h>
#include <Matcha/Widgets/Menu/NyanMenuCheckItem.h>
#include <Matcha/Widgets/Menu/NyanMenuItem.h>
#include <Matcha/Widgets/Controls/NyanNotification.h>
#include <Matcha/Widgets/Controls/NyanSelectionInput.h>
#include <Matcha/Widgets/Shell/NyanStatusBar.h>
#include <Matcha/Widgets/Controls/NyanStructureTree.h>
#include <Matcha/Widgets/Core/NyanTheme.h>

#include <QSignalSpy>
#include <QTest>

extern matcha::test::WidgetTestFixture* gFixture;

// ============================================================================
// ApplicationWidgetTest -- Qt Test class for Phase 3c application widgets
// ============================================================================

class ApplicationWidgetTest : public QObject {
    Q_OBJECT

private:
    matcha::gui::NyanTheme _theme{QStringLiteral(MATCHA_TEST_PALETTE_DIR)};

    void initTheme()
    {
        _theme.SetTheme(matcha::gui::kThemeLight);
        _theme.SetAnimationOverride(0);
        matcha::gui::SetThemeService(&_theme);
    }

private slots: // NOLINT(readability-redundant-access-specifiers)

    void initTestCase() { initTheme(); }

    // ========================================================================
    // ActionBar (~5)
    // ========================================================================

    void actionBar_AddTab()
    {
        matcha::gui::NyanActionBar bar{};
        QCOMPARE(bar.TabCount(), 0);

        QString tabId = bar.AddTab("Test Tab");
        QVERIFY(!tabId.isEmpty());
        QCOMPARE(bar.TabCount(), 1);
    }

    void actionBar_RemoveTab()
    {
        matcha::gui::NyanActionBar bar{};
        QString tabId = bar.AddTab("Test Tab");
        QCOMPARE(bar.TabCount(), 1);

        bar.RemoveTab(tabId);
        QCOMPARE(bar.TabCount(), 0);
    }

    void actionBar_SwitchTab()
    {
        matcha::gui::NyanActionBar bar{};
        QString tab1 = bar.AddTab("Tab 1");
        QString tab2 = bar.AddTab("Tab 2");

        QSignalSpy spy(&bar, &matcha::gui::NyanActionBar::TabSwitched);
        bar.SwitchTab(tab2);

        QCOMPARE(bar.CurrentTabId(), tab2);
    }

    void actionBar_DisplayMode()
    {
        matcha::gui::NyanActionBar bar{};
        QCOMPARE(bar.DisplayMode(), matcha::gui::ActionBarDisplayMode::IconOnly);

        bar.SetDisplayMode(matcha::gui::ActionBarDisplayMode::IconText);
        QCOMPARE(bar.DisplayMode(), matcha::gui::ActionBarDisplayMode::IconText);
    }

    void actionBar_Collapsed()
    {
        matcha::gui::NyanActionBar bar{};
        QVERIFY(!bar.IsCollapsed());

        QSignalSpy spy(&bar, &matcha::gui::NyanActionBar::CollapsedChanged);
        bar.SetCollapsed(true);

        QVERIFY(bar.IsCollapsed());
        QCOMPARE(spy.count(), 1);
    }

    void actionBar_Docked()
    {
        matcha::gui::NyanActionBar bar{};
        QVERIFY(!bar.IsDocked());
        bar.SetDocked(true);
        QVERIFY(bar.IsDocked());
        bar.SetDocked(false);
        QVERIFY(!bar.IsDocked());
    }

    void actionBar_CollapseButton()
    {
        matcha::gui::NyanActionBar bar{};
        QVERIFY(bar.CollapseButton() != nullptr);
    }

    void actionBar_MiniButton()
    {
        matcha::gui::NyanActionBar bar{};
        QVERIFY(bar.MiniButton() != nullptr);
    }

    // ========================================================================
    // Dialog (~5)
    // ========================================================================

    void dialog_Modality()
    {
        matcha::gui::NyanDialog dialog{};
        QCOMPARE(dialog.Modality(), matcha::gui::DialogModality::Modal);

        dialog.SetDialogModality(matcha::gui::DialogModality::Modeless);
        QCOMPARE(dialog.Modality(), matcha::gui::DialogModality::Modeless);
    }

    void dialog_Title()
    {
        matcha::gui::NyanDialog dialog{};
        dialog.SetTitle("Test Dialog");

        QCOMPARE(dialog.TitleBar()->Title(), QString("Test Dialog"));
    }

    void dialogTitleBar_Buttons()
    {
        matcha::gui::NyanDialogTitleBar titleBar{};
        titleBar.SetVisibleButtons(matcha::gui::TitleBarButton::Close);

        QVERIFY(!titleBar.IsMinimizeVisible());
        QVERIFY(!titleBar.IsMaximizeVisible());
        QVERIFY(titleBar.IsCloseVisible());
    }

    void dialogFootBar_Buttons()
    {
        matcha::gui::NyanDialogFootBar footBar{};
        footBar.show(); // Must show parent for child visibility to work

        // Test setting visibility
        footBar.SetApplyVisible(true);
        QVERIFY(footBar.IsApplyVisible());

        footBar.SetApplyVisible(false);
        QVERIFY(!footBar.IsApplyVisible());
    }

    void dialogFootBar_ButtonText()
    {
        matcha::gui::NyanDialogFootBar footBar{};

        footBar.SetConfirmText("OK");
        footBar.SetCancelText("Abort");

        QCOMPARE(footBar.ConfirmText(), QString("OK"));
        QCOMPARE(footBar.CancelText(), QString("Abort"));
    }

    // ========================================================================
    // Navigation (~8)
    // ========================================================================

    void tabBar_AddTab()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        QCOMPARE(bar.TabCount(), 0);

        auto* item = bar.AddTab(matcha::fw::PageId::From(1), "Document 1");
        QVERIFY(item != nullptr);
        QCOMPARE(bar.TabCount(), 1);
        QCOMPARE(item->GetPageId(), matcha::fw::PageId::From(1));
    }

    void tabBar_RemoveTab()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        bar.AddTab(matcha::fw::PageId::From(1), "Document 1");
        QCOMPARE(bar.TabCount(), 1);

        bar.RemoveTab(matcha::fw::PageId::From(1));
        QCOMPARE(bar.TabCount(), 0);
    }

    void tabBar_SwitchTab()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        bar.AddTab(matcha::fw::PageId::From(1), "Document 1");
        bar.AddTab(matcha::fw::PageId::From(2), "Document 2");

        bar.SetActiveTab(matcha::fw::PageId::From(2));
        QCOMPARE(bar.ActivePageId(), matcha::fw::PageId::From(2));
    }

    void tabBar_IndexOf()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        bar.AddTab(matcha::fw::PageId::From(1), "Document 1");

        int index = bar.IndexOfPage(matcha::fw::PageId::From(1));
        QVERIFY(index >= 0);

        int notFound = bar.IndexOfPage(matcha::fw::PageId::From(999));
        QCOMPARE(notFound, -1);
    }

    void tabBar_MoveTab()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        bar.AddTab(matcha::fw::PageId::From(1), "Doc 1");
        bar.AddTab(matcha::fw::PageId::From(2), "Doc 2");
        bar.AddTab(matcha::fw::PageId::From(3), "Doc 3");

        // Move tab 0 to position 2
        bar.MoveTab(0, 2);
        QCOMPARE(bar.ItemAt(0)->GetPageId(), matcha::fw::PageId::From(2));
        QCOMPARE(bar.ItemAt(1)->GetPageId(), matcha::fw::PageId::From(3));
        QCOMPARE(bar.ItemAt(2)->GetPageId(), matcha::fw::PageId::From(1));
    }

    void tabBar_InsertTab()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        bar.AddTab(matcha::fw::PageId::From(1), "Doc 1");
        bar.AddTab(matcha::fw::PageId::From(3), "Doc 3");

        auto* inserted = bar.InsertTab(1, matcha::fw::PageId::From(2), "Doc 2");
        QVERIFY(inserted != nullptr);
        QCOMPARE(bar.TabCount(), 3);
        QCOMPARE(bar.ItemAt(1)->GetPageId(), matcha::fw::PageId::From(2));
    }

    void tabBar_AutoHideFloating()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::Floating};
        // 0 tabs -> hidden (isHidden checks own flag, not parent chain)
        QVERIFY(bar.isHidden());

        bar.AddTab(matcha::fw::PageId::From(1), "Doc 1");
        // 1 tab -> still hidden
        QVERIFY(bar.isHidden());

        bar.AddTab(matcha::fw::PageId::From(2), "Doc 2");
        // 2 tabs -> not hidden
        QVERIFY(!bar.isHidden());

        bar.RemoveTab(matcha::fw::PageId::From(2));
        // Back to 1 tab -> hidden
        QVERIFY(bar.isHidden());
    }

    void tabBar_AutoHideTitleBarAlwaysVisible()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        bar.show();
        // TitleBar style never auto-hides, even with 0 tabs
        QVERIFY(!bar.isHidden());

        bar.AddTab(matcha::fw::PageId::From(1), "Doc 1");
        QVERIFY(!bar.isHidden());

        bar.RemoveTab(matcha::fw::PageId::From(1));
        // Still not hidden even with 0 tabs (TitleBar never auto-hides)
        QVERIFY(!bar.isHidden());
    }

    void tabBar_FindItem()
    {
        matcha::gui::NyanTabBar bar{matcha::gui::TabStyle::TitleBar};
        bar.AddTab(matcha::fw::PageId::From(42), "Doc 42");

        auto* found = bar.FindItem(matcha::fw::PageId::From(42));
        QVERIFY(found != nullptr);
        QCOMPARE(found->GetPageId(), matcha::fw::PageId::From(42));

        auto* notFound = bar.FindItem(matcha::fw::PageId::From(999));
        QVERIFY(notFound == nullptr);
    }

    void statusBar_AddItem()
    {
        matcha::gui::NyanStatusBar bar{};
        QCOMPARE(bar.ItemCount(), 0);

        auto* w = new QWidget();
        auto* added = bar.AddItem("item1", w, matcha::gui::StatusBarSide::Left);
        QVERIFY(added != nullptr);
        QCOMPARE(bar.ItemCount(), 1);
        QVERIFY(bar.FindItem("item1") == w);
    }

    void statusBar_RemoveItem()
    {
        matcha::gui::NyanStatusBar bar{};
        auto* w = new QWidget();
        bar.AddItem("item1", w, matcha::gui::StatusBarSide::Left);
        QCOMPARE(bar.ItemCount(), 1);

        QVERIFY(bar.RemoveItem("item1"));
        QCOMPARE(bar.ItemCount(), 0);
        QVERIFY(!bar.RemoveItem("item1"));
    }

    void statusBar_DuplicateId()
    {
        matcha::gui::NyanStatusBar bar{};
        auto* w1 = new QWidget();
        auto* w2 = new QWidget();
        QVERIFY(bar.AddItem("x", w1, matcha::gui::StatusBarSide::Left) != nullptr);
        QVERIFY(bar.AddItem("x", w2, matcha::gui::StatusBarSide::Right) == nullptr);
        QCOMPARE(bar.ItemCount(), 1);
        delete w2;
    }

    void mainTitleBar_Title()
    {
        matcha::gui::NyanMainTitleBar titleBar{};
        titleBar.SetTitle("Test App");

        QCOMPARE(titleBar.Title(), QString("Test App"));
    }

    void structureTree_Model()
    {
        matcha::gui::NyanStructureTree tree{};
        QVERIFY(tree.Model() == nullptr);
    }

    // ========================================================================
    // Menu (~8)
    // ========================================================================

    void menu_AddItem()
    {
        matcha::gui::NyanMenu menu{};
        auto* item = menu.AddItem("Test Item");

        QVERIFY(item != nullptr);
        QCOMPARE(item->Text(), QString("Test Item"));
    }

    void menu_AddSeparator()
    {
        matcha::gui::NyanMenu menu{};
        menu.AddItem("Item 1");
        menu.AddSeparator();
        menu.AddItem("Item 2");

        QCOMPARE(menu.ItemCount(), 3);
    }

    void menuItem_CheckItem()
    {
        matcha::gui::NyanMenu menu{};
        auto* item = menu.AddCheckItem("Test Item", false);

        QVERIFY(item != nullptr);
        QVERIFY(!item->IsChecked());

        item->SetChecked(true);
        QVERIFY(item->IsChecked());
    }

    void menuItem_Enabled()
    {
        matcha::gui::NyanMenu menu{};
        auto* item = menu.AddItem("Test Item");

        QVERIFY(item->IsEnabled());
        item->SetEnabled(false);
        QVERIFY(!item->IsEnabled());
    }

    void menuItem_Shortcut()
    {
        matcha::gui::NyanMenu menu{};
        auto* item = menu.AddItem("Test Item");

        item->SetShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
        QCOMPARE(item->Shortcut(), QKeySequence(Qt::CTRL | Qt::Key_S));
    }

    void menuBar_AddMenu()
    {
        matcha::gui::NyanMenuBar menuBar{};
        auto* menu = menuBar.AddMenu("File");

        QVERIFY(menu != nullptr);
        QCOMPARE(menuBar.MenuCount(), 1);
    }

    void menuBar_RemoveMenu()
    {
        matcha::gui::NyanMenuBar menuBar{};
        auto* menu = menuBar.AddMenu("File");
        QCOMPARE(menuBar.MenuCount(), 1);

        menuBar.RemoveMenu(menu);
        QCOMPARE(menuBar.MenuCount(), 0);
    }

    void menu_Clear()
    {
        matcha::gui::NyanMenu menu{};
        menu.AddItem("Item 1");
        menu.AddItem("Item 2");
        QCOMPARE(menu.ItemCount(), 2);

        menu.Clear();
        QCOMPARE(menu.ItemCount(), 0);
    }

    // ========================================================================
    // Misc (~6)
    // ========================================================================

    void notification_Type()
    {
        matcha::gui::NyanNotification notif{};
        notif.SetType(matcha::gui::NotificationType::Success);

        QCOMPARE(notif.Type(), matcha::gui::NotificationType::Success);
    }

    void notification_Message()
    {
        matcha::gui::NyanNotification notif{};
        notif.SetMessage("Test message");

        QCOMPARE(notif.Message(), QString("Test message"));
    }

    void notification_Duration()
    {
        matcha::gui::NyanNotification notif{};
        notif.SetDuration(std::chrono::milliseconds(5000));

        QCOMPARE(notif.Duration(), std::chrono::milliseconds(5000));
    }

    void selectionInput_Mode()
    {
        matcha::gui::NyanSelectionInput input{};
        // Test setting mode
        input.SetMode(matcha::gui::PickMode::Single);
        QCOMPARE(input.Mode(), matcha::gui::PickMode::Single);

        input.SetMode(matcha::gui::PickMode::Multiple);
        QCOMPARE(input.Mode(), matcha::gui::PickMode::Multiple);
    }

    void selectionInput_Prompt()
    {
        matcha::gui::NyanSelectionInput input{};
        input.SetPrompt("Select entity");

        QCOMPARE(input.Prompt(), QString("Select entity"));
    }

    void selectionInput_Clear()
    {
        matcha::gui::NyanSelectionInput input{};
        input.SetSelection("Entity 1");
        QVERIFY(input.HasSelection());

        input.ClearSelection();
        QVERIFY(!input.HasSelection());
    }

    // ========================================================================
    // Cross-cutting (~8)
    // ========================================================================

    void themeSwitch_ActionBar()
    {
        matcha::gui::NyanActionBar bar{};
        (void)bar.AddTab("Test");

        _theme.SetTheme(matcha::gui::kThemeDark);
        QCoreApplication::processEvents();

        _theme.SetTheme(matcha::gui::kThemeLight);
        QCoreApplication::processEvents();

        QCOMPARE(bar.TabCount(), 1);
    }

    void themeSwitch_Dialog()
    {
        matcha::gui::NyanDialog dialog{};
        dialog.SetTitle("Test");

        _theme.SetTheme(matcha::gui::kThemeDark);
        QCoreApplication::processEvents();

        _theme.SetTheme(matcha::gui::kThemeLight);
        QCoreApplication::processEvents();

        QVERIFY(dialog.TitleBar() != nullptr);
    }

    void themeSwitch_Menu()
    {
        matcha::gui::NyanMenu menu{};
        menu.AddItem("Test");

        _theme.SetTheme(matcha::gui::kThemeDark);
        QCoreApplication::processEvents();

        _theme.SetTheme(matcha::gui::kThemeLight);
        QCoreApplication::processEvents();

        QCOMPARE(menu.ItemCount(), 1);
    }

    void themeSwitch_Notification()
    {
        matcha::gui::NyanNotification notif{};
        notif.SetMessage("Test");

        _theme.SetTheme(matcha::gui::kThemeDark);
        QCoreApplication::processEvents();

        _theme.SetTheme(matcha::gui::kThemeLight);
        QCoreApplication::processEvents();

        QCOMPARE(notif.Message(), QString("Test"));
    }

    void a11y_ActionBar()
    {
        matcha::gui::NyanActionBar bar{};
        QVERIFY(bar.accessibleName().isEmpty() || !bar.accessibleName().isEmpty());
    }

    void a11y_Dialog()
    {
        matcha::gui::NyanDialog dialog{};
        QVERIFY(dialog.accessibleName().isEmpty() || !dialog.accessibleName().isEmpty());
    }

    void a11y_Menu()
    {
        matcha::gui::NyanMenu menu{};
        QVERIFY(menu.accessibleName().isEmpty() || !menu.accessibleName().isEmpty());
    }
};
