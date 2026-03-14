#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Foundation/ContentStateModel.h>

#include <vector>

using namespace matcha::fw;

TEST_SUITE("fw::ContentStateModel") {

// ============================================================================
// Default state
// ============================================================================

TEST_CASE("Default state is Content") {
    ContentStateModel m;
    CHECK(m.ResolvedState() == ContentState::Content);
    CHECK_FALSE(m.IsLoading());
    CHECK_FALSE(m.HasError());
    CHECK_FALSE(m.IsEmpty());
}

// ============================================================================
// Single flag transitions
// ============================================================================

TEST_CASE("SetLoading -> Loading") {
    ContentStateModel m;
    m.SetLoading(true);
    CHECK(m.ResolvedState() == ContentState::Loading);
    CHECK(m.IsLoading());
}

TEST_CASE("SetLoading false -> Content") {
    ContentStateModel m;
    m.SetLoading(true);
    m.SetLoading(false);
    CHECK(m.ResolvedState() == ContentState::Content);
}

TEST_CASE("SetError -> Error") {
    ContentStateModel m;
    m.SetError({.message = "fail", .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});
    CHECK(m.ResolvedState() == ContentState::Error);
    CHECK(m.HasError());
    CHECK(m.GetErrorInfo().message == "fail");
}

TEST_CASE("ClearError -> Content") {
    ContentStateModel m;
    m.SetError({.message = "fail", .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});
    m.ClearError();
    CHECK(m.ResolvedState() == ContentState::Content);
    CHECK_FALSE(m.HasError());
}

TEST_CASE("SetEmpty -> Empty") {
    ContentStateModel m;
    m.SetEmpty({.title = "No items", .description = {}, .ctaLabel = {}});
    CHECK(m.ResolvedState() == ContentState::Empty);
    CHECK(m.IsEmpty());
    CHECK(m.GetEmptyInfo().title == "No items");
}

TEST_CASE("ClearEmpty -> Content") {
    ContentStateModel m;
    m.SetEmpty({.title = "No items", .description = {}, .ctaLabel = {}});
    m.ClearEmpty();
    CHECK(m.ResolvedState() == ContentState::Content);
    CHECK_FALSE(m.IsEmpty());
}

// ============================================================================
// Priority rules (§6.4.1)
// ============================================================================

TEST_CASE("Loading + Error -> Loading (priority rule)") {
    ContentStateModel m;
    m.SetError({.message = "fail", .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});
    m.SetLoading(true);
    CHECK(m.ResolvedState() == ContentState::Loading);
}

TEST_CASE("Loading done + Error remains -> Error") {
    ContentStateModel m;
    m.SetLoading(true);
    m.SetError({.message = "fail", .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});
    CHECK(m.ResolvedState() == ContentState::Loading);
    m.SetLoading(false);
    CHECK(m.ResolvedState() == ContentState::Error);
}

TEST_CASE("Error + Empty -> Error (priority over Empty)") {
    ContentStateModel m;
    m.SetEmpty({.title = "empty", .description = {}, .ctaLabel = {}});
    m.SetError({.message = "fail", .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});
    CHECK(m.ResolvedState() == ContentState::Error);
}

TEST_CASE("Loading + Empty -> Loading") {
    ContentStateModel m;
    m.SetEmpty({.title = "empty", .description = {}, .ctaLabel = {}});
    m.SetLoading(true);
    CHECK(m.ResolvedState() == ContentState::Loading);
}

TEST_CASE("Loading + Error + Empty -> Loading") {
    ContentStateModel m;
    m.SetLoading(true);
    m.SetError({.message = "fail", .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});
    m.SetEmpty({.title = "empty", .description = {}, .ctaLabel = {}});
    CHECK(m.ResolvedState() == ContentState::Loading);
}

// ============================================================================
// Callback
// ============================================================================

TEST_CASE("Callback fires on state change") {
    ContentStateModel m;
    std::vector<ContentState> log;
    m.OnStateChanged([&](ContentState s) { log.push_back(s); });

    m.SetLoading(true);   // Content -> Loading
    m.SetLoading(true);   // no change, no fire
    m.SetLoading(false);  // Loading -> Content

    REQUIRE(log.size() == 2);
    CHECK(log[0] == ContentState::Loading);
    CHECK(log[1] == ContentState::Content);
}

TEST_CASE("Callback reflects full priority transitions") {
    ContentStateModel m;
    std::vector<ContentState> log;
    m.OnStateChanged([&](ContentState s) { log.push_back(s); });

    m.SetLoading(true);                    // -> Loading
    m.SetError({.message = "err", .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});        // still Loading (no fire)
    m.SetLoading(false);                   // -> Error
    m.ClearError();                        // -> Content

    REQUIRE(log.size() == 3);
    CHECK(log[0] == ContentState::Loading);
    CHECK(log[1] == ContentState::Error);
    CHECK(log[2] == ContentState::Content);
}

// ============================================================================
// ErrorInfo fields
// ============================================================================

TEST_CASE("ErrorInfo preserves all fields") {
    ContentStateModel m;
    m.SetError({
        .message = "Network timeout",
        .detail = "GET /api/mesh 504",
        .severity = ErrorSeverity::Warning,
        .retryable = false,
    });
    const auto& info = m.GetErrorInfo();
    CHECK(info.message == "Network timeout");
    CHECK(info.detail == "GET /api/mesh 504");
    CHECK(info.severity == ErrorSeverity::Warning);
    CHECK_FALSE(info.retryable);
}

// ============================================================================
// EmptyInfo fields
// ============================================================================

TEST_CASE("EmptyInfo preserves all fields") {
    ContentStateModel m;
    m.SetEmpty({
        .title = "No results",
        .description = "Try adjusting filters",
        .ctaLabel = "Clear Filters",
    });
    const auto& info = m.GetEmptyInfo();
    CHECK(info.title == "No results");
    CHECK(info.description == "Try adjusting filters");
    CHECK(info.ctaLabel == "Clear Filters");
}

// ============================================================================
// Multi-callback (Fix #6)
// ============================================================================

TEST_CASE("Multiple callbacks all fire") {
    ContentStateModel m;
    int count1 = 0, count2 = 0;
    m.OnStateChanged([&](ContentState) { ++count1; });
    m.OnStateChanged([&](ContentState) { ++count2; });

    m.SetLoading(true);
    CHECK(count1 == 1);
    CHECK(count2 == 1);
}

TEST_CASE("RemoveCallback stops that callback") {
    ContentStateModel m;
    int count1 = 0, count2 = 0;
    auto id1 = m.OnStateChanged([&](ContentState) { ++count1; });
    m.OnStateChanged([&](ContentState) { ++count2; });

    m.SetLoading(true);   // both fire
    m.RemoveCallback(id1);
    m.SetLoading(false);  // only count2 fires

    CHECK(count1 == 1);
    CHECK(count2 == 2);
}

// ============================================================================
// SetError(string) convenience (Fix #6)
// ============================================================================

TEST_CASE("SetError(string) convenience overload") {
    ContentStateModel m;
    m.SetError("network timeout");
    CHECK(m.HasError());
    CHECK(m.ResolvedState() == ContentState::Error);
    CHECK(m.GetErrorInfo().message == "network timeout");
    CHECK(m.GetErrorInfo().severity == ErrorSeverity::Error);
    CHECK(m.GetErrorInfo().retryable);
}

} // TEST_SUITE
