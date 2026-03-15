#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <Matcha/DSL/Blueprint.h>
#include <Matcha/Tree/ContainerNode.h>

#include <ostream>
#include <string>

#include "doctest.h"

using namespace matcha::dsl;
using namespace matcha::fw;

// ============================================================================
// Blueprint DSL
// ============================================================================

TEST_SUITE("Blueprint") {

TEST_CASE("N factory creates correct NodeType") {
    auto node = N<"PushButton">("btn1");
    auto bp = node.Build();
    CHECK(bp.type == NodeType::PushButton);
    CHECK(bp.id == "btn1");
    CHECK(bp.properties.empty());
    CHECK(bp.children.empty());
}

TEST_CASE("Prop pipe operator") {
    auto bp = (N<"LineEdit">("input") | Prop("placeholder", "Enter name")).Build();
    CHECK(bp.type == NodeType::LineEdit);
    CHECK(bp.properties.size() == 1);
    CHECK(bp.properties[0].first == "placeholder");
    CHECK(bp.properties[0].second == "Enter name");
}

TEST_CASE("multiple properties via chained pipe") {
    auto bp = (N<"CheckBox">("cb")
        | Prop("text", "Dark Mode")
        | Prop("checked", "true")
    ).Build();
    CHECK(bp.properties.size() == 2);
    CHECK(bp.properties[0].first == "text");
    CHECK(bp.properties[1].first == "checked");
}

TEST_CASE("nested children") {
    auto bp = N<"Container">("root")(
        N<"Label">("title") | Prop("text", "Settings"),
        N<"PushButton">("ok") | Prop("text", "OK")
    );
    CHECK(bp.type == NodeType::Container);
    CHECK(bp.id == "root");
    CHECK(bp.children.size() == 2);
    CHECK(bp.children[0].type == NodeType::Label);
    CHECK(bp.children[0].id == "title");
    CHECK(bp.children[0].properties.size() == 1);
    CHECK(bp.children[1].type == NodeType::PushButton);
    CHECK(bp.children[1].id == "ok");
}

TEST_CASE("deeply nested tree") {
    auto bp = N<"Container">("outer")(
        N<"Container">("inner")(
            N<"LineEdit">("input")
        )
    );
    CHECK(bp.children.size() == 1);
    CHECK(bp.children[0].type == NodeType::Container);
    CHECK(bp.children[0].children.size() == 1);
    CHECK(bp.children[0].children[0].type == NodeType::LineEdit);
}

TEST_CASE("empty id is valid") {
    auto bp = N<"Label">().Build();
    CHECK(bp.type == NodeType::Label);
    CHECK(bp.id.empty());
}

} // TEST_SUITE
