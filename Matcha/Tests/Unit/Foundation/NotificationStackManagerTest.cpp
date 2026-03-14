#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file NotificationStackManagerTest.cpp
 * @brief Unit tests for NotificationStackManager.
 */

#include "doctest.h"

#include <Matcha/Foundation/NotificationStackManager.h>

using namespace matcha::fw;
using Ms = std::chrono::milliseconds;

namespace {

auto MakeNotif(NotificationPriority prio, std::string title,
               Ms duration = Ms{5000}, std::string code = {}) -> StackNotification
{
    return {
        .id = 0, .priority = prio, .code = std::move(code),
        .title = std::move(title), .message = {},
        .duration = duration, .actionLabel = {}, .actionCallback = {},
    };
}

} // namespace

TEST_SUITE("NotificationStackManager") {

// ============================================================================
// Push / Query basics
// ============================================================================

TEST_CASE("Push adds to visible, returns non-zero ID") {
    NotificationStackManager mgr;
    auto id = mgr.Push(MakeNotif(NotificationPriority::Normal, "Test"));
    CHECK(id > 0);
    CHECK(mgr.TotalCount() == 1);
    CHECK(mgr.VisibleCount() == 1);
    CHECK(mgr.FindById(id) != nullptr);
}

TEST_CASE("Push multiple within max visible") {
    NotificationStackManager mgr;
    mgr.SetMaxVisible(3);
    mgr.Push(MakeNotif(NotificationPriority::Normal, "A"));
    mgr.Push(MakeNotif(NotificationPriority::Normal, "B"));
    mgr.Push(MakeNotif(NotificationPriority::Normal, "C"));
    CHECK(mgr.VisibleCount() == 3);
    CHECK(mgr.QueuedNotifications().empty());
}

TEST_CASE("Push overflow goes to queue") {
    NotificationStackManager mgr;
    mgr.SetMaxVisible(2);
    mgr.Push(MakeNotif(NotificationPriority::Normal, "A"));
    mgr.Push(MakeNotif(NotificationPriority::Normal, "B"));
    mgr.Push(MakeNotif(NotificationPriority::Normal, "C"));
    CHECK(mgr.VisibleCount() == 2);
    CHECK(mgr.QueuedNotifications().size() == 1);
    CHECK(mgr.TotalCount() == 3);
}

// ============================================================================
// Priority override
// ============================================================================

TEST_CASE("High priority displaces low priority in visible") {
    NotificationStackManager mgr;
    mgr.SetMaxVisible(2);
    mgr.Push(MakeNotif(NotificationPriority::Low, "Low1"));
    mgr.Push(MakeNotif(NotificationPriority::Low, "Low2"));
    CHECK(mgr.VisibleCount() == 2);

    // High priority should displace one of the Low items
    mgr.Push(MakeNotif(NotificationPriority::High, "High1"));
    CHECK(mgr.VisibleCount() == 2);
    CHECK(mgr.QueuedNotifications().size() == 1);

    // Verify high priority is in visible
    bool highVisible = false;
    for (const auto& n : mgr.VisibleNotifications()) {
        if (n.title == "High1") {
            highVisible = true;
        }
    }
    CHECK(highVisible);
}

TEST_CASE("Urgent bypasses max visible limit") {
    NotificationStackManager mgr;
    mgr.SetMaxVisible(2);
    mgr.Push(MakeNotif(NotificationPriority::Normal, "A"));
    mgr.Push(MakeNotif(NotificationPriority::Normal, "B"));
    mgr.Push(MakeNotif(NotificationPriority::Urgent, "URGENT"));
    CHECK(mgr.VisibleCount() == 3); // Urgent bypasses limit
}

// ============================================================================
// Duplicate suppression
// ============================================================================

TEST_CASE("Duplicate code updates existing instead of adding") {
    NotificationStackManager mgr;
    auto id1 = mgr.Push(MakeNotif(NotificationPriority::Normal, "First", Ms{5000}, "SAVE_OK"));
    auto id2 = mgr.Push(MakeNotif(NotificationPriority::Normal, "Second", Ms{5000}, "SAVE_OK"));
    CHECK(id1 == id2); // Same ID returned
    CHECK(mgr.TotalCount() == 1);
    CHECK(mgr.FindById(id1)->title == "Second"); // Updated
}

TEST_CASE("Empty code does not suppress") {
    NotificationStackManager mgr;
    auto id1 = mgr.Push(MakeNotif(NotificationPriority::Normal, "A"));
    auto id2 = mgr.Push(MakeNotif(NotificationPriority::Normal, "B"));
    CHECK(id1 != id2);
    CHECK(mgr.TotalCount() == 2);
}

// ============================================================================
// Dismiss
// ============================================================================

TEST_CASE("Dismiss removes visible notification") {
    NotificationStackManager mgr;
    auto id = mgr.Push(MakeNotif(NotificationPriority::Normal, "Test"));
    CHECK(mgr.Dismiss(id));
    CHECK(mgr.TotalCount() == 0);
}

TEST_CASE("Dismiss promotes from queue") {
    NotificationStackManager mgr;
    mgr.SetMaxVisible(1);
    mgr.Push(MakeNotif(NotificationPriority::Normal, "A"));
    auto idB = mgr.Push(MakeNotif(NotificationPriority::Normal, "B"));
    CHECK(mgr.VisibleCount() == 1);
    CHECK(mgr.QueuedNotifications().size() == 1);

    // Dismiss visible -> B should be promoted
    mgr.Dismiss(mgr.VisibleNotifications()[0].id);
    CHECK(mgr.VisibleCount() == 1);
    CHECK(mgr.VisibleNotifications()[0].id == idB);
}

TEST_CASE("Dismiss returns false for unknown ID") {
    NotificationStackManager mgr;
    CHECK_FALSE(mgr.Dismiss(999));
}

TEST_CASE("Dismiss fires callback") {
    NotificationStackManager mgr;
    NotificationId dismissed = 0;
    mgr.OnDismissed([&](NotificationId id) { dismissed = id; });

    auto id = mgr.Push(MakeNotif(NotificationPriority::Normal, "Test"));
    mgr.Dismiss(id);
    CHECK(dismissed == id);
}

TEST_CASE("DismissAll clears everything") {
    NotificationStackManager mgr;
    mgr.Push(MakeNotif(NotificationPriority::Normal, "A"));
    mgr.Push(MakeNotif(NotificationPriority::Normal, "B"));
    mgr.DismissAll();
    CHECK(mgr.TotalCount() == 0);
}

// ============================================================================
// Auto-dismiss via Tick
// ============================================================================

TEST_CASE("Tick: auto-dismiss after duration expires") {
    NotificationStackManager mgr;
    mgr.Push(MakeNotif(NotificationPriority::Normal, "Short", Ms{100}));
    CHECK(mgr.VisibleCount() == 1);

    mgr.Tick(Ms{50});
    CHECK(mgr.VisibleCount() == 1); // not yet

    mgr.Tick(Ms{60});
    CHECK(mgr.VisibleCount() == 0); // expired
}

TEST_CASE("Tick: duration=0 never auto-dismisses") {
    NotificationStackManager mgr;
    mgr.Push(MakeNotif(NotificationPriority::Normal, "Sticky", Ms{0}));

    mgr.Tick(Ms{99999});
    CHECK(mgr.VisibleCount() == 1);
}

TEST_CASE("Tick: multiple items, only expired ones dismissed") {
    NotificationStackManager mgr;
    mgr.Push(MakeNotif(NotificationPriority::Normal, "Short", Ms{100}));
    mgr.Push(MakeNotif(NotificationPriority::Normal, "Long", Ms{5000}));

    mgr.Tick(Ms{150});
    CHECK(mgr.VisibleCount() == 1);
    CHECK(mgr.VisibleNotifications()[0].title == "Long");
}

// ============================================================================
// FindById
// ============================================================================

TEST_CASE("FindById: finds in visible") {
    NotificationStackManager mgr;
    auto id = mgr.Push(MakeNotif(NotificationPriority::Normal, "Test"));
    const auto* n = mgr.FindById(id);
    REQUIRE(n != nullptr);
    CHECK(n->title == "Test");
}

TEST_CASE("FindById: finds in queue") {
    NotificationStackManager mgr;
    mgr.SetMaxVisible(1);
    mgr.Push(MakeNotif(NotificationPriority::Normal, "A"));
    auto idB = mgr.Push(MakeNotif(NotificationPriority::Normal, "B"));
    const auto* n = mgr.FindById(idB);
    REQUIRE(n != nullptr);
    CHECK(n->title == "B");
}

TEST_CASE("FindById: returns nullptr for unknown") {
    NotificationStackManager mgr;
    CHECK(mgr.FindById(999) == nullptr);
}

} // TEST_SUITE
