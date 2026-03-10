#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Widgets/Controls/NyanPushButton.h>
#include <Matcha/Widgets/Controls/NyanToolButton.h>
#include <Matcha/Widgets/Controls/NyanLineEdit.h>
#include <Matcha/Widgets/Controls/NyanLabel.h>
#include <Matcha/Widgets/Controls/NyanBadge.h>
#include <Matcha/Widgets/Controls/NyanSearchBox.h>
#include <Matcha/Widgets/Controls/NyanBreadcrumb.h>
#include <Matcha/Widgets/Controls/NyanRangeSlider.h>
#include <Matcha/Widgets/Controls/NyanProgressRing.h>
#include <Matcha/Widgets/Controls/NyanSpinBox.h>
#include <Matcha/Widgets/Controls/NyanComboBox.h>
#include <Matcha/Widgets/Controls/NyanToggleSwitch.h>
#include <Matcha/Widgets/Controls/NyanLegend.h>
#include <Matcha/Widgets/Menu/NyanPopConfirm.h>
#include <Matcha/Widgets/Controls/NyanDateTimePicker.h>

#include <utility>

using namespace matcha::gui;

// ============================================================================
// Compile-time enum completeness (static_assert)
// ============================================================================

static_assert(std::to_underlying(ButtonVariant::Count_) == 4,
              "ButtonVariant must have 4 values");

static_assert(std::to_underlying(FlyoutPolicy::Count_) == 3,
              "FlyoutPolicy must have 3 values");

static_assert(std::to_underlying(InputMode::Count_) == 3,
              "InputMode must have 3 values");

static_assert(std::to_underlying(LineEditBorder::Count_) == 2,
              "LineEditBorder must have 2 values");

static_assert(std::to_underlying(LabelRole::Count_) == 4,
              "LabelRole must have 4 values");

static_assert(std::to_underlying(BadgeVariant::Count_) == 6,
              "BadgeVariant must have 6 values");

static_assert(std::to_underlying(SearchMode::Count_) == 2,
              "SearchMode must have 2 values");

static_assert(std::to_underlying(PopConfirmState::Count_) == 4,
              "PopConfirmState must have 4 values");

static_assert(std::to_underlying(PopConfirmButton::Count_) == 3,
              "PopConfirmButton must have 3 values");

static_assert(std::to_underlying(PopConfirmCode::Count_) == 3,
              "PopConfirmCode must have 3 values");

static_assert(std::to_underlying(DateTimeMode::Count_) == 3,
              "DateTimeMode must have 3 values");

// ============================================================================
// Runtime tests
// ============================================================================

TEST_SUITE("CoreWidgetEnums") {

// -- T03.1.1 NyanPushButton ------------------------------------------------

TEST_CASE("ButtonSize underlying values equal pixel heights") {
    CHECK(std::to_underlying(ButtonSize::Small)  == 24);
    CHECK(std::to_underlying(ButtonSize::Medium) == 32);
    CHECK(std::to_underlying(ButtonSize::Large)  == 40);
}

TEST_CASE("ButtonVariant ordering") {
    CHECK(std::to_underlying(ButtonVariant::Primary)   == 0);
    CHECK(std::to_underlying(ButtonVariant::Secondary)  == 1);
    CHECK(std::to_underlying(ButtonVariant::Ghost)      == 2);
    CHECK(std::to_underlying(ButtonVariant::Danger)     == 3);
}

// -- T03.1.2 NyanToolButton ------------------------------------------------

TEST_CASE("FlyoutPolicy ordering") {
    CHECK(std::to_underlying(FlyoutPolicy::Simple)     == 0);
    CHECK(std::to_underlying(FlyoutPolicy::LastUsed)   == 1);
    CHECK(std::to_underlying(FlyoutPolicy::MostCommon) == 2);
}

// -- T03.2.1 NyanLineEdit --------------------------------------------------

TEST_CASE("InputMode ordering") {
    CHECK(std::to_underlying(InputMode::Text)    == 0);
    CHECK(std::to_underlying(InputMode::Integer) == 1);
    CHECK(std::to_underlying(InputMode::Double)  == 2);
}

TEST_CASE("LineEditBorder ordering") {
    CHECK(std::to_underlying(LineEditBorder::Full)    == 0);
    CHECK(std::to_underlying(LineEditBorder::Minimal) == 1);
}

// -- T03.5.1 NyanLabel ------------------------------------------------------

TEST_CASE("LabelRole to FontRole mapping") {
    CHECK(ToFontRole(LabelRole::Title)   == FontRole::Heading);
    CHECK(ToFontRole(LabelRole::Name)    == FontRole::BodyMedium);
    CHECK(ToFontRole(LabelRole::Body)    == FontRole::Body);
    CHECK(ToFontRole(LabelRole::Caption) == FontRole::Caption);
}

// -- T03.5.2 NyanBadge -----------------------------------------------------

TEST_CASE("BadgeVariant completeness") {
    CHECK(std::to_underlying(BadgeVariant::Success) == 0);
    CHECK(std::to_underlying(BadgeVariant::Warning) == 1);
    CHECK(std::to_underlying(BadgeVariant::Error)   == 2);
    CHECK(std::to_underlying(BadgeVariant::Info)    == 3);
    CHECK(std::to_underlying(BadgeVariant::Neutral) == 4);
    CHECK(std::to_underlying(BadgeVariant::Custom)  == 5);
}

// -- T03.7.1 NyanLegend ----------------------------------------------------

TEST_CASE("LegendItem default values") {
    LegendItem item;
    CHECK(item.name.isEmpty());
    CHECK(item.flag  == "<");
    CHECK(item.value == "0");
    CHECK_FALSE(item.color.isValid());
}

// -- T03.8.1 NyanPopConfirm ------------------------------------------------

TEST_CASE("PopConfirmState ordering") {
    CHECK(std::to_underlying(PopConfirmState::Info)     == 0);
    CHECK(std::to_underlying(PopConfirmState::Question) == 1);
    CHECK(std::to_underlying(PopConfirmState::Warn)     == 2);
    CHECK(std::to_underlying(PopConfirmState::Error)    == 3);
}

TEST_CASE("PopConfirmCode ordering") {
    CHECK(std::to_underlying(PopConfirmCode::ConfirmCode) == 0);
    CHECK(std::to_underlying(PopConfirmCode::DenyCode)    == 1);
    CHECK(std::to_underlying(PopConfirmCode::CancelCode)  == 2);
}

// -- T03.7.2 NyanDateTimePicker ---------------------------------------------

TEST_CASE("DateTimeMode ordering") {
    CHECK(std::to_underlying(DateTimeMode::Date)     == 0);
    CHECK(std::to_underlying(DateTimeMode::Time)     == 1);
    CHECK(std::to_underlying(DateTimeMode::DateTime) == 2);
}

// -- T03.3.2 SearchMode -----------------------------------------------------

TEST_CASE("SearchMode ordering") {
    CHECK(std::to_underlying(SearchMode::Instant) == 0);
    CHECK(std::to_underlying(SearchMode::OnEnter) == 1);
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
