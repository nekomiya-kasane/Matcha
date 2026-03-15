#pragma once

/**
 * @file FeedbackPolicy.h
 * @brief Feedback & System Status patterns (Spec §7.11).
 *
 * Foundation-layer descriptors for:
 * - Response time thresholds (§7.11.1)
 * - Feedback channel selection by action type (§7.11.2)
 * - Feedback decision tree (§7.11.3)
 *
 * Qt-free Foundation layer. Widget layer uses these to decide what feedback
 * to present for a given operation.
 *
 * @see Matcha_Design_System_Specification.md §7.11
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <string>

namespace matcha::fw {

// ============================================================================
// ResponseTimeClass (§7.11.1)
// ============================================================================

/**
 * @enum ResponseTimeClass
 * @brief Perceived response time classification.
 */
enum class ResponseTimeClass : uint8_t {
    Instant       = 0,   ///< < 100ms — no feedback needed
    Brief         = 1,   ///< 100ms – 1s — subtle feedback (cursor change)
    Noticeable    = 2,   ///< 1s – 10s — progress indicator (spinner/bar)
    Long          = 3,   ///< > 10s — progress bar + percentage + cancel + ETA
};

// ============================================================================
// FeedbackChannel
// ============================================================================

/**
 * @enum FeedbackChannel
 * @brief Type of feedback to present.
 */
enum class FeedbackChannel : uint8_t {
    None          = 0,
    CursorChange  = 1,   ///< Busy cursor
    Spinner       = 2,   ///< Indeterminate spinner
    ProgressBar   = 3,   ///< Determinate progress bar
    Toast         = 4,   ///< Toast notification
    InlineMessage = 5,   ///< Inline success/error message
    StatusBar     = 6,   ///< StatusBar ambient message
};

// ============================================================================
// FeedbackDescriptor
// ============================================================================

/**
 * @struct FeedbackDescriptor
 * @brief Describes what feedback to show for an operation.
 */
struct FeedbackDescriptor {
    ResponseTimeClass timeClass = ResponseTimeClass::Instant;
    FeedbackChannel   primary   = FeedbackChannel::None;
    FeedbackChannel   secondary = FeedbackChannel::None;
    bool showCancel    = false;   ///< Show cancel button (Long operations)
    bool showProgress  = false;   ///< Show percentage
    bool showEta       = false;   ///< Show estimated time remaining
    std::string message;          ///< Optional message (e.g. "Saving...")
};

// ============================================================================
// FeedbackPolicy
// ============================================================================

/**
 * @class FeedbackPolicy
 * @brief Determines feedback requirements based on operation characteristics.
 */
class MATCHA_EXPORT FeedbackPolicy {
public:
    FeedbackPolicy() = default;

    /**
     * @brief Classify a response time into a category.
     * @param estimatedMs Estimated operation duration in milliseconds.
     */
    [[nodiscard]] static auto Classify(double estimatedMs) -> ResponseTimeClass;

    /**
     * @brief Build a feedback descriptor for an operation.
     * @param estimatedMs Estimated duration (ms).
     * @param actionLabel e.g. "Saving", "Loading", "Exporting"
     * @param determinate true if progress percentage is available.
     */
    [[nodiscard]] static auto BuildFeedback(
        double estimatedMs,
        std::string_view actionLabel = "",
        bool determinate = false) -> FeedbackDescriptor;
};

} // namespace matcha::fw
