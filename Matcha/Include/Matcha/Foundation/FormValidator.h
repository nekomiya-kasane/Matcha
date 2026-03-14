#pragma once

/**
 * @file FormValidator.h
 * @brief Form validation engine with field-level FSM and multi-field coordination.
 *
 * Implements §7.2 Form Validation from the Matcha Design System Specification.
 * - 7 validation rule types: Required, MinLength, MaxLength, Range, Regex, Custom, CrossField
 * - Field state FSM: Pristine → Dirty → Touched → Validating → Valid/Invalid
 * - Trigger timing: OnChange (debounced), OnBlur, OnSubmit
 * - Submit flow with first-invalid-field focus
 *
 * This is a Foundation-layer component with zero Qt dependency.
 *
 * @see Matcha_Design_System_Specification.md §7.2
 */

#include <Matcha/Foundation/Macros.h>

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

// ============================================================================
// FieldState FSM (§7.2.4)
// ============================================================================

/**
 * @enum FieldState
 * @brief States in the field validation FSM.
 *
 * | State      | Visual             | Submit Allowed       |
 * |------------|--------------------|:--------------------:|
 * | Pristine   | Normal border      | Yes (skip validation)|
 * | Dirty      | Normal border      | Depends on trigger   |
 * | Touched    | Normal border      | Depends on trigger   |
 * | Validating | Normal border      | No                   |
 * | Valid      | Normal/Success      | Yes                  |
 * | Invalid    | Error border + msg | No                   |
 */
enum class FieldState : uint8_t {
    Pristine   = 0,
    Dirty      = 1,
    Touched    = 2,
    Validating = 3,
    Valid      = 4,
    Invalid    = 5,
};

// ============================================================================
// ValidationTrigger (§7.2.2)
// ============================================================================

/**
 * @enum ValidationTrigger
 * @brief When validation fires for a field.
 */
enum class ValidationTrigger : uint8_t {
    OnChange = 0,   ///< After each edit (debounced 300ms)
    OnBlur   = 1,   ///< When field loses focus (default)
    OnSubmit = 2,   ///< Only on form submit
};

// ============================================================================
// ValidationRule (§7.2.1)
// ============================================================================

/**
 * @enum RuleType
 * @brief Discriminator for validation rule variants.
 */
enum class RuleType : uint8_t {
    Required,
    MinLength,
    MaxLength,
    Range,
    Regex,
    Custom,
};

/**
 * @struct ValidationRule
 * @brief A single validation rule with error message.
 *
 * Use the static factory methods to construct rules:
 * @code
 *   auto r1 = ValidationRule::Required("Name is required");
 *   auto r2 = ValidationRule::MinLength(3, "At least 3 characters");
 *   auto r3 = ValidationRule::Regex("^[a-z]+$", "Lowercase only");
 *   auto r4 = ValidationRule::Custom([](std::string_view v) {
 *       return v != "admin";
 *   }, "Reserved name");
 * @endcode
 */
struct MATCHA_EXPORT ValidationRule {
    RuleType    type       = RuleType::Required;
    int         intParam   = 0;         ///< Used by MinLength, MaxLength
    double      minParam   = 0.0;       ///< Used by Range (min)
    double      maxParam   = 0.0;       ///< Used by Range (max)
    std::string pattern;                ///< Used by Regex
    std::function<bool(std::string_view)> predicate;  ///< Used by Custom
    std::string errorMessage;           ///< Human-readable error

    // ====================================================================
    // Factory methods
    // ====================================================================

    static auto MakeRequired(std::string msg) -> ValidationRule;
    static auto MakeMinLength(int n, std::string msg) -> ValidationRule;
    static auto MakeMaxLength(int n, std::string msg) -> ValidationRule;
    static auto MakeRange(double min, double max, std::string msg) -> ValidationRule;
    static auto MakeRegex(std::string pattern, std::string msg) -> ValidationRule;
    static auto MakeCustom(std::function<bool(std::string_view)> pred,
                           std::string msg) -> ValidationRule;

    // ====================================================================
    // Evaluation
    // ====================================================================

    /**
     * @brief Test a value against this rule.
     * @param value The field value to validate.
     * @return true if the value passes this rule.
     */
    [[nodiscard]] auto Check(std::string_view value) const -> bool;
};

// ============================================================================
// FieldValidator — single-field FSM + rules (§7.2.4)
// ============================================================================

/**
 * @class FieldValidator
 * @brief Manages validation state for a single field.
 *
 * Tracks the FSM state transitions and validates against a set of rules.
 */
class MATCHA_EXPORT FieldValidator {
public:
    using StateChangedCallback = std::function<void(FieldState /*old*/, FieldState /*new*/)>;

    explicit FieldValidator(std::string fieldId,
                            ValidationTrigger trigger = ValidationTrigger::OnBlur);

    // ====================================================================
    // Configuration
    // ====================================================================

