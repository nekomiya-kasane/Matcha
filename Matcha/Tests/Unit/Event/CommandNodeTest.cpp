#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Event/BaseObject.h>
#include <Matcha/Event/CommandNode.h>
#include <Matcha/Event/EventNode.h>
#include <Matcha/Event/MetaClass.h>
#include <Matcha/Event/Notification.h>
#include <Matcha/Event/NotificationQueue.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

using namespace matcha;

// --------------------------------------------------------------------------- //
// Test helpers
// --------------------------------------------------------------------------- //

// MATCHA_IMPLEMENT_CLASS for test-only classes (defined below class bodies)

class TestNotification : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override {
        return "TestNotification";
    }

    std::string payload;
};

class OtherNotification : public Notification {
public:
    [[nodiscard]] auto ClassName() const -> std::string_view override {
        return "OtherNotification";
    }
};

class TestCommand : public CommandNode {
    MATCHA_DECLARE_CLASS
public:
    using CommandNode::CommandNode;

    [[nodiscard]] auto AnalyseNotification(CommandNode* sender,
                                           Notification& notif) -> PropagationMode override {
        lastSender = sender;
        lastNotifClass = std::string(notif.ClassName());
        analyseCount++;
        return handleNotification ? PropagationMode::DontTransmitToParent
                                  : PropagationMode::TransmitToParent;
    }

    auto Activate(CommandNode* /*sender*/, Notification& /*notif*/) -> StatusChangeRC override {
        activateCount++;
        return StatusChangeRC::Completed;
    }

    auto Deactivate(CommandNode* /*sender*/, Notification& /*notif*/) -> StatusChangeRC override {
        deactivateCount++;
        return StatusChangeRC::Completed;
    }

    auto Cancel(CommandNode* /*sender*/, Notification& /*notif*/) -> StatusChangeRC override {
        cancelCount++;
        return StatusChangeRC::Completed;
    }

    bool handleNotification = false;
    CommandNode* lastSender = nullptr;
    std::string lastNotifClass;
    int analyseCount = 0;
    int activateCount = 0;
    int deactivateCount = 0;
    int cancelCount = 0;

    // Expose protected ForwardToParent for testing
    void TestForwardToParent(Notification& notif) { ForwardToParent(notif); }
};

/// @brief Test root that provides a NotificationQueue (mimics Shell).
class TestRoot : public TestCommand {
public:
    explicit TestRoot(NotificationQueue* q, std::string id = "root")
        : TestCommand(nullptr, std::move(id)), _queue(q) {}

    [[nodiscard]] auto GetNotificationQueue() const -> NotificationQueue* override {
        return _queue;
    }

private:
    NotificationQueue* _queue;
};

// MATCHA_IMPLEMENT_CLASS for test helpers
MATCHA_IMPLEMENT_CLASS(TestCommand, CommandNode)

// =========================================================================== //
// BaseObject tests
// =========================================================================== //

TEST_SUITE("BaseObject") {

TEST_CASE("ClassName returns BaseObject") {
    class ConcreteObj : public BaseObject {};
    ConcreteObj obj;
    // BaseObject::ClassName() is not overridden, so returns "BaseObject"
    CHECK(std::string(static_cast<BaseObject&>(obj).ClassName()) == "BaseObject");
}

TEST_CASE("IsA checks dynamic type") {
    TestCommand cmd;
    BaseObject& base = cmd;

    CHECK(base.IsA<CommandNode>());
    CHECK(base.IsA<EventNode>());
    CHECK(base.IsA<BaseObject>());
    CHECK(base.IsA<TestCommand>());
}

TEST_CASE("IsAKindOf checks by name") {
    TestCommand cmd;

    CHECK(cmd.IsAKindOf("TestCommand"));
    CHECK(cmd.IsAKindOf("CommandNode"));
    CHECK(cmd.IsAKindOf("EventNode"));
    CHECK(cmd.IsAKindOf("BaseObject"));
    CHECK_FALSE(cmd.IsAKindOf("SomethingElse"));
}

TEST_CASE("As downcasts correctly") {
    TestCommand cmd;
    BaseObject& base = cmd;

    CHECK(base.As<TestCommand>() == &cmd);
    CHECK(base.As<CommandNode>() == &cmd);

    // Failed downcast returns nullptr
    EventNode node;
    CHECK(node.As<CommandNode>() == nullptr);
}

} // TEST_SUITE

// =========================================================================== //
// Notification tests
// =========================================================================== //

TEST_SUITE("Notification") {

TEST_CASE("Base notification ClassName") {
    Notification n;
    CHECK(std::string(n.ClassName()) == "Notification");
}

TEST_CASE("Derived notification ClassName") {
    TestNotification tn;
    CHECK(std::string(tn.ClassName()) == "TestNotification");
}

TEST_CASE("IsA checks notification type") {
    TestNotification tn;
    Notification& base = tn;

    CHECK(base.IsA<TestNotification>());
    CHECK_FALSE(base.IsA<OtherNotification>());
}

TEST_CASE("As downcasts notification") {
    TestNotification tn;
    tn.payload = "hello";
    Notification& base = tn;

    auto* typed = base.As<TestNotification>();
    REQUIRE(typed != nullptr);
    CHECK(typed->payload == "hello");

    CHECK(base.As<OtherNotification>() == nullptr);
}

} // TEST_SUITE

