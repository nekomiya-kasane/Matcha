#pragma once

/**
 * @file ErrorBoundary.h
 * @brief Error classification, containment, and recovery strategy.
 *
 * Implements §7.15 Error Boundary & Recovery Pattern from the Matcha Design
 * System Specification. Provides a Qt-free Foundation-layer model for:
 * - Error severity classification (Info, Warning, Error, Fatal)
 * - Error context (Viewport, Panel, Plugin, Network, DataLoad)
 * - Display strategy resolution (Toast, InlineAlert, Dialog, ErrorPage)
 * - Recovery action descriptors
 * - Error aggregation for batch validation scenarios
 *
 * @see Matcha_Design_System_Specification.md §7.15
 */

#include <Matcha/Foundation/Macros.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace matcha::fw {

// ============================================================================
// Error severity (§7.15.1)
// ============================================================================

/**
 * @enum ErrorSeverity
 * @brief Severity classification for error boundary decisions.
 *
 * | Severity | Display         | Auto-Dismiss |
 * |----------|-----------------|:------------:|
 * | Info     | Toast (Info)    | Yes (5s)     |
 * | Warning  | Toast (Warning) | Yes (8s)     |
 * | Error    | Alert/Dialog    | No           |
 * | Fatal    | Error page      | No           |
 */
enum class BoundarySeverity : uint8_t {
    Info    = 0,
    Warning = 1,
    Error   = 2,
    Fatal   = 3,
};

// ============================================================================
// Error context (§7.15.2)
// ============================================================================

/**
 * @enum ErrorContext
 * @brief Where the error occurred — determines display strategy.
 */
enum class ErrorContext : uint8_t {
    Viewport = 0,
    Panel    = 1,
    Plugin   = 2,
    Network  = 3,
    DataLoad = 4,
};

// ============================================================================
// Display strategy (§7.15.2 matrix output)
// ============================================================================

/**
 * @enum DisplayStrategy
 * @brief How the error should be presented to the user.
 */
enum class DisplayStrategy : uint8_t {
    StatusBar    = 0,   ///< Non-intrusive status bar message
    Toast        = 1,   ///< Transient toast notification
    InlineAlert  = 2,   ///< Inline alert banner within region
    Dialog       = 3,   ///< Modal/modeless dialog
    ErrorPage    = 4,   ///< Full-region error page replacement
    EmptyState   = 5,   ///< Empty state with retry (§6.4.5)
};

// ============================================================================
// RecoveryAction
// ============================================================================

/**
 * @struct RecoveryAction
 * @brief Describes a recovery action available to the user.
 */
struct RecoveryAction {
    std::string label;                      ///< Button label (e.g., "Retry")
    std::function<void()> handler;          ///< Action handler
    bool isPrimary = false;                 ///< Primary (accent) vs secondary style
};

// ============================================================================
// ErrorRecord
// ============================================================================

/**
 * @struct ErrorRecord
 * @brief A single error occurrence with full context.
 */
struct ErrorRecord {
    BoundarySeverity severity = BoundarySeverity::Error;
    ErrorContext      context  = ErrorContext::Viewport;
    std::string       code;       ///< Machine-readable error code (e.g., "NET_TIMEOUT")
    std::string       message;    ///< Human-readable: "[What happened]. [What you can do]."
    std::string       detail;     ///< Optional technical detail for logs
    std::string       sourceId;   ///< Originating module/plugin ID
    std::vector<RecoveryAction> actions;  ///< Available recovery actions
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
};

// ============================================================================
// ErrorBoundary (§7.15)
// ============================================================================

/**
 * @class ErrorBoundary
 * @brief Manages error classification, display strategy resolution, and aggregation.
 *
 * **Thread safety**: Not thread-safe. All calls must be from the GUI thread.
 *
 * Usage:
 * @code
 *   ErrorBoundary boundary;
 *   boundary.OnErrorReported([](const ErrorRecord& rec, DisplayStrategy ds) {
 *       // route to appropriate UI display
 *   });
 *
 *   boundary.Report({
 *       .severity = BoundarySeverity::Error,
 *       .context  = ErrorContext::Network,
 *       .code     = "NET_TIMEOUT",
 *       .message  = "Request timed out. Check your connection and retry.",
 *       .actions  = {{.label = "Retry", .handler = []{...}, .isPrimary = true},
 *                    {.label = "Work Offline", .handler = []{...}}},
 *   });
 *
 *   auto summary = boundary.Summarize();  // "2 errors found"
 * @endcode
 */
class MATCHA_EXPORT ErrorBoundary {
public:
    using ReportCallback = std::function<void(const ErrorRecord&, DisplayStrategy)>;

    ErrorBoundary() = default;

    // ====================================================================
    // Display strategy resolution (§7.15.2 matrix)
    // ====================================================================

    /**
     * @brief Resolve the display strategy for a (severity, context) pair.
     *
     * Implements the §7.15.2 display strategy matrix.
     */
    [[nodiscard]] static auto ResolveStrategy(BoundarySeverity severity,
                                               ErrorContext context) -> DisplayStrategy;

    // ====================================================================
    // Auto-dismiss duration (§7.15.1)
    // ====================================================================

    /**
     * @brief Get auto-dismiss duration for a severity level.
     * @return Duration in milliseconds. Returns 0ms for non-dismissible severities.
     */
    [[nodiscard]] static auto AutoDismissDuration(BoundarySeverity severity)
        -> std::chrono::milliseconds;

    // ====================================================================
    // Error reporting
    // ====================================================================

    /**
     * @brief Report an error. Resolves display strategy and notifies observers.
     * @param record The error record to report.
     */
    void Report(ErrorRecord record);

    /**
     * @brief Register a callback for error reports.
     */
    void OnErrorReported(ReportCallback cb);

    // ====================================================================
    // Error aggregation (§7.15.5)
    // ====================================================================

    /**
     * @brief Get all active (unresolved) errors.
     */
    [[nodiscard]] auto ActiveErrors() const -> const std::vector<ErrorRecord>&;

    /**
     * @brief Get count of active errors by severity.
     */
    [[nodiscard]] auto CountBySeverity(BoundarySeverity severity) const -> int;

    /**
     * @brief Get total active error count.
     */
    [[nodiscard]] auto TotalCount() const -> int {
        return static_cast<int>(_errors.size());
    }

    /**
     * @brief Generate a summary string (e.g., "3 errors found").
     */
    [[nodiscard]] auto Summarize() const -> std::string;

    /**
     * @brief Clear all active errors.
     */
    void ClearAll();

    /**
     * @brief Clear errors from a specific source.
     */
    void ClearBySource(std::string_view sourceId);

    /**
     * @brief Clear errors matching a specific code.
     */
    void ClearByCode(std::string_view code);

    /**
     * @brief Check if the boundary has any Fatal errors.
     */
    [[nodiscard]] auto HasFatal() const -> bool;

    /**
     * @brief Get the highest severity among active errors.
     *
     * Returns Info if no errors are active.
     */
    [[nodiscard]] auto HighestSeverity() const -> BoundarySeverity;

private:
    std::vector<ErrorRecord> _errors;
    ReportCallback           _callback;
};

} // namespace matcha::fw
