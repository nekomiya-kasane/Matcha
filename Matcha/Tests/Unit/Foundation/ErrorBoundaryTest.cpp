#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file ErrorBoundaryTest.cpp
 * @brief Unit tests for ErrorBoundary (§7.15).
 */

#include "doctest.h"

#include <Matcha/Foundation/ErrorBoundary.h>

using namespace matcha::fw;
using Ms = std::chrono::milliseconds;

namespace {

// Helper: build ErrorRecord with all fields explicitly initialized
auto MakeRec(BoundarySeverity sev, ErrorContext ctx,
             std::string code = {}, std::string msg = {},
             std::string detail = {}, std::string src = {}) -> ErrorRecord
{
    return {
        .severity = sev, .context = ctx,
        .code = std::move(code), .message = std::move(msg),
        .detail = std::move(detail), .sourceId = std::move(src),
        .actions = {},
    };
}

} // namespace

TEST_SUITE("ErrorBoundary") {

// ============================================================================
// Display strategy matrix (§7.15.2)
// ============================================================================

TEST_CASE("ResolveStrategy: Viewport matrix") {
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Info,    ErrorContext::Viewport) == DisplayStrategy::StatusBar);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Warning, ErrorContext::Viewport) == DisplayStrategy::Toast);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Error,   ErrorContext::Viewport) == DisplayStrategy::InlineAlert);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Fatal,   ErrorContext::Viewport) == DisplayStrategy::ErrorPage);
}

TEST_CASE("ResolveStrategy: Panel matrix") {
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Info,    ErrorContext::Panel) == DisplayStrategy::StatusBar);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Warning, ErrorContext::Panel) == DisplayStrategy::InlineAlert);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Error,   ErrorContext::Panel) == DisplayStrategy::InlineAlert);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Fatal,   ErrorContext::Panel) == DisplayStrategy::ErrorPage);
}

TEST_CASE("ResolveStrategy: Plugin matrix") {
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Info,    ErrorContext::Plugin) == DisplayStrategy::StatusBar);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Warning, ErrorContext::Plugin) == DisplayStrategy::Toast);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Error,   ErrorContext::Plugin) == DisplayStrategy::Toast);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Fatal,   ErrorContext::Plugin) == DisplayStrategy::Toast);
}

TEST_CASE("ResolveStrategy: Network matrix") {
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Info,    ErrorContext::Network) == DisplayStrategy::StatusBar);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Warning, ErrorContext::Network) == DisplayStrategy::Toast);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Error,   ErrorContext::Network) == DisplayStrategy::Dialog);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Fatal,   ErrorContext::Network) == DisplayStrategy::Dialog);
}

TEST_CASE("ResolveStrategy: DataLoad matrix") {
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Info,    ErrorContext::DataLoad) == DisplayStrategy::StatusBar);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Warning, ErrorContext::DataLoad) == DisplayStrategy::StatusBar);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Error,   ErrorContext::DataLoad) == DisplayStrategy::EmptyState);
    CHECK(ErrorBoundary::ResolveStrategy(BoundarySeverity::Fatal,   ErrorContext::DataLoad) == DisplayStrategy::ErrorPage);
}

// ============================================================================
// Auto-dismiss duration (§7.15.1)
// ============================================================================

TEST_CASE("AutoDismissDuration: Info=5s, Warning=8s, Error/Fatal=0") {
    CHECK(ErrorBoundary::AutoDismissDuration(BoundarySeverity::Info)    == Ms{5000});
    CHECK(ErrorBoundary::AutoDismissDuration(BoundarySeverity::Warning) == Ms{8000});
    CHECK(ErrorBoundary::AutoDismissDuration(BoundarySeverity::Error)   == Ms{0});
    CHECK(ErrorBoundary::AutoDismissDuration(BoundarySeverity::Fatal)   == Ms{0});
}

// ============================================================================
// Report + callback
// ============================================================================

