#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include "QtAppGuard.h"

#include <Matcha/UiNodes/Core/A11yRole.h>
#include <Matcha/UiNodes/Core/WidgetNode.h>
#include <Matcha/Widgets/Core/A11yAudit.h>
#include <Matcha/Widgets/Core/ContrastChecker.h>

#include <QColor>
#include <QWidget>

#include <cmath>
#include <string>
#include <utility>

using namespace matcha::fw;
using namespace matcha::gui;

// ===========================================================================
// Minimal concrete WidgetNode for testing
// ===========================================================================

namespace {

class StubWidget final : public WidgetNode {
public:
    explicit StubWidget(std::string id)
        : WidgetNode(std::move(id), NodeType::Custom) {}
protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override
    {
        return new QWidget(parent); // NOLINT
    }
};

} // anonymous namespace

// ===========================================================================
// D1: ContrastChecker
// ===========================================================================

TEST_SUITE("PhaseD::ContrastChecker") {

TEST_CASE("Black on white is 21:1") {
    const double ratio = ContrastChecker::Ratio(QColor(0, 0, 0), QColor(255, 255, 255));
    CHECK(ratio == doctest::Approx(21.0).epsilon(0.01));
}

TEST_CASE("White on white is 1:1") {
    const double ratio = ContrastChecker::Ratio(QColor(255, 255, 255), QColor(255, 255, 255));
    CHECK(ratio == doctest::Approx(1.0).epsilon(0.01));
}

TEST_CASE("Symmetric: Ratio(a,b) == Ratio(b,a)") {
    const QColor a(100, 50, 200);
    const QColor b(220, 230, 240);
    CHECK(ContrastChecker::Ratio(a, b) == doctest::Approx(ContrastChecker::Ratio(b, a)));
}

TEST_CASE("MeetsAA: black on white passes") {
    CHECK(ContrastChecker::MeetsAA(QColor(0, 0, 0), QColor(255, 255, 255)));
}

TEST_CASE("MeetsAA: light gray on white fails") {
    // #ccc on white: ratio ~1.6
    CHECK_FALSE(ContrastChecker::MeetsAA(QColor(204, 204, 204), QColor(255, 255, 255)));
}

TEST_CASE("MeetsAAA: black on white passes") {
    CHECK(ContrastChecker::MeetsAAA(QColor(0, 0, 0), QColor(255, 255, 255)));
}

TEST_CASE("MeetsAAA: dark gray on white may fail") {
    // #767676 on white: ratio ~4.54 — passes AA but not AAA
    CHECK_FALSE(ContrastChecker::MeetsAAA(QColor(0x76, 0x76, 0x76), QColor(255, 255, 255)));
}

TEST_CASE("MeetsAALargeText: 3:1 threshold") {
    CHECK(ContrastChecker::MeetsAALargeText(QColor(0, 0, 0), QColor(255, 255, 255)));
    // #aaaaaa on white: ratio ~2.3 — fails large text AA
    CHECK_FALSE(ContrastChecker::MeetsAALargeText(QColor(0xAA, 0xAA, 0xAA), QColor(255, 255, 255)));
}

TEST_CASE("RelativeLuminance: black is 0, white is 1") {
    CHECK(ContrastChecker::RelativeLuminance(QColor(0, 0, 0)) == doctest::Approx(0.0).epsilon(0.001));
    CHECK(ContrastChecker::RelativeLuminance(QColor(255, 255, 255)) == doctest::Approx(1.0).epsilon(0.001));
}

TEST_CASE("RelativeLuminance: pure red") {
    // L(red) = 0.2126
    CHECK(ContrastChecker::RelativeLuminance(QColor(255, 0, 0)) == doctest::Approx(0.2126).epsilon(0.001));
}

TEST_CASE("SuggestFix: already passing returns same color") {
    const QColor fg(0, 0, 0);
    const QColor bg(255, 255, 255);
    const QColor fixed = ContrastChecker::SuggestFix(fg, bg, 4.5);
    CHECK(fixed == fg);
}

TEST_CASE("SuggestFix: failing pair gets fixed") {
    const QColor fg(200, 200, 200); // light gray
    const QColor bg(255, 255, 255); // white
    const QColor fixed = ContrastChecker::SuggestFix(fg, bg, 4.5);
    CHECK(ContrastChecker::MeetsAA(fixed, bg));
}

TEST_CASE("SuggestFix: dark background gets lightened fg") {
    const QColor fg(50, 50, 50);   // dark gray
    const QColor bg(30, 30, 30);   // very dark
    const QColor fixed = ContrastChecker::SuggestFix(fg, bg, 4.5);
    CHECK(ContrastChecker::MeetsAA(fixed, bg));
}

} // TEST_SUITE ContrastChecker