// =========================================================================== //
// EventNode tests
// =========================================================================== //

TEST_SUITE("EventNode") {

TEST_CASE("Subscribe and dispatch callback") {
    EventNode subscriber;
    EventNode sender;
    int callCount = 0;
    std::string receivedClass;

    subscriber.Subscribe(&sender, "TestNotification", [&](EventNode& /*s*/, Notification& n) {
        callCount++;
        receivedClass = std::string(n.ClassName());
    });

    CHECK(subscriber.SubscriptionCount() == 1);

    TestNotification notif;
    subscriber.DispatchCallbacks(notif, &sender);

    CHECK(callCount == 1);
    CHECK(receivedClass == "TestNotification");
}

TEST_CASE("Wildcard type subscription receives all notifications") {
    EventNode subscriber;
    EventNode sender;
    int callCount = 0;

    subscriber.Subscribe(nullptr, "*", [&](EventNode& /*s*/, Notification& /*n*/) {
        callCount++;
    });

    TestNotification tn;
    OtherNotification on;
    subscriber.DispatchCallbacks(tn, &sender);
    subscriber.DispatchCallbacks(on, &sender);

    CHECK(callCount == 2);
}

TEST_CASE("Non-matching notification type is not dispatched") {
    EventNode subscriber;
    EventNode sender;
    int callCount = 0;

    subscriber.Subscribe(&sender, "TestNotification", [&](EventNode& /*s*/, Notification& /*n*/) {
        callCount++;
    });

    OtherNotification on;
    subscriber.DispatchCallbacks(on, &sender);

    CHECK(callCount == 0);
}

TEST_CASE("Publisher filter: non-matching sender is not dispatched") {
    EventNode subscriber;
    EventNode senderA;
    EventNode senderB;
    int callCount = 0;

    // Subscribe only for senderA
    subscriber.Subscribe(&senderA, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { callCount++; });

    TestNotification notif;
    subscriber.DispatchCallbacks(notif, &senderB);  // wrong sender
    CHECK(callCount == 0);

    subscriber.DispatchCallbacks(notif, &senderA);  // correct sender
    CHECK(callCount == 1);
}

TEST_CASE("Wildcard publisher (nullptr) matches any sender") {
    EventNode subscriber;
    EventNode senderA;
    EventNode senderB;
    int callCount = 0;

    subscriber.Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { callCount++; });

    TestNotification notif;
    subscriber.DispatchCallbacks(notif, &senderA);
    subscriber.DispatchCallbacks(notif, &senderB);

    CHECK(callCount == 2);
}

TEST_CASE("Unsubscribe removes callback") {
    EventNode subscriber;
    int callCount = 0;

    auto id = subscriber.Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { callCount++; });

    CHECK(subscriber.SubscriptionCount() == 1);
    subscriber.Unsubscribe(id);
    CHECK(subscriber.SubscriptionCount() == 0);

    TestNotification notif;
    subscriber.DispatchCallbacks(notif, nullptr);
    CHECK(callCount == 0);
}

TEST_CASE("UnsubscribeAll clears all callbacks") {
    EventNode subscriber;

    subscriber.Subscribe(nullptr, "TestNotification", [](EventNode& /*s*/, Notification& /*n*/) {});
    subscriber.Subscribe(nullptr, "OtherNotification", [](EventNode& /*s*/, Notification& /*n*/) {});
    CHECK(subscriber.SubscriptionCount() == 2);

    subscriber.UnsubscribeAll();
    CHECK(subscriber.SubscriptionCount() == 0);
}

TEST_CASE("ScopedSubscription auto-unsubscribes") {
    EventNode subscriber;
    EventNode sender;
    int callCount = 0;

    {
        ScopedSubscription sub(subscriber, &sender, "TestNotification",
                               [&](EventNode& /*s*/, Notification& /*n*/) { callCount++; });
        CHECK(sub.IsActive());
        CHECK(subscriber.SubscriptionCount() == 1);

        TestNotification notif;
        subscriber.DispatchCallbacks(notif, &sender);
        CHECK(callCount == 1);
    } // sub destroyed -> unsubscribed

    CHECK(subscriber.SubscriptionCount() == 0);

    TestNotification notif;
    subscriber.DispatchCallbacks(notif, &sender);
    CHECK(callCount == 1); // no new calls
}

