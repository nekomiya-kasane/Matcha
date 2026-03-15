/**
 * @file ApplicationWidgetEnumTest.cpp
 * @brief Unit tests for Phase 3c application widget enums and logic.
 */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Widgets/ActionBar/NyanActionBar.h>
#include <Matcha/Widgets/ActionBar/NyanActionTab.h>
#include <Matcha/Widgets/ActionBar/NyanActionToolbar.h>
#include <Matcha/Widgets/Menu/NyanDialog.h>
#include <Matcha/Widgets/Menu/NyanDialogTitleBar.h>
#include <Matcha/Widgets/Menu/NyanDialogFootBar.h>
#include <Matcha/Widgets/Controls/NyanNotification.h>
#include <Matcha/Widgets/Controls/NyanSelectionInput.h>

#include <utility>

using namespace matcha::gui;

// ============================================================================
// Enum Completeness Tests
// ============================================================================

TEST_SUITE("ApplicationWidgetEnums") {

TEST_CASE("ActionBarDisplayMode enum values") {
    CHECK(std::to_underlying(ActionBarDisplayMode::IconOnly) == 0);
    CHECK(std::to_underlying(ActionBarDisplayMode::IconText) == 1);
    CHECK(std::to_underlying(ActionBarDisplayMode::WideSpacing) == 2);
}

TEST_CASE("DockSide enum values") {
    CHECK(std::to_underlying(DockSide::Bottom) == 0);
    CHECK(std::to_underlying(DockSide::Left) == 1);
    CHECK(std::to_underlying(DockSide::Right) == 2);
    CHECK(std::to_underlying(DockSide::Top) == 3);
}

TEST_CASE("TabPersistence enum values") {
    CHECK(std::to_underlying(TabPersistence::General) == 0);
    CHECK(std::to_underlying(TabPersistence::Extension) == 1);
    CHECK(std::to_underlying(TabPersistence::Workshop) == 2);
}

TEST_CASE("DialogModality enum values") {
    CHECK(std::to_underlying(DialogModality::Modal) == 0);
    CHECK(std::to_underlying(DialogModality::SemiModal) == 1);
    CHECK(std::to_underlying(DialogModality::Modeless) == 2);
}

TEST_CASE("DialogResult enum values") {
    CHECK(std::to_underlying(DialogResult::Accepted) == 0);
    CHECK(std::to_underlying(DialogResult::Rejected) == 1);
    CHECK(std::to_underlying(DialogResult::Cancelled) == 2);
}

TEST_CASE("TitleBarButton bitflag values") {
    CHECK(std::to_underlying(TitleBarButton::None) == 0);
    CHECK(std::to_underlying(TitleBarButton::Minimize) == 1);
    CHECK(std::to_underlying(TitleBarButton::Maximize) == 2);
    CHECK(std::to_underlying(TitleBarButton::Close) == 4);
    CHECK(std::to_underlying(TitleBarButton::All) == 7);
}

TEST_CASE("TitleBarButton bitwise operations") {
    auto combined = TitleBarButton::Minimize | TitleBarButton::Close;
    CHECK(std::to_underlying(combined) == 5);

    auto masked = combined & TitleBarButton::Minimize;
    CHECK(masked == TitleBarButton::Minimize);

    auto inverted = ~TitleBarButton::None;
    CHECK(std::to_underlying(inverted) == 255);
}

TEST_CASE("NotificationType enum values") {
    CHECK(std::to_underlying(NotificationType::Info) == 0);
    CHECK(std::to_underlying(NotificationType::Success) == 1);
    CHECK(std::to_underlying(NotificationType::Warning) == 2);
    CHECK(std::to_underlying(NotificationType::Error) == 3);
}

TEST_CASE("PickMode enum values") {
    CHECK(std::to_underlying(PickMode::Disabled) == 0);
    CHECK(std::to_underlying(PickMode::Single) == 1);
    CHECK(std::to_underlying(PickMode::Multiple) == 2);
}

TEST_CASE("DialogButtonCode enum values") {
    CHECK(std::to_underlying(DialogButtonCode::Confirm) == 0);
    CHECK(std::to_underlying(DialogButtonCode::Apply) == 1);
    CHECK(std::to_underlying(DialogButtonCode::Cancel) == 2);
}

} // TEST_SUITE("ApplicationWidgetEnums")

// ============================================================================
// ActionBar Logic Tests
// ============================================================================

TEST_SUITE("ActionBarLogic") {

TEST_CASE("ActionButtonInfo default construction") {
    ActionButtonInfo info;
    CHECK(info.id.isEmpty());
    CHECK(info.text.isEmpty());
    CHECK(info.icon.isNull());
    CHECK(info.tooltip.isEmpty());
    CHECK(info.checkable == false);
}

TEST_CASE("ActionButtonInfo with values") {
    ActionButtonInfo info;
    info.id = "btn_save";
    info.text = "Save";
    info.tooltip = "Save file";
    info.checkable = true;

    CHECK(info.id == "btn_save");
    CHECK(info.text == "Save");
    CHECK(info.tooltip == "Save file");
    CHECK(info.checkable == true);
}

} // TEST_SUITE("ActionBarLogic")
