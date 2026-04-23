#pragma once

/**
 * @file A11yAudit.h
 * @brief 测试时的无障碍审计工具（实现阶段 S5）。
 *
 * A11yAudit 遍历 WidgetNode 子树并报告无障碍违规项：
 * - 交互式控件缺少可访问名称 (accessibleName)
 * - 交互式控件缺少 A11yRole 角色定义
 * - 前景/背景对比度低于 WCAG AA 标准（4.5:1）
 * - 无法通过键盘访问（交互式控件未设置 IsFocusable）
 *
 * 设计用于单元测试或集成测试中作为后置检查调用。
 *
 * @see COCAUI_Design_System_Specification.md 第 18 章无障碍基础设施
 */

#include <string>
#include <vector>

#include "Matcha/Core/Macros.h"
#include "Matcha/Tree/A11yRole.h"

namespace matcha::fw {
  class WidgetNode;
  class UiNode;
}  // namespace matcha::fw

namespace matcha::gui {

  /**
   * @brief 无障碍违规的严重程度。
   */
  enum class A11ySeverity : uint8_t {
    Error = 0,    ///< 必须修复（例如按钮缺少可访问名称）
    Warning = 1,  ///< 应该修复（例如对比度低于 AA 标准）
    Info = 2,     ///< 建议性提示（例如未设置 HelpId）
  };

  /**
   * @brief A11yAudit 发现的单个无障碍违规项。
   */
  struct A11yViolation {
    fw::WidgetNode* widget = nullptr;  ///< 违规的控件
    A11ySeverity severity = A11ySeverity::Error;
    std::string rule;     ///< 规则标识符（例如 "a11y.name.missing"）
    std::string message;  ///< 人类可读的描述信息
  };

  /**
   * @brief WidgetNode 子树的无障碍审计器。
   *
   * 用法：
   * @code
   * auto violations = A11yAudit::Audit(rootNode);
   * CHECK(violations.empty());
   * @endcode
   */
  class MATCHA_EXPORT A11yAudit {
   public:
    /**
     * @brief 审计单个 WidgetNode 的无障碍违规项。
     * @return 发现的违规项列表。
     */
    [[nodiscard]] static auto AuditWidget(fw::WidgetNode* widget) -> std::vector<A11yViolation>;

    /**
     * @brief 审计整个 UiNode 子树的无障碍违规项。
     *
     * 递归访问所有 WidgetNode 后代并收集违规项。
     * @return 整个子树中发现的违规项列表。
     */
    [[nodiscard]] static auto Audit(fw::UiNode* root) -> std::vector<A11yViolation>;

    /**
     * @brief 检查某个角色是否被视为“交互式”（应具有可访问名称）。
     */
    [[nodiscard]] static auto IsInteractiveRole(fw::A11yRole role) -> bool;
  };

}  // namespace matcha::gui