// ===========================================================================
// D1: A11yRole enum
// ===========================================================================

TEST_SUITE("PhaseD::A11yRole") {

TEST_CASE("A11yRole::None is 0") {
    CHECK(std::to_underlying(A11yRole::None) == 0);
}

TEST_CASE("kA11yRoleCount matches Count_") {
    CHECK(kA11yRoleCount == static_cast<std::size_t>(A11yRole::Count_));
    CHECK(kA11yRoleCount > 20); // sanity: we defined 20+ roles
}

TEST_CASE("Interactive roles include Button, CheckBox, Slider") {
    CHECK(A11yAudit::IsInteractiveRole(A11yRole::Button));
    CHECK(A11yAudit::IsInteractiveRole(A11yRole::CheckBox));
    CHECK(A11yAudit::IsInteractiveRole(A11yRole::Slider));
    CHECK(A11yAudit::IsInteractiveRole(A11yRole::LineEdit));
}

TEST_CASE("Non-interactive roles: Dialog, Tooltip, Status") {
    CHECK_FALSE(A11yAudit::IsInteractiveRole(A11yRole::Dialog));
    CHECK_FALSE(A11yAudit::IsInteractiveRole(A11yRole::Tooltip));
    CHECK_FALSE(A11yAudit::IsInteractiveRole(A11yRole::Status));
    CHECK_FALSE(A11yAudit::IsInteractiveRole(A11yRole::None));
}

} // TEST_SUITE A11yRole

// ===========================================================================
// D1: WidgetNode accessibility properties
// ===========================================================================

TEST_SUITE("PhaseD::WidgetNodeA11y") {

TEST_CASE("Default A11yRole is None") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("test");
    CHECK(node.GetA11yRole() == A11yRole::None);
}

TEST_CASE("SetA11yRole / GetA11yRole") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("test");
    node.SetA11yRole(A11yRole::Button);
    CHECK(node.GetA11yRole() == A11yRole::Button);
}

TEST_CASE("Default AccessibleName is empty") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("test");
    CHECK(node.AccessibleName().empty());
}

TEST_CASE("SetAccessibleName / AccessibleName") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("test");
    node.SetAccessibleName("Save Button");
    CHECK(node.AccessibleName() == "Save Button");
}

} // TEST_SUITE WidgetNodeA11y

// ===========================================================================
// D3: A11yAudit
// ===========================================================================

TEST_SUITE("PhaseD::A11yAudit") {

TEST_CASE("Clean widget passes audit") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("btn");
    node.SetA11yRole(A11yRole::Button);
    node.SetAccessibleName("OK");
    node.SetFocusable(true);
    node.SetHelpId("com.test.ok");

    auto violations = A11yAudit::AuditWidget(&node);
    CHECK(violations.empty());
}

TEST_CASE("Interactive role without accessible name -> error") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("btn");
    node.SetA11yRole(A11yRole::Button);
    node.SetFocusable(true);
    // No accessible name set

    auto violations = A11yAudit::AuditWidget(&node);
    bool found = false;
    for (const auto& v : violations) {
        if (v.rule == "a11y.name.missing") { found = true; }
    }
    CHECK(found);
}

