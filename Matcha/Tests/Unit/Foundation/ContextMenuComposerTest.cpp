#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file ContextMenuComposerTest.cpp
 * @brief Unit tests for ContextMenuComposer.
 */

#include "doctest.h"

#include <Matcha/Foundation/ContextMenuComposer.h>

using namespace matcha::fw;

namespace {

auto MakeAction(std::string id, std::string label, std::string shortcut = {}) -> ContextMenuItem
{
    return {
        .id = std::move(id), .label = std::move(label),
        .iconId = {}, .shortcut = std::move(shortcut),
        .kind = MenuItemKind::Action, .enabled = true, .checked = false,
        .handler = {}, .children = {},
    };
}

} // namespace

TEST_SUITE("ContextMenuComposer") {

// ============================================================================
// Group management
// ============================================================================

TEST_CASE("AddGroup creates a group") {
    ContextMenuComposer c;
    c.AddGroup("clipboard", 10);
    CHECK(c.GroupCount() == 1);
    CHECK(c.FindGroup("clipboard") != nullptr);
    CHECK(c.FindGroup("clipboard")->priority == 10);
}

TEST_CASE("AddGroup: duplicate name is no-op") {
    ContextMenuComposer c;
    c.AddGroup("clip", 10);
    c.AddGroup("clip", 50); // ignored
    CHECK(c.GroupCount() == 1);
    CHECK(c.FindGroup("clip")->priority == 10); // unchanged
}

TEST_CASE("SetGroupPriority changes priority") {
    ContextMenuComposer c;
    c.AddGroup("clip", 10);
    CHECK(c.SetGroupPriority("clip", 99));
    CHECK(c.FindGroup("clip")->priority == 99);
}

TEST_CASE("SetGroupPriority returns false for unknown group") {
    ContextMenuComposer c;
    CHECK_FALSE(c.SetGroupPriority("nonexistent", 50));
}

// ============================================================================
// Item management
// ============================================================================

TEST_CASE("AddItem adds to existing group") {
    ContextMenuComposer c;
    c.AddGroup("edit", 10);
    c.AddItem("edit", MakeAction("cut", "Cut"));
    c.AddItem("edit", MakeAction("copy", "Copy"));
    CHECK(c.TotalItemCount() == 2);
    CHECK(c.FindGroup("edit")->items.size() == 2);
}

TEST_CASE("AddItem auto-creates group if not existing") {
    ContextMenuComposer c;
    c.AddItem("newgroup", MakeAction("x", "X"));
    CHECK(c.GroupCount() == 1);
    CHECK(c.FindGroup("newgroup") != nullptr);
    CHECK(c.TotalItemCount() == 1);
}

TEST_CASE("AddSeparator inserts separator item") {
    ContextMenuComposer c;
    c.AddGroup("g", 10);
    c.AddItem("g", MakeAction("a", "A"));
    c.AddSeparator("g");
    c.AddItem("g", MakeAction("b", "B"));
    CHECK(c.FindGroup("g")->items.size() == 3);
    CHECK(c.FindGroup("g")->items[1].kind == MenuItemKind::Separator);
}

// ============================================================================
// Compose: ordering and separators
// ============================================================================

TEST_CASE("Compose: single group, no separators") {
    ContextMenuComposer c;
    c.AddGroup("edit", 10);
    c.AddItem("edit", MakeAction("cut", "Cut"));
    c.AddItem("edit", MakeAction("copy", "Copy"));

    auto result = c.Compose();
    REQUIRE(result.size() == 2);
    CHECK(result[0].label == "Cut");
    CHECK(result[1].label == "Copy");
}

TEST_CASE("Compose: multiple groups get separators between them") {
    ContextMenuComposer c;
    c.AddGroup("clipboard", 10);
    c.AddItem("clipboard", MakeAction("cut", "Cut"));
    c.AddItem("clipboard", MakeAction("copy", "Copy"));

    c.AddGroup("custom", 50);
    c.AddItem("custom", MakeAction("refresh", "Refresh"));

    auto result = c.Compose();
    // Expected: Cut, Copy, [Sep], Refresh
    REQUIRE(result.size() == 4);
    CHECK(result[0].label == "Cut");
    CHECK(result[1].label == "Copy");
    CHECK(result[2].kind == MenuItemKind::Separator);
    CHECK(result[3].label == "Refresh");
}

TEST_CASE("Compose: groups ordered by priority") {
    ContextMenuComposer c;
    c.AddGroup("later", 50);
    c.AddItem("later", MakeAction("b", "B"));

    c.AddGroup("first", 10);
    c.AddItem("first", MakeAction("a", "A"));

    auto result = c.Compose();
    // first(10) before later(50)
    REQUIRE(result.size() == 3); // A, Sep, B
    CHECK(result[0].label == "A");
    CHECK(result[1].kind == MenuItemKind::Separator);
    CHECK(result[2].label == "B");
}

TEST_CASE("Compose: empty groups produce no separators") {
    ContextMenuComposer c;
    c.AddGroup("empty", 5);
    c.AddGroup("filled", 10);
    c.AddItem("filled", MakeAction("x", "X"));

    auto result = c.Compose();
    REQUIRE(result.size() == 1);
    CHECK(result[0].label == "X");
}

TEST_CASE("Compose: empty composer returns empty") {
    ContextMenuComposer c;
    auto result = c.Compose();
    CHECK(result.empty());
}

// ============================================================================
// Submenu items
// ============================================================================

TEST_CASE("Submenu item preserves children") {
    ContextMenuComposer c;
    c.AddGroup("nav", 10);

    ContextMenuItem sub = {
        .id = "open_recent", .label = "Open Recent",
        .iconId = {}, .shortcut = {},
        .kind = MenuItemKind::Submenu, .enabled = true, .checked = false,
        .handler = {},
        .children = {
            MakeAction("r1", "File1.txt"),
            MakeAction("r2", "File2.txt"),
        },
    };
    c.AddItem("nav", std::move(sub));

    auto result = c.Compose();
    REQUIRE(result.size() == 1);
    CHECK(result[0].kind == MenuItemKind::Submenu);
    CHECK(result[0].children.size() == 2);
}

// ============================================================================
// Toggle items
// ============================================================================

TEST_CASE("Toggle item preserves checked state") {
    ContextMenuComposer c;
    c.AddGroup("view", 10);
    c.AddItem("view", {
        .id = "show_grid", .label = "Show Grid",
        .iconId = {}, .shortcut = {},
        .kind = MenuItemKind::Toggle, .enabled = true, .checked = true,
        .handler = {}, .children = {},
    });

    auto result = c.Compose();
    REQUIRE(result.size() == 1);
    CHECK(result[0].kind == MenuItemKind::Toggle);
    CHECK(result[0].checked);
}

// ============================================================================
// Clear
// ============================================================================

TEST_CASE("Clear removes all groups and items") {
    ContextMenuComposer c;
    c.AddGroup("a", 10);
    c.AddItem("a", MakeAction("x", "X"));
    c.Clear();
    CHECK(c.GroupCount() == 0);
    CHECK(c.TotalItemCount() == 0);
}

} // TEST_SUITE
