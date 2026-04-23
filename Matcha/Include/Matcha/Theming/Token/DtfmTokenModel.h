#pragma once

/**
 * @file DtfmTokenModel.h
 * @brief W3C 设计令牌社区组（DTFM）的令牌数据模型。
 *
 * 实现规范 §7.11 中的令牌表示：
 * - 令牌类型：color、dimension、transition、fontFamily、fontWeight、number、string
 * - 令牌值：带标签的联合体（DtfmValue）
 * - 令牌描述符：$type、$value、$description，可选别名（$value = "{ref}"）
 * - 令牌组：分层容器
 * - 导入/导出为兼容 DTFM 的 JSON 字符串（不依赖 Qt，也不使用外部 JSON 库）
 *
 * 导出生成合法的 JSON。导入解析满足 DTFM 令牌文件所需的最小 JSON 子集。
 *
 * @see COCAUI_Design_System_Specification.md §7.11
 */

#include <Matcha/Core/Macros.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace matcha::fw {

  // ============================================================================
  //  DTFM 令牌类型
  // ============================================================================

  /**
   * @enum DtfmType
   * @brief W3C DTFM $type 值。
   */
  enum class DtfmType : uint8_t {
    Color,
    Dimension,
    Transition,
    FontFamily,
    FontWeight,
    Number,
    String,
  };

  [[nodiscard]] MATCHA_EXPORT auto DtfmTypeToString(DtfmType t) -> std::string;
  [[nodiscard]] MATCHA_EXPORT auto DtfmTypeFromString(std::string_view s) -> std::optional<DtfmType>;

  // ============================================================================
  //  DTFM 令牌值
  // ============================================================================

  /**
   * @struct DtfmDimension
   * @brief 带单位的尺寸值。
   */
  struct DtfmDimension {
    double value = 0.0;
    std::string unit = "px";
  };

  /**
   * @struct DtfmTransition
   * @brief 包含持续时间、延迟和计时函数的过渡值。
   */
  struct DtfmTransition {
    double duration = 150.0;                                      ///< ms
    double delay = 0.0;                                           ///< ms
    std::vector<double> timingFunction = {0.33, 0.0, 0.67, 1.0};  ///< cubic-bezier
  };

  /**
   * @brief 带标签的联合体，用于表示 DTFM 令牌值。
   */
  using DtfmValue = std::variant<
      std::string,     ///< 颜色（十六进制字符串）或字符串或 FontFamily 或别名引用
      DtfmDimension,   ///< 尺寸
      DtfmTransition,  ///< 过渡
      double,          ///< 数字或 FontWeight
      std::monostate   ///< 未设置
      >;

  // ============================================================================
  // DtfmToken
  // ============================================================================

  /**
   * @struct DtfmToken
   * @brief DTFM 格式中的单个设计令牌。
   */
  struct DtfmToken {
    std::string name;
    DtfmType type = DtfmType::String;
    DtfmValue value;
    std::string description;
    std::string aliasRef;  ///< 如果 $value 为 "{some.ref}" 则为非空

    [[nodiscard]] auto IsAlias() const -> bool {
      return !aliasRef.empty();
    }
  };

  // ============================================================================
  // DtfmTokenGroup
  // ============================================================================

  /**
   * @struct DtfmTokenGroup
   * @brief 令牌的分层分组（映射到包含子项的 JSON 对象）。
   */
  struct DtfmTokenGroup {
    std::string name;
    std::vector<DtfmToken> tokens;
    std::vector<DtfmTokenGroup> children;
  };

  // ============================================================================
  // DtfmTokenFile
  // ============================================================================

  /**
   * @struct DtfmTokenFile
   * @brief DTFM 令牌文件的根级表示。
   */
  struct DtfmTokenFile {
    std::string schema;  ///< $schema URL
    std::vector<DtfmTokenGroup> groups;
  };

  // ============================================================================
  // DtfmSerializer
  // ============================================================================

  /**
   * @class DtfmSerializer
   * @brief 将 DtfmTokenFile 序列化/反序列化为 JSON 字符串。
   *
   * 使用最小的手工 JSON 生成（无外部库依赖）。
   * 导入使用一个简单的递归下降解析器来处理 DTFM 子集。
   */
  class MATCHA_EXPORT DtfmSerializer {
   public:
    DtfmSerializer() = default;

    /**
     * @brief 将 DtfmTokenFile 导出为 JSON 字符串。
     * @param file 要序列化的令牌文件。
     * @param indent 每级缩进的空格数（0 = 紧凑模式）。
     * @return JSON 字符串。
     */
    [[nodiscard]] static auto Export(const DtfmTokenFile& file, int indent = 2) -> std::string;

    /**
     * @brief 从 JSON 字符串导入 DtfmTokenFile。
     * @param json 要解析的 JSON 字符串。
     * @return 解析后的令牌文件，若解析错误则返回 std::nullopt。
     */
    [[nodiscard]] static auto Import(std::string_view json) -> std::optional<DtfmTokenFile>;

    /**
     * @brief 将 DtfmTokenFile 展开为以点分隔路径命名的扁平令牌列表。
     * @param file 令牌文件。
     * @return "group.subgroup.name" -> DtfmToken 的映射。
     */
    [[nodiscard]] static auto Flatten(const DtfmTokenFile& file) -> std::unordered_map<std::string, DtfmToken>;
  };

}  // namespace matcha::fw
