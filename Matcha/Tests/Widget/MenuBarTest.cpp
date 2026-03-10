/**
 * @file MenuBarTest.cpp
 * @brief Widget tests for NyanMenuBar interaction via simulated user events.
 *
 * Diagnoses the standard menu interaction model:
 *   1. Click button -> open popup menu
 *   2. Click same button again -> close
 *   3. Escape key -> close
 *   4. While open, hover another button -> switch menu
 *   5. Alt+mnemonic -> open
 *   6. Click menu item -> trigger + close chain
 */

#include "MenuBarTest.h"

#include <Matcha/UiNodes/Menu/MenuBarNode.h>
#include <Matcha/UiNodes/Menu/MenuItemNode.h>
#include <Matcha/UiNodes/Menu/MenuNode.h>
#include <Matcha/Widgets/Menu/NyanMenu.h>
#include <Matcha/Widgets/Menu/NyanMenuBar.h>
#include <Matcha/Widgets/Menu/NyanMenuItem.h>

#include <QMouseEvent>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

using namespace matcha::gui;

// Find a direct NyanMenuItem child of a menu by visual order (layout index).
static auto FindMenuItemAt(NyanMenu* menu, int index) -> NyanMenuItem*
{
    auto items = menu->findChildren<NyanMenuItem*>(Qt::FindDirectChildrenOnly);
    if (index >= 0 && index < items.size())
        return items[index];
    return nullptr;
}

// Find the first NyanMenuItem with a submenu indicator in a menu.
static auto FindSubmenuTrigger(NyanMenu* menu) -> NyanMenuItem*
{
    for (auto* item : menu->findChildren<NyanMenuItem*>(Qt::FindDirectChildrenOnly)) {
        if (item->HasSubmenuIndicator())
            return item;
    }
    return nullptr;
}

// Simulate hovering over a menu item by sending MouseMove to the parent menu.
static void HoverMenuItem(NyanMenu* menu, NyanMenuItem* item)
{
    QPoint center = item->geometry().center();
    QMouseEvent move(QEvent::MouseMove, QPointF(center), QPointF(center),
                     item->mapToGlobal(QPointF(center)),
                     Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(menu, &move);
}

void MenuBarTest::clickOpensMenu()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");
    fileMenu->AddItem("Open");

    auto* editMenu = bar.AddMenu("&Edit");
    editMenu->AddItem("Undo");

    QTest::qWait(50);

    // File menu should be closed initially
    QVERIFY(!fileMenu->IsOpen());

    // Click the File button (first child QPushButton)
    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(buttons.size() >= 2);

    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);

    // File menu should now be open
    QVERIFY2(fileMenu->IsOpen(), "File menu should open after clicking its button");
}

void MenuBarTest::clickAgainClosesMenu()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");

    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(!buttons.isEmpty());

    // Open
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());

    // Click again to close
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY2(!fileMenu->IsOpen(), "File menu should close after clicking its button again");
}

void MenuBarTest::escapeClosesMenu()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");

    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(!buttons.isEmpty());

    // Open
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());

    // Escape should close
    QTest::keyClick(fileMenu, Qt::Key_Escape);
    QTest::qWait(100);
    QVERIFY2(!fileMenu->IsOpen(), "Menu should close on Escape key");
}

void MenuBarTest::hoverSwitchesMenu()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");

    auto* editMenu = bar.AddMenu("&Edit");
    editMenu->AddItem("Undo");

    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(buttons.size() >= 2);

    // Open File menu
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());
    QVERIFY(!editMenu->IsOpen());

    // Simulate hover over Edit button by sending Enter event
    // (non-popup path) or by moving mouse to Edit button's global rect
    // In offscreen tests, Enter events work because no popup grab
    QEnterEvent enterEvent(QPointF(5, 5), QPointF(5, 5),
                           buttons[1]->mapToGlobal(QPointF(5, 5)));
    QApplication::sendEvent(buttons[1], &enterEvent);
    QTest::qWait(150);

    // Edit menu should now be open, File should be closed
    QVERIFY2(editMenu->IsOpen(), "Edit menu should open when hovering its button while File is open");
    QVERIFY2(!fileMenu->IsOpen(), "File menu should close when Edit opens");
}

void MenuBarTest::altMnemonicOpensMenu()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");

    auto* editMenu = bar.AddMenu("&Edit");
    editMenu->AddItem("Undo");

    QTest::qWait(50);

    QVERIFY(!fileMenu->IsOpen());

    // Alt+F should open File menu
    QTest::keyPress(&bar, Qt::Key_Alt);
    QTest::qWait(20);
    QTest::keyClick(&bar, Qt::Key_F, Qt::AltModifier);
    QTest::qWait(100);

    QVERIFY2(fileMenu->IsOpen(), "Alt+F should open File menu via mnemonic");
}

