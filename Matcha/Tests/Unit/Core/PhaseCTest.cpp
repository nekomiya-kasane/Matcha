#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "QtAppGuard.h"

#include <Matcha/UiNodes/Core/FocusChain.h>
#include <Matcha/UiNodes/Core/FocusManager.h>
#include <Matcha/UiNodes/Core/InteractionFSM.h>
#include <Matcha/UiNodes/Core/TokenEnums.h>
#include <Matcha/UiNodes/Core/TooltipSpec.h>
#include <Matcha/UiNodes/Core/UiNodeNotification.h>
#include <Matcha/UiNodes/Core/WidgetNode.h>
#include <Matcha/Widgets/Core/IAnimationService.h>

#include <QWidget>

#include <string>
#include <utility>

using namespace matcha::fw;
using namespace matcha::gui;

// ===========================================================================
// Minimal concrete WidgetNode for testing
// ===========================================================================

namespace {

class StubWidgetNode final : public WidgetNode {
public:
    explicit StubWidgetNode(std::string id)
        : WidgetNode(std::move(id), NodeType::Custom) {}
protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override
    {
        return new QWidget(parent); // NOLINT
    }
};

} // anonymous namespace

// ===========================================================================
// C1: Animation Engine — IAnimationService interface
// ===========================================================================

TEST_SUITE("PhaseC::AnimationEngine") {

TEST_CASE("TransitionHandle sentinel") {
    CHECK(TransitionHandle::Invalid == static_cast<TransitionHandle>(0));
}

TEST_CASE("GroupId sentinel") {
    CHECK(GroupId::Invalid == static_cast<GroupId>(0));
}

TEST_CASE("GroupMode enum values") {
    CHECK(GroupMode::Parallel != GroupMode::Sequential);
}

TEST_CASE("GroupAnimationSpec defaults") {
    GroupAnimationSpec spec;
    CHECK(spec.target == nullptr);
    CHECK(spec.property == AnimationPropertyId::Opacity);
    CHECK(spec.duration == AnimationToken::Normal);
    CHECK(spec.easing == EasingToken::OutCubic);
}

TEST_CASE("AnimationPropertyId values") {
    CHECK(std::to_underlying(AnimationPropertyId::Opacity) == 0);
    CHECK(std::to_underlying(AnimationPropertyId::BackgroundColor) == 10);
    CHECK(std::to_underlying(AnimationPropertyId::ArrowRotation) == 100);
    CHECK(std::to_underlying(AnimationPropertyId::UserDefined) == 1000);
}

TEST_CASE("AnimatableValue factories") {
    auto d = AnimatableValue::FromDouble(3.14);
    CHECK(d.type == AnimatableValue::Type::Double);
    CHECK(d.asDouble == doctest::Approx(3.14));

    auto i = AnimatableValue::FromInt(42);
    CHECK(i.type == AnimatableValue::Type::Int);
    CHECK(i.asInt == 42);

    auto c = AnimatableValue::FromRgba(255, 0, 128, 200);
    CHECK(c.type == AnimatableValue::Type::Rgba);
    CHECK(((c.asRgba >> 24) & 0xFF) == 200); // alpha
    CHECK(((c.asRgba >> 16) & 0xFF) == 255); // red
    CHECK(((c.asRgba >> 8) & 0xFF) == 0);    // green
    CHECK((c.asRgba & 0xFF) == 128);          // blue

    auto p = AnimatableValue::FromPoint(10, 20);
    CHECK(p.type == AnimatableValue::Type::Point2D);
    CHECK(p.asPoint.x == 10);
    CHECK(p.asPoint.y == 20);
}

} // TEST_SUITE AnimationEngine

// ===========================================================================
// C4: Spring animation tokens
// ===========================================================================

TEST_SUITE("PhaseC::SpringToken") {

TEST_CASE("EasingToken::Spring exists") {
    CHECK(std::to_underlying(EasingToken::Spring) == 3);
    CHECK(kEasingTokenCount == 4);
}

TEST_CASE("SpringSpec defaults") {
    SpringSpec spec;
    CHECK(spec.mass == doctest::Approx(1.0F));
    CHECK(spec.stiffness == doctest::Approx(200.0F));
    CHECK(spec.damping == doctest::Approx(20.0F));
}

TEST_CASE("SpringSpec custom values") {
    SpringSpec spec{2.0F, 300.0F, 15.0F};
    CHECK(spec.mass == doctest::Approx(2.0F));
    CHECK(spec.stiffness == doctest::Approx(300.0F));
    CHECK(spec.damping == doctest::Approx(15.0F));
}

} // TEST_SUITE SpringToken

