#pragma once

/**
 * @file KeyboardContract.h
 * @brief Keyboard contract descriptors for widget compliance audit (Spec §5.x).
 *
 * Each widget kind declares its expected keyboard bindings as a list of
 * KeyBinding descriptors. These serve as:
 * - Machine-readable spec for automated compliance testing
 * - Documentation source for accessibility audits
 * - Input for ShortcutManager conflict detection
 *
 * Qt-free Foundation layer.
 *
 * @see Matcha_Design_System_Specification.md §5.x Keyboard Contract sections
 */

#include <Matcha/Core/Macros.h>

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace matcha::fw {

// ============================================================================
// KeyBinding
// ============================================================================

/**
 * @struct KeyBinding
 * @brief A single keyboard binding for a widget.
 */
struct KeyBinding {
    std::string key;         ///< Key description (e.g. "Space", "Ctrl+A", "Enter")
    std::string action;      ///< Action description (e.g. "Fire Activated on key-up")
    std::string condition;   ///< Precondition (e.g. "Focused", "Open state")
};

// ============================================================================
// WidgetKeyboardContract
// ============================================================================

/**
 * @struct WidgetKeyboardContract
 * @brief Complete keyboard contract for one widget kind.
 */
struct WidgetKeyboardContract {
    std::string widgetKind;           ///< e.g. "PushButton"
    std::vector<KeyBinding> bindings;
};

// ============================================================================
// KeyboardContractRegistry
// ============================================================================

/**
 * @class KeyboardContractRegistry
 * @brief Registry of keyboard contracts for all widget kinds.
 *
 * Usage:
 * @code
 *   KeyboardContractRegistry reg;
 *   reg.Register(BuildPushButtonContract());
 *   reg.Register(BuildLineEditContract());
 *
 *   const auto* contract = reg.Get("PushButton");
 *   for (const auto& b : contract->bindings) { ... }
 * @endcode
 */
class MATCHA_EXPORT KeyboardContractRegistry {
public:
    KeyboardContractRegistry() = default;

    void Register(WidgetKeyboardContract contract);

    [[nodiscard]] auto Get(std::string_view widgetKind) const
        -> const WidgetKeyboardContract*;

    [[nodiscard]] auto Has(std::string_view widgetKind) const -> bool;

    [[nodiscard]] auto RegisteredKinds() const -> std::vector<std::string_view>;

    [[nodiscard]] auto TotalBindingCount() const -> int;

    void Clear();

private:
    std::unordered_map<std::string, WidgetKeyboardContract> _contracts;
};

// ============================================================================
// Built-in contract builders (§5.x)
// ============================================================================

[[nodiscard]] MATCHA_EXPORT auto BuildPushButtonContract() -> WidgetKeyboardContract;
[[nodiscard]] MATCHA_EXPORT auto BuildLineEditContract() -> WidgetKeyboardContract;
[[nodiscard]] MATCHA_EXPORT auto BuildComboBoxContract() -> WidgetKeyboardContract;
[[nodiscard]] MATCHA_EXPORT auto BuildSpinBoxContract() -> WidgetKeyboardContract;

} // namespace matcha::fw
