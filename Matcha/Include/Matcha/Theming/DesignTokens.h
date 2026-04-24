#pragma once

/**
 * @file DesignTokens.h
 * @brief 设计 Token 系统：Color、Font 以及与 Qt 相关的值类型结构体
 *
 * 无 Qt 依赖的 Token 枚举（Spacing, Radius, Elevation, Animation, Easing,
 * InteractionState, Density, Direction, Icon, Cursor, Layer）定义在
 * matcha::fw::TokenEnums.h 中，此处通过 using 声明导入。
 *
 * 本文件保留 Qt 依赖的类型：ColorToken（解析为 QColor）、
 * FontSpec（包含 QString）、ShadowSpec、StateStyle、VariantStyle。
 *
 * @see TokenEnums.h 无 Qt 枚举定义
 * @see COCAUI_Design_System_Specification.md 第 2-4 章详细 Token 定义
 */

#include <QString>
#include <QtTypes>
#include <array>
#include <cstdint>
#include <utility>

#include "Matcha/Theming/Token/TokenEnums.h"

namespace matcha::gui {

  // 将 fw 层的 Token 枚举导入到 gui 命名空间
  using matcha::fw::AnimationsToken;
  using matcha::fw::ControlHeightToken;
  using matcha::fw::CursorToken;
  using matcha::fw::DensityLevel;
  using matcha::fw::EasingToken;
  using matcha::fw::FontSizePreset;
  using matcha::fw::IconToken;
  using matcha::fw::InteractionState;
  using matcha::fw::LayerToken;
  using matcha::fw::LineToken;
  using matcha::fw::PopupWidthToken;
  using matcha::fw::RadiusToken;
  using matcha::fw::ShadowToken;
  using matcha::fw::SizeToken;
  using matcha::fw::SpaceToken;
  using matcha::fw::SpringSpec;
  using matcha::fw::TextDirection;
  using matcha::fw::TimingTokenId;
  using matcha::fw::TransitionDef;

  // 导入常量
  using matcha::fw::IconId;
  using matcha::fw::ToPixels;

  using matcha::fw::kAnimationsTokenCount;
  using matcha::fw::kBaseSizePx;
  using matcha::fw::kDefaultAnimationMs;
  using matcha::fw::kDefaultTimingMs;
  using matcha::fw::kInteractionStateCount;
  using matcha::fw::kLineTokenCount;
  using matcha::fw::kMatchaIconPrefix;
  using matcha::fw::kShadowTokenCount;
  using matcha::fw::kSizeTokenCount;
  using matcha::fw::kTimingTokenCount;
  using matcha::fw::kZIndexTokenCount;

  // ============================================================================
  // 1. 颜色 Token
  // ============================================================================

  /**
   * @brief 整个设计系统的语义颜色槽位
   *
   * 共 76 个 Token，遵循 COCAUI_Design_System_Specification.md 第 2 章的定义。
   *
   * 分类：
   * - 主题/品牌色：13 个
   * - 功能色（成功/警告/错误）：24 个
   * - 文本色：8 个
   * - 中性色（填充/容器/边框）：22 个
   * - 特殊用途：9 个
   *
   * 通过平坦数组实现 O(1) 查找：`colors[std::to_underlying(token)]`。
   */
  enum class ColorToken : uint16_t {
    // 主题色：品牌色 / 品牌辅助色 (13 tokens + 2 seed tokens) - 规范 2.2
    colorPrimaryBase,  // Seed token
    colorPrimaryBg,
    colorPrimaryBgActive,
    colorPrimaryDisabled,
    colorPrimaryBorder,
    colorPrimaryBgHover,
    colorPrimary,
    colorPrimaryActive,
    colorPrimaryGradient,
    // 用于导航区域，品牌色辅助色
    colorPrimaryNavBase,  // Seed token
    colorPrimaryNav,
    colorPrimaryNavRev,
    colorPrimaryNavSecondaryRev,
    colorPrimaryNavTertiaryRev,
    colorPrimaryNavQuaternaryRev,

    // 功能色：成功/警告/错误 (24 tokens + 3 seed tokens) - 规范 2.3
    colorSuccessBase,  // Seed token
    colorSuccessBg,
    colorSuccessBgActive,
    colorSuccessDisabled,
    colorSuccessBorder,
    colorSuccessHover,
    colorSuccess,
    colorSuccessActive,
    colorSuccessGradient,
    colorWarningBase,  // Seed token
    colorWarningBg,
    colorWarningBgActive,
    colorWarningDisabled,
    colorWarningBorder,
    colorWarningHover,
    colorWarning,
    colorWarningActive,
    colorWarningGradient,
    colorErrorBase,  // Seed token
    colorErrorBg,
    colorErrorBgActive,
    colorErrorDisabled,
    colorErrorBorder,
    colorErrorHover,
    colorError,
    colorErrorActive,
    colorErrorGradient,

    // 文本色：文本 / 反色文本 (8 tokens + 1 seed token) - 规范 2.4
    colorTextBase,  // Seed token
    colorText,
    colorTextSecondary,
    colorTextTertiary,
    colorTextQuaternary,
    colorTextRev,
    colorTextSecondaryRev,
    colorTextTertiaryRev,
    colorTextQuaternaryRev,

