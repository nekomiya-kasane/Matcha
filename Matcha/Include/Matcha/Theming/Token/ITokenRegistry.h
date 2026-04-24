#pragma once

/**
 * @file ITokenRegistry.h
 * @brief 设计令牌查询的无 Qt 依赖抽象接口。
 *
 * ITokenRegistry 位于 matcha::fw 命名空间，且 **零 Qt 依赖**。
 * 它提供了密度感知的间距/圆角查询，使得 fw 层（ContainerNode、WidgetNode）
 * 可以在不接触 Qt 类型的情况下使用这些令牌。
 *
 * 具体的实现是 NyanTheme（位于 matcha::gui），它同时也实现了 IThemeService。
 * 应用程序同时暴露两个接口：
 *   - Tokens() -> ITokenRegistry&   （供 fw 层消费者使用）
 *   - Theme()  -> IThemeService&    （供 gui 层消费者使用）
 *
 * 两者返回相同的底层 NyanTheme 对象。
 *
 * @see COCAUI_Design_System_Specification.md 第 4 章空间系统、第 6 章 IThemeService 接口
 * @see TokenEnums.h 令牌枚举定义
 */

#include "Matcha/Theming/Token/TokenEnums.h"

namespace matcha::fw {

  /**
   * @brief 无 Qt 依赖的设计令牌查询抽象接口。
   *
   * 所有查询方法均标记为 [[nodiscard]] 和 const。它们以 O(1) 时间进行查找，
   * 并透明地应用密度缩放。
   *
   * 线程安全：const 查询方法对并发读取是安全的。
   * SetDensity/SetDirection 必须仅从 GUI 线程调用。
   */
  class ITokenRegistry {
   public:
    virtual ~ITokenRegistry() = default;

    ITokenRegistry(const ITokenRegistry&) = delete;
    ITokenRegistry& operator=(const ITokenRegistry&) = delete;
    ITokenRegistry(ITokenRegistry&&) = delete;
    ITokenRegistry& operator=(ITokenRegistry&&) = delete;

    // ========================================================================
    // Density
    // ========================================================================

    /**
     * @brief 设置当前活动的密度级别。
     *
     * 触发所有依赖密度的令牌重新解析。
     * 仅限 GUI 线程调用。
     */
    virtual void SetDensity(DensityLevel level) = 0;

    /**
     * @brief 查询当前活动的密度级别。
     */
    [[nodiscard]] virtual auto CurrentDensity() const -> DensityLevel = 0;

    /**
     * @brief 查询当前活动的密度缩放因子。
     */
    [[nodiscard]] virtual auto CurrentDensityScale() const -> float = 0;

    // ========================================================================
    // Text Direction
    // ========================================================================

    /**
     * @brief 设置全局文本方向（LTR/RTL）。
     */
    virtual void SetDirection(TextDirection dir) = 0;

    /**
     * @brief 查询当前活动的文本方向。
     */
    [[nodiscard]] virtual auto CurrentDirection() const -> TextDirection = 0;

    // ========================================================================
    // Token Queries (O(1), density-scaled)
    // ========================================================================

    /**
     * @brief 查询间距值（逻辑像素，已应用密度缩放）。
     * @param token 间距令牌。
     * @return basePx * densityScale，四舍五入为整数。
     */
    [[nodiscard]] virtual auto SpacingPx(SpaceToken token) const -> int = 0;

    /**
     * @brief 查询圆角半径值（逻辑像素）。
     * @param token 圆角令牌。
     * @return 像素值（整数）。RadiusToken::Round 返回 255（调用者需使用 min(w,h)/2）。
     */
    [[nodiscard]] virtual auto Radius(RadiusToken token) const -> int = 0;

    /**
     * @brief 查询动画持续时间（毫秒）。
     *
     * 当动画覆盖处于活动状态时（如测试模式），返回覆盖值。
     *
     * @param speed 动画速度预设。
     * @return 持续时间（毫秒），0 表示即时 / 测试模式。
     */
    [[nodiscard]] virtual auto AnimationMs(AnimationsToken speed) const -> int = 0;

    // ========================================================================
    // Interaction Timing Queries (§8.7)
    // ========================================================================

    /**
     * @brief 查询交互计时值（毫秒）。
     *
     * 返回给定计时令牌的、已根据平台调整后的值。
     * 例如，在 Windows 上，DoubleClickWindow 和 HoverDelay 会在初始化时从操作系统查询。
     *
     * @param id 计时令牌标识符。
     * @return 持续时间（毫秒）。
     */
    [[nodiscard]] virtual auto TimingMs(TimingTokenId id) const -> int = 0;

    /**
     * @brief 在运行时覆盖一个计时令牌值。
     *
     * 用于测试（例如将所有延迟设置为 0）或用户偏好设置。
     *
     * @param id    要覆盖的计时令牌。
     * @param ms    新的毫秒值。传入 -1 恢复默认值。
     */
    virtual void SetTimingOverride(TimingTokenId id, int ms) = 0;

   protected:
    ITokenRegistry() = default;
  };

}  // namespace matcha::fw