TEST_CASE("ScopedSubscription move semantics") {
    EventNode subscriber;
    int callCount = 0;

    ScopedSubscription sub1(subscriber, nullptr, "TestNotification",
                            [&](EventNode& /*s*/, Notification& /*n*/) { callCount++; });
    CHECK(subscriber.SubscriptionCount() == 1);

    ScopedSubscription sub2 = std::move(sub1);
    CHECK_FALSE(sub1.IsActive()); // NOLINT(bugprone-use-after-move)
    CHECK(sub2.IsActive());
    CHECK(subscriber.SubscriptionCount() == 1);

    sub2.Release();
    CHECK_FALSE(sub2.IsActive());
    CHECK(subscriber.SubscriptionCount() == 0);
}

TEST_CASE("Multiple subscriptions on same event") {
    EventNode subscriber;
    int count1 = 0;
    int count2 = 0;

    subscriber.Subscribe(nullptr, "TestNotification", [&](EventNode& /*s*/, Notification& /*n*/) { count1++; });
    subscriber.Subscribe(nullptr, "TestNotification", [&](EventNode& /*s*/, Notification& /*n*/) { count2++; });

    TestNotification notif;
    subscriber.DispatchCallbacks(notif, nullptr);

    CHECK(count1 == 1);
    CHECK(count2 == 1);
}

TEST_CASE("SendNotification triggers Subscribe callbacks during propagation") {
    // Chain: sender -> mid -> root
    // Callbacks on each node with publisher=senderPtr (wildcard would also work)
    TestCommand root(nullptr, "root");
    auto mid = std::make_unique<TestCommand>(nullptr, "mid");
    auto sender = std::make_unique<TestCommand>(nullptr, "sender");
    root.AddChild(std::move(mid));
    root.Children()[0]->AddChild(std::move(sender));

    auto* senderPtr = root.Children()[0]->Children()[0].get();
    auto* midPtr = root.Children()[0].get();

    int senderCalls = 0;
    int midCalls = 0;
    int rootCalls = 0;

    // Subscribe with wildcard publisher on each node in the chain
    senderPtr->Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { senderCalls++; });
    midPtr->Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { midCalls++; });
    root.Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { rootCalls++; });

    TestNotification notif;
    senderPtr->SendNotification(senderPtr, notif);

    // All three should fire: sender, mid, root
    CHECK(senderCalls == 1);
    CHECK(midCalls == 1);
    CHECK(rootCalls == 1);
}

TEST_CASE("Subscribe on node sees only matching notification type") {
    TestCommand node(nullptr, "node");
    int testCalls = 0;
    int otherCalls = 0;

    node.Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { testCalls++; });
    node.Subscribe(nullptr, "OtherNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { otherCalls++; });

    TestNotification tn;
    node.SendNotification(&node, tn);

    CHECK(testCalls == 1);
    CHECK(otherCalls == 0);
}

TEST_CASE("Publisher filter during SendNotification propagation") {
    // Chain: child -> root
    // Subscribe on root with publisher=childPtr, should fire.
    // Subscribe on root with publisher=&root, should NOT fire.
    TestCommand root(nullptr, "root");
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    root.AddChild(std::move(child));
    auto* childPtr = root.Children()[0].get();

    int matchCalls = 0;
    int mismatchCalls = 0;

    root.Subscribe(childPtr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { matchCalls++; });
    root.Subscribe(&root, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { mismatchCalls++; });

    TestNotification notif;
    childPtr->SendNotification(childPtr, notif);

    CHECK(matchCalls == 1);
    CHECK(mismatchCalls == 0);
}

TEST_CASE("AddAnalyseNotificationCB type-safe convenience") {
    // subscriber=root, publisher=childPtr
    TestCommand root(nullptr, "root");
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    root.AddChild(std::move(child));
    auto* childPtr = root.Children()[0].get();

    std::string receivedClass;
    root.AddAnalyseNotificationCB<TestNotification>(
        childPtr,
        [&](TestNotification& n) { receivedClass = std::string(n.ClassName()); });

    TestNotification tn;
    childPtr->SendNotification(childPtr, tn);
    CHECK(receivedClass == "TestNotification");

    // Non-matching type should not trigger
    receivedClass.clear();
    OtherNotification on;
    childPtr->SendNotification(childPtr, on);
    CHECK(receivedClass.empty());
}

TEST_CASE("DontTransmitToParent stops callback dispatch on ancestors") {
    // Handler at mid node stops propagation
    class StopperCommand : public CommandNode {
    public:
        using CommandNode::CommandNode;
        auto AnalyseNotification(CommandNode* /*s*/, Notification& /*n*/)
            -> PropagationMode override { return PropagationMode::DontTransmitToParent; }
    };

    TestCommand root(nullptr, "root");
    auto mid = std::make_unique<StopperCommand>(nullptr, "stopper");
    auto sender = std::make_unique<TestCommand>(nullptr, "sender");
    root.AddChild(std::move(mid));
    root.Children()[0]->AddChild(std::move(sender));

    auto* senderPtr = root.Children()[0]->Children()[0].get();
    auto* midPtr = root.Children()[0].get();

    int midCalls = 0;
    int rootCalls = 0;

    // Mid's callback fires BEFORE AnalyseNotification stops propagation
    midPtr->Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { midCalls++; });
    root.Subscribe(nullptr, "TestNotification",
        [&](EventNode& /*s*/, Notification& /*n*/) { rootCalls++; });

    TestNotification notif;
    senderPtr->SendNotification(senderPtr, notif);

    // Mid's callback fires (before AnalyseNotification), root's does not
    CHECK(midCalls == 1);
    CHECK(rootCalls == 0);
}

} // TEST_SUITE

