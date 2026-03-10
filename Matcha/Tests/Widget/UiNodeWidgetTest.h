#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/UiNodes/Core/UiNode.h>
#include <Matcha/UiNodes/Core/WidgetNode.h>
#include <Matcha/UiNodes/Core/WidgetWrapper.h>
#include <Matcha/UiNodes/Controls/LineEditNode.h>
#include <Matcha/UiNodes/Controls/ComboBoxNode.h>
#include <Matcha/UiNodes/Controls/SpinBoxNode.h>
#include <Matcha/UiNodes/Controls/CheckBoxNode.h>
#include <Matcha/UiNodes/Controls/ToggleSwitchNode.h>
#include <Matcha/Widgets/Core/IThemeService.h>
#include <Matcha/Widgets/Core/NyanTheme.h>

#include <QTest>
#include <QWidget>

#include <memory>
#include <string>

extern matcha::test::WidgetTestFixture* gFixture;

// Test subclass for UiNode (non-QWidget)
class TestUiNode : public matcha::fw::UiNode {
public:
    explicit TestUiNode(std::string id,
                        matcha::fw::NodeType type = matcha::fw::NodeType::Custom)
        : UiNode(std::move(id), type)
    {
    }
};

// Test subclass for WidgetNode
class TestWidgetNode : public matcha::fw::WidgetNode {
public:
    explicit TestWidgetNode(std::string id)
        : WidgetNode(std::move(id), matcha::fw::NodeType::Custom)
    {
    }

    bool createWidgetCalled = false;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override {
        createWidgetCalled = true;
        return new QWidget(parent);
    }
};

class UiNodeWidgetTest : public QObject {
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

    // -- WidgetWrapper --

    void wrapperWrapsQWidget()
    {
        QWidget w;
        matcha::fw::WidgetWrapper wrapper("w1", &w);
        QCOMPARE(wrapper.Widget(), &w);
        QCOMPARE(wrapper.Type(), matcha::fw::NodeType::WidgetWrapper);
    }

    void wrapperNullptrWidget()
    {
        matcha::fw::WidgetWrapper wrapper("w2", nullptr);
        QCOMPARE(wrapper.Widget(), nullptr);
    }

    void wrapperAsUiNodeChild()
    {
        TestUiNode root("root", matcha::fw::NodeType::Shell);
        QWidget w;
        auto* child = root.AddNode(
            std::make_unique<matcha::fw::WidgetWrapper>("wrapped", &w));
        QVERIFY(child != nullptr);
        QCOMPARE(child->Widget(), &w);
        QCOMPARE(root.FindById("wrapped"), child);
    }

    void wrapperMetaClassRtti()
    {
        QWidget w;
        matcha::fw::WidgetWrapper wrapper("rtti", &w);
        QVERIFY(wrapper.IsAKindOf("WidgetWrapper"));
        QVERIFY(wrapper.IsAKindOf("UiNode"));
        QVERIFY(wrapper.IsAKindOf("CommandNode"));
        QVERIFY(wrapper.IsAKindOf("BaseObject"));
    }

    // -- WidgetNode --

    void widgetNodeLazyCreation()
    {
        TestWidgetNode node("lazy1");
        QVERIFY(!node.createWidgetCalled);
        QVERIFY(node.Widget() != nullptr);
        QVERIFY(node.createWidgetCalled);
    }

    void widgetNodeSetEnabled()
    {
        TestWidgetNode node("en1");
        node.SetEnabled(false);
        QVERIFY(!node.IsEnabled());
        node.SetEnabled(true);
        QVERIFY(node.IsEnabled());
    }

    void widgetNodeSetToolTip()
    {
        TestWidgetNode node("tip1");
        node.SetToolTip("Hello");
        QCOMPARE(node.Widget()->toolTip(), QString("Hello"));
    }

    void widgetNodeSetMinimumSize()
    {
        TestWidgetNode node("sz1");
        node.SetMinimumSize(100, 50);
        QCOMPARE(node.Widget()->minimumWidth(), 100);
        QCOMPARE(node.Widget()->minimumHeight(), 50);
    }

    void widgetNodeSetFixedSize()
    {
        TestWidgetNode node("fixed1");
        node.SetFixedSize(200, 150);
        QCOMPARE(node.Widget()->minimumWidth(), 200);
        QCOMPARE(node.Widget()->maximumWidth(), 200);
        QCOMPARE(node.Widget()->minimumHeight(), 150);
        QCOMPARE(node.Widget()->maximumHeight(), 150);
    }

