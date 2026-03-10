#pragma once

/**
 * @file MenuBarTest.h
 * @brief Widget tests for NyanMenuBar interaction: click-to-open, hover-to-switch, escape-to-close.
 */

#include <QObject>

class MenuBarTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void clickOpensMenu();
    void clickAgainClosesMenu();
    void escapeClosesMenu();
    void hoverSwitchesMenu();
    void altMnemonicOpensMenu();
    void menuItemTriggeredClosesMenu();
    void submenuOpensOnHover();
    void submenuClosesWithParent();
    void outsideClickClosesEntireChain();
    void escapeClosesSubmenuFirst();
    void submenuItemTriggeredClosesAll();
    void keyboardDownSelectsItem();
    void keyboardRightOpensSubmenu();
    void keyboardLeftClosesSubmenu();
    void menuBarLeftRightNavigation();
    void multipleMenusOnlyOneOpen();

    // -- hover behavior + 3-level submenu tests --
    void hoverNonSubmenuItemClosesSubmenu();
    void hoverDifferentSubmenuItemSwitches();
    void hoverBackToParentFromSubmenuClosesSubmenu();
    void thirdLevelSubmenuOpensOnHover();
    void thirdLevelEscapeClosesInnermostOnly();
    void thirdLevelOutsideClickClosesAll();
    void thirdLevelItemTriggeredClosesAll();
    void safeTriangleZoneKeepsSubmenuOpen();

    // -- mouse back from n-level to n-1 level switching --
    void mouseBackToSameTriggerKeepsSubmenu();
    void mouseBackToDifferentItemClosesSubmenu();
    void mouseBackToDifferentSubmenuTriggerSwitches();

    // -- MenuNode UiNode-layer event routing tests --
    void menuNodeTreeStructure();
    void mouseExitedTowardSignalEmitted();
    void menuNodeRoutesMouseBackToParent();
    void menuNodeRoutesMouseBackToDifferentItem();
    void menuNodeRoutesMouseBackToDifferentSubmenuTrigger();
    void menuNodeThreeLevelRecursiveRouting();
    void menuNodeThreeLevelBackToRootMenu();
};