// =========================================================================== //
// CommandNode tests
// =========================================================================== //

TEST_SUITE("CommandNode") {

TEST_CASE("Constructor sets identity and AddChild sets parent") {
    TestCommand root(nullptr, "root");
    auto childPtr = std::make_unique<TestCommand>(nullptr, "child", CommandMode::Shared);
    auto* child = root.AddChild(std::move(childPtr));

    CHECK(std::string(root.Id()) == "root");
    CHECK(std::string(child->Id()) == "child");
    CHECK(child->Parent() == &root);
    CHECK(child->StartMode() == CommandMode::Shared);
    CHECK(root.ChildCount() == 1);
    CHECK(root.Children()[0].get() == child);
}

TEST_CASE("Default start mode is Undefined") {
    TestCommand cmd;
    CHECK(cmd.StartMode() == CommandMode::Undefined);
}

TEST_CASE("RemoveChild + AddChild transfers ownership between parents") {
    TestCommand root1(nullptr, "root1");
    TestCommand root2(nullptr, "root2");
    auto* child = root1.AddChild(std::make_unique<TestCommand>(nullptr, "child"));

    CHECK(root1.ChildCount() == 1);
    CHECK(root2.ChildCount() == 0);

    auto owned = root1.RemoveChild(child);
    auto* child2 = root2.AddChild(std::move(owned));

    CHECK(root1.ChildCount() == 0);
    CHECK(root2.ChildCount() == 1);
    CHECK(child2->Parent() == &root2);
}

TEST_CASE("RemoveChild detaches and returns ownership") {
    TestCommand root(nullptr, "root");
    auto* child = root.AddChild(std::make_unique<TestCommand>(nullptr, "child"));

    auto owned = root.RemoveChild(child);

    REQUIRE(owned != nullptr);
    CHECK(owned->Parent() == nullptr);
    CHECK(root.ChildCount() == 0);
}

TEST_CASE("Parent destructor destroys children") {
    CommandNode* childRaw = nullptr;
    {
        TestCommand root(nullptr, "root");
        childRaw = root.AddChild(std::make_unique<TestCommand>(nullptr, "child"));
        CHECK(root.ChildCount() == 1);
    }
    // root destroyed -> child destroyed (unique_ptr)
    // childRaw is now dangling, just verify root cleanup worked
    (void)childRaw;
}

TEST_CASE("RemoveChild before parent destruction keeps child alive") {
    auto childOwned = std::make_unique<TestCommand>(nullptr, "child");
    auto* childRaw = childOwned.get();
    {
        TestCommand parent(nullptr, "parent");
        parent.AddChild(std::move(childOwned));
        CHECK(childRaw->Parent() == &parent);
        childOwned.reset(); // release our empty ptr
        auto recovered = parent.RemoveChild(childRaw);
        REQUIRE(recovered != nullptr);
        CHECK(recovered->Parent() == nullptr);
        // prevent destruction by holding it
        childOwned.reset(static_cast<TestCommand*>(recovered.release()));
    }
    CHECK(childOwned->Parent() == nullptr);
}

TEST_CASE("SendNotification delivers to target") {
    TestCommand sender(nullptr, "sender");
    TestCommand target(nullptr, "target");
    target.handleNotification = true;

    TestNotification notif;
    sender.SendNotification(&target, notif);

    CHECK(target.analyseCount == 1);
    CHECK(target.lastSender == &sender);
    CHECK(target.lastNotifClass == "TestNotification");
}

TEST_CASE("Notification propagates to parent when not handled") {
    TestCommand root(nullptr, "root");
    auto* child = static_cast<TestCommand*>(
        root.AddChild(std::make_unique<TestCommand>(nullptr, "child")));
    TestCommand sender(nullptr, "sender");

    // child doesn't handle -> propagates to root
    child->handleNotification = false;
    root.handleNotification = true;

    TestNotification notif;
    sender.SendNotification(child, notif);

    CHECK(child->analyseCount == 1);
    CHECK(root.analyseCount == 1);
    CHECK(root.lastSender == &sender);
}

TEST_CASE("Notification stops propagating when handled") {
    TestCommand root(nullptr, "root");
    auto* child = static_cast<TestCommand*>(
        root.AddChild(std::make_unique<TestCommand>(nullptr, "child")));
    TestCommand sender(nullptr, "sender");

    child->handleNotification = true;

    TestNotification notif;
    sender.SendNotification(child, notif);

    CHECK(child->analyseCount == 1);
    CHECK(root.analyseCount == 0); // not reached
}

TEST_CASE("Notification propagates through multiple levels") {
    TestCommand grandparent(nullptr, "gp");
    auto* parent = static_cast<TestCommand*>(
        grandparent.AddChild(std::make_unique<TestCommand>(nullptr, "p")));
    auto* child = static_cast<TestCommand*>(
        parent->AddChild(std::make_unique<TestCommand>(nullptr, "c")));
    TestCommand sender(nullptr, "s");

    // nobody handles
    TestNotification notif;
    sender.SendNotification(child, notif);

    CHECK(child->analyseCount == 1);
    CHECK(parent->analyseCount == 1);
    CHECK(grandparent.analyseCount == 1);
}

TEST_CASE("Activate / Deactivate / Cancel lifecycle") {
    TestCommand cmd(nullptr, "cmd");
    TestNotification notif;

    CHECK(cmd.Activate(nullptr, notif) == StatusChangeRC::Completed);
    CHECK(cmd.activateCount == 1);

    CHECK(cmd.Deactivate(nullptr, notif) == StatusChangeRC::Completed);
    CHECK(cmd.deactivateCount == 1);

    CHECK(cmd.Cancel(nullptr, notif) == StatusChangeRC::Completed);
    CHECK(cmd.cancelCount == 1);
}

TEST_CASE("RequestDelayedDestruction sets flag") {
    TestCommand cmd;
    CHECK_FALSE(cmd.IsDestructionRequested());

    cmd.RequestDelayedDestruction();
    CHECK(cmd.IsDestructionRequested());
}

TEST_CASE("SetId changes identifier") {
    TestCommand cmd(nullptr, "original");
    CHECK(std::string(cmd.Id()) == "original");

    cmd.SetId("renamed");
    CHECK(std::string(cmd.Id()) == "renamed");
}

TEST_CASE("RTTI chain: TestCommand -> CommandNode -> EventNode -> BaseObject") {
    TestCommand cmd;
    CHECK(std::string(cmd.ClassName()) == "TestCommand");
    CHECK(cmd.IsAKindOf("TestCommand"));
    CHECK(cmd.IsAKindOf("CommandNode"));
    CHECK(cmd.IsAKindOf("EventNode"));
    CHECK(cmd.IsAKindOf("BaseObject"));
}

TEST_CASE("SendNotification to nullptr is safe") {
    TestCommand sender;
    TestNotification notif;
    sender.SendNotification(nullptr, notif); // should not crash
}

TEST_CASE("ForwardToParent sends notification to parent") {
    TestCommand root(nullptr, "root");
    auto* child = static_cast<TestCommand*>(
        root.AddChild(std::make_unique<TestCommand>(nullptr, "child")));
    root.handleNotification = true;

    TestNotification notif;
    child->TestForwardToParent(notif);

    CHECK(root.analyseCount == 1);
    CHECK(root.lastSender == child);
}

TEST_CASE("ForwardToParent with no parent is safe") {
    TestCommand orphan(nullptr, "orphan");
    TestNotification notif;
    orphan.TestForwardToParent(notif); // should not crash
}

} // TEST_SUITE

