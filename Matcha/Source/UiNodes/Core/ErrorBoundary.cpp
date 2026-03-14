/**
 * @file ErrorBoundary.cpp
 * @brief Implementation of ErrorBoundary (§7.15).
 */

#include <Matcha/Foundation/ErrorBoundary.h>

#include <algorithm>
#include <array>
#include <format>

namespace matcha::fw {

using Ms = std::chrono::milliseconds;

// ============================================================================
// Display strategy matrix (§7.15.2)
// ============================================================================
//
//  Rows: Context (Viewport, Panel, Plugin, Network, DataLoad)
//  Cols: Severity (Info, Warning, Error, Fatal)
//
// | Context   | Info      | Warning     | Error          | Fatal      |
// |-----------|-----------|-------------|----------------|------------|
// | Viewport  | StatusBar | Toast       | InlineAlert    | ErrorPage  |
// | Panel     | StatusBar | InlineAlert | InlineAlert    | ErrorPage  |
// | Plugin    | StatusBar | Toast       | Toast          | Toast      |
// | Network   | StatusBar | Toast       | Dialog         | Dialog     |
// | DataLoad  | StatusBar | StatusBar   | EmptyState     | ErrorPage  |

static const std::array<std::array<DisplayStrategy, 4>, 5> kStrategyMatrix = {{
    // Viewport
    {DisplayStrategy::StatusBar, DisplayStrategy::Toast,
     DisplayStrategy::InlineAlert, DisplayStrategy::ErrorPage},
    // Panel
    {DisplayStrategy::StatusBar, DisplayStrategy::InlineAlert,
     DisplayStrategy::InlineAlert, DisplayStrategy::ErrorPage},
    // Plugin
    {DisplayStrategy::StatusBar, DisplayStrategy::Toast,
     DisplayStrategy::Toast, DisplayStrategy::Toast},
    // Network
    {DisplayStrategy::StatusBar, DisplayStrategy::Toast,
     DisplayStrategy::Dialog, DisplayStrategy::Dialog},
    // DataLoad
    {DisplayStrategy::StatusBar, DisplayStrategy::StatusBar,
     DisplayStrategy::EmptyState, DisplayStrategy::ErrorPage},
}};

auto ErrorBoundary::ResolveStrategy(BoundarySeverity severity,
                                     ErrorContext context) -> DisplayStrategy
{
    const auto row = static_cast<std::size_t>(context);
    const auto col = static_cast<std::size_t>(severity);
    if (row >= kStrategyMatrix.size() || col >= kStrategyMatrix[0].size()) {
        return DisplayStrategy::Toast; // safe fallback
    }
    return kStrategyMatrix.at(row).at(col);
}

// ============================================================================
// Auto-dismiss duration (§7.15.1)
// ============================================================================

auto ErrorBoundary::AutoDismissDuration(BoundarySeverity severity) -> Ms
{
    switch (severity) {
    case BoundarySeverity::Info:    return Ms{5000};
    case BoundarySeverity::Warning: return Ms{8000};
    case BoundarySeverity::Error:   return Ms{0};   // no auto-dismiss
    case BoundarySeverity::Fatal:   return Ms{0};   // no auto-dismiss
    }
    return Ms{0};
}

// ============================================================================
// Error reporting
// ============================================================================

void ErrorBoundary::Report(ErrorRecord record)
{
    const auto strategy = ResolveStrategy(record.severity, record.context);
    _errors.push_back(std::move(record));
    if (_callback) {
        _callback(_errors.back(), strategy);
    }
}

void ErrorBoundary::OnErrorReported(ReportCallback cb)
{
    _callback = std::move(cb);
}

// ============================================================================
// Error aggregation (§7.15.5)
// ============================================================================

auto ErrorBoundary::ActiveErrors() const -> const std::vector<ErrorRecord>&
{
    return _errors;
}

auto ErrorBoundary::CountBySeverity(BoundarySeverity severity) const -> int
{
    return static_cast<int>(std::ranges::count_if(_errors, [severity](const ErrorRecord& r) {
        return r.severity == severity;
    }));
}

auto ErrorBoundary::Summarize() const -> std::string
{
    if (_errors.empty()) {
        return "No errors";
    }
    const int total = static_cast<int>(_errors.size());
    if (total == 1) {
        return "1 error found";
    }
    return std::format("{} errors found", total);
}

void ErrorBoundary::ClearAll()
{
    _errors.clear();
}

void ErrorBoundary::ClearBySource(std::string_view sourceId)
{
    std::erase_if(_errors, [sourceId](const ErrorRecord& r) {
        return r.sourceId == sourceId;
    });
}

void ErrorBoundary::ClearByCode(std::string_view code)
{
    std::erase_if(_errors, [code](const ErrorRecord& r) {
        return r.code == code;
    });
}

auto ErrorBoundary::HasFatal() const -> bool
{
    return std::ranges::any_of(_errors, [](const ErrorRecord& r) {
        return r.severity == BoundarySeverity::Fatal;
    });
}

auto ErrorBoundary::HighestSeverity() const -> BoundarySeverity
{
    auto highest = BoundarySeverity::Info;
    for (const auto& r : _errors) {
        if (static_cast<uint8_t>(r.severity) > static_cast<uint8_t>(highest)) {
            highest = r.severity;
        }
    }
    return highest;
}

} // namespace matcha::fw
