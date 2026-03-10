#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "Matcha/UiNodes/Core/UiNode.h"
#include "Matcha/UiNodes/Core/GridConstants.h"

#include <ostream>
#include <string>

using matcha::fw::NodeType;
using matcha::fw::UiNode;

// Concrete test subclass (UiNode is abstract due to MetaClass, but we can
// instantiate it directly since it has no pure virtual besides MetaClass
// overrides which are provided by MATCHA_DECLARE_CLASS).
// Actually UiNode uses MATCHA_DECLARE_CLASS so it IS concrete for MetaClass.
// We just need a simple subclass for testing tree ops.

namespace {

class TestNode : public UiNode {
public:
    explicit TestNode(std::string id, NodeType type = NodeType::Custom,
                      std::string name = "")
        : UiNode(std::move(id), type, std::move(name))
    {
    }
};

} // namespace

// -- NodeType enum tests --

TEST_CASE("NodeType: _Count is last and > 0") {
    CHECK(static_cast<int>(NodeType::_Count) > 0);
    CHECK(static_cast<int>(NodeType::Shell) == 0);
}

TEST_CASE("NodeType: key entries exist") {
    CHECK(static_cast<int>(NodeType::ActionBar) > 0);
    CHECK(static_cast<int>(NodeType::Dialog) > 0);
    CHECK(static_cast<int>(NodeType::StatusBar) > 0);
    CHECK(static_cast<int>(NodeType::ContextMenu) > 0);
    CHECK(static_cast<int>(NodeType::Viewport) > 0);
    CHECK(static_cast<int>(NodeType::WidgetWrapper) > 0);
    CHECK(static_cast<int>(NodeType::Custom) > 0);
}

TEST_CASE("NodeType: Scheme D widget node entries exist") {
    CHECK(static_cast<int>(NodeType::LineEdit) > 0);
    CHECK(static_cast<int>(NodeType::ComboBox) > 0);
    CHECK(static_cast<int>(NodeType::SpinBox) > 0);
    CHECK(static_cast<int>(NodeType::CheckBox) > 0);
    CHECK(static_cast<int>(NodeType::ToggleSwitch) > 0);
    CHECK(static_cast<int>(NodeType::Label) > 0);
    CHECK(static_cast<int>(NodeType::ProgressBar) > 0);
    CHECK(static_cast<int>(NodeType::Slider) > 0);
    CHECK(static_cast<int>(NodeType::ColorPicker) > 0);
    CHECK(static_cast<int>(NodeType::DataTable) > 0);
    CHECK(static_cast<int>(NodeType::PropertyGrid) > 0);
    CHECK(static_cast<int>(NodeType::SearchBox) > 0);
}

// -- UiNode identity tests --

TEST_CASE("UiNode: identity fields") {
    TestNode node("myNode", NodeType::Custom);
    CHECK(node.Id() == "myNode");
    CHECK(node.Type() == NodeType::Custom);
    CHECK(node.Name() == "myNode");  // name defaults to id

    node.SetName("Renamed");
    CHECK(node.Name() == "Renamed");
}

TEST_CASE("UiNode: explicit name") {
    TestNode node("id1", NodeType::Shell);
    CHECK(node.Name() == "id1");

    TestNode node2("id2", NodeType::ActionBar, "My ActionBar");
    CHECK(node2.Id() == "id2");
    CHECK(node2.Name() == "My ActionBar");
}

TEST_CASE("UiNode: Widget() returns nullptr by default") {
    TestNode node("w1");
    CHECK(node.Widget() == nullptr);
}

// -- Tree structure tests --

TEST_CASE("UiNode: AddNode and NodeCount") {
    TestNode root("root", NodeType::Shell);
    CHECK(root.NodeCount() == 0);

    auto child = std::make_unique<TestNode>("child1");
    auto* ptr = root.AddNode(std::move(child));
    CHECK(ptr != nullptr);
    CHECK(root.NodeCount() == 1);
    CHECK(ptr->ParentNode() == &root);
    CHECK(ptr->Id() == "child1");
}

TEST_CASE("UiNode: AddNode nullptr returns nullptr") {
    TestNode root("root", NodeType::Shell);
    auto* result = root.AddNode(nullptr);
    CHECK(result == nullptr);
    CHECK(root.NodeCount() == 0);
}