// =========================================================================== //
// MetaClass tests
// =========================================================================== //

TEST_SUITE("MetaClass") {

TEST_CASE("BaseObject MetaClass is root") {
    CHECK(BaseObject::GetStaticMetaClass() != nullptr);
    CHECK(std::string(BaseObject::GetStaticMetaClass()->name) == "BaseObject");
    CHECK(BaseObject::GetStaticMetaClass()->parent == nullptr);
}

TEST_CASE("EventNode MetaClass parent is BaseObject") {
    const auto* mc = EventNode::GetStaticMetaClass();
    CHECK(std::string(mc->name) == "EventNode");
    CHECK(mc->parent == BaseObject::GetStaticMetaClass());
}

TEST_CASE("CommandNode MetaClass parent is EventNode") {
    const auto* mc = CommandNode::GetStaticMetaClass();
    CHECK(std::string(mc->name) == "CommandNode");
    CHECK(mc->parent == EventNode::GetStaticMetaClass());
}

TEST_CASE("TestCommand MetaClass parent is CommandNode") {
    const auto* mc = TestCommand::GetStaticMetaClass();
    CHECK(std::string(mc->name) == "TestCommand");
    CHECK(mc->parent == CommandNode::GetStaticMetaClass());
}

TEST_CASE("MetaClass::IsAKindOf walks parent chain") {
    const auto* mc = TestCommand::GetStaticMetaClass();
    CHECK(mc->IsAKindOf("TestCommand"));
    CHECK(mc->IsAKindOf("CommandNode"));
    CHECK(mc->IsAKindOf("EventNode"));
    CHECK(mc->IsAKindOf("BaseObject"));
    CHECK_FALSE(mc->IsAKindOf("SomethingElse"));
}

TEST_CASE("GetMetaClass() returns concrete type MetaClass") {
    TestCommand cmd;
    const BaseObject& base = cmd;
    CHECK(base.GetMetaClass() == TestCommand::GetStaticMetaClass());
    CHECK(std::string(base.GetMetaClass()->name) == "TestCommand");
}

TEST_CASE("ClassName delegates to MetaClass") {
    TestCommand cmd;
    CHECK(std::string(cmd.ClassName()) == "TestCommand");

    EventNode node;
    CHECK(std::string(node.ClassName()) == "EventNode");

    // BaseObject direct
    class PlainObj : public BaseObject {};
    PlainObj obj;
    CHECK(std::string(obj.ClassName()) == "BaseObject");
}

TEST_CASE("IsAKindOf delegates to MetaClass chain") {
    TestCommand cmd;
    CHECK(cmd.IsAKindOf("TestCommand"));
    CHECK(cmd.IsAKindOf("CommandNode"));
    CHECK(cmd.IsAKindOf("EventNode"));
    CHECK(cmd.IsAKindOf("BaseObject"));
    CHECK_FALSE(cmd.IsAKindOf("Notification"));
}

// =========================================================================== //
// NotificationQueue tests
// =========================================================================== //

TEST_CASE("NotificationQueue: basic enqueue and flush") {
    NotificationQueue queue;
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    root->AddChild(std::move(child));
    auto* childPtr = dynamic_cast<TestCommand*>(root->Children()[0].get());

    // Subscribe on childPtr for TestNotification
    int received = 0;
    childPtr->Subscribe(nullptr, "TestNotification",
        [&](EventNode&, Notification&) { ++received; });

    // Enqueue a notification targeting childPtr
    auto notif = std::make_shared<TestNotification>();
    queue.Enqueue(root.get(), childPtr, notif);

    CHECK(queue.PendingCount() == 1);
    CHECK(received == 0);

    queue.FlushPending();

    CHECK(queue.PendingCount() == 0);
    CHECK(received == 1);
}

TEST_CASE("NotificationQueue: discards if target destroyed before flush") {
    NotificationQueue queue;
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto target = std::make_unique<TestCommand>(nullptr, "target");

    auto notif = std::make_shared<TestNotification>();
    queue.Enqueue(root.get(), target.get(), notif);
    CHECK(queue.PendingCount() == 1);

    // Destroy target before flush
    target.reset();

    queue.FlushPending();
    // Should silently discard — no crash
    CHECK(queue.PendingCount() == 0);
}

TEST_CASE("NotificationQueue: sender destroyed -> wildcard match") {
    NotificationQueue queue;
    auto sender = std::make_unique<TestCommand>(nullptr, "sender");
    auto target = std::make_unique<TestCommand>(nullptr, "target");

    int wildcardHits = 0;
    int senderHits = 0;
    // Wildcard publisher subscription
    target->Subscribe(nullptr, "TestNotification",
        [&](EventNode&, Notification&) { ++wildcardHits; });
    // Specific publisher subscription
    target->Subscribe(sender.get(), "TestNotification",
        [&](EventNode&, Notification&) { ++senderHits; });

    auto notif = std::make_shared<TestNotification>();
    queue.Enqueue(sender.get(), target.get(), notif);

    // Destroy sender before flush
    sender.reset();

    queue.FlushPending();
    // Wildcard should fire (sender=nullptr matches wildcard)
    CHECK(wildcardHits == 1);
    // Specific publisher should NOT fire (sender is nullptr, not the original)
    CHECK(senderHits == 0);
}

TEST_CASE("NotificationQueue: Clear discards without dispatch") {
    NotificationQueue queue;
    auto node = std::make_unique<TestCommand>(nullptr, "node");
    int received = 0;
    node->Subscribe(nullptr, "*", [&](EventNode&, Notification&) { ++received; });

    queue.Enqueue(node.get(), node.get(), std::make_shared<TestNotification>());
    queue.Enqueue(node.get(), node.get(), std::make_shared<TestNotification>());
    CHECK(queue.PendingCount() == 2);

    queue.Clear();
    CHECK(queue.PendingCount() == 0);
    CHECK(received == 0);
}

TEST_CASE("SendNotificationQueued: falls back to sync when no queue") {
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    root->AddChild(std::move(child));
    auto* childPtr = dynamic_cast<TestCommand*>(root->Children()[0].get());

    int received = 0;
    childPtr->Subscribe(nullptr, "TestNotification",
        [&](EventNode&, Notification&) { ++received; });

    // No queue wired — should dispatch synchronously
    auto notif = std::make_shared<TestNotification>();
    root->SendNotificationQueued(childPtr, notif);
    CHECK(received == 1);
}

TEST_CASE("SendNotificationQueued: enqueues when queue is wired") {
    NotificationQueue queue;
    auto root = std::make_unique<TestRoot>(&queue);

    auto child = std::make_unique<TestCommand>(nullptr, "child");
    root->AddChild(std::move(child));
    auto* childPtr = dynamic_cast<TestCommand*>(root->Children()[0].get());

    int received = 0;
    childPtr->Subscribe(nullptr, "TestNotification",
        [&](EventNode&, Notification&) { ++received; });

    auto notif = std::make_shared<TestNotification>();
    root->SendNotificationQueued(childPtr, notif);

    // Not yet dispatched
    CHECK(received == 0);
    CHECK(queue.PendingCount() == 1);

    queue.FlushPending();
    CHECK(received == 1);
}

TEST_CASE("GetNotificationQueue walks up to root") {
    NotificationQueue queue;
    auto root = std::make_unique<TestRoot>(&queue);

    auto child = std::make_unique<TestCommand>(nullptr, "child");
    root->AddChild(std::move(child));
    auto* childPtr = root->Children()[0].get();

    CHECK(childPtr->GetNotificationQueue() == &queue);
    CHECK(root->GetNotificationQueue() == &queue);
}

TEST_CASE("AliveToken expires on destruction") {
    std::weak_ptr<void> token;
    {
        auto node = std::make_unique<TestCommand>(nullptr, "temp");
        token = node->AliveToken();
        CHECK_FALSE(token.expired());
    }
    CHECK(token.expired());
}

// --------------------------------------------------------------------------- //
// Layer 1: ScopedSubscription survives dead subscriber
// --------------------------------------------------------------------------- //

TEST_CASE("ScopedSubscription::Release is safe after subscriber destroyed") {
    // Layer 1: ScopedSubscription uses weak_ptr to detect dead subscriber.
    // When subscriber is destroyed first, Release() must not crash.
    auto subscriber = std::make_unique<TestCommand>(nullptr, "sub");
    auto publisher = std::make_unique<TestCommand>(nullptr, "pub");

    ScopedSubscription sub(*subscriber, publisher.get(), "TestNotification",
        [](EventNode&, Notification&) {});
    CHECK(sub.IsActive());
    CHECK(subscriber->SubscriptionCount() == 1);

    // Destroy subscriber while ScopedSubscription is still alive
    subscriber.reset();

    // Release must not crash (subscriber is dead, weak_ptr expired)
    sub.Release();
    CHECK_FALSE(sub.IsActive());
}

TEST_CASE("ScopedSubscription dtor is safe after subscriber destroyed") {
    auto publisher = std::make_unique<TestCommand>(nullptr, "pub");
    {
        auto subscriber = std::make_unique<TestCommand>(nullptr, "sub");
        ScopedSubscription sub(*subscriber, publisher.get(), "TestNotification",
            [](EventNode&, Notification&) {});
        // subscriber destroyed here (before sub), sub dtor must not crash
        subscriber.reset();
        // sub goes out of scope here — dtor calls Release on dead subscriber
    }
    // If we reach here, no crash occurred
    CHECK(true);
}

// --------------------------------------------------------------------------- //
// Layer 2: Publisher dtor cleans subscriber callbacks (bidirectional tracking)
// --------------------------------------------------------------------------- //

TEST_CASE("Publisher destruction removes callbacks from subscriber") {
    auto subscriber = std::make_unique<TestCommand>(nullptr, "sub");
    auto publisher = std::make_unique<TestCommand>(nullptr, "pub");

    subscriber->Subscribe(publisher.get(), "TestNotification",
        [](EventNode&, Notification&) {});
    CHECK(subscriber->SubscriptionCount() == 1);

    // Destroy publisher — Layer 2 should proactively remove the callback
    publisher.reset();

    CHECK(subscriber->SubscriptionCount() == 0);
}

TEST_CASE("Publisher destruction removes only its callbacks, not others") {
    auto subscriber = std::make_unique<TestCommand>(nullptr, "sub");
    auto pub1 = std::make_unique<TestCommand>(nullptr, "pub1");
    auto pub2 = std::make_unique<TestCommand>(nullptr, "pub2");

    subscriber->Subscribe(pub1.get(), "TestNotification",
        [](EventNode&, Notification&) {});
    subscriber->Subscribe(pub2.get(), "TestNotification",
        [](EventNode&, Notification&) {});
    subscriber->Subscribe(nullptr, "*",
        [](EventNode&, Notification&) {}); // wildcard
    CHECK(subscriber->SubscriptionCount() == 3);

    // Destroy pub1 — only its callback should be removed
    pub1.reset();
    CHECK(subscriber->SubscriptionCount() == 2);

    // Destroy pub2 — its callback removed, wildcard stays
    pub2.reset();
    CHECK(subscriber->SubscriptionCount() == 1);
}

TEST_CASE("Subscriber destruction removes reverse refs from publisher") {
    auto subscriber = std::make_unique<TestCommand>(nullptr, "sub");
    auto publisher = std::make_unique<TestCommand>(nullptr, "pub");

    subscriber->Subscribe(publisher.get(), "TestNotification",
        [](EventNode&, Notification&) {});

    // Destroy subscriber — should remove reverse ref from publisher
    subscriber.reset();

    // Verify publisher can be destroyed cleanly (no dangling refs)
    publisher.reset();
    CHECK(true); // no crash
}

TEST_CASE("UnsubscribeAll removes reverse refs from all publishers") {
    auto subscriber = std::make_unique<TestCommand>(nullptr, "sub");
    auto pub1 = std::make_unique<TestCommand>(nullptr, "pub1");
    auto pub2 = std::make_unique<TestCommand>(nullptr, "pub2");

    subscriber->Subscribe(pub1.get(), "TestNotification",
        [](EventNode&, Notification&) {});
    subscriber->Subscribe(pub2.get(), "OtherNotification",
        [](EventNode&, Notification&) {});
    CHECK(subscriber->SubscriptionCount() == 2);

    subscriber->UnsubscribeAll();
    CHECK(subscriber->SubscriptionCount() == 0);

    // Publishers should be cleanly destroyable
    pub1.reset();
    pub2.reset();
    CHECK(true);
}

// --------------------------------------------------------------------------- //
// Layer 3: RemoveChild auto-detaches cross-boundary subscriptions
// --------------------------------------------------------------------------- //

TEST_CASE("RemoveChild cleans up cross-boundary subscriptions (subscriber inside, publisher outside)") {
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    auto* childPtr = child.get();

    root->AddChild(std::move(child));

    // Child subscribes to root (publisher outside subtree)
    childPtr->Subscribe(root.get(), "TestNotification",
        [](EventNode&, Notification&) {});
    CHECK(childPtr->SubscriptionCount() == 1);

    // Remove child from tree — cross-boundary subscription should be cleaned
    auto owned = root->RemoveChild(childPtr);
    CHECK(owned != nullptr);
    CHECK(childPtr->SubscriptionCount() == 0);
}

TEST_CASE("RemoveChild cleans up cross-boundary subscriptions (subscriber outside, publisher inside)") {
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    auto* childPtr = child.get();

    root->AddChild(std::move(child));

    // Root subscribes to child (publisher inside detached subtree)
    root->Subscribe(childPtr, "TestNotification",
        [](EventNode&, Notification&) {});
    CHECK(root->SubscriptionCount() == 1);

    // Remove child — root's subscription to child should be cleaned
    auto owned = root->RemoveChild(childPtr);
    CHECK(owned != nullptr);
    CHECK(root->SubscriptionCount() == 0);
}

TEST_CASE("RemoveChild preserves intra-subtree subscriptions") {
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto parent = std::make_unique<TestCommand>(nullptr, "parent");
    auto* parentPtr = parent.get();
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    auto* childPtr = child.get();

    parentPtr->AddChild(std::move(child));
    root->AddChild(std::move(parent));

    // Child subscribes to parent (both inside the subtree being removed)
    childPtr->Subscribe(parentPtr, "TestNotification",
        [](EventNode&, Notification&) {});
    CHECK(childPtr->SubscriptionCount() == 1);

    // Remove parent subtree from root
    auto owned = root->RemoveChild(parentPtr);
    CHECK(owned != nullptr);

    // Intra-subtree subscription should be preserved
    CHECK(childPtr->SubscriptionCount() == 1);
}

TEST_CASE("RemoveChild preserves wildcard subscriptions") {
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto child = std::make_unique<TestCommand>(nullptr, "child");
    auto* childPtr = child.get();

    root->AddChild(std::move(child));

    // Child has a wildcard subscription (publisher=nullptr)
    childPtr->Subscribe(nullptr, "*",
        [](EventNode&, Notification&) {});
    CHECK(childPtr->SubscriptionCount() == 1);

    // Remove child — wildcard subscription should be preserved
    auto owned = root->RemoveChild(childPtr);
    CHECK(owned != nullptr);
    CHECK(childPtr->SubscriptionCount() == 1);
}

TEST_CASE("RemoveChild deep subtree cleans external subscriptions recursively") {
    auto root = std::make_unique<TestCommand>(nullptr, "root");
    auto a = std::make_unique<TestCommand>(nullptr, "a");
    auto* aPtr = a.get();
    auto b = std::make_unique<TestCommand>(nullptr, "b");
    auto* bPtr = b.get();
    auto c = std::make_unique<TestCommand>(nullptr, "c");
    auto* cPtr = c.get();

    // Tree: root -> a -> b -> c
    bPtr->AddChild(std::move(c));
    aPtr->AddChild(std::move(b));
    root->AddChild(std::move(a));

    // c subscribes to root (external)
    cPtr->Subscribe(root.get(), "TestNotification",
        [](EventNode&, Notification&) {});
    // b subscribes to a (internal to subtree)
    bPtr->Subscribe(aPtr, "TestNotification",
        [](EventNode&, Notification&) {});

    CHECK(cPtr->SubscriptionCount() == 1);
    CHECK(bPtr->SubscriptionCount() == 1);

    // Remove subtree at a
    auto owned = root->RemoveChild(aPtr);

    // c's external subscription to root: cleaned
    CHECK(cPtr->SubscriptionCount() == 0);
    // b's internal subscription to a: preserved
    CHECK(bPtr->SubscriptionCount() == 1);
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
