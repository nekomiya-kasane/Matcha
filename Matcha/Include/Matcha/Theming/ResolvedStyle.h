#pragma once

/**
 * @file ResolvedStyle.h
 * @brief 供 paintEvent 消费的、完全解析后的控件样式快照。
 *
 * ResolvedStyle 是 IThemeService::Resolve() 的输出。它包含控件在
 * paintEvent 中所需的所有视觉属性，这些属性已从令牌完全解析为具体的 Qt 值。
 * 控件不再需要自行解引用令牌。
 *
 * @see COCAUI_Design_System_Specification.md §6.5 声明式样式解析 API
 * @see IThemeService::Resolve()
 */

#include <Matcha/Theming/DesignTokens.h>

#include <QColor>
#include <QFont>
#include <optional>

namespace matcha::gui {

  /**
   * @brief 针对特定（变体，状态）的控件完全解析后的视觉样式。
   *
   * 由 `IThemeService::Resolve(kind, variantIndex, state)` 生成。
   * 所有值都是具体的：颜色为 QColor，几何尺寸为密度缩放后的像素值，
   * 字体为完全构造好的 QFont。控件在 paintEvent 中直接消费此结构体，
   * 无需再进行任何令牌查找。
   *
   * 用法示例：
   * @code
   * auto style = Theme().Resolve(WidgetKind::PushButton,
   *                               std::to_underlying(_variant),
   *                               currentState());
   * p.setOpacity(style.opacity);
   * p.setBrush(style.background);
   * p.setPen(QPen(style.border, style.borderWidthPx));
   * p.drawRoundedRect(r, style.radiusPx, style.radiusPx);
   * p.setFont(style.font);
   * p.setPen(style.foreground);
   * p.drawText(textRect, Qt::AlignVCenter, text());
   * @endcode
   */
  struct ResolvedStyle {
    // -- 解析后的颜色 --
    QColor background;  ///< 背景填充色
    QColor foreground;  ///< 文本 / 图标颜色
    QColor border;      ///< 边框描边色

    // -- 解析后的几何尺寸（已应用密度缩放）--
    int radiusPx = 0;       ///< 圆角半径（像素）
    int paddingHPx = 0;     ///< 水平内容内边距（像素）
    int paddingVPx = 0;     ///< 垂直内容内边距（像素）
    int gapPx = 0;          ///< 图标与文本间距（像素）
    int minHeightPx = 0;    ///< 最小组件高度（像素）
    int borderWidthPx = 0;  ///< 边框描边宽度（像素）

    // -- 解析后的字体排版 --
    QFont font;            ///< 完全构造、平台解析后的字体
    int lineHeightPx = 0;  ///< 行高（像素）= fontSizePx * lineHeightMultiplier

    // -- 解析后的视觉效果 --
    ShadowSpec shadow;     ///< 盒阴影参数
    float opacity = 1.0F;  ///< 控件不透明度（0.0..1.0）

    // -- 解析后的过渡动画 --
    int durationMs = 0;  ///< 动画持续时间（毫秒）
    int easingType = 0;  ///< 状态转换的 QEasingCurve::Type 值
  };

  /**
   * @brief 单实例样式覆盖（级联层 3 —— 最高优先级）。
   *
   * 允许单个控件实例在基础级联解析完成后，对 `ResolvedStyle` 的任意字段进行补丁覆盖。
   * 每个 `std::optional` 字段充当一个稀疏补丁：若 `has_value()`，则替换对应的解析值；
   * 否则保留基础值。
   *
   * 用法示例：
   * @code
   * InstanceStyleOverride ov;
   * ov.background = QColor("#FF0000");
   * ov.radiusPx   = 12;
   * auto style = Theme().Resolve(WidgetKind::PushButton, 0,
   *                               InteractionState::Normal, &ov);
   * @endcode
   *
   * @see COCAUI_Design_System_Specification.md §9.7 ResolvedStyle 输出
   */
  struct InstanceStyleOverride {
    // -- 颜色覆盖 --
    std::optional<QColor> background;
    std::optional<QColor> foreground;
    std::optional<QColor> border;

    // -- 几何尺寸覆盖（具体像素值，调用者已应用密度缩放）--
    std::optional<int> radiusPx;
    std::optional<int> paddingHPx;
    std::optional<int> paddingVPx;
    std::optional<int> gapPx;
    std::optional<int> minHeightPx;
    std::optional<int> borderWidthPx;

    // -- 视觉效果覆盖 --
    std::optional<float> opacity;

    // -- 过渡动画覆盖 --
    std::optional<int> durationMs;

    /**
     * @brief 将此覆盖应用到已解析的样式上（原地补丁）。
     * @param style 要打补丁的样式。
     */
    void ApplyTo(ResolvedStyle& style) const {
      if (background) {
        style.background = *background;
      }
      if (foreground) {
        style.foreground = *foreground;
      }
      if (border) {
        style.border = *border;
      }
      if (radiusPx) {
        style.radiusPx = *radiusPx;
      }
      if (paddingHPx) {
        style.paddingHPx = *paddingHPx;
      }
      if (paddingVPx) {
        style.paddingVPx = *paddingVPx;
      }
      if (gapPx) {
        style.gapPx = *gapPx;
      }
      if (minHeightPx) {
        style.minHeightPx = *minHeightPx;
      }
      if (borderWidthPx) {
        style.borderWidthPx = *borderWidthPx;
      }
      if (opacity) {
        style.opacity = *opacity;
      }
      if (durationMs) {
        style.durationMs = *durationMs;
      }
    }
  };

}  // namespace matcha::gui
