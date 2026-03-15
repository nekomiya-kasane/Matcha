#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Tree/Composition/ActionBar/ActionBarNode.h>
#include <Matcha/Tree/Composition/Document/DocumentArea.h>
#include <Matcha/Tree/Composition/Shell/Shell.h>
#include <Matcha/Tree/Composition/Shell/StatusBarNode.h>
#include <Matcha/Tree/Composition/Shell/WindowNode.h>
#include <Matcha/Tree/Composition/Shell/WorkspaceFrame.h>
#include <Matcha/Theming/IThemeService.h>
#include <Matcha/Theming/NyanTheme.h>

#include <QTest>

extern matcha::test::WidgetTestFixture* gFixture;

class ShellWindowTest : public QObject {
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

    void windowNode_BuildWindow_createsQMainWindow()
    {
        matcha::fw::WindowNode win("test-win", matcha::fw::WindowId::From(1),
                                    matcha::fw::WindowKind::Main);
        QVERIFY(!win.IsBuilt());
        win.BuildWindow(nullptr);
        QVERIFY(win.IsBuilt());
        QVERIFY(win.Widget() != nullptr);
    }

    void windowNode_hasWorkspaceFrameChild()
    {
        matcha::fw::WindowNode win("test-win", matcha::fw::WindowId::From(1),
                                    matcha::fw::WindowKind::Main);
        win.BuildWindow(nullptr);

        bool found = false;
        for (size_t i = 0; i < win.NodeCount(); ++i) {
            if (win.NodeAt(i)->Type() == matcha::fw::NodeType::WorkspaceFrame) {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }

    void windowNode_hasStatusBarChild()
    {
        matcha::fw::WindowNode win("test-win", matcha::fw::WindowId::From(1),
                                    matcha::fw::WindowKind::Main);
        win.BuildWindow(nullptr);

        bool found = false;
        for (size_t i = 0; i < win.NodeCount(); ++i) {
            if (win.NodeAt(i)->Type() == matcha::fw::NodeType::StatusBar) {
                found = true;
                break;
            }
        }
        QVERIFY(found);
    }

    void windowNode_workspaceFrame_hasActionBar()
    {
        matcha::fw::WindowNode win("test-win", matcha::fw::WindowId::From(1),
                                    matcha::fw::WindowKind::Main);
        win.BuildWindow(nullptr);

        for (size_t i = 0; i < win.NodeCount(); ++i) {
            if (win.NodeAt(i)->Type() == matcha::fw::NodeType::WorkspaceFrame) {
                auto* ws = static_cast<matcha::fw::WorkspaceFrame*>(win.NodeAt(i));
                auto ab = ws->GetActionBar();
                QVERIFY(ab.get() != nullptr);
                return;
            }
        }
        QFAIL("WorkspaceFrame not found");
    }

    void windowNode_workspaceFrame_hasDocumentArea()
    {
        matcha::fw::WindowNode win("test-win", matcha::fw::WindowId::From(1),
                                    matcha::fw::WindowKind::Main);
        win.BuildWindow(nullptr);

        for (size_t i = 0; i < win.NodeCount(); ++i) {
            if (win.NodeAt(i)->Type() == matcha::fw::NodeType::WorkspaceFrame) {
                auto* ws = static_cast<matcha::fw::WorkspaceFrame*>(win.NodeAt(i));
                auto da = ws->GetDocumentArea();
                QVERIFY(da.get() != nullptr);
                return;
            }
        }
        QFAIL("WorkspaceFrame not found");
    }

    void shell_getActionBar_viaWorkspaceFrame()
    {
        matcha::fw::Shell shell;
        auto win = std::make_unique<matcha::fw::WindowNode>(
            "main", matcha::fw::WindowId::From(1),
            matcha::fw::WindowKind::Main);
        win->BuildWindow(nullptr);
        shell.AddNode(std::move(win));

        auto ab = shell.GetActionBar();
        QVERIFY(ab.get() != nullptr);
    }

    void shell_getStatusBar_directChild()
    {
        matcha::fw::Shell shell;
        auto win = std::make_unique<matcha::fw::WindowNode>(
            "main", matcha::fw::WindowId::From(1),
            matcha::fw::WindowKind::Main);
        win->BuildWindow(nullptr);
        shell.AddNode(std::move(win));

        auto sb = shell.GetStatusBar();
        QVERIFY(sb.get() != nullptr);
    }

    void shell_getDocumentArea_viaWorkspaceFrame()
    {
        matcha::fw::Shell shell;
        auto win = std::make_unique<matcha::fw::WindowNode>(
            "main", matcha::fw::WindowId::From(1),
            matcha::fw::WindowKind::Main);
        win->BuildWindow(nullptr);
        shell.AddNode(std::move(win));

        auto da = shell.GetDocumentArea();
        QVERIFY(da.get() != nullptr);
    }
};
