#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file ShortcutManagerTest.cpp
 * @brief Unit tests for ShortcutManager.
 */

#include "doctest.h"

#include <Matcha/Interaction/Input/ShortcutManager.h>

using namespace matcha::fw;

namespace {

auto MakeEntry(std::string id, std::string key, ShortcutScope scope,
               std::function<void()> handler = {}) -> ShortcutEntry
{
    return {
        .id = std::move(id), .keySequence = std::move(key),
        .scope = scope, .description = {},
        .handler = std::move(handler), .enabled = true,
    };
}

} // namespace

TEST_SUITE("ShortcutManager") {

// ============================================================================
// Registration
// ============================================================================

TEST_CASE("Register and FindById") {
    ShortcutManager mgr;
    CHECK(mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global)));
    CHECK(mgr.Count() == 1);

    const auto* entry = mgr.FindById("file.save");
    REQUIRE(entry != nullptr);
    CHECK(entry->keySequence == "Ctrl+S");
    CHECK(entry->scope == ShortcutScope::Global);
}

TEST_CASE("Register rejects duplicate ID") {
    ShortcutManager mgr;
    CHECK(mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global)));
    CHECK_FALSE(mgr.Register(MakeEntry("file.save", "Ctrl+Shift+S", ShortcutScope::Global)));
    CHECK(mgr.Count() == 1);
}

TEST_CASE("Unregister removes entry") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global));
    CHECK(mgr.Unregister("file.save"));
    CHECK(mgr.Count() == 0);
    CHECK(mgr.FindById("file.save") == nullptr);
}

TEST_CASE("Unregister returns false for unknown ID") {
    ShortcutManager mgr;
    CHECK_FALSE(mgr.Unregister("nonexistent"));
}

TEST_CASE("Rebind changes key sequence") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global));
    CHECK(mgr.Rebind("file.save", "Ctrl+Shift+S"));
    CHECK(mgr.FindById("file.save")->keySequence == "Ctrl+Shift+S");
}

TEST_CASE("Rebind returns false for unknown ID") {
    ShortcutManager mgr;
    CHECK_FALSE(mgr.Rebind("nonexistent", "Ctrl+X"));
}

TEST_CASE("SetEnabled toggles enabled state") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global));
    CHECK(mgr.SetEnabled("file.save", false));
    CHECK_FALSE(mgr.FindById("file.save")->enabled);
    CHECK(mgr.SetEnabled("file.save", true));
    CHECK(mgr.FindById("file.save")->enabled);
}

// ============================================================================
// Dispatch
// ============================================================================

TEST_CASE("Dispatch invokes matching handler") {
    ShortcutManager mgr;
    int callCount = 0;
    mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global, [&] { ++callCount; }));

    CHECK(mgr.Dispatch("Ctrl+S", ShortcutScope::Global));
    CHECK(callCount == 1);
}

TEST_CASE("Dispatch returns false for unmatched key") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global));
    CHECK_FALSE(mgr.Dispatch("Ctrl+X", ShortcutScope::Global));
}

TEST_CASE("Dispatch skips disabled entries") {
    ShortcutManager mgr;
    int callCount = 0;
    mgr.Register(MakeEntry("file.save", "Ctrl+S", ShortcutScope::Global, [&] { ++callCount; }));
    mgr.SetEnabled("file.save", false);

    CHECK_FALSE(mgr.Dispatch("Ctrl+S", ShortcutScope::Global));
    CHECK(callCount == 0);
}

TEST_CASE("Dispatch: narrowest scope wins") {
    ShortcutManager mgr;
    std::string winner;

    mgr.Register(MakeEntry("global.f2", "F2", ShortcutScope::Global,
                            [&] { winner = "global"; }));
    mgr.Register(MakeEntry("panel.f2", "F2", ShortcutScope::Panel,
                            [&] { winner = "panel"; }));

    // Dispatch from Panel scope — Panel should win (narrower)
    mgr.Dispatch("F2", ShortcutScope::Panel);
    CHECK(winner == "panel");
}

