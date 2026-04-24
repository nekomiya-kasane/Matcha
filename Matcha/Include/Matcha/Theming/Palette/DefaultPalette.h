#pragma once

/**
 * @file DefaultPalette.h
 * @brief 浅色与深色主题的编译期默认颜色调色板
 *
 * 本文件提供内置的中性色、特殊用途颜色的默认值，以及用于生成色调调色板的种子颜色。
 * 这使得 NyanTheme 可以在不依赖外部 JSON 文件的情况下完成初始化，随后再用 JSON 中的值进行覆盖。
 *
 * 语义色相 Token（5 个色相 × 10 个色阶 = 50 个颜色）**不**存储在此处。
 * 它们始终由 TonalPaletteGenerator 在运行时根据本文件中的种子颜色动态生成。
 *
 * 颜色格式：0xAARRGGBB（Qt QRgb 格式）
 *
 * @see docs/07_Declarative_Style_RFC.md 第 5.7 节
 * @see NyanTheme::LoadPalette() 了解覆盖逻辑
 */

#include <Matcha/Theming/DesignTokens.h>

#include <array>
#include <cstdint>

namespace matcha::gui::defaults {

  // ============================================================================
  // 中性色 + 特殊用途颜色默认值（共 31 个 Token）
  // ============================================================================

  /// @brief 索引常量，对应 ColorToken 枚举中中性色和特殊用途颜色的顺序。

  /// @brief 浅色主题默认中性色与特殊用途颜色（31 个条目）。
  /// 这些值直接取自规范 §2.5 和 §2.6 的 Light 列。
inline constexpr std::array<uint32_t, kColorTokenCount> kLightDefaultColors = {
  ///< @brief 由于算法暂时不存在，先直接使用规范 §2.53 列的 Light 值。
    // === 主题色：品牌色 / 品牌辅助色 (15 个) ===
    0xFF0066FF,  // colorPrimaryBase (seed)
    0xFFF0F7FF,  // colorPrimaryBg
    0xFFD6E8FF,  // colorPrimaryBgActive
    0xFFBDDAFF,  // colorPrimaryDisabled
    0xFF6BB0FF,  // colorPrimaryBorder
    0xFF2986FF,  // colorPrimaryBgHover
    0xFF0066FF,  // colorPrimary
    0xFF004FD9,  // colorPrimaryActive
    0x00000000,  // colorPrimaryGradient (暂用透明，实际应为渐变)
    0xFF0059B3,  // colorPrimaryNavBase (seed)
    0xFF0059B3,  // colorPrimaryNav
    0xCCFFFFFF,  // colorPrimaryNavRev (80% 白色)
    0xA6FFFFFF,  // colorPrimaryNavSecondaryRev (65% 白色)
    0x66FFFFFF,  // colorPrimaryNavTertiaryRev (40% 白色)
    0x40FFFFFF,  // colorPrimaryNavQuaternaryRev (25% 白色)

    // === 功能色：成功/警告/错误 (27 个) ===
    // 成功色
    0xFF32CE99,  // colorSuccessBase (seed)
    0xFFF0FFF7,  // colorSuccessBg
    0xFFE0FFF1,  // colorSuccessBgActive
    0xFFB0F5D7,  // colorSuccessDisabled
    0xFF82E8BF,  // colorSuccessBorder
    0xFF58DBAB,  // colorSuccessHover
    0xFF32CE99,  // colorSuccess
    0xFF20A87F,  // colorSuccessActive
    0x00000000,  // colorSuccessGradient
    // 警告色
    0xFFED7B2F,  // colorWarningBase (seed)
    0xFFFFF8F0,  // colorWarningBg
    0xFFFFECD6,  // colorWarningBgActive
    0xFFFFD6AD,  // colorWarningDisabled
    0xFFFFBE85,  // colorWarningBorder
    0xFFFA9F5A,  // colorWarningHover
    0xFFED7B2F,  // colorWarning
    0xFFCE5C1E,  // colorWarningActive
    0x00000000,  // colorWarningGradient
    // 错误色
    0xFFCC1423,  // colorErrorBase (seed)
    0xFFFFF1F0,  // colorErrorBg
    0xFFFFE0E0,  // colorErrorBgActive
    0xFFFFC8C7,  // colorErrorDisabled
    0xFFFF9EA0,  // colorErrorBorder
    0xFFD93840,  // colorErrorHover
    0xFFCC1423,  // colorError
    0xFFA6081B,  // colorErrorActive
    0x00000000,  // colorErrorGradient

    // === 文本色：文本 / 反色文本 (9 个) ===
    0xFF000000,  // colorTextBase (seed)
    0xE0000000,  // colorText (88% 黑色)
    0xA6000000,  // colorTextSecondary (65% 黑色)
    0x66000000,  // colorTextTertiary (40% 黑色)
    0x40000000,  // colorTextQuaternary (25% 黑色)
    0xE0FFFFFF,  // colorTextRev (88% 白色)
    0xA6FFFFFF,  // colorTextSecondaryRev (65% 白色)
    0x66FFFFFF,  // colorTextTertiaryRev (40% 白色)
    0x40FFFFFF,  // colorTextQuaternaryRev (25% 白色)

    // === 中性色阶：填充 / 交互 / 边框 (23 个) ===
    0xFFFFFFFF,  // colorBgBase (seed)
    0xFFFFFFFF,  // colorFill
    0xFFFAFAFC,  // colorFillSecondary
    0xFFEEEEF0,  // colorFillTertiary
    0xFFBEBEC0,  // colorFillQuaternary
    0x0D000000,  // colorFillHover
    0x1A000000,  // colorFillSecondaryHover
    0x33000000,  // colorFillTertiaryHover
    0x66000000,  // colorFillQuaternaryHover
    0xFFFFFFFF,  // colorBgContainer
    0xFFFAFAFC,  // colorBgContainerSecondary
    0xFFF4F4F6,  // colorBgContainerTertiary
    0xFFEEEEF0,  // colorBgContainerQuaternary
    0xFF58585A,  // colorBgContainerQuinary
    0xFFFFFFFF,  // colorBgRender
    0xA6FFFFFF,  // colorBgRenderSecondary
    0x66FFFFFF,  // colorBgRenderTertiary
    0x1AFFFFFF,  // colorBgRenderQuaternary
    0xFFD8D8DA,  // colorBorder
    0xFFBEBEC0,  // colorBorderSecondary
    0xFFE3E3E6,  // colorDivider
    0xFFD8D8DA,  // colorDividerSecondary
    0xFF8C8C8E,  // colorDividerTertiary

    // === 特殊用途 Token (9 个) ===
    0xFFFFFFFF,  // OnAccent
    0xFFD6E8FF,  // OnAccentSecondary
    0xFF0066FF,  // Focus
    0xFFD6E8FF,  // Selection
    0xFF0066FF,  // Link
    0x4D000000,  // Scrim
    0xE6FFFFFF,  // Overlay
    0x1A000000,  // Shadow
    0xFFD8D8DA,  // Separator
};

