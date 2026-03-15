#pragma once

/**
 * @file DestructiveActionPolicy.h
 * @brief Destructive action severity classification and confirmation patterns (Spec §7.17).
 *
 * Provides a Foundation-layer policy engine that:
 * - Classifies actions by severity (Low, Medium, High, Critical)
 * - Determines required safeguard (none, single confirm, 2-step, 2-step + typing)
 * - Generates confirmation dialog descriptors
 *
 * Qt-free Foundation layer. Widget layer renders the dialogs.
 *
 * @see Matcha_Design_System_Specification.md §7.17
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <string>

namespace matcha::fw {

// ============================================================================
// ActionSeverity
// ============================================================================

/**
 * @enum ActionSeverity
 * @brief Severity classification for destructive actions (§7.17.1).
 */
enum class ActionSeverity : uint8_t {
    Low      = 0,   ///< Easily undoable (e.g. delete node with Ctrl+Z)
    Medium   = 1,   ///< Undoable but with side effects (e.g. close unsaved tab)
    High     = 2,   ///< Irreversible or broad impact (e.g. "Delete All", "Reset")
    Critical = 3,   ///< Affects external systems (e.g. overwrite export, publish)
};

// ============================================================================
// ConfirmationMode
// ============================================================================

/**
 * @enum ConfirmationMode
 * @brief Required confirmation pattern per severity.
 */
enum class ConfirmationMode : uint8_t {
    None           = 0,   ///< No confirmation; rely on Undo
    SingleConfirm  = 1,   ///< Single confirmation dialog
    TwoStep        = 2,   ///< 2-step confirmation dialog
    TwoStepTyping  = 3,   ///< 2-step + explicit typing of confirmation word
};

// ============================================================================
// ConfirmationDescriptor
// ============================================================================

/**
 * @struct ConfirmationDescriptor
 * @brief Describes a confirmation dialog to present before a destructive action.
 */
struct ConfirmationDescriptor {
    ConfirmationMode mode = ConfirmationMode::None;
    std::string title;              ///< e.g. "Delete 3 selected nodes?"
    std::string description;        ///< e.g. "This action can be undone with Ctrl+Z."
    std::string confirmLabel;       ///< e.g. "Delete" (action verb, never "OK")
    std::string cancelLabel  = "Cancel";
    std::string confirmWord;        ///< For TwoStepTyping: word user must type (UPPERCASE)
    bool        undoAvailable = false;
};

// ============================================================================
// DestructiveActionPolicy
// ============================================================================

/**
 * @class DestructiveActionPolicy
 * @brief Maps action severity to confirmation requirements.
 */
class MATCHA_EXPORT DestructiveActionPolicy {
public:
    DestructiveActionPolicy() = default;

    /**
     * @brief Get the required confirmation mode for a severity level.
     */
    [[nodiscard]] static auto RequiredConfirmation(ActionSeverity severity)
        -> ConfirmationMode;

    /**
     * @brief Build a confirmation descriptor for a destructive action.
     * @param severity Action severity.
     * @param actionVerb e.g. "Delete", "Reset", "Overwrite"
     * @param targetDescription e.g. "3 selected nodes", "all settings"
     * @param undoAvailable Whether Undo is available after the action.
     * @return Descriptor for the confirmation dialog (mode=None if no confirm needed).
     */
    [[nodiscard]] static auto BuildConfirmation(
        ActionSeverity severity,
        std::string_view actionVerb,
        std::string_view targetDescription,
        bool undoAvailable = false) -> ConfirmationDescriptor;

    /**
     * @brief Generate the confirmation word from an action verb (UPPERCASE).
     */
    [[nodiscard]] static auto MakeConfirmWord(std::string_view actionVerb) -> std::string;
};

} // namespace matcha::fw
