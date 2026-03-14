#pragma once

/**
 * @file ContentStateModel.h
 * @brief Three-state content model for Loading/Empty/Error/Content regions.
 *
 * Implements the priority rule from §6.4.1:
 *   loading && error   → Loading
 *   !loading && error  → Error
 *   !loading && !error && empty → Empty
 *   otherwise          → Content
 *
 * Qt-free. Used by ContentStateOverlay (UiNode layer) and standalone.
 *
 * @see Matcha_Design_System_Specification.md §6.4
 */

#include "Matcha/Foundation/Macros.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace matcha::fw {

/**
 * @brief Resolved display state for a content region.
 */
enum class ContentState : uint8_t {
    Content,   ///< Normal widget content
    Loading,   ///< Skeleton shimmer or spinner
    Empty,     ///< Illustration + title + description + CTA
    Error,     ///< Error icon + message + retry button
};

/**
 * @brief Error severity for ErrorState display.
 */
enum class ErrorSeverity : uint8_t {
    Warning,     ///< Non-fatal, recoverable
    Error,       ///< Fatal for this region, retry possible
    Critical,    ///< System-level, escalation needed
};

/**
 * @brief Error info for ErrorState.
 */
struct ErrorInfo {
    std::string message;         ///< Human-readable error text
    std::string detail;          ///< Technical detail (optional)
    ErrorSeverity severity = ErrorSeverity::Error;
    bool retryable = true;       ///< Show retry button?
};

/**
 * @brief Empty state info for EmptyState.
 */
struct EmptyInfo {
    std::string title;           ///< e.g. "No items"
    std::string description;     ///< e.g. "Click + to add your first item"
    std::string ctaLabel;        ///< Call-to-action button label (optional)
};

/**
 * @brief Manages the three-state model for a content region.
 *
 * Callers set the three boolean flags (loading, error, empty) and the
 * model resolves the display state using §6.4.1 priority logic. A
 * callback fires when the resolved state changes.
 */
class MATCHA_EXPORT ContentStateModel {
public:
    using StateChangedCallback = std::function<void(ContentState newState)>;
    using CallbackId = uint64_t;

    ContentStateModel() = default;

    // -- Flag setters (each triggers re-resolve) --

    void SetLoading(bool loading);
    void SetError(const ErrorInfo& info);
    void ClearError();
    void SetEmpty(const EmptyInfo& info);
    void ClearEmpty();

    /**
     * @brief Convenience: set error with just a message string.
     */
    void SetError(const std::string& message);

    // -- Queries --

    [[nodiscard]] auto IsLoading() const -> bool { return _loading; }
    [[nodiscard]] auto HasError() const -> bool { return _hasError; }
    [[nodiscard]] auto IsEmpty() const -> bool { return _isEmpty; }

    [[nodiscard]] auto GetErrorInfo() const -> const ErrorInfo& { return _errorInfo; }
    [[nodiscard]] auto GetEmptyInfo() const -> const EmptyInfo& { return _emptyInfo; }

    /**
     * @brief Resolved display state per §6.4.1 priority rule.
     */
    [[nodiscard]] auto ResolvedState() const -> ContentState;

    // -- Callbacks (multi-subscriber) --

    /**
     * @brief Register a state-change callback. Returns an ID for removal.
     */
    auto OnStateChanged(StateChangedCallback cb) -> CallbackId;

    /**
     * @brief Remove a previously registered callback.
     */
    void RemoveCallback(CallbackId id);

private:
    void Resolve();

    bool _loading  = false;
    bool _hasError = false;
    bool _isEmpty  = false;

    ErrorInfo _errorInfo;
    EmptyInfo _emptyInfo;

    ContentState _resolved = ContentState::Content;

    struct CallbackEntry { CallbackId id; StateChangedCallback cb; };
    std::vector<CallbackEntry> _callbacks;
    CallbackId _nextId = 1;
};

} // namespace matcha::fw