    // 中性色阶：填充 / 交互 / 边框 (22 tokens + 1 seed token) - 规范 2.5
    colorBgBase,  // Seed token
    colorFill,
    colorFillSecondary,
    colorFillTertiary,
    colorFillQuaternary,
    colorFillHover,
    colorFillSecondaryHover,
    colorFillTertiaryHover,
    colorFillQuaternaryHover,
    colorBgContainer,
    colorBgContainerSecondary,
    colorBgContainerTertiary,
    colorBgContainerQuaternary,
    colorBgContainerQuinary,
    colorBgRender,
    colorBgRenderSecondary,
    colorBgRenderTertiary,
    colorBgRenderQuaternary,
    colorBorder,
    colorBorderSecondary,
    colorDivider,
    colorDividerSecondary,
    colorDividerTertiary,

    // 特殊用途Token (9 tokens) - 规范 2.6
    OnAccent,
    OnAccentSecondary,
    Focus,
    Selection,
    Link,
    Scrim,
    Overlay,
    Shadow,
    Separator,

    Count_  ///< Sentinel for array sizing. Must be last.
  };

  /// @brief ColorToken 枚举值的编译期数量
  inline constexpr auto kColorTokenCount = static_cast<std::size_t>(ColorToken::Count_);

  // 文末用到的 InteractionState 已在 matcha::fw::TokenEnums.h 中定义

  // ============================================================================
  // 3. 字体排版 Token
  // ============================================================================

  /**
   * @brief 语义字体角色，映射到平台相关的 FontSpec 值
   *
   * `IThemeService::Font(FontRole)` 返回 `const FontSpec&`。
   */
  enum class FontRole : uint8_t {
    fontWeightRegular,  // Seed token
    fontWeightMedium,   // Seed token
    fontWeightBold,     // Seed token
    fontItalic,         // Seed token
    fontLineHeight,     // Seed token
    fontLetterSpacing,  // Seed token
    fontSizeBase,       // Seed token
    fontSizeXS,
    fontSizeSM,
    fontSizeMD,
    fontSizeLG,
    fontSizeXL,
    fontSizeXXL,
    Count_  ///< Sentinel for array sizing. Must be last.
  };

  /// @brief FontRole 枚举值的编译期数量
  inline constexpr auto kFontRoleCount = static_cast<std::size_t>(FontRole::Count_);

  /**
   * @brief 平台解析后的具体字体规格
   *
   * 平台字体族在启动时通过 `QFontDatabase::systemFont` 选定。
   * 尺寸为与 DPI 无关的点大小。
   */
  struct FontSpec {
    QString family;                    ///< 例如 Windows 为 "Segoe UI"，macOS 为 "SF Pro"
    int sizeInPt = 9;                  ///< 与 DPI 无关的点大小
    int weight = 400;                  ///< QFont::Weight 枚举值（Normal=400）
    bool italic = false;               ///< 是否斜体
    qreal lineHeightMultiplier = 1.4;  ///< 行高为字体大小的倍数
    qreal letterSpacing = 0.0;         ///< 字间距（像素），0 表示默认
  };

  // 间距、圆角、阴影 Token 已在 matcha::fw::TokenEnums.h 中定义

  /**
   * @brief 由 ElevationToken 派生出的具体阴影参数
   *
   * NyanTheme 中的算法根据高度枚举级别计算 offsetY、blurRadius 和 opacity。
   */
  struct ShadowSpec {
    int offsetX = 0;      ///< 水平阴影偏移（通常为 0）
    int offsetY = 0;      ///< 垂直阴影偏移
    int blurRadius = 0;   ///< 高斯模糊半径
    qreal opacity = 0.0;  ///< 阴影颜色相对于背景色的不透明度
  };

  // 动画 Token 已在 matcha::fw::TokenEnums.h 中定义

  // ============================================================================
  // 8. 状态颜色与变体样式（颜色映射）
  // ============================================================================

  /**
   * @brief 每个交互状态下的视觉样式：颜色 + 不透明度 + 边框宽度 + 光标
   *
   * 用于 VariantStyle 内部，定义 Widget 在每个 InteractionState 下的外观。
   * 扩展了原有的 StateColors，加入了声明式 Widget 样式所需的非颜色视觉属性（RFC-07）。
   *
   * @see docs/07_Declarative_Style_RFC.md §5.1
   */
  struct StateStyle {
    ColorToken background = ColorToken::colorBgBase;
    ColorToken foreground = ColorToken::colorTextBase;
    ColorToken border = ColorToken::colorBorder;
    float opacity = 1.0F;                            ///< 0.0~1.0，作用于整个 Widget
    SpaceToken borderWidth = SpaceToken::marginXXS;  ///< 边框描边宽度（默认 1px）
    CursorToken cursor = CursorToken::Default;       ///< 光标形状（默认）
  };

  /**
   * @brief 每个变体的颜色映射表：包含 8 个 InteractionState 条目
   *
   * 简单 Widget 拥有 1 个 VariantStyle。多变体 Widget（如 PushButton 有
   * Primary/Secondary/Ghost/Danger）则拥有 N 个 VariantStyle 条目，通过变体枚举索引。
   */
  struct VariantStyle {
    std::array<StateStyle, kInteractionStateCount> colors = {};
  };

}  // namespace matcha::gui
