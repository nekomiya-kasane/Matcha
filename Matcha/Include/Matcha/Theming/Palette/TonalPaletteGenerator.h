#pragma once

/**
 * @file TonalPaletteGenerator.h
 * @brief 基于 OKLCH 色彩空间的色调调色板生成器，从种子色生成 10 级梯度。
 *
 * 使用感知均匀的 OKLCH 色彩空间，从单个种子色生成 10 级色调梯度。
 * 遵循 Ant Design v5 的 10 步模型。
 *
 * 算法流程：
 *   1. 将种子色 QColor（sRGB）转换为 OKLCH（L, C, H）
 *   2. 在 10 个等级上分布明度：
 *      - 浅色主题：L 从 0.96（第 1 级）到 0.30（第 10 级）
 *      - 深色主题：L 从 0.20（第 1 级）到 0.90（第 10 级）
 *   3. 保持种子色的色度与色相不变
 *   4. 限制在 sRGB 色域内（必要时降低色度）
 *   5. 转换回 QColor
 *
 * 10 个等级映射到 Ant Design 的 Token 命名：
 *   [0] Bg,       [1] BgHover,    [2] Border,     [3] BorderHover, [4] Hover,
 *   [5] Base,     [6] Active,     [7] TextHover,  [8] Text,        [9] TextActive
 *
 * @see plan.md S11 了解设计原理。
 * @see https://bottosson.github.io/posts/oklab/ 了解 OKLCH 规范。
 */

#include <QColor>
#include <array>

#include "Matcha/Core/Macros.h"
#include "Matcha/Theming/DesignTokens.h"

namespace matcha::gui {

  /**
   * @brief OKLCH 色彩表示（明度、色度、色相）。
   */
  struct OklchColor {
    float L = 0.0f;  ///< Lightness [0, 1]
    float C = 0.0f;  ///< Chroma [0, ~0.37]
    float H = 0.0f;  ///< Hue in degrees [0, 360)
  };

  /**
   * @brief 静态工具类，用于从种子色生成色调调色板。
   */
  class MATCHA_EXPORT TonalPaletteGenerator {
   public:
    static constexpr int kToneCount = 10;

    /**
     * @brief 为浅色主题生成 10 级色调梯度。
     * @param seed 种子色（sRGB 空间）。
     * @return 包含 10 个 QColor 的数组：第 1 级（最亮）到第 10 级（最暗）。
     */
    [[nodiscard]] static auto GenerateLight(QColor seed) -> std::array<QColor, kToneCount>;

    /**
     * @brief 为深色主题生成 10 级色调梯度。
     * @param seed 种子色（sRGB 空间）。
     * @return 包含 10 个 QColor 的数组：第 1 级（最暗）到第 10 级（最亮）。
     */
    [[nodiscard]] static auto GenerateDark(QColor seed) -> std::array<QColor, kToneCount>;

    // -- 色彩空间转换（公开以支持测试） --

    /**
     * @brief 将 sRGB 格式的 QColor 转换为 OKLCH。
     */
    [[nodiscard]] static auto SrgbToOklch(QColor color) -> OklchColor;

    /**
     * @brief 将 OKLCH 转换为 sRGB 格式的 QColor，并限制在色域内。
     */
    [[nodiscard]] static auto OklchToSrgb(OklchColor oklch) -> QColor;
  };

}  // namespace matcha::gui