void MenuBarTest::menuItemTriggeredClosesMenu()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    auto* newItem = fileMenu->AddItem("New");

    QSignalSpy spy(newItem, &NyanMenuItem::Triggered);

    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(!buttons.isEmpty());

    // Open menu
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());

    // Click the "New" item
    QTest::mouseClick(newItem, Qt::LeftButton);
    QTest::qWait(100);

    QVERIFY2(spy.count() >= 1, "MenuItem Triggered signal should fire on click");
    QVERIFY2(!fileMenu->IsOpen(), "Menu should close after item is triggered");
}

// -- 10 new tests: submenu + standard interaction rules --------------

void MenuBarTest::submenuOpensOnHover()
{
    NyanMenu menu;
    menu.AddItem("Normal");
    auto* sub = menu.AddSubmenu("More");
    sub->AddItem("SubItem1");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);
    QVERIFY(menu.IsOpen());

    // Find the submenu trigger item (last NyanMenuItem with indicator)
    auto items = menu.findChildren<NyanMenuItem*>();
    NyanMenuItem* submenuItem = nullptr;
    for (auto* it : items) {
        if (it->HasSubmenuIndicator()) { submenuItem = it; break; }
    }
    QVERIFY(submenuItem != nullptr);

    // Hover via MouseMove on parent menu
    HoverMenuItem(&menu, submenuItem);
    QTest::qWait(300); // wait > kSubmenuDelay (200ms)

    QVERIFY2(sub->IsOpen(), "Submenu should open after hovering its trigger item");

    menu.Close();
}

void MenuBarTest::submenuClosesWithParent()
{
    NyanMenu menu;
    auto* sub = menu.AddSubmenu("More");
    sub->AddItem("SubItem1");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu via hover
    auto items = menu.findChildren<NyanMenuItem*>();
    NyanMenuItem* submenuItem = nullptr;
    for (auto* it : items) {
        if (it->HasSubmenuIndicator()) { submenuItem = it; break; }
    }
    QVERIFY(submenuItem != nullptr);
    HoverMenuItem(&menu, submenuItem);
    QTest::qWait(300);
    QVERIFY(sub->IsOpen());

    // Close parent -- submenu must also close
    menu.Close();
    QTest::qWait(50);

    QVERIFY2(!sub->IsOpen(), "Submenu should close when parent menu closes");
    QVERIFY2(!menu.IsOpen(), "Parent menu should be closed");
}

void MenuBarTest::outsideClickClosesEntireChain()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");
    auto* sub = fileMenu->AddSubmenu("Recent");
    sub->AddItem("doc1.txt");

    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(!buttons.isEmpty());

    // Open File menu
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());

    // Open submenu via hover
    auto items = fileMenu->findChildren<NyanMenuItem*>();
    NyanMenuItem* submenuItem = nullptr;
    for (auto* item : items) {
        if (item->HasSubmenuIndicator()) {
            submenuItem = item;
            break;
        }
    }
    QVERIFY(submenuItem != nullptr);
    HoverMenuItem(fileMenu, submenuItem);
    QTest::qWait(300);
    QVERIFY2(sub->IsOpen(), "Submenu should be open before outside click test");

    // Simulate Qt::Popup auto-dismiss by calling hide() directly.
    // This bypasses NyanMenu::Close() so _explicitClose stays false,
    // triggering the cascade in hideEvent.
    sub->hide();
    QTest::qWait(150);

    QVERIFY2(!sub->IsOpen(), "Submenu should close on outside click");
    QVERIFY2(!fileMenu->IsOpen(), "Parent menu should also close on outside click (cascade)");
}

void MenuBarTest::escapeClosesSubmenuFirst()
{
    NyanMenu menu;
    menu.AddItem("Normal");
    auto* sub = menu.AddSubmenu("More");
    sub->AddItem("SubItem1");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu via hover
    auto items = menu.findChildren<NyanMenuItem*>();
    NyanMenuItem* submenuItem = nullptr;
    for (auto* it : items) {
        if (it->HasSubmenuIndicator()) { submenuItem = it; break; }
    }
    QVERIFY(submenuItem != nullptr);
    HoverMenuItem(&menu, submenuItem);
    QTest::qWait(300);
    QVERIFY(sub->IsOpen());

    // Escape on submenu should close submenu but not parent
    QTest::keyClick(sub, Qt::Key_Escape);
    QTest::qWait(100);

    QVERIFY2(!sub->IsOpen(), "Escape should close submenu");
    QVERIFY2(menu.IsOpen(), "Parent menu should remain open after Escape closes submenu");

    menu.Close();
}

void MenuBarTest::submenuItemTriggeredClosesAll()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");
    auto* sub = fileMenu->AddSubmenu("Recent");
    auto* subItem = sub->AddItem("doc1.txt");

    QSignalSpy spy(subItem, &NyanMenuItem::Triggered);
    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());

    // Open submenu via hover
    auto items = fileMenu->findChildren<NyanMenuItem*>();
    NyanMenuItem* submenuItem = nullptr;
    for (auto* item : items) {
        if (item->HasSubmenuIndicator()) {
            submenuItem = item;
            break;
        }
    }
    QVERIFY(submenuItem != nullptr);
    HoverMenuItem(fileMenu, submenuItem);
    QTest::qWait(300);
    QVERIFY(sub->IsOpen());

    // Click sub-item
    QTest::mouseClick(subItem, Qt::LeftButton);
    QTest::qWait(100);

    QVERIFY2(spy.count() >= 1, "Submenu item should fire Triggered");
    QVERIFY2(!sub->IsOpen(), "Submenu should close after item triggered");
    QVERIFY2(!fileMenu->IsOpen(), "Parent menu should close after submenu item triggered");
}