    void AddRule(ValidationRule rule);
    [[nodiscard]] auto FieldId() const -> const std::string& { return _fieldId; }
    [[nodiscard]] auto Trigger() const -> ValidationTrigger { return _trigger; }
    void SetTrigger(ValidationTrigger t) { _trigger = t; }

    // ====================================================================
    // FSM events
    // ====================================================================

    /**
     * @brief Notify that the user edited the field value.
     *
     * Transitions: Pristine→Dirty, Valid→Dirty, Invalid→Dirty.
     * If trigger is OnChange, also validates.
     */
    void OnEdit(std::string_view newValue);

    /**
     * @brief Notify that the field lost focus.
     *
     * Transitions: Dirty→Touched, then if trigger is OnBlur, validates.
     */
    void OnBlur();

    /**
     * @brief Force validation (used by form-level OnSubmit).
     * @return true if valid.
     */
    auto Validate() -> bool;

    // ====================================================================
    // Query
    // ====================================================================

    [[nodiscard]] auto State() const -> FieldState { return _state; }
    [[nodiscard]] auto Value() const -> const std::string& { return _value; }
    [[nodiscard]] auto ErrorMessage() const -> const std::string& { return _errorMessage; }
    [[nodiscard]] auto IsValid() const -> bool {
        return _state == FieldState::Valid || _state == FieldState::Pristine;
    }
    [[nodiscard]] auto RuleCount() const -> int {
        return static_cast<int>(_rules.size());
    }

    /**
     * @brief Reset field to Pristine state with empty value.
     */
    void Reset();

    // ====================================================================
    // Observer
    // ====================================================================

    void OnStateChanged(StateChangedCallback cb);

private:
    void SetState(FieldState s);
    auto RunRules() -> bool;

    std::string              _fieldId;
    ValidationTrigger        _trigger = ValidationTrigger::OnBlur;
    FieldState               _state   = FieldState::Pristine;
    std::string              _value;
    std::string              _errorMessage;
    std::vector<ValidationRule> _rules;
    StateChangedCallback     _stateCallback;
};

// ============================================================================
// FormValidator — multi-field coordinator (§7.2.5)
// ============================================================================

/**
 * @struct FormSubmitResult
 * @brief Result of form-level submit validation.
 */
struct FormSubmitResult {
    bool        valid            = true;
    std::string firstInvalidId;          ///< Field ID of first invalid field
    int         errorCount       = 0;
    std::vector<std::string> errorFieldIds;
};

/**
 * @class FormValidator
 * @brief Coordinates validation across multiple fields with submit flow.
 *
 * Usage:
 * @code
 *   FormValidator form;
 *   auto& name = form.AddField("name", ValidationTrigger::OnBlur);
 *   name.AddRule(ValidationRule::MakeRequired("Name is required"));
 *   name.AddRule(ValidationRule::MakeMinLength(2, "At least 2 chars"));
 *
 *   auto& email = form.AddField("email", ValidationTrigger::OnChange);
 *   email.AddRule(ValidationRule::MakeRegex(".*@.*\\..*", "Invalid email"));
 *
 *   form.OnSubmit([](const FormSubmitResult& r) {
 *       if (r.valid) { saveData(); }
 *   });
 *
 *   form.Submit();
 * @endcode
 */
class MATCHA_EXPORT FormValidator {
public:
    using SubmitCallback = std::function<void(const FormSubmitResult&)>;

    FormValidator() = default;

    // ====================================================================
    // Field management
    // ====================================================================

    /**
     * @brief Add a new field to the form.
     * @return Reference to the created FieldValidator.
     */
    auto AddField(std::string fieldId,
                  ValidationTrigger trigger = ValidationTrigger::OnBlur) -> FieldValidator&;

    /**
     * @brief Find a field by ID.
     * @return Pointer to the field, or nullptr if not found.
     */
    [[nodiscard]] auto FindField(std::string_view fieldId) -> FieldValidator*;
    [[nodiscard]] auto FindField(std::string_view fieldId) const -> const FieldValidator*;

    [[nodiscard]] auto FieldCount() const -> int {
        return static_cast<int>(_fields.size());
    }

    // ====================================================================
    // Submit flow (§7.2.5)
    // ====================================================================

    /**
     * @brief Validate all fields and invoke submit callback.
     * @return FormSubmitResult with validation outcome.
     */
    auto Submit() -> FormSubmitResult;

    /**
     * @brief Register submit callback.
     */
    void OnSubmit(SubmitCallback cb);

    // ====================================================================
    // Query
    // ====================================================================

    /**
     * @brief Check if all fields are currently valid (or pristine).
     */
    [[nodiscard]] auto IsFormValid() const -> bool;

    /**
     * @brief Get summary string for error aggregation (§7.15.5).
     */
    [[nodiscard]] auto ErrorSummary() const -> std::string;

    /**
     * @brief Reset all fields to Pristine state.
     */
    void Reset();

private:
    std::vector<FieldValidator> _fields;
    SubmitCallback              _submitCallback;
};

} // namespace matcha::fw