TEST_CASE("UiNode: RemoveNode") {
    TestNode root("root", NodeType::Shell);
    auto* c1 = root.AddNode(std::make_unique<TestNode>("c1"));
    root.AddNode(std::make_unique<TestNode>("c2"));
    CHECK(root.NodeCount() == 2);

    auto removed = root.RemoveNode(c1);
    CHECK(removed != nullptr);
    CHECK(removed->Id() == "c1");
    CHECK(removed->ParentNode() == nullptr);
    CHECK(root.NodeCount() == 1);
}

TEST_CASE("UiNode: RemoveNode not found returns nullptr") {
    TestNode root("root", NodeType::Shell);
    TestNode other("other");
    auto result = root.RemoveNode(&other);
    CHECK(result == nullptr);
}

TEST_CASE("UiNode: RemoveNode nullptr returns nullptr") {
    TestNode root("root", NodeType::Shell);
    auto result = root.RemoveNode(nullptr);
    CHECK(result == nullptr);
}

TEST_CASE("UiNode: FindById") {
    TestNode root("root", NodeType::Shell);
    root.AddNode(std::make_unique<TestNode>("a"));
    auto* b = root.AddNode(std::make_unique<TestNode>("b"));
    b->AddNode(std::make_unique<TestNode>("b1"));

    CHECK(root.FindById("a") != nullptr);
    CHECK(root.FindById("b") != nullptr);
    CHECK(root.FindById("b1") != nullptr);
    CHECK(root.FindById("notexist") == nullptr);
}

TEST_CASE("UiNode: FindByName") {
    TestNode root("root", NodeType::Shell);
    root.AddNode(std::make_unique<TestNode>("x", NodeType::Custom, "Alpha"));
    root.AddNode(std::make_unique<TestNode>("y", NodeType::Custom, "Beta"));

    CHECK(root.FindByName("Alpha") != nullptr);
    CHECK(root.FindByName("Alpha")->Id() == "x");
    CHECK(root.FindByName("Beta") != nullptr);
    CHECK(root.FindByName("Gamma") == nullptr);
}

TEST_CASE("UiNode: NodeAt and NodeCount") {
    TestNode root("root", NodeType::Shell);
    root.AddNode(std::make_unique<TestNode>("a"));
    root.AddNode(std::make_unique<TestNode>("b"));

    CHECK(root.NodeCount() == 2);
    CHECK(root.NodeAt(0)->Id() == "a");
    CHECK(root.NodeAt(1)->Id() == "b");
}

// -- MetaClass RTTI tests --

TEST_CASE("UiNode: MetaClass RTTI") {
    TestNode node("m1");
    CHECK(node.IsAKindOf("UiNode"));
    CHECK(node.IsAKindOf("CommandNode"));
    CHECK(node.IsAKindOf("EventNode"));
    CHECK(node.IsAKindOf("BaseObject"));
}

TEST_CASE("UiNode: ClassName") {
    TestNode node("m2");
    // TestNode doesn't have MATCHA_DECLARE_CLASS, so ClassName falls through to UiNode
    // Actually TestNode inherits UiNode's MetaClass. Let's just check it's not empty.
    CHECK(!node.ClassName().empty());
}

// -- Traversal tests --

TEST_CASE("UiNode: Descendants generator") {
    TestNode root("root", NodeType::Shell);
    auto* a = root.AddNode(std::make_unique<TestNode>("a", NodeType::ActionBar));
    a->AddNode(std::make_unique<TestNode>("a1", NodeType::ActionTab));
    a->AddNode(std::make_unique<TestNode>("a2", NodeType::ActionToolbar));
    root.AddNode(std::make_unique<TestNode>("b", NodeType::Dialog));

    std::vector<std::string> ids;
    for (auto* node : root.Descendants()) {
        ids.emplace_back(node->Id());
    }
    CHECK(ids.size() == 4);
    CHECK(ids[0] == "a");
    CHECK(ids[1] == "a1");
    CHECK(ids[2] == "a2");
    CHECK(ids[3] == "b");
}

TEST_CASE("UiNode: DescendantsOfType") {
    TestNode root("root", NodeType::Shell);
    auto* a = root.AddNode(std::make_unique<TestNode>("a", NodeType::ActionBar));
    a->AddNode(std::make_unique<TestNode>("a1", NodeType::ActionTab));
    a->AddNode(std::make_unique<TestNode>("a2", NodeType::ActionTab));
    root.AddNode(std::make_unique<TestNode>("b", NodeType::Dialog));

    std::vector<std::string> ids;
    for (auto* node : root.DescendantsOfType(NodeType::ActionTab)) {
        ids.emplace_back(node->Id());
    }
    CHECK(ids.size() == 2);
    CHECK(ids[0] == "a1");
    CHECK(ids[1] == "a2");
}

