#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/UiNodes/Core/SelectionModel.h>

#include <vector>

using namespace matcha::fw;

TEST_SUITE("fw::SelectionModel") {

// ============================================================================
// Construction & Defaults
// ============================================================================

TEST_CASE("Default construction") {
    SelectionModel sel;
    CHECK(sel.Mode() == SelectionMode::Single);
    CHECK(sel.ItemCount() == 0);
    CHECK(sel.IsEmpty());
    CHECK(sel.Anchor() == -1);
    CHECK(sel.Focus() == -1);
    CHECK(sel.SelectionCount() == 0);
}

TEST_CASE("Construction with mode") {
    SelectionModel sel(SelectionMode::Multi);
    CHECK(sel.Mode() == SelectionMode::Multi);
}

// ============================================================================
// Single Mode
// ============================================================================

TEST_CASE("Single mode: plain click selects one item") {
    SelectionModel sel(SelectionMode::Single);
    sel.SetItemCount(10);

    sel.Click(3);
    CHECK(sel.SelectionCount() == 1);
    CHECK(sel.IsSelected(3));
    CHECK(sel.Anchor() == 3);
    CHECK(sel.Focus() == 3);
}

TEST_CASE("Single mode: click replaces previous selection") {
    SelectionModel sel(SelectionMode::Single);
    sel.SetItemCount(10);

    sel.Click(3);
    sel.Click(7);
    CHECK(sel.SelectionCount() == 1);
    CHECK_FALSE(sel.IsSelected(3));
    CHECK(sel.IsSelected(7));
}

TEST_CASE("Single mode: Ctrl+Click still selects single") {
    SelectionModel sel(SelectionMode::Single);
    sel.SetItemCount(10);

    sel.Click(3);
    sel.Click(5, ClickModifier::Ctrl);
    CHECK(sel.SelectionCount() == 1);
    CHECK(sel.IsSelected(5));
}

TEST_CASE("Single mode: Shift+Click still selects single") {
    SelectionModel sel(SelectionMode::Single);
    sel.SetItemCount(10);

    sel.Click(3);
    sel.Click(7, ClickModifier::Shift);
    CHECK(sel.SelectionCount() == 1);
    CHECK(sel.IsSelected(7));
}

// ============================================================================
// Multi Mode
// ============================================================================

TEST_CASE("Multi mode: Ctrl+Click toggles items") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);

    sel.Click(2);
    sel.Click(5, ClickModifier::Ctrl);
    CHECK(sel.SelectionCount() == 2);
    CHECK(sel.IsSelected(2));
    CHECK(sel.IsSelected(5));

    // Toggle off item 2
    sel.Click(2, ClickModifier::Ctrl);
    CHECK(sel.SelectionCount() == 1);
    CHECK_FALSE(sel.IsSelected(2));
    CHECK(sel.IsSelected(5));
}

TEST_CASE("Multi mode: plain click clears + selects") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);

    sel.Click(2);
    sel.Click(5, ClickModifier::Ctrl);
    sel.Click(3, ClickModifier::Ctrl);
    CHECK(sel.SelectionCount() == 3);

    // Plain click clears all, selects only item 7
    sel.Click(7);
    CHECK(sel.SelectionCount() == 1);
    CHECK(sel.IsSelected(7));
}

TEST_CASE("Multi mode: Shift+Click creates range") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);

    sel.Click(3);          // anchor=3
    sel.Click(7, ClickModifier::Shift);  // range [3,7]
    CHECK(sel.SelectionCount() == 5);
    for (int i = 3; i <= 7; ++i) {
        CHECK(sel.IsSelected(i));
    }
    CHECK(sel.Anchor() == 3);
    CHECK(sel.Focus() == 7);
}

TEST_CASE("Multi mode: Shift+Click reverse range") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);

    sel.Click(7);          // anchor=7
    sel.Click(3, ClickModifier::Shift);  // range [3,7]
    CHECK(sel.SelectionCount() == 5);
    for (int i = 3; i <= 7; ++i) {
        CHECK(sel.IsSelected(i));
    }
    CHECK(sel.Anchor() == 7);
    CHECK(sel.Focus() == 3);
}

// ============================================================================
// SelectAll / Clear
// ============================================================================

TEST_CASE("SelectAll in Multi mode") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(5);

    sel.SelectAll();
    CHECK(sel.SelectionCount() == 5);
    CHECK(sel.Anchor() == 0);
    CHECK(sel.Focus() == 4);
    for (int i = 0; i < 5; ++i) {
        CHECK(sel.IsSelected(i));
    }
}

