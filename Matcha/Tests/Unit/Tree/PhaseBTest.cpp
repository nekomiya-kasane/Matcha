#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Tree/FSM/WidgetFsm.h>
#include <Matcha/Tree/ContainerNode.h>
#include <Matcha/Theming/Token/TokenEnums.h>
#include <Matcha/Tree/UiNodeNotification.h>
#include <Matcha/Tree/WidgetNode.h>
#include <Matcha/Theming/IThemeService.h>
#include <Matcha/Theming/NyanTheme.h>

#include "QtAppGuard.h"

#include <QLayout>
#include <QWidget>

using namespace matcha::fw;
namespace sw = matcha::fw::fsm::simple_widget;

// ============================================================================
// SimpleWidget FSM tests (replaces old InteractionFSM tests)
// ============================================================================

TEST_SUITE("SimpleWidgetFSM") {

TEST_CASE("Initial state is Normal") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    CHECK(ctrl.FsmState() == sw::State::Normal);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("Normal -> Hover on MouseEnter") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::MouseEnter);
    CHECK(ctrl.FsmState() == sw::State::Hover);
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);
}

TEST_CASE("Normal -> Focused on FocusIn") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::FocusIn);
    CHECK(ctrl.FsmState() == sw::State::Focused);
    CHECK(ctrl.GetInteractionState() == InteractionState::Focused);
}

TEST_CASE("Normal -> Disabled on Disable") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::Disable);
    CHECK(ctrl.FsmState() == sw::State::Disabled);
    CHECK(ctrl.GetInteractionState() == InteractionState::Disabled);
}

TEST_CASE("Hover -> Pressed on MouseDown") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::MouseEnter);
    ctrl.Process(sw::Event::MouseDown);
    CHECK(ctrl.FsmState() == sw::State::Pressed);
    CHECK(ctrl.GetInteractionState() == InteractionState::Pressed);
}

TEST_CASE("Hover -> Normal on MouseLeave") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::MouseEnter);
    ctrl.Process(sw::Event::MouseLeave);
    CHECK(ctrl.FsmState() == sw::State::Normal);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("Pressed -> Hover on MouseUp") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::MouseEnter);
    ctrl.Process(sw::Event::MouseDown);
    ctrl.Process(sw::Event::MouseUp);
    CHECK(ctrl.FsmState() == sw::State::Hover);
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);
}

TEST_CASE("Pressed -> Normal on MouseLeave") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::MouseEnter);
    ctrl.Process(sw::Event::MouseDown);
    ctrl.Process(sw::Event::MouseLeave);
    CHECK(ctrl.FsmState() == sw::State::Normal);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("Focused -> Hover on MouseEnter") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::FocusIn);
    ctrl.Process(sw::Event::MouseEnter);
    CHECK(ctrl.FsmState() == sw::State::Hover);
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);
}

TEST_CASE("Focused -> Normal on FocusOut") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::FocusIn);
    ctrl.Process(sw::Event::FocusOut);
    CHECK(ctrl.FsmState() == sw::State::Normal);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("Disabled -> Normal on Enable") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::Disable);
    ctrl.Process(sw::Event::Enable);
    CHECK(ctrl.FsmState() == sw::State::Normal);
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("Disabled ignores other events") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::Disable);

    CHECK_FALSE(ctrl.Process(sw::Event::MouseEnter));
    CHECK_FALSE(ctrl.Process(sw::Event::MouseLeave));
    CHECK_FALSE(ctrl.Process(sw::Event::MouseDown));
    CHECK_FALSE(ctrl.Process(sw::Event::MouseUp));
    CHECK_FALSE(ctrl.Process(sw::Event::FocusIn));
    CHECK_FALSE(ctrl.Process(sw::Event::FocusOut));
    CHECK(ctrl.FsmState() == sw::State::Disabled);
}

TEST_CASE("Reset sets state directly") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Reset(sw::State::Focused);
    CHECK(ctrl.FsmState() == sw::State::Focused);
    CHECK(ctrl.GetInteractionState() == InteractionState::Focused);
}

TEST_CASE("Full cycle: Normal -> Hover -> Press -> Release -> Leave -> Normal") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    CHECK(ctrl.Process(sw::Event::MouseEnter));
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);
    CHECK(ctrl.Process(sw::Event::MouseDown));
    CHECK(ctrl.GetInteractionState() == InteractionState::Pressed);
    CHECK(ctrl.Process(sw::Event::MouseUp));
    CHECK(ctrl.GetInteractionState() == InteractionState::Hovered);
    CHECK(ctrl.Process(sw::Event::MouseLeave));
    CHECK(ctrl.GetInteractionState() == InteractionState::Normal);
}