    void widgetNodeMetaClassRtti()
    {
        TestWidgetNode node("rtti2");
        QVERIFY(node.IsAKindOf("WidgetNode"));
        QVERIFY(node.IsAKindOf("UiNode"));
        QVERIFY(node.IsAKindOf("CommandNode"));
        QVERIFY(node.IsAKindOf("BaseObject"));
    }

    // -- LineEditNode --

    void lineEditNodeSetText()
    {
        matcha::fw::LineEditNode node("le1");
        node.SetText("hello");
        QCOMPARE(QString::fromStdString(node.Text()), QString("hello"));
    }

    void lineEditNodePlaceholder()
    {
        matcha::fw::LineEditNode node("le2");
        node.SetPlaceholder("Enter value");
        QVERIFY(node.Widget() != nullptr);
    }

    void lineEditNodeReadOnly()
    {
        matcha::fw::LineEditNode node("le3");
        node.SetReadOnly(true);
        QVERIFY(node.Widget() != nullptr);
    }

    void lineEditNodeMaxLength()
    {
        matcha::fw::LineEditNode node("le4");
        node.SetMaxLength(10);
        QVERIFY(node.Widget() != nullptr);
    }

    void lineEditNodeRtti()
    {
        matcha::fw::LineEditNode node("le6");
        QVERIFY(node.IsAKindOf("LineEditNode"));
        QVERIFY(node.IsAKindOf("WidgetNode"));
        QVERIFY(node.IsAKindOf("UiNode"));
    }

    // -- ComboBoxNode --

    void comboBoxNodeAddItem()
    {
        matcha::fw::ComboBoxNode node("cb1");
        node.AddItem("Alpha");
        node.AddItem("Beta");
        node.SetCurrentIndex(1);
        QCOMPARE(node.CurrentIndex(), 1);
        QCOMPARE(QString::fromStdString(node.CurrentText()), QString("Beta"));
    }

    void comboBoxNodeAddItems()
    {
        matcha::fw::ComboBoxNode node("cb2");
        std::vector<std::string> items = {"A", "B", "C"};
        node.AddItems(items);
        QCOMPARE(node.CurrentIndex(), 0);
    }

    void comboBoxNodeClear()
    {
        matcha::fw::ComboBoxNode node("cb3");
        node.AddItem("X");
        node.Clear();
        QCOMPARE(node.CurrentIndex(), -1);
    }

    void comboBoxNodeRtti()
    {
        matcha::fw::ComboBoxNode node("cb5");
        QVERIFY(node.IsAKindOf("ComboBoxNode"));
        QVERIFY(node.IsAKindOf("WidgetNode"));
    }

    // -- SpinBoxNode --

    void spinBoxNodeSetValue()
    {
        matcha::fw::SpinBoxNode node("sb1");
        node.SetRange(0, 100);
        node.SetValue(42);
        QCOMPARE(node.Value(), 42);
    }

    void spinBoxNodeRtti()
    {
        matcha::fw::SpinBoxNode node("sb3");
        QVERIFY(node.IsAKindOf("SpinBoxNode"));
        QVERIFY(node.IsAKindOf("WidgetNode"));
    }

    // -- CheckBoxNode --

    void checkBoxNodeSetChecked()
    {
        matcha::fw::CheckBoxNode node("chk1");
        QVERIFY(!node.IsChecked());
        node.SetChecked(true);
        QVERIFY(node.IsChecked());
    }

    void checkBoxNodeSetText()
    {
        matcha::fw::CheckBoxNode node("chk2");
        node.SetText("Option A");
        QVERIFY(node.Widget() != nullptr);
    }

    void checkBoxNodeRtti()
    {
        matcha::fw::CheckBoxNode node("chk4");
        QVERIFY(node.IsAKindOf("CheckBoxNode"));
        QVERIFY(node.IsAKindOf("WidgetNode"));
    }

    // -- ToggleSwitchNode --

    void toggleSwitchNodeSetChecked()
    {
        matcha::fw::ToggleSwitchNode node("ts1");
        QVERIFY(!node.IsChecked());
        node.SetChecked(true);
        QVERIFY(node.IsChecked());
    }

    void toggleSwitchNodeRtti()
    {
        matcha::fw::ToggleSwitchNode node("ts2");
        QVERIFY(node.IsAKindOf("ToggleSwitchNode"));
        QVERIFY(node.IsAKindOf("WidgetNode"));
    }
};