TEST_CASE("Interactive role not focusable -> warning") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("btn");
    node.SetA11yRole(A11yRole::CheckBox);
    node.SetAccessibleName("Accept Terms");
    // IsFocusable not set

    auto violations = A11yAudit::AuditWidget(&node);
    bool found = false;
    for (const auto& v : violations) {
        if (v.rule == "a11y.focus.unreachable") { found = true; }
    }
    CHECK(found);
}

TEST_CASE("Focusable widget with no role -> error") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("mystery");
    node.SetFocusable(true);
    // No A11yRole set

    auto violations = A11yAudit::AuditWidget(&node);
    bool found = false;
    for (const auto& v : violations) {
        if (v.rule == "a11y.role.missing") { found = true; }
    }
    CHECK(found);
}

TEST_CASE("Interactive role without HelpId -> info") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget node("btn");
    node.SetA11yRole(A11yRole::Button);
    node.SetAccessibleName("Cancel");
    node.SetFocusable(true);
    // No HelpId set

    auto violations = A11yAudit::AuditWidget(&node);
    bool found = false;
    for (const auto& v : violations) {
        if (v.rule == "a11y.helpid.missing") { found = true; }
    }
    CHECK(found);
}

TEST_CASE("Audit subtree collects all violations") {
    matcha::test::QtAppGuard::Ensure();
    StubWidget root("root");
    auto a = std::make_unique<StubWidget>("a");
    auto b = std::make_unique<StubWidget>("b");

    a->SetA11yRole(A11yRole::Button);
    a->SetFocusable(true);
    // a: missing name + missing helpid

    b->SetA11yRole(A11yRole::Slider);
    b->SetAccessibleName("Volume");
    b->SetFocusable(true);
    // b: missing helpid only

    root.AddNode(std::move(a));
    root.AddNode(std::move(b));

    auto violations = A11yAudit::Audit(&root);
    CHECK(violations.size() >= 2); // at least name.missing + helpid.missing
}

TEST_CASE("Audit nullptr returns empty") {
    auto violations = A11yAudit::Audit(nullptr);
    CHECK(violations.empty());
}

TEST_CASE("AuditWidget nullptr returns empty") {
    auto violations = A11yAudit::AuditWidget(nullptr);
    CHECK(violations.empty());
}

} // TEST_SUITE A11yAudit

// ===========================================================================
// D7 — InteractionFSM Migration Verification
// ===========================================================================

#include "../../../Source/Widgets/Core/InteractionEventFilter.h"

#include <Matcha/Widgets/Core/NyanTheme.h>

