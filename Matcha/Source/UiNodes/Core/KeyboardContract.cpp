/**
 * @file KeyboardContract.cpp
 * @brief Implementation of KeyboardContractRegistry and built-in contracts.
 */

#include <Matcha/Foundation/KeyboardContract.h>

namespace matcha::fw {

// ============================================================================
// KeyboardContractRegistry
// ============================================================================

void KeyboardContractRegistry::Register(WidgetKeyboardContract contract)
{
    const std::string key = contract.widgetKind;
    _contracts[key] = std::move(contract);
}

auto KeyboardContractRegistry::Get(std::string_view widgetKind) const
    -> const WidgetKeyboardContract*
{
    auto it = _contracts.find(std::string(widgetKind));
    if (it == _contracts.end()) {
        return nullptr;
    }
    return &it->second;
}

auto KeyboardContractRegistry::Has(std::string_view widgetKind) const -> bool
{
    return _contracts.contains(std::string(widgetKind));
}

auto KeyboardContractRegistry::RegisteredKinds() const -> std::vector<std::string_view>
{
    std::vector<std::string_view> result;
    result.reserve(_contracts.size());
    for (const auto& [key, contract] : _contracts) {
        result.emplace_back(key);
    }
    return result;
}

auto KeyboardContractRegistry::TotalBindingCount() const -> int
{
    int total = 0;
    for (const auto& [key, contract] : _contracts) {
        total += static_cast<int>(contract.bindings.size());
    }
    return total;
}

void KeyboardContractRegistry::Clear()
{
    _contracts.clear();
}

// ============================================================================
// Built-in contracts (§5.x)
// ============================================================================

auto BuildPushButtonContract() -> WidgetKeyboardContract
{
    return {
        .widgetKind = "PushButton",
        .bindings = {
            {"Space",        "Fire Activated on key-up",         "Focused"},
            {"Enter",        "Fire Activated on key-down",       "Focused"},
            {"Tab",          "Move focus to next widget",        "Focused"},
            {"Shift+Tab",    "Move focus to previous widget",    "Focused"},
            {"Alt+Mnemonic", "Focus + fire Activated",           "Mnemonic assigned"},
        },
    };
}

auto BuildLineEditContract() -> WidgetKeyboardContract
{
    return {
        .widgetKind = "LineEdit",
        .bindings = {
            {"Any printable", "Insert character at cursor",       "Focused"},
            {"Backspace",     "Delete char before cursor",        "Focused"},
            {"Delete",        "Delete char after cursor",         "Focused"},
            {"Left",          "Move cursor left",                 "Focused"},
            {"Right",         "Move cursor right",                "Focused"},
            {"Home",          "Move cursor to start",             "Focused"},
            {"End",           "Move cursor to end",               "Focused"},
            {"Ctrl+A",        "Select all",                       "Focused"},
            {"Ctrl+C",        "Copy selection",                   "Focused"},
            {"Ctrl+X",        "Cut selection",                    "Focused"},
            {"Ctrl+V",        "Paste from clipboard",             "Focused"},
            {"Ctrl+Z",        "Undo",                             "Focused"},
            {"Ctrl+Y",        "Redo",                             "Focused"},
            {"Enter",         "Fire ReturnPressed + EditingFinished", "Focused"},
            {"Escape",        "Revert to value at focus-enter",   "Focused, if configured"},
            {"Tab",           "Move focus out",                   "Focused"},
            {"Shift+Tab",     "Move focus out (backward)",        "Focused"},
        },
    };
}

auto BuildComboBoxContract() -> WidgetKeyboardContract
{
    return {
        .widgetKind = "ComboBox",
        .bindings = {
            {"Space",      "Open dropdown",                     "Closed + Focused"},
            {"Enter",      "Open dropdown / Select item",       "Closed: open; Open: select + close"},
            {"Alt+Down",   "Open dropdown",                     "Closed"},
            {"Alt+Up",     "Close dropdown",                    "Open"},
            {"Up",         "Cycle selection / Navigate up",     "Closed: cycle; Open: navigate"},
            {"Down",       "Cycle selection / Navigate down",   "Closed: cycle; Open: navigate"},
            {"Home",       "Select/highlight first item",       "Any"},
            {"End",        "Select/highlight last item",        "Any"},
            {"Escape",     "Close without selecting",           "Open"},
            {"Type-ahead", "Jump to first matching item",       "Any"},
        },
    };
}

auto BuildSpinBoxContract() -> WidgetKeyboardContract
{
    return {
        .widgetKind = "SpinBox",
        .bindings = {
            {"Up",         "Increment by step",                 "Focused"},
            {"Down",       "Decrement by step",                 "Focused"},
            {"Page Up",    "Increment by 10x step",             "Focused"},
            {"Page Down",  "Decrement by 10x step",             "Focused"},
            {"Home",       "Set to minimum",                    "Focused"},
            {"End",        "Set to maximum",                    "Focused"},
            {"Digits",     "Edit value as text",                "Focused"},
            {"Enter",      "Commit value, fire EditingFinished","Focused"},
        },
    };
}

} // namespace matcha::fw
