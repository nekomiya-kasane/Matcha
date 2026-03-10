#pragma once

/**
 * @file A11yAudit.h
 * @brief Test-time accessibility audit utility (S5).
 *
 * A11yAudit walks a WidgetNode subtree and reports accessibility violations:
 * - Missing accessibleName on interactive widgets
 * - Missing A11yRole on interactive widgets
 * - Foreground/background contrast below WCAG AA (4.5:1)
 * - Focus unreachable via keyboard (IsFocusable not set on interactive widgets)
 *
 * Designed to be called from unit/integration tests as a post-check.
 */

#include "Matcha/Foundation/Macros.h"
#include "Matcha/UiNodes/Core/A11yRole.h"

#include <string>
#include <vector>

namespace matcha::fw {
class WidgetNode;
class UiNode;
} // namespace matcha::fw

namespace matcha::gui {

/**
 * @brief Severity of an accessibility violation.
 */
enum class A11ySeverity : uint8_t {
    Error   = 0, ///< Must fix (e.g., missing accessible name on button)
    Warning = 1, ///< Should fix (e.g., contrast below AA)
    Info    = 2, ///< Advisory (e.g., no HelpId set)
};

/**
 * @brief Single accessibility violation found by A11yAudit.
 */
struct A11yViolation {
    fw::WidgetNode* widget = nullptr;    ///< The offending widget
    A11ySeverity    severity = A11ySeverity::Error;
    std::string     rule;                ///< Rule identifier (e.g. "a11y.name.missing")
    std::string     message;             ///< Human-readable description
};

/**
 * @brief Accessibility auditor for WidgetNode subtrees.
 *
 * Usage:
 * @code
 * auto violations = A11yAudit::Audit(rootNode);
 * CHECK(violations.empty());
 * @endcode
 */
class MATCHA_EXPORT A11yAudit {
public:
    /**
     * @brief Audit a single WidgetNode for accessibility violations.
     * @return List of violations found.
     */
    [[nodiscard]] static auto AuditWidget(fw::WidgetNode* widget) -> std::vector<A11yViolation>;

    /**
     * @brief Audit an entire UiNode subtree for accessibility violations.
     *
     * Recursively visits all WidgetNode descendants and collects violations.
     * @return List of violations found across the subtree.
     */
    [[nodiscard]] static auto Audit(fw::UiNode* root) -> std::vector<A11yViolation>;

    /**
     * @brief Check if a role is considered "interactive" (should have accessible name).
     */
    [[nodiscard]] static auto IsInteractiveRole(fw::A11yRole role) -> bool;
};

} // namespace matcha::gui
