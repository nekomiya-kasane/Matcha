/**
 * @file DestructiveActionPolicy.cpp
 * @brief Implementation of DestructiveActionPolicy.
 */

#include <Matcha/Feedback/DestructiveActionPolicy.h>

#include <algorithm>
#include <format>

namespace matcha::fw {

auto DestructiveActionPolicy::RequiredConfirmation(ActionSeverity severity)
    -> ConfirmationMode
{
    switch (severity) {
    case ActionSeverity::Low:      return ConfirmationMode::None;
    case ActionSeverity::Medium:   return ConfirmationMode::SingleConfirm;
    case ActionSeverity::High:     return ConfirmationMode::TwoStep;
    case ActionSeverity::Critical: return ConfirmationMode::TwoStepTyping;
    }
    return ConfirmationMode::None;
}

auto DestructiveActionPolicy::BuildConfirmation(
    ActionSeverity severity,
    std::string_view actionVerb,
    std::string_view targetDescription,
    bool undoAvailable) -> ConfirmationDescriptor
{
    const auto mode = RequiredConfirmation(severity);

    if (mode == ConfirmationMode::None) {
        ConfirmationDescriptor none;
        none.mode = ConfirmationMode::None;
        return none;
    }

    ConfirmationDescriptor desc;
    desc.mode = mode;
    desc.title = std::format("{} {}?", actionVerb, targetDescription);
    desc.confirmLabel = std::string(actionVerb);
    desc.cancelLabel = "Cancel";
    desc.undoAvailable = undoAvailable;

    if (undoAvailable) {
        desc.description = std::format("This action can be undone with Ctrl+Z.");
    } else if (severity == ActionSeverity::High || severity == ActionSeverity::Critical) {
        desc.description = "This cannot be undone.";
    } else {
        desc.description = std::format("Are you sure you want to {} {}?",
                                        actionVerb, targetDescription);
    }

    if (mode == ConfirmationMode::TwoStepTyping) {
        desc.confirmWord = MakeConfirmWord(actionVerb);
    }

    return desc;
}

auto DestructiveActionPolicy::MakeConfirmWord(std::string_view actionVerb) -> std::string
{
    std::string result(actionVerb);
    std::ranges::transform(result, result.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return result;
}

} // namespace matcha::fw