// ===========================================================================
// C5: Focus chain
// ===========================================================================

TEST_SUITE("PhaseC::FocusChain") {

TEST_CASE("WidgetNode focus properties default") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    CHECK_FALSE(node.IsFocusable());
    CHECK(node.TabIndex() == -1);
}

TEST_CASE("SetFocusable / IsFocusable") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetFocusable(true);
    CHECK(node.IsFocusable());
    node.SetFocusable(false);
    CHECK_FALSE(node.IsFocusable());
}

TEST_CASE("SetTabIndex / TabIndex") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetTabIndex(5);
    CHECK(node.TabIndex() == 5);
    node.SetTabIndex(-1);
    CHECK(node.TabIndex() == -1);
}

TEST_CASE("FocusChain::Collect empty tree") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode root("root");
    auto chain = FocusChain::Collect(&root);
    CHECK(chain.empty());
}

TEST_CASE("FocusChain::Collect single focusable") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode root("root");
    root.SetFocusable(true);
    auto chain = FocusChain::Collect(&root);
    CHECK(chain.size() == 1);
    CHECK(chain[0] == &root);
}

TEST_CASE("FocusChain::Next wraps around") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode a("a"), b("b"), c("c");
    a.SetFocusable(true);
    b.SetFocusable(true);
    c.SetFocusable(true);

    std::vector<WidgetNode*> chain = {&a, &b, &c};
    CHECK(FocusChain::Next(chain, &a) == &b);
    CHECK(FocusChain::Next(chain, &b) == &c);
    CHECK(FocusChain::Next(chain, &c) == &a); // wraps
}

TEST_CASE("FocusChain::Previous wraps around") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode a("a"), b("b"), c("c");
    a.SetFocusable(true);
    b.SetFocusable(true);
    c.SetFocusable(true);

    std::vector<WidgetNode*> chain = {&a, &b, &c};
    CHECK(FocusChain::Previous(chain, &a) == &c); // wraps
    CHECK(FocusChain::Previous(chain, &b) == &a);
    CHECK(FocusChain::Previous(chain, &c) == &b);
}

TEST_CASE("FocusChain::Next with nullptr current returns first") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode a("a");
    a.SetFocusable(true);
    std::vector<WidgetNode*> chain = {&a};
    CHECK(FocusChain::Next(chain, nullptr) == &a);
}

TEST_CASE("FocusChain::Next on empty chain returns nullptr") {
    std::vector<WidgetNode*> chain;
    CHECK(FocusChain::Next(chain, nullptr) == nullptr);
}

TEST_CASE("FocusChain::Collect respects TabIndex ordering") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode root("root");
    auto a = std::make_unique<StubWidgetNode>("a");
    auto b = std::make_unique<StubWidgetNode>("b");
    auto c = std::make_unique<StubWidgetNode>("c");

    a->SetFocusable(true);
    a->SetTabIndex(3);
    b->SetFocusable(true);
    b->SetTabIndex(1);
    c->SetFocusable(true);
    c->SetTabIndex(2);

    auto* aPtr = a.get();
    auto* bPtr = b.get();
    auto* cPtr = c.get();

    root.AddNode(std::move(a));
    root.AddNode(std::move(b));
    root.AddNode(std::move(c));

    auto chain = FocusChain::Collect(&root);
    REQUIRE(chain.size() == 3);
    CHECK(chain[0] == bPtr); // tabIndex 1
    CHECK(chain[1] == cPtr); // tabIndex 2
    CHECK(chain[2] == aPtr); // tabIndex 3
}

} // TEST_SUITE FocusChain

// ===========================================================================
// C6: Focus scope
// ===========================================================================

TEST_SUITE("PhaseC::FocusScope") {

TEST_CASE("UiNode::IsFocusScope defaults to false") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    CHECK_FALSE(node.IsFocusScope());
}

TEST_CASE("SetFocusScope / IsFocusScope") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetFocusScope(true);
    CHECK(node.IsFocusScope());
    node.SetFocusScope(false);
    CHECK_FALSE(node.IsFocusScope());
}

