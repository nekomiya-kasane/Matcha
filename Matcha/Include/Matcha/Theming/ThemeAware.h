#pragma once

/**
 * @file ThemeAware.h
 * @brief 消费主题的控件的混入基类。
 *
 * 所有使用设计令牌进行绘制的 Matcha 控件都继承自 `ThemeAware`。
 * 该混入类自动连接到 `IThemeService::ThemeChanged` 信号，
 * 缓存控件对应 `WidgetKind` 的 `WidgetStyleSheet`，并暴露
 * `StyleSheet()` 和 `Theme()` 访问器。子类需实现 `OnThemeChanged()`
 * 以触发重绘。
 *
 * 注意：此类 **不是** QObject —— 它直接使用 `QMetaObject::Connection`，
 * 并在析构函数中断开连接。
 *
 * @see COCAUI_Design_System_Specification.md §6.11 ThemeChanged 信号与 ThemeAware 混入
 * @see IThemeService.h 主题服务接口
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/IThemeService.h>
#include <Matcha/Theming/WidgetStyleSheet.h>

#include <QMetaObject>

namespace matcha::fw {
  class WidgetNode;
}  // namespace matcha::fw

namespace matcha::gui {

  /**
   * @brief 供从主题引擎消费设计令牌的控件使用的混入基类。
   *
   * **用法模式**（阶段 3 控件）：
   * @code
   * class NyanPushButton : public QWidget, public ThemeAware {
   * public:
   *     explicit NyanPushButton(QWidget* parent = nullptr)
   *         : QWidget(parent)
   *         , ThemeAware(WidgetKind::PushButton)
   *     {}
   * protected:
   *     void OnThemeChanged() override { update(); }
   * };
   * @endcode
   *
   * **生命周期**：
   * 1. 构造函数连接到 `IThemeService::ThemeChanged` 信号，并获取初始的
   *    `WidgetStyleSheet`。
   * 2. 当主题切换时，连接回调会刷新缓存的样式表，并调用子类的
   *    `OnThemeChanged()` 方法。
   * 3. 析构函数断开信号连接。
   */
  class MATCHA_EXPORT ThemeAware {
   public:
    /**
     * @brief 构造混入类，连接到全局主题服务。
     * @param kind 用于样式表解析的控件种类。
     * @pre 必须已调用 matcha::gui::SetThemeService()。
     */
    explicit ThemeAware(WidgetKind kind);

    /// @brief 析构函数断开与 ThemeChanged 信号的连接。
    virtual ~ThemeAware();

    ThemeAware(const ThemeAware&) = delete;
    ThemeAware& operator=(const ThemeAware&) = delete;
    ThemeAware(ThemeAware&&) = delete;
    ThemeAware& operator=(ThemeAware&&) = delete;

   protected:
    /// @brief 访问此控件种类对应的缓存样式表。
    [[nodiscard]] auto StyleSheet() const -> const WidgetStyleSheet& {
      return _sheet;
    }

    /// @brief 访问主题服务（用于直接查询令牌）。
    [[nodiscard]] auto Theme() const -> const IThemeService& {
      return _theme;
    }

    /// @brief 访问主题服务（非 const，用于创建子控件）。
    [[nodiscard]] auto ThemeService() -> IThemeService& {
      return _theme;
    }

    /**
     * @brief 当主题更改时被调用。
     *
     * 子类实现此方法以触发重绘（`update()`）或重新计算依赖布局的值。
     */
    virtual void OnThemeChanged() = 0;

    /**
     * @brief 使用此控件的 WidgetStyleSheet 中的过渡配置对属性进行动画。
     *
     * 持续时间和缓动曲线从 `StyleSheet().transition` 解析。
     * 内部委托给全局 AnimationService。
     *
     * @param target   要动画的 WidgetNode。
     * @param property 要动画的属性。
     * @param from     起始值。
     * @param to       目标值。
     * @return 运行动画的句柄（若服务不可用则返回无效句柄）。
     */
    auto AnimateTransition(
        fw::WidgetNode* target, fw::AnimationPropertyId property, fw::AnimatableValue from, fw::AnimatableValue to
    ) -> fw::TransitionHandle;

    /**
     * @brief 使用显式的持续时间和缓动曲线对属性进行动画（忽略样式表配置）。
     */
    auto AnimateTransition(
        fw::WidgetNode* target, fw::AnimationPropertyId property, fw::AnimatableValue from, fw::AnimatableValue to,
        fw::AnimationsToken duration, fw::EasingToken easing
    ) -> fw::TransitionHandle;

    /**
     * @brief 如果控件具有键盘焦点，则在其周围绘制焦点环。
     *
     * 在需要显示焦点指示器的控件的 paintEvent() 末尾调用。
     * 仅当焦点是通过键盘（Tab/Shift+Tab 或快捷键）获得时才会绘制，
     * 鼠标点击获得焦点时不绘制。
     *
     * @param widget 要绘制焦点环的控件。
     * @param theme  用于颜色解析的主题服务。
     * @param radius 圆角半径（默认 4）。
     */
    static void PaintFocusRing(QWidget* widget, const IThemeService& theme, int radius = 4);

   private:
    IThemeService& _theme;          ///< 主题服务的非拥有引用
    WidgetKind _kind;               ///< 用于样式表解析的控件种类
    WidgetStyleSheet _sheet;        ///< 缓存的样式表快照
    QMetaObject::Connection _conn;  ///< ThemeChanged 信号连接
  };

}  // namespace matcha::gui
