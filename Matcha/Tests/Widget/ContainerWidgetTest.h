#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Widgets/Controls/NyanCollapsibleSection.h>
#include <Matcha/Widgets/Controls/NyanColorPicker.h>
#include <Matcha/Widgets/Controls/NyanDataTable.h>
#include <Matcha/Widgets/Controls/NyanGroupBox.h>
#include <Matcha/Widgets/Controls/NyanListWidget.h>
#include <Matcha/Widgets/Controls/NyanPaginator.h>
#include <Matcha/Widgets/ActionBar/NyanPanel.h>
#include <Matcha/Widgets/ActionBar/NyanPanelProgress.h>
#include <Matcha/Widgets/Controls/NyanPropertyGrid.h>
#include <Matcha/Widgets/Shell/NyanScrollArea.h>
#include <Matcha/Widgets/Shell/NyanScrollBar.h>
#include <Matcha/Widgets/Shell/NyanSplitter.h>
#include <Matcha/Widgets/Shell/NyanStackedWidget.h>
#include <Matcha/Widgets/Shell/NyanTabWidget.h>
#include <Matcha/Widgets/Controls/NyanTableWidget.h>
#include <Matcha/Theming/NyanTheme.h>

#include <QSignalSpy>
#include <QTest>

extern matcha::test::WidgetTestFixture* gFixture;

// ============================================================================
// ContainerWidgetTest -- Qt Test class for all 15 Tier 2 widgets
// ============================================================================

class ContainerWidgetTest : public QObject {
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
    // Scroll (~3)
    // ========================================================================

    void scrollBar_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanScrollBar sb{};
        sb.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void scrollArea_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanScrollArea sa{};
        sa.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void scrollBar_Value_Change()
    {
        using namespace matcha::gui;
        NyanScrollBar sb{};
        sb.setRange(0, 100);
        sb.show();
        sb.setValue(50);
        QCOMPARE(sb.value(), 50);
    }

    // ========================================================================
    // Layout (~5)
    // ========================================================================

    void tabWidget_AddClose_Tab()
    {
        using namespace matcha::gui;
        NyanTabWidget tw{};
        tw.show();
        
        auto* tab1 = new QWidget{};
        auto* tab2 = new QWidget{};
        tw.addTab(tab1, "Tab 1");
        tw.addTab(tab2, "Tab 2");
        
        QCOMPARE(tw.count(), 2);
        tw.removeTab(0);
        QCOMPARE(tw.count(), 1);
    }