void MenuBarTest::keyboardDownSelectsItem()
{
    NyanMenu menu;
    menu.AddItem("Alpha");
    auto* item2 = menu.AddItem("Beta");
    menu.AddItem("Gamma");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);
    QVERIFY(menu.IsOpen());

    // Down arrow should navigate items
    QTest::keyClick(&menu, Qt::Key_Down);
    QTest::qWait(50);
    QTest::keyClick(&menu, Qt::Key_Down);
    QTest::qWait(50);

    // Enter should trigger the item
    QSignalSpy spy(item2, &NyanMenuItem::Triggered);
    QTest::keyClick(&menu, Qt::Key_Return);
    QTest::qWait(100);

    // Menu should close after Enter
    QVERIFY2(!menu.IsOpen(), "Menu should close after Enter key activates item");
}

void MenuBarTest::keyboardRightOpensSubmenu()
{
    NyanMenu menu;
    menu.AddItem("Normal");
    auto* sub = menu.AddSubmenu("More");
    sub->AddItem("SubA");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Navigate down to the submenu item
    QTest::keyClick(&menu, Qt::Key_Down);
    QTest::qWait(50);
    QTest::keyClick(&menu, Qt::Key_Down);
    QTest::qWait(50);

    // Right arrow should open the submenu
    QTest::keyClick(&menu, Qt::Key_Right);
    QTest::qWait(250);

    QVERIFY2(sub->IsOpen(), "Right arrow on submenu item should open submenu");

    menu.Close();
}

void MenuBarTest::keyboardLeftClosesSubmenu()
{
    NyanMenu menu;
    menu.AddItem("Normal");
    auto* sub = menu.AddSubmenu("More");
    sub->AddItem("SubA");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu via hover
    auto items = menu.findChildren<NyanMenuItem*>();
    NyanMenuItem* submenuItem = nullptr;
    for (auto* it : items) {
        if (it->HasSubmenuIndicator()) { submenuItem = it; break; }
    }
    QVERIFY(submenuItem != nullptr);
    HoverMenuItem(&menu, submenuItem);
    QTest::qWait(300);
    QVERIFY(sub->IsOpen());

    // Left arrow on submenu should close it and return focus to parent
    QTest::keyClick(sub, Qt::Key_Left);
    QTest::qWait(100);

    QVERIFY2(!sub->IsOpen(), "Left arrow should close submenu");
    QVERIFY2(menu.IsOpen(), "Parent menu should stay open after Left");

    menu.Close();
}

void MenuBarTest::menuBarLeftRightNavigation()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");
    auto* editMenu = bar.AddMenu("&Edit");
    editMenu->AddItem("Undo");
    auto* viewMenu = bar.AddMenu("&View");
    viewMenu->AddItem("Zoom");

    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(buttons.size() >= 3);

    // Open File menu
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());

    // Right arrow on the menu should switch to Edit
    QTest::keyClick(fileMenu, Qt::Key_Right);
    QTest::qWait(150);

    QVERIFY2(editMenu->IsOpen(), "Right arrow should switch to Edit menu");
    QVERIFY2(!fileMenu->IsOpen(), "File menu should close on Right navigation");

    // Another Right arrow should switch to View
    QTest::keyClick(editMenu, Qt::Key_Right);
    QTest::qWait(150);

    QVERIFY2(viewMenu->IsOpen(), "Right arrow should switch to View menu");
    QVERIFY2(!editMenu->IsOpen(), "Edit menu should close on Right navigation");

    viewMenu->Close();
}

