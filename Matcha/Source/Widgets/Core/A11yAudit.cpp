#include "Matcha/Widgets/Core/A11yAudit.h"

#include "Matcha/UiNodes/Core/MnemonicManager.h"
#include "Matcha/UiNodes/Core/UiNode.h"
#include "Matcha/UiNodes/Core/WidgetNode.h"
#include "Matcha/Widgets/Core/MnemonicState.h"

#include <unordered_map>

namespace matcha::gui {

auto A11yAudit::IsInteractiveRole(fw::A11yRole role) -> bool
{
    switch (role) {
    case fw::A11yRole::Button:
    case fw::A11yRole::CheckBox:
    case fw::A11yRole::RadioButton:
    case fw::A11yRole::Slider:
    case fw::A11yRole::SpinBox:
    case fw::A11yRole::ComboBox:
    case fw::A11yRole::LineEdit:
    case fw::A11yRole::Toggle:
    case fw::A11yRole::Tab:
    case fw::A11yRole::MenuItem:
    case fw::A11yRole::TreeItem:
    case fw::A11yRole::ListItem:
    case fw::A11yRole::TableCell:
        return true;
    default:
        return false;
    }
}

auto A11yAudit::AuditWidget(fw::WidgetNode* widget) -> std::vector<A11yViolation>
{
    std::vector<A11yViolation> violations;
    if (widget == nullptr) {
        return violations;
    }

    const auto role = widget->GetA11yRole();

    // Rule: Interactive widgets must have a non-None A11yRole
    if (role == fw::A11yRole::None && widget->IsFocusable()) {
        violations.push_back({
            widget,
            A11ySeverity::Error,
            "a11y.role.missing",
            "Focusable widget has no A11yRole set"
        });
    }

    // Rule: Interactive roles must have a non-empty accessible name
    if (IsInteractiveRole(role) && widget->AccessibleName().empty()) {
        violations.push_back({
            widget,
            A11ySeverity::Error,
            "a11y.name.missing",
            "Interactive widget (role=" + std::to_string(static_cast<int>(role))
                + ") has empty accessible name"
        });
    }

    // Rule: Interactive widgets should be keyboard-focusable
    if (IsInteractiveRole(role) && !widget->IsFocusable()) {
        violations.push_back({
            widget,
            A11ySeverity::Warning,
            "a11y.focus.unreachable",
            "Interactive widget has A11yRole but IsFocusable() is false"
        });
    }

    // Rule: Interactive widgets should have a HelpId
    if (IsInteractiveRole(role) && widget->HelpId().empty()) {
        violations.push_back({
            widget,
            A11ySeverity::Info,
            "a11y.helpid.missing",
            "Interactive widget has no HelpId set"
        });
    }

    return violations;
}

auto A11yAudit::Audit(fw::UiNode* root) -> std::vector<A11yViolation>
{
    std::vector<A11yViolation> all;
    if (root == nullptr) {
        return all;
    }

    // Collect per-widget violations + mnemonic characters for duplicate detection
    std::unordered_map<char16_t, std::vector<fw::WidgetNode*>> mnemonicMap;

    root->TraverseDepthFirst([&](fw::UiNode* node) {
        auto* wn = dynamic_cast<fw::WidgetNode*>(node);
        if (wn != nullptr) {
            auto v = AuditWidget(wn);
            all.insert(all.end(), v.begin(), v.end());

            // Collect mnemonic characters from accessible names
            auto name = wn->AccessibleName();
            if (!name.empty()) {
                auto parsed = gui::MnemonicState::Parse(QString::fromStdString(name));
                if (!parsed.mnemonicChar.isNull()) {
                    auto ch = parsed.mnemonicChar.toUpper().unicode();
                    mnemonicMap[ch].push_back(wn);
                }
            }
        }
    });

    // Rule: mnemonic.duplicate — same mnemonic character on multiple widgets
    for (const auto& [ch, widgets] : mnemonicMap) {
        if (widgets.size() > 1) {
            for (auto* wn : widgets) {
                all.push_back({
                    wn,
                    A11ySeverity::Warning,
                    "mnemonic.duplicate",
                    "Mnemonic '" + std::string(1, static_cast<char>(ch))
                        + "' is used by " + std::to_string(widgets.size()) + " widgets"
                });
            }
        }
    }

    return all;
}

} // namespace matcha::gui