TEST_CASE("FindEnclosingFocusScope returns self when scope") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("scope");
    node.SetFocusScope(true);
    CHECK(node.FindEnclosingFocusScope() == &node);
}

TEST_CASE("FindEnclosingFocusScope walks up to ancestor") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode root("root");
    root.SetFocusScope(true);
    auto child = std::make_unique<StubWidgetNode>("child");
    auto* childPtr = child.get();
    root.AddNode(std::move(child));

    CHECK(childPtr->FindEnclosingFocusScope() == &root);
}

TEST_CASE("FindEnclosingFocusScope returns nullptr when none") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("orphan");
    CHECK(node.FindEnclosingFocusScope() == nullptr);
}

TEST_CASE("FocusChain::Collect stops at child focus scope boundary") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode root("root");

    // a is focusable, direct child of root
    auto a = std::make_unique<StubWidgetNode>("a");
    a->SetFocusable(true);
    auto* aPtr = a.get();

    // scope is a focus scope child of root
    auto scope = std::make_unique<StubWidgetNode>("scope");
    scope->SetFocusScope(true);

    // b is focusable, child of scope (should NOT be collected from root)
    auto b = std::make_unique<StubWidgetNode>("b");
    b->SetFocusable(true);
    scope->AddNode(std::move(b));

    root.AddNode(std::move(a));
    root.AddNode(std::move(scope));

    auto chain = FocusChain::Collect(&root);
    REQUIRE(chain.size() == 1);
    CHECK(chain[0] == aPtr);
}

TEST_CASE("FocusChain::Collect within scope collects scope children") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode scope("scope");
    scope.SetFocusScope(true);

    auto a = std::make_unique<StubWidgetNode>("a");
    a->SetFocusable(true);
    auto* aPtr = a.get();

    auto b = std::make_unique<StubWidgetNode>("b");
    b->SetFocusable(true);
    auto* bPtr = b.get();

    scope.AddNode(std::move(a));
    scope.AddNode(std::move(b));

    auto chain = FocusChain::Collect(&scope);
    REQUIRE(chain.size() == 2);
    CHECK(chain[0] == aPtr);
    CHECK(chain[1] == bPtr);
}

} // TEST_SUITE FocusScope

// ===========================================================================
// C7: Tooltip
// ===========================================================================

TEST_SUITE("PhaseC::Tooltip") {

TEST_CASE("TooltipSpec default is empty") {
    TooltipSpec spec;
    CHECK(spec.IsEmpty());
    CHECK(spec.title.empty());
    CHECK(spec.shortcut.empty());
    CHECK(spec.description.empty());
    CHECK(spec.iconId.empty());
    CHECK(spec.position == TooltipPosition::Auto);
}

TEST_CASE("TooltipSpec non-empty") {
    TooltipSpec spec;
    spec.title = "Save";
    spec.shortcut = "Ctrl+S";
    CHECK_FALSE(spec.IsEmpty());
}

TEST_CASE("WidgetNode SetTooltip / HasTooltip") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    CHECK_FALSE(node.HasTooltip());

    TooltipSpec spec;
    spec.title = "Undo";
    spec.shortcut = "Ctrl+Z";
    node.SetTooltip(std::move(spec));

    CHECK(node.HasTooltip());
    CHECK(node.Tooltip().title == "Undo");
    CHECK(node.Tooltip().shortcut == "Ctrl+Z");
}

TEST_CASE("WidgetNode clear tooltip") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    TooltipSpec spec;
    spec.title = "Help";
    node.SetTooltip(std::move(spec));
    CHECK(node.HasTooltip());

    node.SetTooltip(TooltipSpec{}); // clear
    CHECK_FALSE(node.HasTooltip());
}

} // TEST_SUITE Tooltip

// ===========================================================================
// C9: Help system properties
// ===========================================================================

TEST_SUITE("PhaseC::HelpSystem") {

TEST_CASE("WidgetNode help properties default empty") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    CHECK(node.StatusHint().empty());
    CHECK(node.WhatsThis().empty());
    CHECK(node.HelpId().empty());
}

TEST_CASE("SetStatusHint / StatusHint") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetStatusHint("Ready");
    CHECK(node.StatusHint() == "Ready");
}

TEST_CASE("SetWhatsThis / WhatsThis") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetWhatsThis("This button saves your file.");
    CHECK(node.WhatsThis() == "This button saves your file.");
}