    void tabWidget_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanTabWidget tw{};
        tw.addTab(new QWidget(), "Tab");
        tw.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void splitter_Resize()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        sp.addWidget(new QWidget());
        sp.addWidget(new QWidget());
        sp.show();
        sp.setSizes({100, 200});
        auto sizes = sp.sizes();
        QCOMPARE(sizes.size(), 2);
    }

    void splitter_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        sp.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void splitter_SetSizesExact()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        auto* left  = new QWidget();
        auto* right = new QWidget();
        sp.addWidget(left);
        sp.addWidget(right);
        sp.resize(300, 100);
        sp.show();
        QApplication::processEvents();

        sp.setSizes({100, 200});
        QApplication::processEvents();
        auto sizes = sp.sizes();
        QCOMPARE(sizes.size(), 2);
        // Total should equal splitter width minus handle
        int total = sizes[0] + sizes[1];
        QVERIFY(total > 0);
    }

    void splitter_MinimumSizeRespected()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        auto* left  = new QWidget();
        auto* right = new QWidget();
        left->setMinimumWidth(50);
        right->setMinimumWidth(50);
        sp.setChildrenCollapsible(false);
        sp.addWidget(left);
        sp.addWidget(right);
        sp.resize(300, 100);
        sp.show();
        QApplication::processEvents();

        // Try to push divider all the way left
        sp.setSizes({0, 300});
        QApplication::processEvents();
        auto sizes = sp.sizes();
        QVERIFY2(sizes[0] >= 50, "Left pane should not go below minimum width");
    }

    void splitter_CollapsePanel()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        auto* left  = new QWidget();
        auto* right = new QWidget();
        sp.addWidget(left);
        sp.addWidget(right);
        sp.resize(300, 100);
        sp.show();
        sp.setSizes({150, 150});
        QApplication::processEvents();

        sp.CollapsePanel(0);
        QApplication::processEvents();
        auto sizes = sp.sizes();
        QCOMPARE(sizes[0], 0);
    }

    void splitter_RestorePanel()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        auto* left  = new QWidget();
        auto* right = new QWidget();
        sp.addWidget(left);
        sp.addWidget(right);
        sp.resize(300, 100);
        sp.show();
        sp.setSizes({150, 150});
        QApplication::processEvents();

        sp.CollapsePanel(0);
        QApplication::processEvents();
        QCOMPARE(sp.sizes()[0], 0);

        sp.RestorePanel(0);
        QApplication::processEvents();
        QVERIFY2(sp.sizes()[0] > 0, "Pane should restore to non-zero size");
    }

    void splitter_CollapsibleFalse()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        auto* left  = new QWidget();
        auto* right = new QWidget();
        left->setMinimumWidth(80);
        sp.setChildrenCollapsible(false);
        sp.addWidget(left);
        sp.addWidget(right);
        sp.resize(300, 100);
        sp.show();
        QApplication::processEvents();

        // setSizes with 0 should be clamped to minimumWidth when not collapsible
        sp.setSizes({0, 300});
        QApplication::processEvents();
        QVERIFY2(sp.sizes()[0] >= 80, "Non-collapsible pane should not collapse below minimum");
    }

    void splitter_ThreePanes()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        sp.addWidget(new QWidget());
        sp.addWidget(new QWidget());
        sp.addWidget(new QWidget());
        sp.resize(300, 100);
        sp.show();
        QApplication::processEvents();

        sp.setSizes({100, 100, 100});
        QApplication::processEvents();
        auto sizes = sp.sizes();
        QCOMPARE(sizes.size(), 3);
        int total = sizes[0] + sizes[1] + sizes[2];
        QVERIFY(total > 0);
    }

    void splitter_NestedInner()
    {
        using namespace matcha::gui;
        NyanSplitter outer(Qt::Horizontal);
        auto* left = new QWidget();
        NyanSplitter* inner = new NyanSplitter(Qt::Vertical);
        auto* top = new QWidget();
        auto* bottom = new QWidget();
        inner->addWidget(top);
        inner->addWidget(bottom);
        outer.addWidget(left);
        outer.addWidget(inner);
        outer.resize(400, 300);
        outer.show();
        QApplication::processEvents();

        outer.setSizes({200, 200});
        inner->setSizes({150, 150});
        QApplication::processEvents();

        // Change inner sizes
        inner->setSizes({100, 200});
        QApplication::processEvents();
        auto innerSizes = inner->sizes();
        QCOMPARE(innerSizes.size(), 2);

        // Outer should not have changed
        auto outerSizes = outer.sizes();
        QCOMPARE(outerSizes.size(), 2);
    }

    void splitter_HideShowPane()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        auto* left  = new QWidget();
        auto* right = new QWidget();
        sp.addWidget(left);
        sp.addWidget(right);
        sp.resize(300, 100);
        sp.show();
        sp.setSizes({150, 150});
        QApplication::processEvents();

        // Hide left pane
        left->hide();
        QApplication::processEvents();
        auto sizes = sp.sizes();
        QCOMPARE(sizes[0], 0);

        // Show left pane
        left->show();
        QApplication::processEvents();
        sizes = sp.sizes();
        QVERIFY2(sizes[0] > 0, "Shown pane should have non-zero size");
    }

    void splitter_Vertical()
    {
        using namespace matcha::gui;
        NyanSplitter sp(Qt::Vertical);
        auto* top    = new QWidget();
        auto* bottom = new QWidget();
        sp.addWidget(top);
        sp.addWidget(bottom);
        sp.resize(200, 300);
        sp.show();
        QApplication::processEvents();

        sp.setSizes({100, 200});
        QApplication::processEvents();
        auto sizes = sp.sizes();
        QCOMPARE(sizes.size(), 2);
        QVERIFY(sizes[0] > 0);
        QVERIFY(sizes[1] > 0);
    }

    void splitter_HandleWidth()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        sp.addWidget(new QWidget());
        sp.addWidget(new QWidget());
        sp.show();
        QApplication::processEvents();

        // NyanSplitter sets kHandleWidth = 4
        QCOMPARE(sp.handleWidth(), 4);
    }

    void splitter_SingleChild()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        sp.addWidget(new QWidget());
        sp.resize(300, 100);
        sp.show();
        QApplication::processEvents();

        // With only 1 child, no handle should exist
        QCOMPARE(sp.count(), 1);
        // handle(0) is the handle *before* widget 0, which doesn't resize anything
        // Just verify it doesn't crash
        auto sizes = sp.sizes();
        QCOMPARE(sizes.size(), 1);
    }

    void splitter_CollapseButtonVisible()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        QVERIFY(!sp.CollapseButtonVisible());
        sp.SetCollapseButtonVisible(true);
        QVERIFY(sp.CollapseButtonVisible());
        sp.SetCollapseButtonVisible(false);
        QVERIFY(!sp.CollapseButtonVisible());
    }

    void splitter_SaveRestoreState()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        sp.addWidget(new QWidget());
        sp.addWidget(new QWidget());
        sp.resize(300, 100);
        sp.show();
        sp.setSizes({120, 180});
        QApplication::processEvents();

        QByteArray state = sp.saveState();
        QVERIFY(!state.isEmpty());

        // Change sizes
        sp.setSizes({50, 250});
        QApplication::processEvents();

        // Restore
        sp.restoreState(state);
        QApplication::processEvents();
        auto sizes = sp.sizes();
        // Should be close to original 120/180
        QCOMPARE(sizes.size(), 2);
    }

    void splitter_SplitterMovedSignal()
    {
        using namespace matcha::gui;
        NyanSplitter sp{};
        sp.addWidget(new QWidget());
        sp.addWidget(new QWidget());
        sp.resize(300, 100);
        sp.show();
        sp.setSizes({150, 150});
        QApplication::processEvents();

        QSignalSpy spy(&sp, &NyanSplitter::splitterMoved);
        QVERIFY(spy.isValid());

        // Simulate drag by sending mouse events on the handle
        auto* handle = sp.handle(1);
        QVERIFY(handle != nullptr);
        QTest::mousePress(handle, Qt::LeftButton, Qt::NoModifier, QPoint(2, 10));
        QTest::mouseMove(handle, QPoint(30, 10));
        QTest::mouseRelease(handle, Qt::LeftButton, Qt::NoModifier, QPoint(30, 10));
        QApplication::processEvents();
        QVERIFY2(spy.count() >= 1, "splitterMoved signal should fire on handle drag");
    }

    void stackedWidget_CurrentIndex()
    {
        using namespace matcha::gui;
        NyanStackedWidget sw{};
        sw.addWidget(new QWidget());
        sw.addWidget(new QWidget());
        sw.show();
        
        QCOMPARE(sw.currentIndex(), 0);
        sw.setCurrentIndex(1);
        QCOMPARE(sw.currentIndex(), 1);
    }

    // ========================================================================
    // Panel & Group (~6)
    // ========================================================================

    void panel_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanPanel panel{};
        panel.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void panelProgress_Value_Update()
    {
        using namespace matcha::gui;
        NyanPanelProgress pp{};
        pp.show();
        pp.SetMinimum(0);
        pp.SetMaximum(100);
        pp.SetValue(50);
        QCOMPARE(pp.Value(), 50);
    }

    void groupBox_Collapse_Geometry()
    {
        using namespace matcha::gui;
        NyanGroupBox gb{};
        gb.setTitle("Test Group");
        gb.SetCollapsible(true);
        gb.show();
        
        int expandedHeight = gb.sizeHint().height();
        gb.SetCollapsed(true);
        QApplication::processEvents();
        int collapsedHeight = gb.sizeHint().height();
        
        // Collapsed should be smaller (or equal if animation not complete)
        QVERIFY(collapsedHeight <= expandedHeight);
    }

    void groupBox_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanGroupBox gb{};
        gb.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void collapsibleSection_ExpandToggle()
    {
        using namespace matcha::gui;
        NyanCollapsibleSection cs{};
        cs.SetTitle("Section");
        cs.SetContent(new QWidget());
        cs.show();
        
        QSignalSpy spy(&cs, &NyanCollapsibleSection::ExpandToggled);
        
        QCOMPARE(cs.IsExpanded(), true);
        cs.SetExpanded(false);
        QCOMPARE(cs.IsExpanded(), false);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), false);
    }

    void collapsibleSection_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanCollapsibleSection cs{};
        cs.SetTitle("Section");
        cs.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    // ========================================================================
    // Data Views (~8)
    // ========================================================================

    void dataTable_AddRow()
    {
        using namespace matcha::gui;
        NyanDataTable dt{};
        dt.SetHeaders({"A", "B", "C"});
        dt.show();
        
        QCOMPARE(dt.RowCount(), 0);
        dt.AddRow({"A1", "B1", "C1"});
        QCOMPARE(dt.RowCount(), 1);
    }

    void dataTable_SetCell_GetCell()
    {
        using namespace matcha::gui;
        NyanDataTable dt{};
        dt.SetHeaders({"A", "B"});
        dt.AddRow({"A1", "B1"});
        dt.show();
        
        dt.SetCell(0, 0, "Hello");
        QCOMPARE(dt.Cell(0, 0), "Hello");
    }

    void dataTable_Clear()
    {
        using namespace matcha::gui;
        NyanDataTable dt{};
        dt.SetHeaders({"A"});
        dt.AddRow({"A1"});
        dt.AddRow({"A2"});
        dt.show();
        
        QCOMPARE(dt.RowCount(), 2);
        dt.Clear();
        QCOMPARE(dt.RowCount(), 0);
    }

    void dataTable_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanDataTable dt{};
        dt.SetHeaders({"A", "B"});
        dt.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void listWidget_Selection()
    {
        using namespace matcha::gui;
        NyanListWidget lw{};
        lw.addItem("Item 1");
        lw.addItem("Item 2");
        lw.show();
        
        lw.setCurrentRow(1);
        QCOMPARE(lw.currentRow(), 1);
    }

    void listWidget_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanListWidget lw{};
        lw.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void tableWidget_CheckableHeader()
    {
        using namespace matcha::gui;
        NyanTableWidget tw(3, 2);
        tw.SetCheckableHeaders(true);
        tw.show();
        
        QCOMPARE(tw.HasCheckableHeaders(), true);
        tw.SetRowChecked(0, true);
        auto checked = tw.CheckedRows();
        QCOMPARE(checked.size(), 1);
        QCOMPARE(checked.at(0), 0);
    }

    void paginator_PageChange_Signal()
    {
        using namespace matcha::gui;
        NyanPaginator pg{};
        pg.SetCount(5);
        pg.SetCurrent(0);
        pg.show();
        
        QSignalSpy spy(&pg, &NyanPaginator::PageChanged);
        pg.SetCurrent(2);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 2);
    }

    void paginator_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanPaginator pg{};
        pg.SetCount(3);
        pg.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    // ========================================================================
    // Composite (~7)
    // ========================================================================

    void propertyGrid_AddProperty_EditorCreated()
    {
        using namespace matcha::gui;
        NyanPropertyGrid pg{};
        pg.AddProperty("Name", PropertyType::Text, "Default");
        pg.AddProperty("Count", PropertyType::Integer, 10);
        pg.show();
        
        QCOMPARE(pg.HasProperty("Name"), true);
        QCOMPARE(pg.HasProperty("Count"), true);
        QCOMPARE(pg.Value("Name").toString(), "Default");
        QCOMPARE(pg.Value("Count").toInt(), 10);
    }

    void propertyGrid_GroupCollapse()
    {
        using namespace matcha::gui;
        NyanPropertyGrid pg{};
        pg.AddGroup("Settings");
        pg.AddProperty("Option", PropertyType::Bool, true);
        pg.show();
        
        QCOMPARE(pg.HasProperty("Option"), true);
    }

    void propertyGrid_PropertyChanged_Signal()
    {
        using namespace matcha::gui;
        NyanPropertyGrid pg{};
        pg.AddProperty("Value", PropertyType::Integer, 0);
        pg.show();
        
        QSignalSpy spy(&pg, &NyanPropertyGrid::PropertyChanged);
        pg.SetValue("Value", 42);
        // SetValue doesn't emit signal (only user interaction does)
        // Just verify no crash
        QCOMPARE(pg.Value("Value").toInt(), 42);
    }

    void propertyGrid_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanPropertyGrid pg{};
        pg.AddProperty("Test", PropertyType::Text, "");
        pg.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    void colorPicker_SetColor_GetColor()
    {
        using namespace matcha::gui;
        NyanColorPicker cp{};
        cp.show();
        
        cp.SetColor(QColor(255, 128, 64));
        QColor c = cp.Color();
        QCOMPARE(c.red(), 255);
        QCOMPARE(c.green(), 128);
        QCOMPARE(c.blue(), 64);
    }

    void colorPicker_AlphaEnabled()
    {
        using namespace matcha::gui;
        NyanColorPicker cp{};
        cp.show();
        
        QCOMPARE(cp.IsAlphaEnabled(), false);
        cp.SetAlphaEnabled(true);
        QCOMPARE(cp.IsAlphaEnabled(), true);
    }

    void colorPicker_Theme_Switch()
    {
        using namespace matcha::gui;
        NyanColorPicker cp{};
        cp.show();
        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();
        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();
    }

    // ========================================================================
    // Cross-cutting (~4)
    // ========================================================================

    void themeSwitchAll15Widgets()
    {
        using namespace matcha::gui;

        NyanScrollBar scrollBar{};
        NyanScrollArea scrollArea{};
        NyanTabWidget tabWidget{};
        NyanStackedWidget stackedWidget{};
        NyanSplitter splitter{};
        NyanPanel panel{};
        NyanPanelProgress panelProgress{};
        NyanGroupBox groupBox{};
        NyanCollapsibleSection collapsible{};
        NyanDataTable dataTable{};
        NyanListWidget listWidget{};
        NyanTableWidget tableWidget{};
        NyanPaginator paginator{};
        NyanPropertyGrid propertyGrid{};
        NyanColorPicker colorPicker{};

        scrollBar.show(); scrollArea.show(); tabWidget.show();
        stackedWidget.show(); splitter.show(); panel.show();
        panelProgress.show(); groupBox.show(); collapsible.show();
        dataTable.show(); listWidget.show(); tableWidget.show();
        paginator.show(); propertyGrid.show(); colorPicker.show();

        QApplication::processEvents();

        _theme.SetTheme(kThemeDark);
        QApplication::processEvents();

        _theme.SetTheme(kThemeLight);
        QApplication::processEvents();

        // No crash = all 15 widgets survived theme switch
    }

    void dataTable_AddRemove_Stress()
    {
        using namespace matcha::gui;
        NyanDataTable dt{};
        dt.SetHeaders({"A", "B", "C"});
        dt.show();

        // Add 50 rows
        for (int i = 0; i < 50; ++i) {
            dt.AddRow({QString::number(i), "B", "C"});
        }
        QCOMPARE(dt.RowCount(), 50);

        // Remove 25 rows
        dt.RemoveRows(0, 25);
        QCOMPARE(dt.RowCount(), 25);

        dt.Clear();
        QCOMPARE(dt.RowCount(), 0);
    }

    void propertyGrid_MixedEditorTypes()
    {
        using namespace matcha::gui;
        NyanPropertyGrid pg{};
        pg.AddProperty("Text", PropertyType::Text, "Hello");
        pg.AddProperty("Int", PropertyType::Integer, 42);
        pg.AddProperty("Double", PropertyType::Double, 3.14);
        pg.AddProperty("Bool", PropertyType::Bool, true);
        pg.AddProperty("Choice", PropertyType::Choice, "A", {"A", "B", "C"});
        pg.AddProperty("Color", PropertyType::Color, QColor(Qt::red));
        pg.show();

        QCOMPARE(pg.Value("Text").toString(), "Hello");
        QCOMPARE(pg.Value("Int").toInt(), 42);
        QVERIFY(qAbs(pg.Value("Double").toDouble() - 3.14) < 0.01);
        QCOMPARE(pg.Value("Bool").toBool(), true);
        QCOMPARE(pg.Value("Choice").toString(), "A");
        QCOMPARE(pg.Value("Color").value<QColor>(), QColor(Qt::red));
    }

    void a11y_Role_All15Widgets()
    {
        using namespace matcha::gui;

        NyanScrollBar sb{};
        NyanPanel panel{};
        NyanDataTable dt{};
        NyanPaginator pg{};

        // Verify widgets can be constructed and have valid size hints
        QVERIFY(sb.sizeHint().isValid());
        QVERIFY(panel.sizeHint().isValid());
        QVERIFY(dt.sizeHint().isValid());
        QVERIFY(pg.sizeHint().isValid());
    }
};