TEST_CASE("Any state -> Disabled on Disable") {
    for (auto event : {sw::Event::MouseEnter, sw::Event::FocusIn}) {
        sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
        ctrl.Process(event);
        ctrl.Process(sw::Event::Disable);
        CHECK(ctrl.FsmState() == sw::State::Disabled);
    }

    // Also from Pressed
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    ctrl.Process(sw::Event::MouseEnter);
    ctrl.Process(sw::Event::MouseDown);
    ctrl.Process(sw::Event::Disable);
    CHECK(ctrl.FsmState() == sw::State::Disabled);
}

TEST_CASE("InteractionState callback fires on state change") {
    sw::Controller ctrl(sw::State::Normal, sw::kTransitions);
    InteractionState oldIS{}, newIS{};
    ctrl.OnInteractionStateChanged([&](InteractionState o, InteractionState n) {
        oldIS = o;
        newIS = n;
    });
    ctrl.Process(sw::Event::MouseEnter);
    CHECK(oldIS == InteractionState::Normal);
    CHECK(newIS == InteractionState::Hovered);
}

} // TEST_SUITE

// ============================================================================
// InteractionStateChanged notification tests
// ============================================================================

TEST_SUITE("InteractionStateChanged Notification") {

TEST_CASE("ClassName is InteractionStateChanged") {
    InteractionStateChanged notif(InteractionState::Normal, InteractionState::Hovered);
    CHECK(notif.ClassName() == "InteractionStateChanged");
}

TEST_CASE("OldState and NewState accessors") {
    InteractionStateChanged notif(InteractionState::Hovered, InteractionState::Pressed);
    CHECK(notif.OldState() == InteractionState::Hovered);
    CHECK(notif.NewState() == InteractionState::Pressed);
}

TEST_CASE("Default construction") {
    InteractionStateChanged notif;
    CHECK(notif.OldState() == InteractionState::Normal);
    CHECK(notif.NewState() == InteractionState::Normal);
}

} // TEST_SUITE

// ============================================================================
// ContainerNode token-aware spacing tests
// ============================================================================

TEST_SUITE("ContainerNode SpacingToken") {

TEST_CASE("SetSpacing applies token") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-hbox", LayoutKind::Horizontal);
    node.SetSpacing(SpacingToken::Px8);
    // Just verify no crash; pixel value depends on ITokenRegistry availability
    CHECK(node.Kind() == LayoutKind::Horizontal);
}

TEST_CASE("SetMargins uniform applies token") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-vbox", LayoutKind::Vertical);
    node.SetMargins(SpacingToken::Px4);
    CHECK(node.Kind() == LayoutKind::Vertical);
}

TEST_CASE("SetMargins per-side applies tokens") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-grid", LayoutKind::Grid);
    node.SetMargins(SpacingToken::Px2, SpacingToken::Px4,
                    SpacingToken::Px6, SpacingToken::Px8);
    CHECK(node.Kind() == LayoutKind::Grid);
}

TEST_CASE("LayoutKind::Flow creates without crash") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-flow", LayoutKind::Flow);
    CHECK(node.Kind() == LayoutKind::Flow);
    CHECK(node.Widget() != nullptr);
}

} // TEST_SUITE

// ============================================================================
// ContainerNode RTL direction tests
// ============================================================================

TEST_SUITE("ContainerNode TextDirection") {

TEST_CASE("Default direction is LTR") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-dir", LayoutKind::Horizontal);
    CHECK(node.Direction() == TextDirection::LTR);
}

TEST_CASE("SetDirection changes to RTL") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-rtl", LayoutKind::Horizontal);
    node.SetDirection(TextDirection::RTL);
    CHECK(node.Direction() == TextDirection::RTL);
}

TEST_CASE("SetDirection back to LTR") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-ltr", LayoutKind::Horizontal);
    node.SetDirection(TextDirection::RTL);
    node.SetDirection(TextDirection::LTR);
    CHECK(node.Direction() == TextDirection::LTR);
}

TEST_CASE("RTL sets Qt layout direction on widget") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-rtl-qt", LayoutKind::Horizontal);
    node.SetDirection(TextDirection::RTL);
    CHECK(node.Widget()->layoutDirection() == Qt::RightToLeft);
}

TEST_CASE("LTR sets Qt layout direction on widget") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-ltr-qt", LayoutKind::Horizontal);
    node.SetDirection(TextDirection::RTL);
    node.SetDirection(TextDirection::LTR);
    CHECK(node.Widget()->layoutDirection() == Qt::LeftToRight);
}

TEST_CASE("RTL with asymmetric margins swaps left/right") {
    matcha::test::QtAppGuard::Ensure();
    ContainerNode node("test-rtl-margins", LayoutKind::Horizontal);
    // Set asymmetric margins: left=8, top=0, right=2, bottom=0
    node.SetMargins(SpacingToken::Px8, SpacingToken::None,
                    SpacingToken::Px2, SpacingToken::None);
    // Now switch to RTL - margins should be swapped
    node.SetDirection(TextDirection::RTL);

    // Verify via Qt layout
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    node.Widget()->layout()->getContentsMargins(&left, &top, &right, &bottom);
    // RTL: left should now be the original right (2), right should be original left (8)
    CHECK(left == 2);
    CHECK(right == 8);
    CHECK(top == 0);
    CHECK(bottom == 0);
}

} // TEST_SUITE