TEST_CASE("SetHelpId / HelpId") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetHelpId("com.nyan.save_button");
    CHECK(node.HelpId() == "com.nyan.save_button");
}

TEST_CASE("FromWidget returns WidgetNode from QWidget") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    auto* w = node.Widget();
    REQUIRE(w != nullptr);
    CHECK(WidgetNode::FromWidget(w) == &node);
}

TEST_CASE("FromWidget returns nullptr for unknown QWidget") {
    matcha::test::QtAppGuard::Ensure();
    QWidget plain;
    CHECK(WidgetNode::FromWidget(&plain) == nullptr);
    CHECK(WidgetNode::FromWidget(nullptr) == nullptr);
}

TEST_CASE("SetAccessibleName before widget creation applies on EnsureWidget") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetAccessibleName("Save Button");
    auto* w = node.Widget(); // triggers EnsureWidget
    REQUIRE(w != nullptr);
    CHECK(w->accessibleName() == "Save Button");
}

TEST_CASE("SetWhatsThis forwards to QWidget") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    auto* w = node.Widget();
    REQUIRE(w != nullptr);
    node.SetWhatsThis("This saves your file.");
    CHECK(w->whatsThis() == "This saves your file.");
}

TEST_CASE("SetWhatsThis before widget creation applies on EnsureWidget") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetWhatsThis("Deferred help text");
    auto* w = node.Widget();
    REQUIRE(w != nullptr);
    CHECK(w->whatsThis() == "Deferred help text");
}

TEST_CASE("SetStatusHint forwards to QWidget statusTip") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    auto* w = node.Widget();
    REQUIRE(w != nullptr);
    node.SetStatusHint("Ready to save");
    CHECK(w->statusTip() == "Ready to save");
}

TEST_CASE("SetStatusHint before widget creation applies on EnsureWidget") {
    matcha::test::QtAppGuard::Ensure();
    StubWidgetNode node("test");
    node.SetStatusHint("Deferred status");
    auto* w = node.Widget();
    REQUIRE(w != nullptr);
    CHECK(w->statusTip() == "Deferred status");
}

} // TEST_SUITE HelpSystem

// ===========================================================================
// C8+C10: Notification types
// ===========================================================================

TEST_SUITE("PhaseC::Notifications") {

TEST_CASE("LocaleChanged notification") {
    LocaleChanged notif("zh_CN");
    CHECK(notif.ClassName() == "LocaleChanged");
    CHECK(notif.Locale() == "zh_CN");
}

TEST_CASE("HelpRequested notification") {
    HelpRequested notif("com.nyan.export", true);
    CHECK(notif.ClassName() == "HelpRequested");
    CHECK(notif.GetHelpId() == "com.nyan.export");
    CHECK(notif.IsWhatsThisMode());
}

TEST_CASE("HelpRequested default") {
    HelpRequested notif;
    CHECK(notif.GetHelpId().empty());
    CHECK_FALSE(notif.IsWhatsThisMode());
}

TEST_CASE("AnimationStarted notification") {
    AnimationStarted notif(AnimationPropertyId::Opacity,
                           static_cast<TransitionHandle>(42));
    CHECK(notif.ClassName() == "AnimationStarted");
    CHECK(notif.Property() == AnimationPropertyId::Opacity);
    CHECK(notif.Handle() == static_cast<TransitionHandle>(42));
}

TEST_CASE("AnimationCompleted notification") {
    AnimationCompleted notif(AnimationPropertyId::BackgroundColor,
                             static_cast<TransitionHandle>(7));
    CHECK(notif.ClassName() == "AnimationCompleted");
    CHECK(notif.Property() == AnimationPropertyId::BackgroundColor);
    CHECK(notif.Handle() == static_cast<TransitionHandle>(7));
}

TEST_CASE("AnimationCancelled notification default") {
    AnimationCancelled notif;
    CHECK(notif.ClassName() == "AnimationCancelled");
    CHECK(notif.Property() == AnimationPropertyId::Opacity);
    CHECK(notif.Handle() == TransitionHandle::Invalid);
}

TEST_CASE("InteractionStateChanged notification") {
    InteractionStateChanged notif(InteractionState::Normal, InteractionState::Hovered);
    CHECK(notif.ClassName() == "InteractionStateChanged");
    CHECK(notif.OldState() == InteractionState::Normal);
    CHECK(notif.NewState() == InteractionState::Hovered);
}

} // TEST_SUITE Notifications