#include <Matcha/Widgets/Controls/NyanPushButton.h>
#include <Matcha/Widgets/Controls/NyanCheckBox.h>
#include <Matcha/Widgets/Controls/NyanRadioButton.h>
#include <Matcha/Widgets/Controls/NyanComboBox.h>
#include <Matcha/Widgets/Controls/NyanToggleSwitch.h>
#include <Matcha/Widgets/Controls/NyanLineEdit.h>
#include <Matcha/Widgets/Controls/NyanSpinBox.h>
#include <Matcha/Widgets/Controls/NyanDoubleSpinBox.h>
#include <Matcha/Widgets/Controls/NyanSlider.h>
#include <Matcha/Widgets/Controls/NyanToolButton.h>
#include <Matcha/Widgets/Controls/NyanSearchBox.h>
#include <Matcha/Widgets/Controls/NyanSelectionInput.h>
#include <Matcha/Widgets/Controls/NyanRangeSlider.h>
#include <Matcha/Widgets/Controls/NyanColorPicker.h>
#include <Matcha/Widgets/Controls/NyanDateTimePicker.h>
#include <Matcha/Widgets/Controls/NyanCascader.h>
#include <Matcha/Widgets/Controls/NyanTransfer.h>
#include <Matcha/Widgets/Controls/NyanColorSwatch.h>
#include <Matcha/Widgets/Controls/NyanGroupBox.h>
#include <Matcha/Widgets/Controls/NyanCollapsibleSection.h>
#include <Matcha/Widgets/Controls/NyanDataTable.h>
#include <Matcha/Widgets/Controls/NyanListWidget.h>
#include <Matcha/Widgets/Controls/NyanTableWidget.h>
#include <Matcha/Widgets/Controls/NyanStructureTree.h>
#include <Matcha/Widgets/Controls/NyanPropertyGrid.h>
#include <Matcha/Widgets/Controls/NyanFormLayout.h>
#include <Matcha/Widgets/Controls/NyanLabel.h>
#include <Matcha/Widgets/Controls/NyanTag.h>
#include <Matcha/Widgets/Controls/NyanBadge.h>
#include <Matcha/Widgets/Controls/NyanAlert.h>
#include <Matcha/Widgets/Controls/NyanMessage.h>
#include <Matcha/Widgets/Controls/NyanNotification.h>
#include <Matcha/Widgets/Controls/NyanProgressBar.h>
#include <Matcha/Widgets/Controls/NyanProgressRing.h>
#include <Matcha/Widgets/Controls/NyanAvatar.h>
#include <Matcha/Widgets/Controls/NyanBreadcrumb.h>
#include <Matcha/Widgets/Controls/NyanLegend.h>
#include <Matcha/Widgets/Controls/NyanLegendSlider.h>
#include <Matcha/Widgets/Controls/NyanLine.h>
#include <Matcha/Widgets/Controls/NyanPaginator.h>
#include <Matcha/Widgets/Controls/NyanRichTooltip.h>