TEST_CASE("UiNode: ChildrenOfType") {
    TestNode root("root", NodeType::Shell);
    root.AddNode(std::make_unique<TestNode>("a", NodeType::ActionBar));
    root.AddNode(std::make_unique<TestNode>("b", NodeType::StatusBar));
    root.AddNode(std::make_unique<TestNode>("c", NodeType::StatusBar));

    std::vector<std::string> ids;
    for (auto* node : root.ChildrenOfType(NodeType::StatusBar)) {
        ids.emplace_back(node->Id());
    }
    CHECK(ids.size() == 2);
    CHECK(ids[0] == "b");
    CHECK(ids[1] == "c");
}

TEST_CASE("UiNode: TraverseDepthFirst pre-order") {
    TestNode root("root", NodeType::Shell);
    auto* a = root.AddNode(std::make_unique<TestNode>("a"));
    a->AddNode(std::make_unique<TestNode>("a1"));
    root.AddNode(std::make_unique<TestNode>("b"));

    std::vector<std::string> ids;
    root.TraverseDepthFirst([&](UiNode* n) {
        ids.emplace_back(n->Id());
    });
    CHECK(ids.size() == 4);
    CHECK(ids[0] == "root");
    CHECK(ids[1] == "a");
    CHECK(ids[2] == "a1");
    CHECK(ids[3] == "b");
}

TEST_CASE("UiNode: ForEachNode") {
    TestNode root("root", NodeType::Shell);
    root.AddNode(std::make_unique<TestNode>("x"));
    root.AddNode(std::make_unique<TestNode>("y"));

    int count = 0;
    root.ForEachNode([&](UiNode*) { ++count; });
    CHECK(count == 3);
}

TEST_CASE("UiNode: Descendants on leaf returns empty") {
    TestNode leaf("leaf");
    int count = 0;
    for ([[maybe_unused]] auto* _ : leaf.Descendants()) {
        ++count;
    }
    CHECK(count == 0);
}

// -- GridConstants tests --

using matcha::fw::GridConstants;

TEST_CASE("GridConstants: basic constants") {
    CHECK(GridConstants::kColumnCount == 24);
    CHECK(GridConstants::kColumnWidth == 72);
    CHECK(GridConstants::kGutterWidth == 8);
    CHECK(GridConstants::kMargin == 4);
}

TEST_CASE("GridConstants: total width") {
    constexpr int expected = (2 * 4) + (24 * 72) + (23 * 8);
    static_assert(GridConstants::kTotalWidth == expected);
    CHECK(GridConstants::kTotalWidth == expected);
}

TEST_CASE("GridConstants: ColumnSpan") {
    CHECK(GridConstants::ColumnSpan(0) == 0);
    CHECK(GridConstants::ColumnSpan(1) == 72);
    CHECK(GridConstants::ColumnSpan(2) == 72 * 2 + 8);
    CHECK(GridConstants::ColumnSpan(24) == GridConstants::kTotalWidth - 2 * GridConstants::kMargin);
    CHECK(GridConstants::ColumnSpan(100) == GridConstants::ColumnSpan(24));
    static_assert(GridConstants::ColumnSpan(3) == 3 * 72 + 2 * 8);
}

TEST_CASE("GridConstants: EqualDivision") {
    CHECK(GridConstants::EqualDivision(1000, 0) == 0);
    CHECK(GridConstants::EqualDivision(1000, 2) == (1000 - 8) / 2);
    CHECK(GridConstants::EqualDivision(1000, 4) == (1000 - 3 * 8) / 4);
}

TEST_CASE("GridConstants: VerticalSpacing") {
    CHECK(GridConstants::VerticalSpacing(-1) == 4);
    CHECK(GridConstants::VerticalSpacing(0) == 4);
    CHECK(GridConstants::VerticalSpacing(1) == 8);
    CHECK(GridConstants::VerticalSpacing(2) == 12);
    CHECK(GridConstants::VerticalSpacing(3) == 16);
    CHECK(GridConstants::VerticalSpacing(99) == 16);
}

