#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file FormValidatorTest.cpp
 * @brief Unit tests for FormValidator (§7.2).
 */

#include "doctest.h"

#include <Matcha/Foundation/FormValidator.h>

using namespace matcha::fw;

TEST_SUITE("FormValidator") {

// ============================================================================
// ValidationRule::Check (§7.2.1)
// ============================================================================

TEST_CASE("Required: empty fails, non-empty passes") {
    auto rule = ValidationRule::MakeRequired("required");
    CHECK_FALSE(rule.Check(""));
    CHECK(rule.Check("x"));
    CHECK(rule.Check("hello world"));
}

TEST_CASE("MinLength: boundary check") {
    auto rule = ValidationRule::MakeMinLength(3, "min3");
    CHECK_FALSE(rule.Check(""));
    CHECK_FALSE(rule.Check("ab"));
    CHECK(rule.Check("abc"));
    CHECK(rule.Check("abcd"));
}

TEST_CASE("MaxLength: boundary check") {
    auto rule = ValidationRule::MakeMaxLength(5, "max5");
    CHECK(rule.Check(""));
    CHECK(rule.Check("abcde"));
    CHECK_FALSE(rule.Check("abcdef"));
}

TEST_CASE("Range: numeric bounds") {
    auto rule = ValidationRule::MakeRange(0.0, 100.0, "0-100");
    CHECK(rule.Check("0"));
    CHECK(rule.Check("50"));
    CHECK(rule.Check("100"));
    CHECK_FALSE(rule.Check("-1"));
    CHECK_FALSE(rule.Check("101"));
    CHECK_FALSE(rule.Check("abc")); // not a number
}

TEST_CASE("Regex: pattern matching") {
    auto rule = ValidationRule::MakeRegex("^[a-z]+$", "lowercase");
    CHECK(rule.Check("abc"));
    CHECK_FALSE(rule.Check("ABC"));
    CHECK_FALSE(rule.Check("123"));
    CHECK_FALSE(rule.Check(""));
}

TEST_CASE("Custom: predicate function") {
    auto rule = ValidationRule::MakeCustom(
        [](std::string_view v) { return v != "admin"; },
        "reserved");
    CHECK(rule.Check("user"));
    CHECK_FALSE(rule.Check("admin"));
}

TEST_CASE("Custom: null predicate passes") {
    auto rule = ValidationRule::MakeCustom(nullptr, "null");
    CHECK(rule.Check("anything"));
}

// ============================================================================
// FieldValidator FSM (§7.2.4)
// ============================================================================

TEST_CASE("FieldValidator: initial state is Pristine") {
    FieldValidator fv("name");
    CHECK(fv.State() == FieldState::Pristine);
    CHECK(fv.IsValid());
    CHECK(fv.Value().empty());
}

TEST_CASE("FieldValidator: OnEdit transitions Pristine -> Dirty") {
    FieldValidator fv("name");
    fv.OnEdit("hello");
    CHECK(fv.State() == FieldState::Dirty);
    CHECK(fv.Value() == "hello");
}

TEST_CASE("FieldValidator: OnBlur transitions Dirty -> Touched") {
    FieldValidator fv("name", ValidationTrigger::OnSubmit);
    fv.OnEdit("x");
    CHECK(fv.State() == FieldState::Dirty);
    fv.OnBlur();
    CHECK(fv.State() == FieldState::Touched);
}

TEST_CASE("FieldValidator: OnBlur trigger validates on blur") {
    FieldValidator fv("name", ValidationTrigger::OnBlur);
    fv.AddRule(ValidationRule::MakeRequired("required"));
    fv.OnEdit("hello");
    fv.OnBlur();
    CHECK(fv.State() == FieldState::Valid);
}

TEST_CASE("FieldValidator: OnBlur trigger catches invalid") {
    FieldValidator fv("name", ValidationTrigger::OnBlur);
    fv.AddRule(ValidationRule::MakeRequired("required"));
    fv.OnEdit("");
    fv.OnBlur();
    CHECK(fv.State() == FieldState::Invalid);
    CHECK(fv.ErrorMessage() == "required");
}

TEST_CASE("FieldValidator: OnChange trigger validates on edit") {
    FieldValidator fv("email", ValidationTrigger::OnChange);
    fv.AddRule(ValidationRule::MakeRequired("required"));
    fv.OnEdit("a@b.c");
    CHECK(fv.State() == FieldState::Valid);

    fv.OnEdit("");
    CHECK(fv.State() == FieldState::Invalid);
}

TEST_CASE("FieldValidator: Valid -> Dirty on re-edit") {
    FieldValidator fv("name", ValidationTrigger::OnChange);
    fv.AddRule(ValidationRule::MakeRequired("required"));
    fv.OnEdit("hello");
    CHECK(fv.State() == FieldState::Valid);
    // State goes Dirty then immediately revalidates (OnChange trigger)
    fv.OnEdit("world");
    CHECK(fv.State() == FieldState::Valid);
}

TEST_CASE("FieldValidator: Validate forces validation") {
    FieldValidator fv("name");
    fv.AddRule(ValidationRule::MakeMinLength(3, "min3"));
    fv.OnEdit("ab");
    const bool ok = fv.Validate();
    CHECK_FALSE(ok);
    CHECK(fv.State() == FieldState::Invalid);
    CHECK(fv.ErrorMessage() == "min3");
}

TEST_CASE("FieldValidator: multiple rules, first failure wins") {
    FieldValidator fv("name");
    fv.AddRule(ValidationRule::MakeRequired("required"));
    fv.AddRule(ValidationRule::MakeMinLength(3, "min3"));
    fv.OnEdit("");
    fv.Validate();
    CHECK(fv.ErrorMessage() == "required"); // first rule fails

    fv.OnEdit("ab");
    fv.Validate();
    CHECK(fv.ErrorMessage() == "min3"); // second rule fails
}

TEST_CASE("FieldValidator: Reset returns to Pristine") {
    FieldValidator fv("name", ValidationTrigger::OnChange);
    fv.AddRule(ValidationRule::MakeRequired("required"));
    fv.OnEdit("hello");
    CHECK(fv.State() == FieldState::Valid);
    fv.Reset();
    CHECK(fv.State() == FieldState::Pristine);
    CHECK(fv.Value().empty());
    CHECK(fv.ErrorMessage().empty());
}

TEST_CASE("FieldValidator: state change callback fires") {
    FieldValidator fv("name");
    std::vector<std::pair<FieldState, FieldState>> transitions;
    fv.OnStateChanged([&](FieldState from, FieldState to) {
        transitions.emplace_back(from, to);
    });
    fv.OnEdit("x");
    REQUIRE(transitions.size() == 1);
    CHECK(transitions[0].first == FieldState::Pristine);
    CHECK(transitions[0].second == FieldState::Dirty);
}

// ============================================================================
// FormValidator (§7.2.5)
// ============================================================================

TEST_CASE("FormValidator: AddField and FindField") {
    FormValidator form;
    auto& f = form.AddField("name");
    CHECK(form.FieldCount() == 1);
    CHECK(form.FindField("name") == &f);
    CHECK(form.FindField("nonexistent") == nullptr);
}

TEST_CASE("FormValidator: Submit validates all fields") {
    FormValidator form;
    auto& name = form.AddField("name");
    name.AddRule(ValidationRule::MakeRequired("Name required"));
    auto& email = form.AddField("email");
    email.AddRule(ValidationRule::MakeRequired("Email required"));

    // Both empty -> both invalid
    auto result = form.Submit();
    CHECK_FALSE(result.valid);
    CHECK(result.errorCount == 2);
    CHECK(result.firstInvalidId == "name");
    CHECK(result.errorFieldIds.size() == 2);
}

TEST_CASE("FormValidator: Submit succeeds when all valid") {
    FormValidator form;
    auto& name = form.AddField("name");
    name.AddRule(ValidationRule::MakeRequired("required"));
    name.OnEdit("Alice");

    auto& age = form.AddField("age");
    age.AddRule(ValidationRule::MakeRange(0, 150, "0-150"));
    age.OnEdit("25");

    auto result = form.Submit();
    CHECK(result.valid);
    CHECK(result.errorCount == 0);
    CHECK(result.firstInvalidId.empty());
}

TEST_CASE("FormValidator: OnSubmit callback is called") {
    FormValidator form;
    auto& f = form.AddField("name");
    f.AddRule(ValidationRule::MakeRequired("required"));
    f.OnEdit("Alice");

    bool callbackFired = false;
    form.OnSubmit([&](const FormSubmitResult& r) {
        callbackFired = true;
        CHECK(r.valid);
    });
    form.Submit();
    CHECK(callbackFired);
}

TEST_CASE("FormValidator: IsFormValid") {
    FormValidator form;
    auto& f = form.AddField("name");
    f.AddRule(ValidationRule::MakeRequired("required"));
    CHECK(form.IsFormValid()); // Pristine counts as valid

    f.OnEdit("hello");
    f.Validate();
    CHECK(form.IsFormValid());

    f.OnEdit("");
    f.Validate();
    CHECK_FALSE(form.IsFormValid());
}

TEST_CASE("FormValidator: ErrorSummary") {
    FormValidator form;
    auto& a = form.AddField("a");
    a.AddRule(ValidationRule::MakeRequired("required"));
    auto& b = form.AddField("b");
    b.AddRule(ValidationRule::MakeRequired("required"));

    CHECK(form.ErrorSummary() == "No errors");

    form.Submit(); // both empty -> both invalid
    CHECK(form.ErrorSummary() == "2 validation errors");
}

TEST_CASE("FormValidator: Reset clears all fields") {
    FormValidator form;
    auto& f = form.AddField("name", ValidationTrigger::OnChange);
    f.AddRule(ValidationRule::MakeRequired("required"));
    f.OnEdit("hello");
    CHECK(f.State() == FieldState::Valid);

    form.Reset();
    CHECK(f.State() == FieldState::Pristine);
    CHECK(f.Value().empty());
}

} // TEST_SUITE
