#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/Tree/PopupNode.h"
#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/WidgetNotification.h"
#include "Matcha/Tree/Composition/Menu/PopConfirmNode.h"
#include "Matcha/Interaction/PopupPositioner.h"

#include <ostream>
#include <string>

using matcha::fw::NodeType;
using matcha::fw::PopupBehavior;
using matcha::fw::PopupNode;
using matcha::fw::PopupPlacement;
using matcha::fw::PopConfirmNode;
using matcha::fw::UiNode;

// ============================================================================
// Concrete test subclass — minimal PopupNode for unit testing
// ============================================================================

namespace {

class TestPopup : public PopupNode {
public:
    explicit TestPopup(std::string id,
                       PopupBehavior behavior = PopupBehavior::Dropdown)
        : PopupNode(std::move(id), NodeType::Popup, behavior)
    {
    }

    int openedCount  = 0;
    int closedCount  = 0;
    bool contentCreated = false;

protected:
    auto CreatePopupContent(QWidget* /*parent*/) -> QWidget* override
    {
        contentCreated = true;
        // Return nullptr for headless testing — no actual widget created.
        return nullptr;
    }

    auto PreferredSize() -> matcha::fw::Size override
    {
        return {200, 100};
    }

    void OnOpened() override { ++openedCount; }
    void OnClosed() override { ++closedCount; }
};

class TestAnchor : public UiNode {
public:
    explicit TestAnchor(std::string id)
        : UiNode(std::move(id), NodeType::Custom) {}
};

} // namespace

// ============================================================================
// PopupNode — Construction
// ============================================================================

