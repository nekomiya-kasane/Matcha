/**
 * @file FeedbackPolicy.cpp
 * @brief Implementation of FeedbackPolicy.
 */

#include <Matcha/Foundation/FeedbackPolicy.h>

namespace matcha::fw {

auto FeedbackPolicy::Classify(double estimatedMs) -> ResponseTimeClass
{
    if (estimatedMs < 100.0) {
        return ResponseTimeClass::Instant;
    }
    if (estimatedMs < 1000.0) {
        return ResponseTimeClass::Brief;
    }
    if (estimatedMs < 10000.0) {
        return ResponseTimeClass::Noticeable;
    }
    return ResponseTimeClass::Long;
}

auto FeedbackPolicy::BuildFeedback(double estimatedMs,
                                    std::string_view actionLabel,
                                    bool determinate) -> FeedbackDescriptor
{
    FeedbackDescriptor desc;
    desc.timeClass = Classify(estimatedMs);
    desc.message = std::string(actionLabel);

    switch (desc.timeClass) {
    case ResponseTimeClass::Instant:
        desc.primary = FeedbackChannel::None;
        break;

    case ResponseTimeClass::Brief:
        desc.primary = FeedbackChannel::CursorChange;
        desc.secondary = FeedbackChannel::StatusBar;
        break;

    case ResponseTimeClass::Noticeable:
        desc.primary = determinate ? FeedbackChannel::ProgressBar
                                   : FeedbackChannel::Spinner;
        desc.secondary = FeedbackChannel::StatusBar;
        desc.showProgress = determinate;
        break;

    case ResponseTimeClass::Long:
        desc.primary = FeedbackChannel::ProgressBar;
        desc.secondary = FeedbackChannel::StatusBar;
        desc.showCancel = true;
        desc.showProgress = true;
        desc.showEta = true;
        break;
    }

    return desc;
}

} // namespace matcha::fw