// ===========================================================================
// FocusManager
// ===========================================================================

TEST_SUITE("PhaseC::FocusManager") {

TEST_CASE("FocusManager default state") {
    FocusManager mgr;
    CHECK(mgr.FocusedNode() == nullptr);
    CHECK(mgr.PreviousFocusedNode() == nullptr);
    CHECK(mgr.ActiveRegionId().empty());
    CHECK(mgr.Regions().empty());
}

TEST_CASE("NotifyFocusGained tracks focused node") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;
    StubWidgetNode a("a");
    a.SetFocusable(true);

    mgr.NotifyFocusGained(&a);
    CHECK(mgr.FocusedNode() == &a);
    CHECK(mgr.PreviousFocusedNode() == nullptr);
}

TEST_CASE("NotifyFocusGained updates previous") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;
    StubWidgetNode a("a");
    StubWidgetNode b("b");
    a.SetFocusable(true);
    b.SetFocusable(true);

    mgr.NotifyFocusGained(&a);
    mgr.NotifyFocusGained(&b);
    CHECK(mgr.FocusedNode() == &b);
    CHECK(mgr.PreviousFocusedNode() == &a);
}

TEST_CASE("NotifyFocusLost clears focused") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;
    StubWidgetNode a("a");
    a.SetFocusable(true);

    mgr.NotifyFocusGained(&a);
    mgr.NotifyFocusLost(&a);
    CHECK(mgr.FocusedNode() == nullptr);
    CHECK(mgr.PreviousFocusedNode() == &a);
}

TEST_CASE("NotifyFocusLost ignores wrong node") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;
    StubWidgetNode a("a");
    StubWidgetNode b("b");
    a.SetFocusable(true);
    b.SetFocusable(true);

    mgr.NotifyFocusGained(&a);
    mgr.NotifyFocusLost(&b); // b is not focused
    CHECK(mgr.FocusedNode() == &a); // unchanged
}

TEST_CASE("FocusNext / FocusPrevious traverse within scope") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;

    StubWidgetNode root("root");
    root.SetFocusScope(true);
    auto a = std::make_unique<StubWidgetNode>("a");
    auto b = std::make_unique<StubWidgetNode>("b");
    auto c = std::make_unique<StubWidgetNode>("c");
    a->SetFocusable(true);
    b->SetFocusable(true);
    c->SetFocusable(true);
    auto* aPtr = a.get();
    auto* bPtr = b.get();
    auto* cPtr = c.get();
    root.AddNode(std::move(a));
    root.AddNode(std::move(b));
    root.AddNode(std::move(c));

    // Start at a
    mgr.NotifyFocusGained(aPtr);
    auto* next = mgr.FocusNext(aPtr);
    CHECK(next == bPtr);

    next = mgr.FocusNext(bPtr);
    CHECK(next == cPtr);

    // Wrap around
    next = mgr.FocusNext(cPtr);
    CHECK(next == aPtr);

    // Previous
    auto* prev = mgr.FocusPrevious(aPtr);
    CHECK(prev == cPtr);
}

TEST_CASE("RegisterRegion / UnregisterRegion") {
    FocusManager mgr;
    mgr.RegisterRegion({"toolbar", nullptr, {}, 1});
    mgr.RegisterRegion({"viewport", nullptr, {}, 2});
    mgr.RegisterRegion({"property", nullptr, {}, 3});

    CHECK(mgr.Regions().size() == 3);
    CHECK(mgr.Regions()[0].id == "toolbar");
    CHECK(mgr.Regions()[1].id == "viewport");
    CHECK(mgr.Regions()[2].id == "property");

    mgr.UnregisterRegion("viewport");
    CHECK(mgr.Regions().size() == 2);
    CHECK(mgr.Regions()[0].id == "toolbar");
    CHECK(mgr.Regions()[1].id == "property");
}

TEST_CASE("FocusRegionById moves focus to first focusable in region") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;

    StubWidgetNode regionRoot("region");
    auto a = std::make_unique<StubWidgetNode>("a");
    a->SetFocusable(true);
    auto* aPtr = a.get();
    regionRoot.AddNode(std::move(a));

    mgr.RegisterRegion({"test_region", &regionRoot, regionRoot.AliveToken(), 1});
    bool ok = mgr.FocusRegionById("test_region");
    CHECK(ok);
    CHECK(mgr.FocusedNode() == aPtr);
    CHECK(mgr.ActiveRegionId() == "test_region");
}