TEST_CASE("Report stores error and fires callback") {
    ErrorBoundary boundary;
    DisplayStrategy reportedStrategy = DisplayStrategy::StatusBar;
    std::string reportedMsg;

    boundary.OnErrorReported([&](const ErrorRecord& rec, DisplayStrategy ds) {
        reportedMsg = rec.message;
        reportedStrategy = ds;
    });

    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Network,
                            "NET_TIMEOUT", "Request timed out."));

    CHECK(reportedMsg == "Request timed out.");
    CHECK(reportedStrategy == DisplayStrategy::Dialog);
    CHECK(boundary.TotalCount() == 1);
}

TEST_CASE("Report without callback does not crash") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Info, ErrorContext::Viewport));
    CHECK(boundary.TotalCount() == 1);
}

// ============================================================================
// Aggregation (§7.15.5)
// ============================================================================

TEST_CASE("CountBySeverity filters correctly") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Panel));
    boundary.Report(MakeRec(BoundarySeverity::Warning, ErrorContext::Panel));
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Network));

    CHECK(boundary.CountBySeverity(BoundarySeverity::Error) == 2);
    CHECK(boundary.CountBySeverity(BoundarySeverity::Warning) == 1);
    CHECK(boundary.CountBySeverity(BoundarySeverity::Info) == 0);
}

TEST_CASE("Summarize: empty -> 'No errors'") {
    ErrorBoundary boundary;
    CHECK(boundary.Summarize() == "No errors");
}

TEST_CASE("Summarize: 1 error -> '1 error found'") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Panel));
    CHECK(boundary.Summarize() == "1 error found");
}

TEST_CASE("Summarize: N errors -> 'N errors found'") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Panel));
    boundary.Report(MakeRec(BoundarySeverity::Warning, ErrorContext::Plugin));
    boundary.Report(MakeRec(BoundarySeverity::Info, ErrorContext::Viewport));
    CHECK(boundary.Summarize() == "3 errors found");
}

// ============================================================================
// ClearAll / ClearBySource / ClearByCode
// ============================================================================

TEST_CASE("ClearAll removes all errors") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Panel));
    boundary.Report(MakeRec(BoundarySeverity::Warning, ErrorContext::Plugin));
    boundary.ClearAll();
    CHECK(boundary.TotalCount() == 0);
}

TEST_CASE("ClearBySource removes only matching source") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Plugin, {}, {}, {}, "pluginA"));
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Plugin, {}, {}, {}, "pluginB"));
    boundary.ClearBySource("pluginA");
    CHECK(boundary.TotalCount() == 1);
    CHECK(boundary.ActiveErrors()[0].sourceId == "pluginB");
}

TEST_CASE("ClearByCode removes only matching code") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Network, "NET_TIMEOUT"));
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Network, "NET_REFUSED"));
    boundary.ClearByCode("NET_TIMEOUT");
    CHECK(boundary.TotalCount() == 1);
    CHECK(boundary.ActiveErrors()[0].code == "NET_REFUSED");
}

// ============================================================================
// HasFatal / HighestSeverity
// ============================================================================

TEST_CASE("HasFatal returns true when Fatal exists") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Panel));
    CHECK_FALSE(boundary.HasFatal());
    boundary.Report(MakeRec(BoundarySeverity::Fatal, ErrorContext::Plugin));
    CHECK(boundary.HasFatal());
}

TEST_CASE("HighestSeverity returns maximum active severity") {
    ErrorBoundary boundary;
    CHECK(boundary.HighestSeverity() == BoundarySeverity::Info); // empty -> Info
    boundary.Report(MakeRec(BoundarySeverity::Warning, ErrorContext::Panel));
    CHECK(boundary.HighestSeverity() == BoundarySeverity::Warning);
    boundary.Report(MakeRec(BoundarySeverity::Fatal, ErrorContext::Plugin));
    CHECK(boundary.HighestSeverity() == BoundarySeverity::Fatal);
}

// ============================================================================
// ActiveErrors returns reference
// ============================================================================

TEST_CASE("ActiveErrors returns stored records") {
    ErrorBoundary boundary;
    boundary.Report(MakeRec(BoundarySeverity::Error, ErrorContext::Network, {}, "timeout"));
    const auto& errors = boundary.ActiveErrors();
    REQUIRE(errors.size() == 1);
    CHECK(errors[0].message == "timeout");
}

} // TEST_SUITE