void MenuBarTest::multipleMenusOnlyOneOpen()
{
    NyanMenuBar bar;
    bar.resize(400, 24);
    bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&bar));

    auto* fileMenu = bar.AddMenu("&File");
    fileMenu->AddItem("New");
    auto* editMenu = bar.AddMenu("&Edit");
    editMenu->AddItem("Undo");
    auto* viewMenu = bar.AddMenu("&View");
    viewMenu->AddItem("Zoom");

    QTest::qWait(50);

    auto buttons = bar.findChildren<QPushButton*>();
    QVERIFY(buttons.size() >= 3);

    // Open File
    QTest::mouseClick(buttons[0], Qt::LeftButton);
    QTest::qWait(100);
    QVERIFY(fileMenu->IsOpen());
    QVERIFY(!editMenu->IsOpen());
    QVERIFY(!viewMenu->IsOpen());

    // Open Edit (via hover while File is open)
    QEnterEvent enter1(QPointF(5, 5), QPointF(5, 5),
                       buttons[1]->mapToGlobal(QPointF(5, 5)));
    QApplication::sendEvent(buttons[1], &enter1);
    QTest::qWait(150);

    QVERIFY2(!fileMenu->IsOpen(), "File should close when Edit opens");
    QVERIFY2(editMenu->IsOpen(), "Edit should be open");
    QVERIFY2(!viewMenu->IsOpen(), "View should still be closed");

    // Open View (via hover)
    QEnterEvent enter2(QPointF(5, 5), QPointF(5, 5),
                       buttons[2]->mapToGlobal(QPointF(5, 5)));
    QApplication::sendEvent(buttons[2], &enter2);
    QTest::qWait(150);

    QVERIFY2(!fileMenu->IsOpen(), "File should still be closed");
    QVERIFY2(!editMenu->IsOpen(), "Edit should close when View opens");
    QVERIFY2(viewMenu->IsOpen(), "View should now be open");

    // At no point should multiple menus be open simultaneously
    // (verified by the assertions above at each step)
}

// ============================================================================
// Hover behavior + 3-level submenu tests
// ============================================================================

void MenuBarTest::hoverNonSubmenuItemClosesSubmenu()
{
    // When a submenu is open and user hovers a non-submenu item in the parent,
    // the submenu should close.
    NyanMenu menu;
    menu.AddItem("Normal");
    auto* sub = menu.AddSubmenu("Sub");
    sub->AddItem("SubItem");
    menu.AddItem("Another");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu via hover
    auto* subItem = FindMenuItemAt(&menu, 1);
    QVERIFY(subItem);
    HoverMenuItem(&menu, subItem);
    QTest::qWait(300);
    QVERIFY2(sub->IsOpen(), "Submenu should open on hover");

    // Now hover the non-submenu item "Another"
    auto* anotherItem = FindMenuItemAt(&menu, 2);
    QVERIFY(anotherItem);
    HoverMenuItem(&menu, anotherItem);
    QTest::qWait(300);

    QVERIFY2(!sub->IsOpen(), "Submenu should close when hovering non-submenu item");
    QVERIFY2(menu.IsOpen(), "Parent menu should remain open");

    menu.Close();
}

void MenuBarTest::hoverDifferentSubmenuItemSwitches()
{
    // When one submenu is open and user hovers a different submenu-trigger
    // item, the old submenu should close and the new one should open.
    NyanMenu menu;
    auto* sub1 = menu.AddSubmenu("Sub1");
    sub1->AddItem("S1-Item");
    auto* sub2 = menu.AddSubmenu("Sub2");
    sub2->AddItem("S2-Item");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open first submenu
    auto* trigger1 = FindMenuItemAt(&menu, 0);
    QVERIFY(trigger1);
    HoverMenuItem(&menu, trigger1);
    QTest::qWait(300);
    QVERIFY2(sub1->IsOpen(), "Sub1 should open");

    // Hover second submenu trigger
    auto* trigger2 = FindMenuItemAt(&menu, 1);
    QVERIFY(trigger2);
    HoverMenuItem(&menu, trigger2);
    QTest::qWait(300);

    QVERIFY2(!sub1->IsOpen(), "Sub1 should close when switching to Sub2");
    QVERIFY2(sub2->IsOpen(), "Sub2 should open");

    menu.Close();
}

void MenuBarTest::hoverBackToParentFromSubmenuClosesSubmenu()
{
    // When a submenu is open and user moves mouse back to the parent menu
    // on a non-submenu item, the submenu should close.
    NyanMenu menu;
    menu.AddItem("Top");
    auto* sub = menu.AddSubmenu("Sub");
    sub->AddItem("SubItem");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu
    auto* subTrigger = FindMenuItemAt(&menu, 1);
    QVERIFY(subTrigger);
    HoverMenuItem(&menu, subTrigger);
    QTest::qWait(300);
    QVERIFY(sub->IsOpen());

    // Move mouse back to parent menu's "Top" item
    auto* topItem = FindMenuItemAt(&menu, 0);
    QVERIFY(topItem);
    HoverMenuItem(&menu, topItem);
    QTest::qWait(300);

    QVERIFY2(!sub->IsOpen(), "Submenu should close when hovering back to parent");
    QVERIFY2(menu.IsOpen(), "Parent menu stays open");

    menu.Close();
}

void MenuBarTest::thirdLevelSubmenuOpensOnHover()
{
    // File > Open Recent > From Template (3 levels)
    NyanMenu menu;
    auto* sub = menu.AddSubmenu("Open Recent");
    sub->AddItem("Part1.stp");
    auto* sub2 = sub->AddSubmenu("From Template");
    sub2->AddItem("Blank Part");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open L1 submenu
    auto* l1Trigger = FindSubmenuTrigger(&menu);
    QVERIFY(l1Trigger);
    HoverMenuItem(&menu, l1Trigger);
    QTest::qWait(300);
    QVERIFY2(sub->IsOpen(), "L1 submenu should open");

    // Open L2 submenu
    auto* l2Trigger = FindMenuItemAt(sub, 1);
    QVERIFY(l2Trigger);
    HoverMenuItem(sub, l2Trigger);
    QTest::qWait(300);
    QVERIFY2(sub2->IsOpen(), "L2 submenu (From Template) should open");
    QVERIFY2(sub->IsOpen(), "L1 submenu should remain open");
    QVERIFY2(menu.IsOpen(), "Root menu should remain open");

    menu.Close();
}

