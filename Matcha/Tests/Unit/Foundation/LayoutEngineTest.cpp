#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Foundation/LayoutEngine.h>

#include <array>

using namespace matcha::fw;

TEST_SUITE("fw::LayoutEngine") {

// ============================================================================
// ComputeFlex — basic
// ============================================================================

TEST_CASE("Flex: empty children returns empty") {
    FlexLayoutParams params{.containerMainExtent = 400, .containerCrossExtent = 100};
    auto r = LayoutEngine::ComputeFlex(params, {});
    CHECK(r.empty());
}

TEST_CASE("Flex: all fixed children, Start alignment") {
    FlexLayoutParams params{.containerMainExtent = 400, .containerCrossExtent = 100};

    std::array children = {
        ChildConstraint{.preferredSize = 100},
        ChildConstraint{.preferredSize = 80},
        ChildConstraint{.preferredSize = 60},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 3);
    CHECK(r[0].mainExtent == 100);
    CHECK(r[1].mainExtent == 80);
    CHECK(r[2].mainExtent == 60);
    CHECK(r[0].mainOffset == 0);
    CHECK(r[1].mainOffset == 100);
    CHECK(r[2].mainOffset == 180);
}

TEST_CASE("Flex: fixed children with gap") {
    FlexLayoutParams params{
        .containerMainExtent = 400,
        .containerCrossExtent = 100,
        .gap = 10,
    };

    std::array children = {
        ChildConstraint{.preferredSize = 100},
        ChildConstraint{.preferredSize = 100},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 2);
    CHECK(r[0].mainOffset == 0);
    CHECK(r[0].mainExtent == 100);
    CHECK(r[1].mainOffset == 110);  // 100 + 10 gap
    CHECK(r[1].mainExtent == 100);
}

TEST_CASE("Flex: single flex child fills remaining") {
    FlexLayoutParams params{.containerMainExtent = 400, .containerCrossExtent = 100};

    std::array children = {
        ChildConstraint{.preferredSize = 100},
        ChildConstraint{.flex = 1},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 2);
    CHECK(r[0].mainExtent == 100);
    CHECK(r[1].mainExtent == 300);
}

TEST_CASE("Flex: two flex children split evenly") {
    FlexLayoutParams params{.containerMainExtent = 400, .containerCrossExtent = 100};

    std::array children = {
        ChildConstraint{.flex = 1},
        ChildConstraint{.flex = 1},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 2);
    CHECK(r[0].mainExtent == 200);
    CHECK(r[1].mainExtent == 200);
}

TEST_CASE("Flex: weighted flex children") {
    FlexLayoutParams params{.containerMainExtent = 300, .containerCrossExtent = 100};

    std::array children = {
        ChildConstraint{.flex = 1},
        ChildConstraint{.flex = 2},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 2);
    CHECK(r[0].mainExtent == 100);
    CHECK(r[1].mainExtent == 200);
}

TEST_CASE("Flex: flex child clamped to minSize") {
    FlexLayoutParams params{.containerMainExtent = 200, .containerCrossExtent = 100};

    std::array children = {
        ChildConstraint{.preferredSize = 150},
        ChildConstraint{.minSize = 80, .flex = 1},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 2);
    CHECK(r[1].mainExtent >= 80);
}

TEST_CASE("Flex: flex child clamped to maxSize") {
    FlexLayoutParams params{.containerMainExtent = 600, .containerCrossExtent = 100};

    std::array children = {
        ChildConstraint{.maxSize = 100, .flex = 1},
        ChildConstraint{.flex = 1},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 2);
    CHECK(r[0].mainExtent <= 100);
}

// ============================================================================
// ComputeFlex — alignment
// ============================================================================

TEST_CASE("Flex: Center alignment") {
    FlexLayoutParams params{
        .containerMainExtent = 400,
        .containerCrossExtent = 100,
        .mainAlign = MainAxisAlignment::Center,
    };

    std::array children = {
        ChildConstraint{.preferredSize = 100},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 1);
    CHECK(r[0].mainOffset == 150);  // (400 - 100) / 2
}

TEST_CASE("Flex: End alignment") {
    FlexLayoutParams params{
        .containerMainExtent = 400,
        .containerCrossExtent = 100,
        .mainAlign = MainAxisAlignment::End,
    };

    std::array children = {
        ChildConstraint{.preferredSize = 100},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 1);
    CHECK(r[0].mainOffset == 300);  // 400 - 100
}

TEST_CASE("Flex: SpaceBetween with 3 children") {
    FlexLayoutParams params{
        .containerMainExtent = 300,
        .containerCrossExtent = 100,
        .mainAlign = MainAxisAlignment::SpaceBetween,
    };

    std::array children = {
        ChildConstraint{.preferredSize = 50},
        ChildConstraint{.preferredSize = 50},
        ChildConstraint{.preferredSize = 50},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 3);
    CHECK(r[0].mainOffset == 0);
    // leftover = 150, between 2 gaps: 75 each
    CHECK(r[1].mainOffset == 125);  // 50 + 75
    CHECK(r[2].mainOffset == 250);  // 50 + 75 + 50 + 75
}

// ============================================================================
// ComputeFlex — cross-axis
// ============================================================================

TEST_CASE("Flex: Stretch cross alignment fills container") {
    FlexLayoutParams params{
        .containerMainExtent = 400,
        .containerCrossExtent = 200,
        .crossAlign = CrossAxisAlignment::Stretch,
    };

    std::array children = {
        ChildConstraint{.preferredSize = 100, .crossPreferredSize = 50},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 1);
    CHECK(r[0].crossExtent == 200);
    CHECK(r[0].crossOffset == 0);
}

TEST_CASE("Flex: Center cross alignment") {
    FlexLayoutParams params{
        .containerMainExtent = 400,
        .containerCrossExtent = 200,
        .crossAlign = CrossAxisAlignment::Center,
    };

    std::array children = {
        ChildConstraint{.preferredSize = 100, .crossPreferredSize = 80},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 1);
    CHECK(r[0].crossExtent == 80);
    CHECK(r[0].crossOffset == 60);  // (200 - 80) / 2
}

// ============================================================================
// ComputeFlex — padding
// ============================================================================

TEST_CASE("Flex: padding reduces available space") {
    FlexLayoutParams params{
        .containerMainExtent = 400,
        .containerCrossExtent = 100,
        .paddingStart = 20,
        .paddingEnd = 20,
    };

    std::array children = {
        ChildConstraint{.flex = 1},
    };

    auto r = LayoutEngine::ComputeFlex(params, children);
    REQUIRE(r.size() == 1);
    CHECK(r[0].mainExtent == 360);  // 400 - 20 - 20
    CHECK(r[0].mainOffset == 20);
}

// ============================================================================
// ResolveColumns
// ============================================================================

TEST_CASE("Grid: resolve fixed columns") {
    std::array cols = {
        ColumnDef{.type = ColumnDef::Type::Fixed, .value = 100.0F},
        ColumnDef{.type = ColumnDef::Type::Fixed, .value = 200.0F},
    };

    auto w = LayoutEngine::ResolveColumns(cols, {}, 500, 10);
    REQUIRE(w.size() == 2);
    CHECK(w[0] == 100);
    CHECK(w[1] == 200);
}

TEST_CASE("Grid: resolve fraction columns") {
    std::array cols = {
        ColumnDef{.type = ColumnDef::Type::Fraction, .value = 1.0F},
        ColumnDef{.type = ColumnDef::Type::Fraction, .value = 2.0F},
    };

    auto w = LayoutEngine::ResolveColumns(cols, {}, 310, 10);
    REQUIRE(w.size() == 2);
    // 310 - 10 gap = 300 available; 1fr = 100, 2fr = 200
    CHECK(w[0] == 100);
    CHECK(w[1] == 200);
}

TEST_CASE("Grid: mixed fixed + fraction columns") {
    std::array cols = {
        ColumnDef{.type = ColumnDef::Type::Fixed, .value = 100.0F},
        ColumnDef{.type = ColumnDef::Type::Fraction, .value = 1.0F},
    };

    auto w = LayoutEngine::ResolveColumns(cols, {}, 310, 10);
    REQUIRE(w.size() == 2);
    CHECK(w[0] == 100);
    // 310 - 10 gap - 100 fixed = 200 for 1fr
    CHECK(w[1] == 200);
}

TEST_CASE("Grid: auto columns use measured widths") {
    std::array cols = {
        ColumnDef{.type = ColumnDef::Type::Auto},
        ColumnDef{.type = ColumnDef::Type::Auto},
    };
    std::array autoWidths = {80, 120};

    auto w = LayoutEngine::ResolveColumns(cols, autoWidths, 500, 0);
    REQUIRE(w.size() == 2);
    CHECK(w[0] == 80);
    CHECK(w[1] == 120);
}

// ============================================================================
// ComputeGrid
// ============================================================================

TEST_CASE("Grid: 4 children in 2 columns") {
    GridLayoutParams params{.containerWidth = 400, .containerHeight = 300};
    std::array colWidths = {200, 200};
    std::array<std::pair<int,int>, 4> sizes = {{{200,50}, {200,50}, {200,80}, {200,80}}};

    auto r = LayoutEngine::ComputeGrid(params, colWidths, sizes);
    REQUIRE(r.size() == 4);

    // Row 0: children 0,1
    CHECK(r[0].x == 0);
    CHECK(r[0].y == 0);
    CHECK(r[0].width == 200);
    CHECK(r[0].height == 50);

    CHECK(r[1].x == 200);
    CHECK(r[1].y == 0);

    // Row 1: children 2,3
    CHECK(r[2].x == 0);
    CHECK(r[2].y == 50);
    CHECK(r[2].height == 80);

    CHECK(r[3].x == 200);
    CHECK(r[3].y == 50);
}

TEST_CASE("Grid: with gaps and padding") {
    GridLayoutParams params{
        .containerWidth = 500, .containerHeight = 300,
        .hGap = 10, .vGap = 8,
        .paddingLeft = 5, .paddingTop = 5,
    };
    std::array colWidths = {100, 100};
    std::array<std::pair<int,int>, 4> sizes = {{{100,40}, {100,40}, {100,60}, {100,60}}};

    auto r = LayoutEngine::ComputeGrid(params, colWidths, sizes);
    REQUIRE(r.size() == 4);

    CHECK(r[0].x == 5);       // paddingLeft
    CHECK(r[0].y == 5);       // paddingTop
    CHECK(r[1].x == 115);     // 5 + 100 + 10
    CHECK(r[2].y == 53);      // 5 + 40 + 8
}

// ============================================================================
// ComputeWrapFlex (Fix #9)
// ============================================================================

TEST_CASE("WrapFlex: single row when all fit") {
    FlexLayoutParams params {
        .containerMainExtent = 300, .containerCrossExtent = 100,
        .gap = 10, .paddingStart = 0, .paddingEnd = 0,
        .crossPaddingStart = 0, .crossPaddingEnd = 0,
    };
    std::array children = {
        ChildConstraint{.preferredSize = 80},
        ChildConstraint{.preferredSize = 80},
        ChildConstraint{.preferredSize = 80},
    };
    auto r = LayoutEngine::ComputeWrapFlex(params, children);
    REQUIRE(r.size() == 3);
    // All on same cross row
    CHECK(r[0].crossOffset == r[1].crossOffset);
    CHECK(r[1].crossOffset == r[2].crossOffset);
}

TEST_CASE("WrapFlex: wraps to second row") {
    FlexLayoutParams params {
        .containerMainExtent = 200, .containerCrossExtent = 200,
        .gap = 10, .paddingStart = 0, .paddingEnd = 0,
        .crossPaddingStart = 0, .crossPaddingEnd = 0,
    };
    std::array children = {
        ChildConstraint{.preferredSize = 80, .crossPreferredSize = 30},
        ChildConstraint{.preferredSize = 80, .crossPreferredSize = 30},
        ChildConstraint{.preferredSize = 80, .crossPreferredSize = 30}, // 80+10+80+10+80 = 260 > 200
    };
    auto r = LayoutEngine::ComputeWrapFlex(params, children);
    REQUIRE(r.size() == 3);
    // First two on row 0, third on row 1
    CHECK(r[0].crossOffset == r[1].crossOffset);
    CHECK(r[2].crossOffset > r[0].crossOffset);
}

TEST_CASE("WrapFlex: empty input") {
    FlexLayoutParams params{.containerMainExtent = 100, .containerCrossExtent = 100};
    auto r = LayoutEngine::ComputeWrapFlex(params, {});
    CHECK(r.empty());
}

// ============================================================================
// Grid cellAlign (Fix #2)
// ============================================================================

TEST_CASE("Grid cellAlign::Start — child smaller than cell") {
    GridLayoutParams params {
        .hGap = 0, .vGap = 0,
        .paddingLeft = 0, .paddingTop = 0,
        .cellAlign = CrossAxisAlignment::Start,
    };
    std::array colWidths = {200};
    std::array<std::pair<int,int>, 1> sizes = {{{80, 40}}}; // child 80x40 in 200-wide cell
    auto r = LayoutEngine::ComputeGrid(params, colWidths, sizes);
    REQUIRE(r.size() == 1);
    CHECK(r[0].x == 0);
    CHECK(r[0].y == 0);
    CHECK(r[0].width == 80);
    CHECK(r[0].height == 40);
}

TEST_CASE("Grid cellAlign::Center — child centered in cell") {
    GridLayoutParams params {
        .hGap = 0, .vGap = 0,
        .paddingLeft = 0, .paddingTop = 0,
        .cellAlign = CrossAxisAlignment::Center,
    };
    std::array colWidths = {200};
    std::array<std::pair<int,int>, 1> sizes = {{{80, 40}}};
    auto r = LayoutEngine::ComputeGrid(params, colWidths, sizes);
    REQUIRE(r.size() == 1);
    CHECK(r[0].x == 60);  // (200-80)/2
    CHECK(r[0].y == 0);   // (40-40)/2
    CHECK(r[0].width == 80);
    CHECK(r[0].height == 40);
}

TEST_CASE("Grid cellAlign::End — child aligned to end") {
    GridLayoutParams params {
        .hGap = 0, .vGap = 0,
        .paddingLeft = 0, .paddingTop = 0,
        .cellAlign = CrossAxisAlignment::End,
    };
    std::array colWidths = {200};
    std::array<std::pair<int,int>, 1> sizes = {{{80, 40}}};
    auto r = LayoutEngine::ComputeGrid(params, colWidths, sizes);
    REQUIRE(r.size() == 1);
    CHECK(r[0].x == 120);  // 200-80
    CHECK(r[0].y == 0);    // 40-40
    CHECK(r[0].width == 80);
    CHECK(r[0].height == 40);
}

TEST_CASE("Grid cellAlign::Stretch — default, fills cell") {
    GridLayoutParams params {
        .hGap = 0, .vGap = 0,
        .paddingLeft = 0, .paddingTop = 0,
        .cellAlign = CrossAxisAlignment::Stretch,
    };
    std::array colWidths = {200};
    std::array<std::pair<int,int>, 1> sizes = {{{80, 40}}};
    auto r = LayoutEngine::ComputeGrid(params, colWidths, sizes);
    REQUIRE(r.size() == 1);
    CHECK(r[0].x == 0);
    CHECK(r[0].y == 0);
    CHECK(r[0].width == 200);
    CHECK(r[0].height == 40);
}

} // TEST_SUITE
