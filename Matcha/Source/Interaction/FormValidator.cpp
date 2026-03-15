/**
 * @file FormValidator.cpp
 * @brief Implementation of FormValidator (§7.2).
 */

#include <Matcha/Interaction/FormValidator.h>

#include <algorithm>
#include <charconv>
#include <format>
#include <regex>

namespace matcha::fw {

// ============================================================================
// ValidationRule factories
// ============================================================================

auto ValidationRule::MakeRequired(std::string msg) -> ValidationRule
{
    return {.type = RuleType::Required, .intParam = 0, .minParam = 0.0, .maxParam = 0.0,
            .pattern = {}, .predicate = {}, .errorMessage = std::move(msg)};
}

auto ValidationRule::MakeMinLength(int n, std::string msg) -> ValidationRule
{
    return {.type = RuleType::MinLength, .intParam = n, .minParam = 0.0, .maxParam = 0.0,
            .pattern = {}, .predicate = {}, .errorMessage = std::move(msg)};
}

auto ValidationRule::MakeMaxLength(int n, std::string msg) -> ValidationRule
{
    return {.type = RuleType::MaxLength, .intParam = n, .minParam = 0.0, .maxParam = 0.0,
            .pattern = {}, .predicate = {}, .errorMessage = std::move(msg)};
}

auto ValidationRule::MakeRange(double min, double max, std::string msg) -> ValidationRule
{
    return {.type = RuleType::Range, .intParam = 0, .minParam = min, .maxParam = max,
            .pattern = {}, .predicate = {}, .errorMessage = std::move(msg)};
}

auto ValidationRule::MakeRegex(std::string pat, std::string msg) -> ValidationRule
{
    return {.type = RuleType::Regex, .intParam = 0, .minParam = 0.0, .maxParam = 0.0,
            .pattern = std::move(pat), .predicate = {}, .errorMessage = std::move(msg)};
}

auto ValidationRule::MakeCustom(std::function<bool(std::string_view)> pred,
                                std::string msg) -> ValidationRule
{
    return {.type = RuleType::Custom, .intParam = 0, .minParam = 0.0, .maxParam = 0.0,
            .pattern = {}, .predicate = std::move(pred), .errorMessage = std::move(msg)};
}

// ============================================================================
// ValidationRule::Check
// ============================================================================

auto ValidationRule::Check(std::string_view value) const -> bool
{
    switch (type) {
    case RuleType::Required:
        return !value.empty();

    case RuleType::MinLength:
        return static_cast<int>(value.size()) >= intParam;

    case RuleType::MaxLength:
        return static_cast<int>(value.size()) <= intParam;

    case RuleType::Range: {
        double num = 0.0;
        const char* first = value.data();
        const char* last  = first + value.size();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto [ptr, ec] = std::from_chars(first, last, num);
        if (ec != std::errc{}) {
            return false; // not a valid number
        }
        return num >= minParam && num <= maxParam;
    }

    case RuleType::Regex: {
        try {
            const std::regex re(pattern);
            return std::regex_match(value.begin(), value.end(), re);
        } catch (const std::regex_error&) {
            return false; // invalid regex pattern
        }
    }

    case RuleType::Custom:
        return predicate ? predicate(value) : true;
    }
    return true;
}

// ============================================================================
// FieldValidator
// ============================================================================

FieldValidator::FieldValidator(std::string fieldId, ValidationTrigger trigger)
    : _fieldId(std::move(fieldId))
    , _trigger(trigger)
{
}

void FieldValidator::AddRule(ValidationRule rule)
{
    _rules.push_back(std::move(rule));
}

void FieldValidator::OnEdit(std::string_view newValue)
{
    _value = std::string(newValue);

    // Transition to Dirty from any state that allows editing
    if (_state == FieldState::Pristine ||
        _state == FieldState::Valid ||
        _state == FieldState::Invalid) {
        SetState(FieldState::Dirty);
    }

    // OnChange trigger: validate immediately
    if (_trigger == ValidationTrigger::OnChange) {
        Validate();
    }
}

void FieldValidator::OnBlur()
{
    // Dirty → Touched
    if (_state == FieldState::Dirty) {
        SetState(FieldState::Touched);
    }

    // OnBlur trigger: validate after touch
    if (_trigger == ValidationTrigger::OnBlur &&
        (_state == FieldState::Touched || _state == FieldState::Dirty)) {
        Validate();
    }
}

auto FieldValidator::Validate() -> bool
{
    SetState(FieldState::Validating);
    const bool ok = RunRules();
    SetState(ok ? FieldState::Valid : FieldState::Invalid);
    return ok;
}

void FieldValidator::Reset()
{
    _value.clear();
    _errorMessage.clear();
    _state = FieldState::Pristine;
}

void FieldValidator::OnStateChanged(StateChangedCallback cb)
{
    _stateCallback = std::move(cb);
}

void FieldValidator::SetState(FieldState s)
{
    if (_state == s) {
        return;
    }
    const auto old = _state;
    _state = s;
    if (_stateCallback) {
        _stateCallback(old, _state);
    }
}

auto FieldValidator::RunRules() -> bool
{
    _errorMessage.clear();
    for (const auto& rule : _rules) {
        if (!rule.Check(_value)) {
            _errorMessage = rule.errorMessage;
            return false;
        }
    }
    return true;
}

// ============================================================================
// FormValidator
// ============================================================================

auto FormValidator::AddField(std::string fieldId, ValidationTrigger trigger) -> FieldValidator&
{
    _fields.emplace_back(std::move(fieldId), trigger);
    return _fields.back();
}

auto FormValidator::FindField(std::string_view fieldId) -> FieldValidator*
{
    for (auto& f : _fields) {
        if (f.FieldId() == fieldId) {
            return &f;
        }
    }
    return nullptr;
}

auto FormValidator::FindField(std::string_view fieldId) const -> const FieldValidator*
{
    for (const auto& f : _fields) {
        if (f.FieldId() == fieldId) {
            return &f;
        }
    }
    return nullptr;
}

auto FormValidator::Submit() -> FormSubmitResult
{
    FormSubmitResult result;

    for (auto& field : _fields) {
        const bool ok = field.Validate();
        if (!ok) {
            result.valid = false;
            result.errorFieldIds.push_back(field.FieldId());
            ++result.errorCount;
            if (result.firstInvalidId.empty()) {
                result.firstInvalidId = field.FieldId();
            }
        }
    }

    if (_submitCallback) {
        _submitCallback(result);
    }
    return result;
}

void FormValidator::OnSubmit(SubmitCallback cb)
{
    _submitCallback = std::move(cb);
}

auto FormValidator::IsFormValid() const -> bool
{
    return std::ranges::all_of(_fields, [](const FieldValidator& f) {
        return f.IsValid();
    });
}

auto FormValidator::ErrorSummary() const -> std::string
{
    int invalid = 0;
    for (const auto& f : _fields) {
        if (f.State() == FieldState::Invalid) {
            ++invalid;
        }
    }
    if (invalid == 0) {
        return "No errors";
    }
    if (invalid == 1) {
        return "1 validation error";
    }
    return std::format("{} validation errors", invalid);
}

void FormValidator::Reset()
{
    for (auto& field : _fields) {
        field.Reset();
    }
}

} // namespace matcha::fw
