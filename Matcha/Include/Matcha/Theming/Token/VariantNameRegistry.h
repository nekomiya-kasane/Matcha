#pragma once

/**
 * @file VariantNameRegistry.h
 * @brief 变体名称 ↔ 索引的注册表（规范 §11.3 变体系统架构）。
 *
 * 每个 WidgetKind 都有一组规范的变体名称（例如 PushButton 具有
 * "primary"、"secondary"、"ghost"、"danger"）。此注册表用于在
 * 人类可读的变体名称与 WidgetStyleSheet::variants[] 内部使用的
 * 整数索引之间进行映射。
 *
 * 无 Qt 依赖的 Foundation 层组件。
 *
 * @see COCAUI_Design_System_Specification.md §11.3 变体系统架构
 */

#include <Matcha/Core/Macros.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace matcha::fw {

  // ============================================================================
  // VariantNameRegistry
  // ============================================================================

  /**
   * @class VariantNameRegistry
   * @brief 按 widget 种类映射变体名称与索引。
   *
   * 用法示例：
   * @code
   *   VariantNameRegistry reg;
   *   reg.RegisterKind("PushButton", {"primary", "secondary", "ghost", "danger"});
   *
   *   auto idx = reg.IndexOf("PushButton", "ghost");  // -> 2
   *   auto name = reg.NameOf("PushButton", 2);         // -> "ghost"
   * @endcode
   */
  class MATCHA_EXPORT VariantNameRegistry {
   public:
    VariantNameRegistry() = default;

    /**
     * @brief 为某个 widget 种类注册规范的变体名称。
     *
     * 顺序决定索引：names[0] → 索引 0，names[1] → 索引 1，依此类推。
     * 如果该种类已存在注册，则会被替换。
     *
     * @param widgetKind   Widget 种类标识符（例如 "PushButton"）。
     * @param variantNames 有序的规范变体名称列表。
     */
    void RegisterKind(std::string_view widgetKind, std::vector<std::string> variantNames);

    /**
     * @brief 根据变体名称查找其索引。
     * @return 索引值，若未找到则返回 std::nullopt。
     */
    [[nodiscard]] auto IndexOf(std::string_view widgetKind, std::string_view variantName) const -> std::optional<int>;

    /**
     * @brief 根据索引查找其变体名称。
     * @return 名称字符串视图，若索引越界则返回 std::nullopt。
     */
    [[nodiscard]] auto NameOf(std::string_view widgetKind, int index) const -> std::optional<std::string_view>;

    /**
     * @brief 获取某个 widget 种类的所有变体名称。
     * @return 变体名称列表的指针，若该种类未注册则返回 nullptr。
     */
    [[nodiscard]] auto VariantNames(std::string_view widgetKind) const -> const std::vector<std::string>*;

    /**
     * @brief 获取某个 widget 种类的变体数量。
     */
    [[nodiscard]] auto VariantCount(std::string_view widgetKind) const -> int;

    /**
     * @brief 检查某个 widget 种类是否已注册。
     */
    [[nodiscard]] auto HasKind(std::string_view widgetKind) const -> bool;

    /**
     * @brief 获取所有已注册的 widget 种类名称。
     */
    [[nodiscard]] auto RegisteredKinds() const -> std::vector<std::string_view>;

    /**
     * @brief 移除某个 widget 种类的注册。
     * @return 如果找到并移除则返回 true。
     */
    auto UnregisterKind(std::string_view widgetKind) -> bool;

    /**
     * @brief 移除所有注册信息。
     */
    void Clear();

   private:
    struct KindEntry {
      std::vector<std::string> names;
      std::unordered_map<std::string, int> nameToIndex;
    };

    std::unordered_map<std::string, KindEntry> _kinds;
  };

}  // namespace matcha::fw