void MenuBarTest::thirdLevelEscapeClosesInnermostOnly()
{
    NyanMenu menu;
    auto* sub = menu.AddSubmenu("Open Recent");
    sub->AddItem("Part1.stp");
    auto* sub2 = sub->AddSubmenu("From Template");
    sub2->AddItem("Blank Part");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open L1
    auto* l1Trigger = FindSubmenuTrigger(&menu);
    QVERIFY(l1Trigger);
    HoverMenuItem(&menu, l1Trigger);
    QTest::qWait(300);
    QVERIFY(sub->IsOpen());

    // Open L2
    auto* l2Trigger = FindMenuItemAt(sub, 1);
    QVERIFY(l2Trigger);
    HoverMenuItem(sub, l2Trigger);
    QTest::qWait(300);
    QVERIFY(sub2->IsOpen());

    // Escape on L2 should close L2 only
    QTest::keyClick(sub2, Qt::Key_Escape);
    QTest::qWait(150);

    QVERIFY2(!sub2->IsOpen(), "L2 submenu should close on Escape");
    QVERIFY2(sub->IsOpen(), "L1 submenu should remain open after L2 Escape");
    QVERIFY2(menu.IsOpen(), "Root menu should remain open");

    menu.Close();
}

void MenuBarTest::thirdLevelOutsideClickClosesAll()
{
    NyanMenu menu;
    auto* sub = menu.AddSubmenu("Open Recent");
    sub->AddItem("Part1.stp");
    auto* sub2 = sub->AddSubmenu("From Template");
    sub2->AddItem("Blank Part");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open L1
    auto* l1Trigger = FindSubmenuTrigger(&menu);
    QVERIFY(l1Trigger);
    HoverMenuItem(&menu, l1Trigger);
    QTest::qWait(300);

    // Open L2
    auto* l2Trigger = FindMenuItemAt(sub, 1);
    QVERIFY(l2Trigger);
    HoverMenuItem(sub, l2Trigger);
    QTest::qWait(300);
    QVERIFY(sub2->IsOpen());

    // Simulate outside click (auto-dismiss) on the innermost popup
    sub2->hide();
    QTest::qWait(150);

    QVERIFY2(!sub2->IsOpen(), "L2 should close");
    QVERIFY2(!sub->IsOpen(), "L1 should cascade-close");
    QVERIFY2(!menu.IsOpen(), "Root should cascade-close");
}

void MenuBarTest::thirdLevelItemTriggeredClosesAll()
{
    NyanMenu menu;
    auto* sub = menu.AddSubmenu("Open Recent");
    sub->AddItem("Part1.stp");
    auto* sub2 = sub->AddSubmenu("From Template");
    auto* leaf = sub2->AddItem("Blank Part");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open L1
    auto* l1Trigger = FindSubmenuTrigger(&menu);
    QVERIFY(l1Trigger);
    HoverMenuItem(&menu, l1Trigger);
    QTest::qWait(300);

    // Open L2
    auto* l2Trigger = FindMenuItemAt(sub, 1);
    QVERIFY(l2Trigger);
    HoverMenuItem(sub, l2Trigger);
    QTest::qWait(300);
    QVERIFY(sub2->IsOpen());

    // Click the leaf item
    QTest::mouseClick(leaf, Qt::LeftButton);
    QTest::qWait(150);

    QVERIFY2(!sub2->IsOpen(), "L2 should close after item triggered");
    QVERIFY2(!sub->IsOpen(), "L1 should close after item triggered");
    QVERIFY2(!menu.IsOpen(), "Root should close after item triggered");
}

void MenuBarTest::safeTriangleZoneKeepsSubmenuOpen()
{
    // When moving mouse diagonally toward an open submenu, the safe triangle
    // zone should prevent premature closing.
    NyanMenu menu;
    menu.AddItem("Normal");
    auto* sub = menu.AddSubmenu("Sub");
    sub->AddItem("SubItem");
    menu.AddItem("Another");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu
    auto* subTrigger = FindMenuItemAt(&menu, 1);
    QVERIFY(subTrigger);
    HoverMenuItem(&menu, subTrigger);
    QTest::qWait(300);
    QVERIFY2(sub->IsOpen(), "Submenu should be open");

    // Send a MouseMove that is within the safe triangle zone (toward submenu)
    // The submenu anchor is at the right edge of subTrigger; moving right
    // should be in the safe zone.
    QPoint center = subTrigger->geometry().center();
    QPoint towardSubmenu(center.x() + 30, center.y() + 5);
    QMouseEvent move(QEvent::MouseMove, QPointF(towardSubmenu),
                     QPointF(towardSubmenu),
                     menu.mapToGlobal(QPointF(towardSubmenu)),
                     Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&menu, &move);
    QTest::qWait(100);

    QVERIFY2(sub->IsOpen(), "Submenu should stay open in safe triangle zone");

    menu.Close();
}