TEST_SUITE("D7_InteractionFSM_Migration") {

/// Helper: check that a widget has exactly one InteractionEventFilter child.
template <typename W>
void CheckHasFilter(const char* name)
{
    matcha::test::QtAppGuard::Ensure();
    static NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    static bool inited = false;
    if (!inited) { theme.SetTheme(kThemeLight); SetThemeService(&theme); inited = true; }
    W widget;
    auto* filter = widget.template findChild<InteractionEventFilter*>();
    INFO("Widget: ", name);
    REQUIRE(filter != nullptr);
    CHECK(filter->Fsm().CurrentState() == matcha::fw::InteractionState::Normal);
}

TEST_CASE("PushButton has InteractionEventFilter")       { CheckHasFilter<NyanPushButton>("PushButton"); }
TEST_CASE("CheckBox has InteractionEventFilter")         { CheckHasFilter<NyanCheckBox>("CheckBox"); }
TEST_CASE("RadioButton has InteractionEventFilter")      { CheckHasFilter<NyanRadioButton>("RadioButton"); }
TEST_CASE("ComboBox has InteractionEventFilter")         { CheckHasFilter<NyanComboBox>("ComboBox"); }
TEST_CASE("ToggleSwitch has InteractionEventFilter")     { CheckHasFilter<NyanToggleSwitch>("ToggleSwitch"); }
TEST_CASE("LineEdit has InteractionEventFilter")         { CheckHasFilter<NyanLineEdit>("LineEdit"); }
TEST_CASE("SpinBox has InteractionEventFilter")          { CheckHasFilter<NyanSpinBox>("SpinBox"); }
TEST_CASE("DoubleSpinBox has InteractionEventFilter")    { CheckHasFilter<NyanDoubleSpinBox>("DoubleSpinBox"); }
TEST_CASE("Slider has InteractionEventFilter")           { CheckHasFilter<NyanSlider>("Slider"); }
TEST_CASE("ToolButton has InteractionEventFilter")       { CheckHasFilter<NyanToolButton>("ToolButton"); }
TEST_CASE("SearchBox has InteractionEventFilter")        { CheckHasFilter<NyanSearchBox>("SearchBox"); }
TEST_CASE("SelectionInput has InteractionEventFilter")   { CheckHasFilter<NyanSelectionInput>("SelectionInput"); }
TEST_CASE("RangeSlider has InteractionEventFilter")      { CheckHasFilter<NyanRangeSlider>("RangeSlider"); }
TEST_CASE("ColorPicker has InteractionEventFilter")      { CheckHasFilter<NyanColorPicker>("ColorPicker"); }
TEST_CASE("DateTimePicker has InteractionEventFilter")   { CheckHasFilter<NyanDateTimePicker>("DateTimePicker"); }
TEST_CASE("Cascader has InteractionEventFilter")         { CheckHasFilter<NyanCascader>("Cascader"); }
TEST_CASE("Transfer has InteractionEventFilter")         { CheckHasFilter<NyanTransfer>("Transfer"); }
TEST_CASE("ColorSwatch has InteractionEventFilter")      { CheckHasFilter<NyanColorSwatch>("ColorSwatch"); }
TEST_CASE("GroupBox has InteractionEventFilter")         { CheckHasFilter<NyanGroupBox>("GroupBox"); }
TEST_CASE("CollapsibleSection has InteractionEventFilter") { CheckHasFilter<NyanCollapsibleSection>("CollapsibleSection"); }
TEST_CASE("DataTable has InteractionEventFilter")        { CheckHasFilter<NyanDataTable>("DataTable"); }
TEST_CASE("ListWidget has InteractionEventFilter")       { CheckHasFilter<NyanListWidget>("ListWidget"); }
TEST_CASE("TableWidget has InteractionEventFilter")      { CheckHasFilter<NyanTableWidget>("TableWidget"); }
TEST_CASE("StructureTree has InteractionEventFilter")    { CheckHasFilter<NyanStructureTree>("StructureTree"); }
TEST_CASE("PropertyGrid has InteractionEventFilter")     { CheckHasFilter<NyanPropertyGrid>("PropertyGrid"); }
TEST_CASE("FormLayout has InteractionEventFilter")       { CheckHasFilter<NyanFormLayout>("FormLayout"); }
TEST_CASE("Label has InteractionEventFilter")            { CheckHasFilter<NyanLabel>("Label"); }
TEST_CASE("Tag has InteractionEventFilter")              { CheckHasFilter<NyanTag>("Tag"); }
TEST_CASE("Badge has InteractionEventFilter")            { CheckHasFilter<NyanBadge>("Badge"); }
TEST_CASE("Alert has InteractionEventFilter")            { CheckHasFilter<NyanAlert>("Alert"); }
TEST_CASE("Message has InteractionEventFilter")          { CheckHasFilter<NyanMessage>("Message"); }
TEST_CASE("Notification has InteractionEventFilter")     { CheckHasFilter<NyanNotification>("Notification"); }
TEST_CASE("ProgressBar has InteractionEventFilter")      { CheckHasFilter<NyanProgressBar>("ProgressBar"); }
TEST_CASE("ProgressRing has InteractionEventFilter")     { CheckHasFilter<NyanProgressRing>("ProgressRing"); }
TEST_CASE("Avatar has InteractionEventFilter")           { CheckHasFilter<NyanAvatar>("Avatar"); }
TEST_CASE("Breadcrumb has InteractionEventFilter")       { CheckHasFilter<NyanBreadcrumb>("Breadcrumb"); }
TEST_CASE("Legend has InteractionEventFilter")           { CheckHasFilter<NyanLegend>("Legend"); }
TEST_CASE("LegendSlider has InteractionEventFilter")     { CheckHasFilter<NyanLegendSlider>("LegendSlider"); }
TEST_CASE("Line has InteractionEventFilter")             { CheckHasFilter<NyanLine>("Line"); }
TEST_CASE("Paginator has InteractionEventFilter")        { CheckHasFilter<NyanPaginator>("Paginator"); }
TEST_CASE("RichTooltip has InteractionEventFilter")      { CheckHasFilter<NyanRichTooltip>("RichTooltip"); }

} // TEST_SUITE D7_InteractionFSM_Migration

#ifdef __clang__
#pragma clang diagnostic pop
#endif