TEST_CASE("SelectAll is no-op in Single mode") {
    SelectionModel sel(SelectionMode::Single);
    sel.SetItemCount(5);
    sel.Click(2);

    sel.SelectAll();
    CHECK(sel.SelectionCount() == 1);
}

TEST_CASE("Clear resets everything") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);
    sel.Click(3);
    sel.Click(5, ClickModifier::Ctrl);

    sel.Clear();
    CHECK(sel.IsEmpty());
    CHECK(sel.Anchor() == -1);
    CHECK(sel.Focus() == -1);
}

// ============================================================================
// MoveFocus
// ============================================================================

TEST_CASE("MoveFocus without extend selects single") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);
    sel.Click(5);

    sel.MoveFocus(1);  // down
    CHECK(sel.SelectionCount() == 1);
    CHECK(sel.IsSelected(6));
    CHECK(sel.Focus() == 6);
}

TEST_CASE("MoveFocus with extend creates range") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);
    sel.Click(5);

    sel.MoveFocus(1, true);  // Shift+Down
    sel.MoveFocus(1, true);  // Shift+Down again
    CHECK(sel.SelectionCount() == 3);
    CHECK(sel.IsSelected(5));
    CHECK(sel.IsSelected(6));
    CHECK(sel.IsSelected(7));
    CHECK(sel.Anchor() == 5);
    CHECK(sel.Focus() == 7);
}

TEST_CASE("MoveFocus clamps at boundaries") {
    SelectionModel sel(SelectionMode::Single);
    sel.SetItemCount(5);
    sel.Click(4);

    sel.MoveFocus(1);  // try to go past end
    CHECK(sel.Focus() == 4);  // unchanged

    sel.Click(0);
    sel.MoveFocus(-1);  // try to go before start
    CHECK(sel.Focus() == 0);  // unchanged
}

// ============================================================================
// Mode Change
// ============================================================================

TEST_CASE("SetMode to None clears selection") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);
    sel.Click(3);
    sel.Click(5, ClickModifier::Ctrl);

    sel.SetMode(SelectionMode::None);
    CHECK(sel.IsEmpty());
}

TEST_CASE("SetMode to Single trims to focus") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);
    sel.Click(3);
    sel.Click(5, ClickModifier::Ctrl);
    sel.Click(7, ClickModifier::Ctrl);

    sel.SetMode(SelectionMode::Single);
    CHECK(sel.SelectionCount() == 1);
    CHECK(sel.IsSelected(7));  // focus was 7
}

// ============================================================================
// SetItemCount clears
// ============================================================================

TEST_CASE("SetItemCount clears selection") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);
    sel.Click(3);
    sel.Click(5, ClickModifier::Ctrl);

    sel.SetItemCount(20);
    CHECK(sel.IsEmpty());
}

// ============================================================================
// Callback
// ============================================================================

TEST_CASE("OnChanged callback fires on selection change") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);

    int callCount = 0;
    sel.OnChanged([&callCount](const SelectionModel&) { ++callCount; });

    sel.Click(3);
    CHECK(callCount == 1);
    sel.Click(5, ClickModifier::Ctrl);
    CHECK(callCount == 2);
    sel.Clear();
    CHECK(callCount == 3);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_CASE("Click out-of-range index is ignored") {
    SelectionModel sel(SelectionMode::Single);
    sel.SetItemCount(5);

    sel.Click(-1);
    CHECK(sel.IsEmpty());

    sel.Click(5);  // one past end
    CHECK(sel.IsEmpty());
}

TEST_CASE("None mode: Click is no-op") {
    SelectionModel sel(SelectionMode::None);
    sel.SetItemCount(5);

    sel.Click(2);
    CHECK(sel.IsEmpty());
}

TEST_CASE("SelectedIndices returns sorted vector") {
    SelectionModel sel(SelectionMode::Multi);
    sel.SetItemCount(10);

    sel.Click(7);
    sel.Click(3, ClickModifier::Ctrl);
    sel.Click(5, ClickModifier::Ctrl);

    auto indices = sel.SelectedIndices();
    CHECK(indices.size() == 3);
    CHECK(indices[0] == 3);
    CHECK(indices[1] == 5);
    CHECK(indices[2] == 7);
}

} // TEST_SUITE