// ============================================================================
// Mouse back from n-level to n-1 level switching
// ============================================================================

void MenuBarTest::mouseBackToSameTriggerKeepsSubmenu()
{
    // When submenu is open and mouse moves back to the trigger item (item a)
    // that opened it, the submenu should stay open.
    NyanMenu menu;
    menu.AddItem("Normal");
    auto* sub = menu.AddSubmenu("Sub");
    sub->AddItem("SubItem1");
    sub->AddItem("SubItem2");
    menu.AddItem("Another");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu via hover on trigger
    auto* trigger = FindMenuItemAt(&menu, 1); // "Sub"
    QVERIFY(trigger);
    HoverMenuItem(&menu, trigger);
    QTest::qWait(300);
    QVERIFY2(sub->IsOpen(), "Submenu should be open");

    // Simulate: mouse was in submenu, now moves back to the same trigger item
    // (this is the "mouse back to item a" case)
    HoverMenuItem(&menu, trigger);
    QTest::qWait(100);

    QVERIFY2(sub->IsOpen(), "Submenu should remain open when hovering its own trigger");
    QVERIFY2(menu.IsOpen(), "Parent menu should remain open");

    menu.Close();
}

void MenuBarTest::mouseBackToDifferentItemClosesSubmenu()
{
    // When submenu is open from item a, and mouse moves to a different
    // non-submenu item b in the parent menu, the submenu should close.
    NyanMenu menu;
    menu.AddItem("ItemA");       // index 0
    auto* sub = menu.AddSubmenu("SubMenu"); // index 1
    sub->AddItem("SubItem");
    menu.AddItem("ItemB");       // index 2

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu
    auto* trigger = FindMenuItemAt(&menu, 1);
    QVERIFY(trigger);
    HoverMenuItem(&menu, trigger);
    QTest::qWait(300);
    QVERIFY(sub->IsOpen());

    // Mouse back to "ItemB" (non-submenu)
    auto* itemB = FindMenuItemAt(&menu, 2);
    QVERIFY(itemB);
    HoverMenuItem(&menu, itemB);
    QTest::qWait(300);

    QVERIFY2(!sub->IsOpen(), "Submenu should close when hovering a different non-submenu item");
    QVERIFY2(menu.IsOpen(), "Parent menu should remain open");

    menu.Close();
}

void MenuBarTest::mouseBackToDifferentSubmenuTriggerSwitches()
{
    // Menu has two submenu triggers: Sub1 (item 0) and Sub2 (item 1).
    // Open Sub1, then hover Sub2: Sub1 should close, Sub2 should open after delay.
    NyanMenu menu;
    auto* sub1 = menu.AddSubmenu("Sub1");
    sub1->AddItem("S1-A");
    sub1->AddItem("S1-B");
    auto* sub2 = menu.AddSubmenu("Sub2");
    sub2->AddItem("S2-A");

    menu.Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open Sub1
    auto* trigger1 = FindMenuItemAt(&menu, 0);
    QVERIFY(trigger1);
    HoverMenuItem(&menu, trigger1);
    QTest::qWait(300);
    QVERIFY2(sub1->IsOpen(), "Sub1 should be open");
    QVERIFY2(!sub2->IsOpen(), "Sub2 should be closed");

    // Hover Sub2 trigger -- should close Sub1 immediately and open Sub2 after delay
    auto* trigger2 = FindMenuItemAt(&menu, 1);
    QVERIFY(trigger2);
    HoverMenuItem(&menu, trigger2);

    // Immediately after hover, Sub1 should already be closed
    QTest::qWait(50);
    QVERIFY2(!sub1->IsOpen(), "Sub1 should close immediately when hovering Sub2 trigger");

    // After delay, Sub2 should open
    QTest::qWait(300);
    QVERIFY2(sub2->IsOpen(), "Sub2 should open after hover delay");
    QVERIFY2(menu.IsOpen(), "Parent menu should remain open");

    menu.Close();
}

// ============================================================================
// MenuNode UiNode-layer event routing tests
// ============================================================================

void MenuBarTest::menuNodeTreeStructure()
{
    // Verify MenuNode tree mirrors the NyanMenu widget tree.
    matcha::fw::MenuNode root("root");
    auto* itemA = root.AddItem("ItemA");
    auto* sub = root.AddSubmenu("Sub");
    auto* itemB = sub->AddItem("SubItemB");

    QCOMPARE(root.NodeCount(), static_cast<size_t>(2));
    QVERIFY(root.NodeAt(0) == itemA);
    QVERIFY(root.NodeAt(1) == sub);
    QCOMPARE(sub->NodeCount(), static_cast<size_t>(1));
    QVERIFY(sub->NodeAt(0) == itemB);

    // Widget pointers should be non-null
    QVERIFY(root.Menu() != nullptr);
    QVERIFY(sub->Menu() != nullptr);
    QVERIFY(itemA->Widget() != nullptr);
    QVERIFY(itemB->Widget() != nullptr);

    // Parent relationships
    QVERIFY(sub->ParentNode() == &root);
    QVERIFY(itemB->ParentNode() == sub);
}