TEST_SUITE("PopupNode") {

TEST_CASE("Construction sets correct type and behavior") {
    TestPopup p("test-dropdown", PopupBehavior::Dropdown);
    CHECK(p.Type() == NodeType::Popup);
    CHECK(p.Behavior() == PopupBehavior::Dropdown);
    CHECK_FALSE(p.IsOpen());
    CHECK(p.Anchor() == nullptr);
}

TEST_CASE("Construction with Tooltip behavior") {
    TestPopup p("test-tooltip", PopupBehavior::Tooltip);
    CHECK(p.Behavior() == PopupBehavior::Tooltip);
}

TEST_CASE("Construction with Floating behavior") {
    TestPopup p("test-floating", PopupBehavior::Floating);
    CHECK(p.Behavior() == PopupBehavior::Floating);
}

// ============================================================================
// PopupNode — Tree integration
// ============================================================================

TEST_CASE("PopupNode can be added as child of UiNode") {
    TestAnchor parent("parent");
    auto popup = std::make_unique<TestPopup>("popup");
    auto* raw = popup.get();
    parent.AddNode(std::move(popup));

    CHECK(parent.NodeCount() == 1);
    CHECK(parent.NodeAt(0) == raw);
    CHECK(raw->ParentNode() == &parent);
}

TEST_CASE("PopupNode can be found by FindById") {
    TestAnchor root("root");
    auto popup = std::make_unique<TestPopup>("my-popup");
    root.AddNode(std::move(popup));

    auto* found = root.FindById("my-popup");
    CHECK(found != nullptr);
    CHECK(found->Type() == NodeType::Popup);
}

TEST_CASE("PopupNode participates in Descendants traversal") {
    TestAnchor root("root");
    root.AddNode(std::make_unique<TestPopup>("p1"));
    root.AddNode(std::make_unique<TestPopup>("p2"));

    int count = 0;
    for ([[maybe_unused]] auto* n : root.Descendants()) {
        ++count;
    }
    CHECK(count == 2);
}

TEST_CASE("PopupNode participates in DescendantsOfType") {
    TestAnchor root("root");
    root.AddNode(std::make_unique<TestPopup>("p1"));
    root.AddNode(std::make_unique<TestAnchor>("other"));

    int popupCount = 0;
    for ([[maybe_unused]] auto* n : root.DescendantsOfType(NodeType::Popup)) {
        ++popupCount;
    }
    CHECK(popupCount == 1);
}

// ============================================================================
// PopupNode — Configuration
// ============================================================================

TEST_CASE("SetCloseOnEscape / SetAutoPosition / SetOffset / SetMinHeight") {
    TestPopup p("p");
    p.SetCloseOnEscape(false);
    p.SetAutoPosition(false);
    p.SetOffset({10, 20});
    p.SetMinHeight(80);
    // No crash, no observable getters needed for these config values.
    // Behavior is tested via integration tests with actual Qt widgets.
    CHECK(p.Behavior() == PopupBehavior::Dropdown);
}

// ============================================================================
// PopupNode — OpenAtPoint / Close state machine
// ============================================================================

TEST_CASE("OpenAtPoint sets state to open") {
    TestPopup p("p");
    p.OpenAtPoint({100, 200});
    CHECK(p.IsOpen());
    CHECK(p.openedCount == 1);
    // contentCreated depends on QApplication availability; not checked in headless tests.
    CHECK(p.Anchor() == nullptr);
}

TEST_CASE("Close after OpenAtPoint sets state to closed") {
    TestPopup p("p");
    p.OpenAtPoint({100, 200});
    CHECK(p.IsOpen());

    p.Close();
    CHECK_FALSE(p.IsOpen());
    CHECK(p.closedCount == 1);
}

TEST_CASE("Close when already closed is no-op") {
    TestPopup p("p");
    CHECK_FALSE(p.IsOpen());
    p.Close();
    CHECK_FALSE(p.IsOpen());
    CHECK(p.closedCount == 0);
}

TEST_CASE("Open while already open closes first then reopens") {
    TestPopup p("p");
    p.OpenAtPoint({100, 200});
    CHECK(p.openedCount == 1);

    p.OpenAtPoint({300, 400});
    // Should have closed then opened
    CHECK(p.openedCount == 2);
    CHECK(p.closedCount == 1);
    CHECK(p.IsOpen());
}

// ============================================================================
// PopupNode — Open with anchor
// ============================================================================

TEST_CASE("Open with anchor stores the anchor pointer") {
    TestAnchor anchor("anchor");
    TestPopup popup("popup");

    popup.Open(&anchor, PopupPlacement::BottomStart);
    CHECK(popup.IsOpen());
    CHECK(popup.Anchor() == &anchor);
}

// ============================================================================
// PopupNode — Notification emission
// ============================================================================

TEST_CASE("PopupOpened notification is sent on Open") {
    TestAnchor root("root");
    auto popup = std::make_unique<TestPopup>("popup");
    auto* p = popup.get();
    root.AddNode(std::move(popup));

    bool received = false;
    root.Subscribe(nullptr, "PopupOpened",
        [&](matcha::EventNode& /*sender*/, matcha::Notification& /*notif*/) {
            received = true;
        });

    p->OpenAtPoint({0, 0});
    CHECK(received);
}

TEST_CASE("PopupClosed notification is sent on Close") {
    TestAnchor root("root");
    auto popup = std::make_unique<TestPopup>("popup");
    auto* p = popup.get();
    root.AddNode(std::move(popup));

    bool received = false;
    root.Subscribe(nullptr, "PopupClosed",
        [&](matcha::EventNode& /*sender*/, matcha::Notification& /*notif*/) {
            received = true;
        });

    p->OpenAtPoint({0, 0});
    p->Close();
    CHECK(received);
}

// ============================================================================
// PopupNode — NodeType lookup
// ============================================================================

TEST_CASE("NodeType Popup has correct name in lookup table") {
    CHECK(matcha::fw::NodeTypeName(NodeType::Popup) == "Popup");
}

TEST_CASE("ParseNodeType Popup works") {
    CHECK(matcha::fw::ParseNodeType("Popup") == NodeType::Popup);
}

// ============================================================================
// PopConfirmNode — Construction
// ============================================================================

TEST_CASE("PopConfirmNode construction") {
    PopConfirmNode node("confirm1");
    CHECK(node.Type() == NodeType::Popup);
    CHECK(node.Behavior() == PopupBehavior::Dropdown);
    CHECK_FALSE(node.IsOpen());
}

TEST_CASE("PopConfirmNode can be added as child") {
    TestAnchor parent("parent");
    auto node = std::make_unique<PopConfirmNode>("confirm");
    auto* raw = node.get();
    parent.AddNode(std::move(node));

    CHECK(parent.FindById("confirm") == raw);
}

// ============================================================================
// PopupBehavior — all enum values are distinct
// ============================================================================

TEST_CASE("PopupBehavior enum values") {
    CHECK(PopupBehavior::Dropdown != PopupBehavior::Tooltip);
    CHECK(PopupBehavior::Tooltip != PopupBehavior::Floating);
    CHECK(PopupBehavior::Dropdown != PopupBehavior::Floating);
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