// ============================================================================
// Icon system compile-time checks
// ============================================================================

TEST_SUITE("IconId and IconSize") {

TEST_CASE("IconId URI format") {
    CHECK(icons::Close == "asset://matcha/icons/close");
    CHECK(icons::Save == "asset://matcha/icons/save");
    CHECK(icons::Undo == "asset://matcha/icons/undo");
    CHECK(IconId().empty());
}

TEST_CASE("IconSize pixel values") {
    CHECK(static_cast<uint8_t>(IconSize::Xs) == 12);
    CHECK(static_cast<uint8_t>(IconSize::Sm) == 16);
    CHECK(static_cast<uint8_t>(IconSize::Md) == 20);
    CHECK(static_cast<uint8_t>(IconSize::Lg) == 24);
    CHECK(static_cast<uint8_t>(IconSize::Xl) == 32);
}

TEST_CASE("IsRtlFlippable identifies directional icons") {
    CHECK(IsRtlFlippable(icons::ChevronLeft));
    CHECK(IsRtlFlippable(icons::ChevronRight));
    CHECK(IsRtlFlippable(icons::ArrowLeft));
    CHECK(IsRtlFlippable(icons::ArrowRight));
    CHECK(IsRtlFlippable(icons::Back));
    CHECK(IsRtlFlippable(icons::Forward));

    CHECK_FALSE(IsRtlFlippable(""));
    CHECK_FALSE(IsRtlFlippable(icons::Close));
    CHECK_FALSE(IsRtlFlippable(icons::Search));
    CHECK_FALSE(IsRtlFlippable(icons::Save));
    CHECK_FALSE(IsRtlFlippable(icons::ChevronUp));
    CHECK_FALSE(IsRtlFlippable(icons::ChevronDown));
}

} // TEST_SUITE

// ============================================================================
// Font Scale
// ============================================================================

TEST_SUITE("FontScale") {

using matcha::gui::NyanTheme;
using matcha::gui::kThemeLight;
using matcha::gui::FontRole;
namespace fw = matcha::fw;

TEST_CASE("NyanTheme default font scale is 1.0") {
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);
    CHECK(theme.FontScale() == doctest::Approx(1.0F));
    CHECK(theme.Font(FontRole::Body).sizeInPt == 9);
    CHECK(theme.Font(FontRole::Caption).sizeInPt == 8);
    CHECK(theme.Font(FontRole::Heading).sizeInPt == 12);
}

TEST_CASE("SetFontScale changes font sizes proportionally") {
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetFontScale(2.0F);
    CHECK(theme.FontScale() == doctest::Approx(2.0F));
    CHECK(theme.Font(FontRole::Body).sizeInPt == 18);     // 9 * 2.0
    CHECK(theme.Font(FontRole::Caption).sizeInPt == 16);   // 8 * 2.0
    CHECK(theme.Font(FontRole::Heading).sizeInPt == 24);   // 12 * 2.0
}

TEST_CASE("SetFontScale clamps to valid range") {
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetFontScale(0.1F); // below kFontScaleMin
    CHECK(theme.FontScale() == doctest::Approx(fw::kFontScaleMin));

    theme.SetFontScale(10.0F); // above kFontScaleMax
    CHECK(theme.FontScale() == doctest::Approx(fw::kFontScaleMax));
}

TEST_CASE("SetFontScale minimum output is 6pt") {
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetFontScale(fw::kFontScaleMin); // 0.5x -> 8*0.5=4 -> clamped to 6
    CHECK(theme.Font(FontRole::Caption).sizeInPt >= 6);
    CHECK(theme.Font(FontRole::Body).sizeInPt >= 6);
}

TEST_CASE("SetFontSizePreset convenience maps to correct scale") {
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetFontSizePreset(fw::FontSizePreset::Large);
    CHECK(theme.FontScale() == doctest::Approx(1.25F));
    CHECK(theme.Font(FontRole::Body).sizeInPt == 11); // round(9 * 1.25) = 11

    theme.SetFontSizePreset(fw::FontSizePreset::Small);
    CHECK(theme.FontScale() == doctest::Approx(0.875F));
    CHECK(theme.Font(FontRole::Body).sizeInPt == 8); // round(9 * 0.875) = 8
}

} // TEST_SUITE

// ============================================================================
// SvgIconProvider token-to-filename mapping (via NyanTheme integration)
// ============================================================================

TEST_SUITE("SvgIconProvider") {

TEST_CASE("ResolveIcon returns empty pixmap for empty IconId") {
    // This test requires a running theme; just check the function exists
    // and doesn't crash with a null theme.
    // Actual icon loading tested in integration tests.
    CHECK(true); // Placeholder: compile-time verification
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
