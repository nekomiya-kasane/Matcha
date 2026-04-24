#pragma once

/**
 * @file ContrastChecker.h
 * @brief WCAG 2.1 对比度计算工具（S5 阶段实现）
 *
 * 提供静态工具方法，用于计算前景色与背景色之间的亮度对比度，
 * 遵循 WCAG 2.1 无障碍指南。
 *
 * 所有方法均为纯函数，无副作用。
 */

#include <QColor>

#include "Matcha/Core/Macros.h"

namespace matcha::gui {

  /**
   * @brief WCAG 2.1 对比度检查器。
   *
   * 参考标准：https://www.w3.org/TR/WCAG21/#contrast-minimum
   *
   * 相对亮度计算公式：
   *   L = 0.2126 * R_lin + 0.7152 * G_lin + 0.0722 * B_lin
   * 其中：
   *   R_lin = (R/255 <= 0.04045) ? (R/255) / 12.92 : ((R/255 + 0.055) / 1.055)^2.4
   *   G_lin 和 B_lin 同理计算。
   *
   * 对比度 = (L_较亮 + 0.05) / (L_较暗 + 0.05)
   */
  class MATCHA_EXPORT ContrastChecker {
   public:
    /**
     * @brief 计算颜色的相对亮度（WCAG 2.1 公式）。
     * @param color 输入颜色（sRGB 空间）
     * @return 亮度值，范围 [0.0, 1.0]。0 表示黑色，1 表示白色。
     */
    [[nodiscard]] static auto RelativeLuminance(QColor color) -> double;

    /**
     * @brief 计算两种颜色之间的 WCAG 对比度。
     * @param fg 前景色
     * @param bg 背景色
     * @return 对比度值，范围 [1.0, 21.0]。数值越高对比越强烈。
     */
    [[nodiscard]] static auto Ratio(QColor fg, QColor bg) -> double;

    /**
     * @brief 检查颜色组合是否满足 WCAG AA 级（正常文本）标准。
     *        要求对比度 >= 4.5:1。
     * @param fg 前景色
     * @param bg 背景色
     * @return 满足则返回 true。
     */
    [[nodiscard]] static auto MeetsAA(QColor fg, QColor bg) -> bool;

    /**
     * @brief 检查颜色组合是否满足 WCAG AAA 级（正常文本）标准。
     *        要求对比度 >= 7:1。
     * @param fg 前景色
     * @param bg 背景色
     * @return 满足则返回 true。
     */
    [[nodiscard]] static auto MeetsAAA(QColor fg, QColor bg) -> bool;

    /**
     * @brief 检查颜色组合是否满足 WCAG AA 级（大文本）标准。
     *        大文本（>=18pt 常规或 >=14pt 粗体）要求对比度 >= 3:1。
     * @param fg 前景色
     * @param bg 背景色
     * @return 满足则返回 true。
     */
    [[nodiscard]] static auto MeetsAALargeText(QColor fg, QColor bg) -> bool;

    /**
     * @brief 建议一个修改后的前景色，使其满足指定的对比度目标。
     *
     * 通过调整前景色的亮度（HSL 空间）来达到与背景色的目标对比度。
     * 若原前景色已满足目标，则直接返回原色。
     *
     * @param fg           待调整的前景色
     * @param bg           背景色（保持不变）
     * @param targetRatio  期望的最低对比度（例如 AA 级为 4.5）
     * @return 调整后的前景色。
     */
    [[nodiscard]] static auto SuggestFix(QColor fg, QColor bg, double targetRatio) -> QColor;
  };

}  // namespace matcha::gui