TEST_CASE("FocusNextRegion cycles through regions") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;

    StubWidgetNode r1("r1");
    auto a1 = std::make_unique<StubWidgetNode>("a1");
    a1->SetFocusable(true);
    r1.AddNode(std::move(a1));

    StubWidgetNode r2("r2");
    auto a2 = std::make_unique<StubWidgetNode>("a2");
    a2->SetFocusable(true);
    r2.AddNode(std::move(a2));

    mgr.RegisterRegion({"region1", &r1, r1.AliveToken(), 1});
    mgr.RegisterRegion({"region2", &r2, r2.AliveToken(), 2});

    auto id1 = mgr.FocusNextRegion();
    CHECK(id1 == "region1");

    auto id2 = mgr.FocusNextRegion();
    CHECK(id2 == "region2");

    // Wrap
    auto id3 = mgr.FocusNextRegion();
    CHECK(id3 == "region1");
}

TEST_CASE("SaveFocusState / RestoreFocusState (legacy wrappers)") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;
    StubWidgetNode a("a");
    StubWidgetNode b("b");
    a.SetFocusable(true);
    b.SetFocusable(true);

    mgr.NotifyFocusGained(&a);
    mgr.SaveFocusState();

    mgr.NotifyFocusGained(&b);
    CHECK(mgr.FocusedNode() == &b);

    mgr.RestoreFocusState();
    CHECK(mgr.FocusedNode() == &a);
}

TEST_CASE("PushFocusState / PopFocusState nested stack") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;
    StubWidgetNode a("a");
    StubWidgetNode b("b");
    StubWidgetNode c("c");
    a.SetFocusable(true);
    b.SetFocusable(true);
    c.SetFocusable(true);

    // Level 0: focus on a
    mgr.NotifyFocusGained(&a);
    mgr.PushFocusState();
    CHECK(mgr.FocusRestoreDepth() == 1);

    // Level 1: open dialog, focus on b
    mgr.NotifyFocusGained(&b);
    mgr.PushFocusState();
    CHECK(mgr.FocusRestoreDepth() == 2);

    // Level 2: open nested dialog, focus on c
    mgr.NotifyFocusGained(&c);
    CHECK(mgr.FocusedNode() == &c);

    // Close nested dialog -> restores to b
    mgr.PopFocusState();
    CHECK(mgr.FocusedNode() == &b);
    CHECK(mgr.FocusRestoreDepth() == 1);

    // Close first dialog -> restores to a
    mgr.PopFocusState();
    CHECK(mgr.FocusedNode() == &a);
    CHECK(mgr.FocusRestoreDepth() == 0);

    // Pop on empty stack is no-op
    mgr.PopFocusState();
    CHECK(mgr.FocusedNode() == &a);
}

TEST_CASE("Region lifetime safety: expired rootToken purges region") {
    matcha::test::QtAppGuard::Ensure();
    FocusManager mgr;

    // Create a region root in a limited scope
    auto regionRoot = std::make_unique<StubWidgetNode>("region");
    auto a = std::make_unique<StubWidgetNode>("a");
    a->SetFocusable(true);
    regionRoot->AddNode(std::move(a));

    auto token = regionRoot->AliveToken();
    mgr.RegisterRegion({"volatile_region", regionRoot.get(), token, 1});
    CHECK(mgr.Regions().size() == 1);

    // Destroy the region root -> token expires
    regionRoot.reset();

    // FocusRegionById should purge the stale region
    bool ok = mgr.FocusRegionById("volatile_region");
    CHECK_FALSE(ok);
    CHECK(mgr.Regions().empty());
}

TEST_CASE("Global accessor SetFocusManager / GetFocusManager / HasFocusManager") {
    CHECK_FALSE(HasFocusManager()); // cleared by previous test or default

    FocusManager mgr;
    SetFocusManager(&mgr);
    CHECK(HasFocusManager());
    CHECK(GetFocusManager() == &mgr);

    SetFocusManager(nullptr);
    CHECK_FALSE(HasFocusManager());
    CHECK(GetFocusManager() == nullptr);
}

} // TEST_SUITE FocusManager

#ifdef __clang__
#pragma clang diagnostic pop
#endif