void MenuBarTest::mouseExitedTowardSignalEmitted()
{
    // Verify NyanMenu emits MouseExitedToward when mouse moves outside its rect.
    matcha::fw::MenuNode root("root");
    root.AddItem("ItemA");
    root.AddItem("ItemB");

    auto* menu = root.Menu();
    menu->Popup(QPoint(100, 100));
    QTest::qWait(100);
    QVERIFY(menu->IsOpen());

    QSignalSpy spy(menu, &NyanMenu::MouseExitedToward);

    // Send mouse move outside menu rect (far above)
    QPoint outside(-50, -50);
    QMouseEvent move(QEvent::MouseMove, QPointF(outside), QPointF(outside),
                     menu->mapToGlobal(QPointF(outside)),
                     Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(menu, &move);

    QVERIFY2(spy.count() >= 1, "MouseExitedToward should fire when mouse exits menu rect");

    menu->Close();
}

void MenuBarTest::menuNodeRoutesMouseBackToParent()
{
    // Scenario: submenu open from trigger A, mouse exits submenu into
    // parent menu area over the same trigger A -> submenu stays open.
    matcha::fw::MenuNode root("root");
    root.AddItem("Normal");
    auto* subNode = root.AddSubmenu("Sub");
    subNode->AddItem("SubItem");

    auto* rootMenu = root.Menu();
    auto* subMenu = subNode->Menu();

    rootMenu->Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu via hover
    auto* trigger = FindMenuItemAt(rootMenu, 1);
    QVERIFY(trigger);
    HoverMenuItem(rootMenu, trigger);
    QTest::qWait(300);
    QVERIFY2(subMenu->IsOpen(), "Submenu should be open");

    // Now call HandleExternalMouseMove with position over the trigger item
    // (simulates MenuNode routing from child to parent)
    QPoint triggerGlobal = trigger->mapToGlobal(trigger->rect().center());
    rootMenu->HandleExternalMouseMove(triggerGlobal);
    QTest::qWait(100);

    QVERIFY2(subMenu->IsOpen(), "Submenu should stay open when routed back to same trigger");
    QVERIFY2(rootMenu->IsOpen(), "Parent menu should remain open");

    rootMenu->Close();
}

void MenuBarTest::menuNodeRoutesMouseBackToDifferentItem()
{
    // Scenario: submenu open from trigger, mouse exits submenu into
    // parent menu area over a non-submenu item -> submenu closes.
    matcha::fw::MenuNode root("root");
    root.AddItem("ItemA");
    auto* subNode = root.AddSubmenu("Sub");
    subNode->AddItem("SubItem");
    root.AddItem("ItemB");

    auto* rootMenu = root.Menu();
    auto* subMenu = subNode->Menu();

    rootMenu->Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open submenu
    auto* trigger = FindMenuItemAt(rootMenu, 1);
    QVERIFY(trigger);
    HoverMenuItem(rootMenu, trigger);
    QTest::qWait(300);
    QVERIFY(subMenu->IsOpen());

    // Route mouse to ItemB (non-submenu)
    auto* itemB = FindMenuItemAt(rootMenu, 2);
    QVERIFY(itemB);
    QPoint itemBGlobal = itemB->mapToGlobal(itemB->rect().center());
    rootMenu->HandleExternalMouseMove(itemBGlobal);
    QTest::qWait(100);

    QVERIFY2(!subMenu->IsOpen(), "Submenu should close when routed to different non-submenu item");
    QVERIFY2(rootMenu->IsOpen(), "Parent menu should remain open");

    rootMenu->Close();
}

void MenuBarTest::menuNodeRoutesMouseBackToDifferentSubmenuTrigger()
{
    // Scenario: Sub1 open, mouse exits Sub1 into parent menu area over
    // Sub2 trigger -> Sub1 closes, Sub2 opens after delay.
    matcha::fw::MenuNode root("root");
    auto* sub1Node = root.AddSubmenu("Sub1");
    sub1Node->AddItem("S1-A");
    auto* sub2Node = root.AddSubmenu("Sub2");
    sub2Node->AddItem("S2-A");

    auto* rootMenu = root.Menu();
    auto* sub1Menu = sub1Node->Menu();
    auto* sub2Menu = sub2Node->Menu();

    rootMenu->Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open Sub1
    auto* trigger1 = FindMenuItemAt(rootMenu, 0);
    QVERIFY(trigger1);
    HoverMenuItem(rootMenu, trigger1);
    QTest::qWait(300);
    QVERIFY(sub1Menu->IsOpen());

    // Route mouse to Sub2 trigger
    auto* trigger2 = FindMenuItemAt(rootMenu, 1);
    QVERIFY(trigger2);
    QPoint trigger2Global = trigger2->mapToGlobal(trigger2->rect().center());
    rootMenu->HandleExternalMouseMove(trigger2Global);

    // Sub1 should close immediately
    QTest::qWait(50);
    QVERIFY2(!sub1Menu->IsOpen(), "Sub1 should close when routed to Sub2 trigger");

    // Sub2 should open after delay
    QTest::qWait(300);
    QVERIFY2(sub2Menu->IsOpen(), "Sub2 should open after hover delay");
    QVERIFY2(rootMenu->IsOpen(), "Parent menu should remain open");

    rootMenu->Close();
}

void MenuBarTest::menuNodeThreeLevelRecursiveRouting()
{
    // 3-level deep: root > L1 > L2
    // Mouse exits L2 into L1 area over a non-submenu item -> L2 closes.
    matcha::fw::MenuNode root("root");
    auto* l1Node = root.AddSubmenu("L1");
    l1Node->AddItem("L1-Normal");
    auto* l2Node = l1Node->AddSubmenu("L2");
    l2Node->AddItem("L2-Item");

    auto* rootMenu = root.Menu();
    auto* l1Menu = l1Node->Menu();
    auto* l2Menu = l2Node->Menu();

    rootMenu->Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open L1
    auto* l1Trigger = FindMenuItemAt(rootMenu, 0);
    QVERIFY(l1Trigger);
    HoverMenuItem(rootMenu, l1Trigger);
    QTest::qWait(300);
    QVERIFY(l1Menu->IsOpen());

    // Open L2
    auto* l2Trigger = FindMenuItemAt(l1Menu, 1);
    QVERIFY(l2Trigger);
    HoverMenuItem(l1Menu, l2Trigger);
    QTest::qWait(300);
    QVERIFY(l2Menu->IsOpen());

    // Route mouse from L2 back to L1's non-submenu item ("L1-Normal")
    auto* l1Normal = FindMenuItemAt(l1Menu, 0);
    QVERIFY(l1Normal);
    QPoint l1NormalGlobal = l1Normal->mapToGlobal(l1Normal->rect().center());
    l1Menu->HandleExternalMouseMove(l1NormalGlobal);
    QTest::qWait(100);

    QVERIFY2(!l2Menu->IsOpen(), "L2 should close when routed back to L1 non-submenu item");
    QVERIFY2(l1Menu->IsOpen(), "L1 should remain open");
    QVERIFY2(rootMenu->IsOpen(), "Root should remain open");

    rootMenu->Close();
}

void MenuBarTest::menuNodeThreeLevelBackToRootMenu()
{
    // 3-level deep: root > L1 > L2
    // Mouse exits L2 all the way to the root menu area.
    // L2 and L1 should both close; root stays open.
    matcha::fw::MenuNode root("root");
    root.AddItem("RootItem");
    auto* l1Node = root.AddSubmenu("L1");
    l1Node->AddItem("L1-Item");
    auto* l2Node = l1Node->AddSubmenu("L2");
    l2Node->AddItem("L2-Item");

    auto* rootMenu = root.Menu();
    auto* l1Menu = l1Node->Menu();
    auto* l2Menu = l2Node->Menu();

    rootMenu->Popup(QPoint(100, 100));
    QTest::qWait(100);

    // Open L1
    auto* l1Trigger = FindMenuItemAt(rootMenu, 1);
    QVERIFY(l1Trigger);
    HoverMenuItem(rootMenu, l1Trigger);
    QTest::qWait(300);
    QVERIFY(l1Menu->IsOpen());

    // Open L2
    auto* l2Trigger = FindMenuItemAt(l1Menu, 1);
    QVERIFY(l2Trigger);
    HoverMenuItem(l1Menu, l2Trigger);
    QTest::qWait(300);
    QVERIFY(l2Menu->IsOpen());

    // Route mouse from L2 all the way back to root's "RootItem"
    // This simulates the UiNode routing chain: L2 -> L1 -> root
    auto* rootItem = FindMenuItemAt(rootMenu, 0);
    QVERIFY(rootItem);
    QPoint rootItemGlobal = rootItem->mapToGlobal(rootItem->rect().center());

    // The signal from L2 goes to L1 (OnChildSubmenuMouseExited),
    // L1 checks if pos is in its rect -- it's not, so it propagates to root.
    // Root finds the pos is in its rect and calls HandleExternalMouseMove.
    // This closes L1's active submenu (L2), and since pos is over a
    // non-submenu item in root, root closes its active submenu (L1).
    rootMenu->HandleExternalMouseMove(rootItemGlobal);
    QTest::qWait(100);

    QVERIFY2(!l2Menu->IsOpen(), "L2 should close");
    QVERIFY2(!l1Menu->IsOpen(), "L1 should close when root handles hover on non-submenu item");
    QVERIFY2(rootMenu->IsOpen(), "Root menu should remain open");

    rootMenu->Close();
}