TEST_CASE("Dispatch: falls through from narrow to broad scope") {
    ShortcutManager mgr;
    std::string winner;

    mgr.Register(MakeEntry("global.save", "Ctrl+S", ShortcutScope::Global,
                            [&] { winner = "global"; }));

    // Dispatch from Dialog scope — no Dialog entry, falls to Global
    mgr.Dispatch("Ctrl+S", ShortcutScope::Dialog);
    CHECK(winner == "global");
}

TEST_CASE("Dispatch: Dialog scope entry takes priority") {
    ShortcutManager mgr;
    std::string winner;

    mgr.Register(MakeEntry("global.esc", "Escape", ShortcutScope::Global,
                            [&] { winner = "global"; }));
    mgr.Register(MakeEntry("dialog.esc", "Escape", ShortcutScope::Dialog,
                            [&] { winner = "dialog"; }));

    mgr.Dispatch("Escape", ShortcutScope::Dialog);
    CHECK(winner == "dialog");
}

// ============================================================================
// Query
// ============================================================================

TEST_CASE("FindByKey returns all entries with matching key") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("a", "Ctrl+S", ShortcutScope::Global));
    mgr.Register(MakeEntry("b", "Ctrl+S", ShortcutScope::Panel));
    mgr.Register(MakeEntry("c", "Ctrl+X", ShortcutScope::Global));

    auto matches = mgr.FindByKey("Ctrl+S");
    CHECK(matches.size() == 2);
}

TEST_CASE("EntriesInScope filters by scope") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("a", "Ctrl+S", ShortcutScope::Global));
    mgr.Register(MakeEntry("b", "F2", ShortcutScope::Panel));
    mgr.Register(MakeEntry("c", "Ctrl+Z", ShortcutScope::Global));

    auto globals = mgr.EntriesInScope(ShortcutScope::Global);
    CHECK(globals.size() == 2);

    auto panels = mgr.EntriesInScope(ShortcutScope::Panel);
    CHECK(panels.size() == 1);
}

// ============================================================================
// Conflict detection
// ============================================================================

TEST_CASE("DetectConflicts: same key + same scope = conflict") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("a", "Ctrl+S", ShortcutScope::Global));
    mgr.Register(MakeEntry("b", "Ctrl+S", ShortcutScope::Global));

    auto conflicts = mgr.DetectConflicts();
    REQUIRE(conflicts.size() == 1);
    CHECK(conflicts[0].existingId == "a");
    CHECK(conflicts[0].newId == "b");
    CHECK(conflicts[0].keySequence == "Ctrl+S");
}

TEST_CASE("DetectConflicts: same key + different scope = no conflict") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("a", "Ctrl+S", ShortcutScope::Global));
    mgr.Register(MakeEntry("b", "Ctrl+S", ShortcutScope::Panel));

    auto conflicts = mgr.DetectConflicts();
    CHECK(conflicts.empty());
}

TEST_CASE("WouldConflict checks before registration") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("a", "Ctrl+S", ShortcutScope::Global));

    CHECK(mgr.WouldConflict("Ctrl+S", ShortcutScope::Global));
    CHECK_FALSE(mgr.WouldConflict("Ctrl+S", ShortcutScope::Panel));
    CHECK_FALSE(mgr.WouldConflict("Ctrl+X", ShortcutScope::Global));
}

// ============================================================================
// Bulk operations
// ============================================================================

TEST_CASE("Clear removes all entries") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("a", "Ctrl+S", ShortcutScope::Global));
    mgr.Register(MakeEntry("b", "Ctrl+X", ShortcutScope::Panel));
    mgr.Clear();
    CHECK(mgr.Count() == 0);
}

TEST_CASE("AllEntries returns all registered") {
    ShortcutManager mgr;
    mgr.Register(MakeEntry("a", "Ctrl+S", ShortcutScope::Global));
    mgr.Register(MakeEntry("b", "F2", ShortcutScope::Panel));

    const auto& all = mgr.AllEntries();
    CHECK(all.size() == 2);
}

} // TEST_SUITE