  /// @brief 深色主题默认中性色与特殊用途颜色（31 个条目）。
  /// 这些值直接取自规范 §2.5 和 §2.6 的 Dark 列。
  inline constexpr std::array<uint32_t, kColorTokenCount> kDarkNeutralColors = {
  ///< @brief 由于算法暂时不存在，先直接使用规范 §2.53 列的 Light 值。
    // === 主题色：品牌色 / 品牌辅助色 (15 个) ===
    0xFF0066FF,  // colorPrimaryBase (seed)
    0xFF3B444F,  // colorPrimaryBg
    0xFF334966,  // colorPrimaryBgActive
    0xFF27508D,  // colorPrimaryDisabled
    0xFF1658BC,  // colorPrimaryBorder
    0xFF2986FF,  // colorPrimaryBgHover
    0xFF0066FF,  // colorPrimary
    0xFF004FD9,  // colorPrimaryActive
    0x00000000,  // colorPrimaryGradient (暂用透明，实际应为渐变)
    0xFF1E1E1E,  // colorPrimaryNavBase (seed)
    0xFF1E1E1E,  // colorPrimaryNav
    0xCCFFFFFF,  // colorPrimaryNavRev (80% 白色)
    0xA6FFFFFF,  // colorPrimaryNavSecondaryRev (65% 白色)
    0x66FFFFFF,  // colorPrimaryNavTertiaryRev (40% 白色)
    0x40FFFFFF,  // colorPrimaryNavQuaternaryRev (25% 白色)

    // === 功能色：成功/警告/错误 (27 个) ===
    // 成功色
    0xFF32CE99,  // colorSuccessBase (seed)
    0xFF3F4844,  // colorSuccessBg
    0xFFE0FFF1,  // colorSuccessBgActive
    0xFF3E574B,  // colorSuccessDisabled
    0xFF3B795F,  // colorSuccessBorder
    0xFF369C77,  // colorSuccessHover
    0xFF32CE99,  // colorSuccess
    0xFF20A87F,  // colorSuccessActive
    0x00000000,  // colorSuccessGradient(暂用透明，实际应为渐变)
    // 警告色
    0xFFED7B2F,  // colorWarningBase (seed)
    0xFF4A443E,  // colorWarningBg
    0xFF5C4B3D,  // colorWarningBgActive
    0xFF865A39,  // colorWarningDisabled
    0xFFB06634,  // colorWarningBorder
    0xFFFA9F5A,  // colorWarningHover
    0xFFED7B2F,  // colorWarning
    0xFFCE5C1E,  // colorWarningActive
    0x00000000,  // colorWarningGradient(暂用透明，实际应为渐变)
    // 错误色
    0xFFCC1423,  // colorErrorBase (seed)
    0xFF4B3E3D,  // colorErrorBg
    0xFF573A39,  // colorErrorBgActive
    0xFF6B3333,  // colorErrorDisabled
    0xFF8D282B,  // colorErrorBorder
    0xFFD93840,  // colorErrorHover
    0xFFCC1423,  // colorError
    0xFFA6081B,  // colorErrorActive
    0x00000000,  // colorErrorGradient

    // === 文本色：文本 / 反色文本 (9 个) ===
    0xFFFFFFFF,  // colorTextBase (seed)
    0xE0FFFFFF,  // colorText (88% 黑色)
    0xA6FFFFFF,  // colorTextSecondary (65% 黑色)
    0x66FFFFFF,  // colorTextTertiary (40% 黑色)
    0x40FFFFFF,  // colorTextQuaternary (25% 黑色)
    0xE0000000,  // colorTextRev (88% 白色)
    0xA6000000,  // colorTextSecondaryRev (65% 白色)
    0x66000000,  // colorTextTertiaryRev (40% 白色)
    0x40000000,  // colorTextQuaternaryRev (25% 白色)

    // === 中性色阶：填充 / 交互 / 边框 (23 个) ===
    0xFF000000,  // colorBgBase (seed)
    0xFF404040,  // colorFill
    0xFF454545,  // colorFillSecondary
    0xFF505050,  // colorFillTertiary
    0xFF696969,  // colorFillQuaternary
    0xEBFFFFFF,  // colorFillHover
    0x26FFFFFF,  // colorFillSecondaryHover
    0x40FFFFFF,  // colorFillTertiaryHover
    0x66FFFFFF,  // colorFillQuaternaryHover
    0xFF404040,  // colorBgContainer
    0xFF3A3A3A,  // colorBgContainerSecondary
    0xFF343434,  // colorBgContainerTertiary
    0xFF2E2E2E,  // colorBgContainerQuaternary
    0xFFF2F2F2,  // colorBgContainerQuinary
    0xFF404040,  // colorBgRender
    0xA6404040,  // colorBgRenderSecondary
    0x66404040,  // colorBgRenderTertiary
    0x1A404040,  // colorBgRenderQuaternary
    0xFF505050,  // colorBorder
    0xFF5A5A5A,  // colorBorderSecondary
    0xFF2B2B2B,  // colorDivider
    0xFF282828,  // colorDividerSecondary
    0xFF8C8C8C,  // colorDividerTertiary

    // === 特殊用途 Token (9 个) (暂时给个默认值，文档中没有定义)===
    0xFFFFFFFF,  // OnAccent
    0xFFD6E8FF,  // OnAccentSecondary
    0xFF0066FF,  // Focus
    0xFFD6E8FF,  // Selection
    0xFF0066FF,  // Link
    0x4D000000,  // Scrim
    0xE6FFFFFF,  // Overlay
    0x1A000000,  // Shadow
    0xFFD8D8DA,  // Separator
};

  // ============================================================================
  // 种子颜色默认值（每个主题 7 个色相）
  // ============================================================================

  /// @brief 用于 OKLCH 色调调色板生成的种子颜色（浅色主题）。
  /// 顺序：colorPrimaryBase, colorPrimaryNavBase, colorSuccessBase, colorWarningBase, colorErrorBase, colorTextBase,
  /// colorBgBase 这些值对应规范 §2.1.1 中的 colorPrimaryBase, colorSuccessBase 等。
  inline constexpr std::array<uint32_t, 7> kLightSeeds = {
    0xFF0066FF,  // colorPrimaryBase
    0xFF0059B3,  // colorPrimaryNavBase
    0xFF32CE99,  // colorSuccessBase
    0xFFED7B2F,  // colorWarningBase
    0xFFCC1423,  // colorErrorBase
    0xFF000000,  // colorTextBase
    0xFFFFFFFF,  // colorBgBase
  };

  /// @brief 用于 OKLCH 色调调色板生成的种子颜色（深色主题）。
  inline constexpr std::array<uint32_t, 7> kDarkSeeds = {
    0xFF0066FF,  // colorPrimaryBase
    0xFF1E1E1E,  // colorPrimaryNavBase
    0xFF32CE99,  // colorSuccessBase
    0xFFED7B2F,  // colorWarningBase
    0xFFCC1423,  // colorErrorBase
    0xFF000000,  // colorTextBase
    0xFFFFFFFF,  // colorBgBase
  };

  // ============================================================================
  // 字体族默认值
  // ============================================================================

  /// @brief 各平台的默认字体族名称。
  inline constexpr const char* kFontFamilyWin = "Segoe UI";
  inline constexpr const char* kFontFamilyMac = "SF Pro Text";
  inline constexpr const char* kFontFamilyLinux = "Noto Sans";
  inline constexpr const char* kFontFamilyMono = "Cascadia Code";

}  // namespace matcha::gui::defaults