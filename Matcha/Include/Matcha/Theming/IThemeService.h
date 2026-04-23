#pragma once

/**
 * @file IThemeService.h
 * @brief 设计 Token 系统的抽象主题服务接口。
 *
 * `IThemeService` 继承自 `fw::ITokenRegistry`（无 Qt 依赖的 Token 查询），
 * 并扩展了 Qt 相关的功能：
 * - 主题生命周期（`SetTheme` / `CurrentTheme`）
 * - 颜色查询（解析为 QColor）
 * - 字体查询（解析为包含 QString 的 FontSpec）
 * - 阴影查询（ShadowSpec）
 * - 缓动曲线查询（QEasingCurve::Type）
 * - 按控件种类的复合样式表（`ResolveStyleSheet`）
 * - 组件级 Token 覆盖（每个控件类的偏差）
 * - 动态 Token 注册（插件扩展，ADR-015）
 * - 测试时的动画覆盖（确定性执行）
 *
 * `NyanTheme` 提供了这两个接口的具体实现。
 *
 * @see ITokenRegistry.h 无 Qt 依赖的基接口。
 * @see DesignTokens.h Token 类型定义。
 * @see WidgetStyleSheet.h WidgetKind 和 WidgetStyleSheet 定义。
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/DesignTokens.h>
#include <Matcha/Theming/ResolvedStyle.h>
#include <Matcha/Theming/Token/ITokenRegistry.h>
#include <Matcha/Theming/WidgetStyleSheet.h>

#include <QColor>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <optional>
#include <span>
#include <string_view>

namespace matcha::gui {

  // ============================================================================
  // ThemeId  (string-based, supports unlimited user themes)
  // ============================================================================

  /// @brief 主题标识符 —— 一个纯 QString 名称（例如 "Light"、"MyCustomDark"）。
  using ThemeId = QString;

  /// @brief 内置主题名称常量。
  inline const QString kThemeLight = QStringLiteral("Light");
  inline const QString kThemeDark = QStringLiteral("Dark");
  inline const QString kThemeHighContrast = QStringLiteral("HighContrast");

  /**
   * @brief 主题的系统级亮色/暗色分类。
   *
   * 每个主题属于 Light 或 Dark 家族。这用于控制：
   * - `colorSeeds` 色调调色板的生成方向
   * - `DynamicColor` 浅色/深色分支的选择
   * - 操作系统深色模式集成（系统主题自动切换）
   */
  enum class ThemeMode : uint8_t {
    Light = 0,
    Dark = 1,
  };

  // ============================================================================
  // IThemeService
  // ============================================================================

  /**
   * @brief 设计 Token 系统主题引擎的抽象接口。
   *
   * 所有查询方法都是 `[[nodiscard]]` 和 `const` —— 它们在 Token 存储上执行 O(1)
   * 平坦数组查找，Token 存储由 `SetTheme()` 构建。
   *
   * 控件通过 `IThemeService&` 访问 Token（从 `ThemeAware` 混入类或应用程序 Shell 直接获取）。
   *
   * **线程安全**：所有 const 查询方法对于并发读取是安全的。
   * `SetTheme()` 必须仅从 GUI 线程调用。
   */
  class MATCHA_EXPORT IThemeService : public QObject, public fw::ITokenRegistry {
    Q_OBJECT

   public:
    ~IThemeService() override = default;

    IThemeService(const IThemeService&) = delete;
    IThemeService& operator=(const IThemeService&) = delete;
    IThemeService(IThemeService&&) = delete;
    IThemeService& operator=(IThemeService&&) = delete;

    // ITokenRegistry::SpacingPx, Radius, AnimationMs, SetDensity, SetDirection
    // 是继承而来的，必须由 NyanTheme 实现。

    // ========================================================================
    // Theme Lifecycle
    // ========================================================================

    /**
     * @brief 切换到指定的主题。
     *
     * 重建所有 Token 数组（颜色、字体、阴影、样式表），并发出 `ThemeChanged(name)` 信号。
     * 必须从 GUI 线程调用。
     *
     * @param name 主题名称（必须是内置主题或已注册的主题）。
     */
    virtual void SetTheme(const QString& name) = 0;

    /**
     * @brief 查询当前活动的主题名称。
     * @return 当前活动的主题名称。
     */
    [[nodiscard]] virtual auto CurrentTheme() const -> const QString& = 0;

    /**
     * @brief 查询当前活动主题的 ThemeMode（Light/Dark）。
     * @return 活动主题的 ThemeMode。
     */
    [[nodiscard]] virtual auto CurrentMode() const -> ThemeMode = 0;

    /**
     * @brief 注册一个自 JSON 调色板文件支持的自定义主题。
     *
     * JSON 文件可能包含`"extends"`键，命名任何已注册的主题。
     * 当调用`SetTheme(name)`时，首先加载基础主题，然后将自定义 JSON 的值叠加到基础主题上。
     *
     * 没有对自定义主题数量的限制。
     *
     * @param name     唯一的主题名称（例如 "OceanBlue"）。不能为空。
     * @param jsonPath JSON 调色板文件的绝对路径。
     * @param mode     主题的 Light 或 Dark 家族分类。
     * @return 注册成功返回 true，jsonPath 不存在时返回 false。
     */
    virtual auto RegisterTheme(const QString& name, const QString& jsonPath, ThemeMode mode) -> bool = 0;

    // ========================================================================
    // Qt-Dependent Token Queries (Color, Font, Shadow, Easing)
    // SpacingPx, Radius, AnimationMs are inherited from ITokenRegistry.
    // ========================================================================

    /**
     * @brief 将语义颜色 Token 解析为具体的 QColor。
     * @param token 语义颜色槽位。
     * @return 当前主题下解析出的颜色。
     */
    [[nodiscard]] virtual auto Color(ColorToken token) const -> QColor = 0;

    /**
     * @brief 带交互状态叠加的颜色 Token 解析。
     *
     * @deprecated 该重载目前仅委托给 `Color(token)`，没有应用任何状态相关的亮度/Alpha 偏移。
     *             要获得正确的每状态颜色，请使用 `Resolve(kind, variantIndex, state)`，
     *             它会从 WidgetStyleSheet 的变体/状态颜色矩阵中读取。
     *
     * @param token 基础语义颜色槽位。
     * @param state 控件交互状态（当前未使用）。
     * @return 当前主题的基础颜色（state 参数被忽略）。
     *
     * @see Resolve() 推荐的声明式样式 API。
     */
    [[nodiscard]] virtual auto Color(ColorToken token, InteractionState state) const -> QColor = 0;

    /**
     * @brief 按角色查询字体规范。
     * @param role 语义字体角色。
     * @return 平台相关的 FontSpec（家族、大小、字重等）。
     */
    [[nodiscard]] virtual auto Font(FontRole role) const -> const FontSpec& = 0;

    /**
     * @brief 设置全局字体缩放系数，应用于所有 FontRole 的基础大小。
     *
     * 系数被限制在 [kFontScaleMin, kFontScaleMax] 范围内。
     * 触发字体重建并发出 ThemeChanged 信号。
     *
     * @param factor 缩放倍数（1.0 = 无缩放）。
     */
    virtual void SetFontScale(float factor) = 0;

    /**
     * @brief 获取当前的全局字体缩放系数。
     * @return 字体缩放系数（默认 1.0）。
     */
    [[nodiscard]] virtual auto FontScale() const -> float = 0;

    /**
     * @brief 便捷方法：从预设设置字体缩放。
     *
     * 等价于 `SetFontScale(FontSizePresetScale(preset))`。
     */
    virtual void SetFontSizePreset(fw::FontSizePreset preset) {
      SetFontScale(fw::FontSizePresetScale(preset));
    }

    /**
     * @brief 按高度级别查询盒阴影参数。
     * @param token 高度 Token。
     * @return 阴影规范（偏移、模糊、不透明度）。
     */
    [[nodiscard]] virtual auto Shadow(ShadowToken token) const -> const ShadowSpec& = 0;

    /**
     * @brief 按 Token 查询缓动曲线类型。
     * @param easing 缓动预设。
     * @return 适合用于 QPropertyAnimation 的 QEasingCurve::Type 值。
     */
    [[nodiscard]] virtual auto Easing(EasingToken easing) const -> int = 0;

    /**
     * @brief 查询全局默认弹簧动力学参数。
     *
     * 当选择 EasingToken::Spring 且没有显式 SpringSpec 覆盖时，
     * 由 AnimationService 使用。可以通过调色板文件中的 `"spring"` JSON 对象
     * 按主题定制。
     *
     * @return 包含 mass、stiffness、damping 值的 SpringSpec。
     */
    [[nodiscard]] virtual auto Spring() const -> const fw::SpringSpec& = 0;

    // ========================================================================
    // Icon Resolution (asset:// URI)
    // ========================================================================

    /**
     * @brief 将图标 URI 解析为栅格化的 QPixmap。
     *
     * 返回的像素图已经过主题着色（currentColor 替换为适当的主题颜色）、
     * DPI 感知和密度缩放。结果在内部缓存；对相同的 (iconId, size, color)
     * 三元组重复调用会返回缓存的像素图。
     *
     * @param iconId 图标 URI（例如 "asset://matcha/icons/save"）。空字符串返回空像素图。
     * @param size   目标图标尺寸。
     * @param color  覆盖着色颜色。如果无效，则使用 TextPrimary。
     * @return 栅格化的像素图，如果 iconId 为空或未找到则返回空 QPixmap。
     */
    [[nodiscard]] virtual auto ResolveIcon(const fw::IconId& iconId, fw::IconToken size, QColor color) const -> QPixmap
        = 0;

    /// @brief 使用默认主题前景色的便捷重载。
    [[nodiscard]] auto ResolveIcon(const fw::IconId& iconId, fw::IconToken size) const -> QPixmap {
      return ResolveIcon(iconId, size, QColor{});
    }

    /**
     * @brief 注册一个图标目录以进行自动扫描。
     *
     * 目录中的所有 `.svg` 文件都会在给定的 URI 前缀下注册。
     * 例如，RegisterIconDirectory("asset://matcha/icons/", "/path/to/Icons")
     * 会将 `/path/to/Icons/save.svg` 注册为 `"asset://matcha/icons/save"`。
     *
     * @param uriPrefix URI 前缀（例如 "asset://matcha/icons/"）。
     * @param dirPath   图标目录的绝对文件系统路径。
     * @return 注册的图标数量。
     */
    virtual auto RegisterIconDirectory(std::string_view uriPrefix, const QString& dirPath) -> int = 0;

    /**
     * @brief 使图标缓存失效（在 ThemeChanged 时内部调用）。
     */
    virtual void InvalidateIconCache() = 0;

    // ========================================================================
    // Per-Widget Composite Style Sheet
    // ========================================================================

    /**
     * @brief 解析某个控件种类的复合样式表。
     *
     * 返回预构建的 `WidgetStyleSheet` 的引用，其中包含几何 Token 和变体颜色映射。
     * 这是控件 `paintEvent` 实现的主要 API。
     *
     * @param kind 控件类型标识符。
     * @return 样式表的 const 引用（在下一次 `SetTheme()` 调用之前有效）。
     */
    [[nodiscard]] virtual auto ResolveStyleSheet(WidgetKind kind) const -> const WidgetStyleSheet& = 0;

    // ========================================================================
    // Declarative Style Resolution (RFC-07)
    // ========================================================================
    /**
     * @brief 为指定变体和状态的控件解析完全具体的样式。
     *
     * 这是 RFC-07 声明式控件绘制的核心 API。返回一个 `ResolvedStyle`，
     * 其中所有 Token 已解析为具体的 Qt 值：QColor、密度缩放后的像素值、
     * QFont、ShadowSpec 和过渡参数。
     *
     * 控件在 paintEvent 中调用此方法一次，而不是多次调用 `Color()`/`Font()`。
     *
     * @param kind         控件类型。
     * @param variantIndex `WidgetStyleSheet::variants` 中的索引（单变体控件为 0）。
     * @param state        当前交互状态。
     * @return 完全解析的样式快照。
     *
     * @see docs/07_Declarative_Style_RFC.md 第 5.5 节
     */
    [[nodiscard]] virtual auto Resolve(WidgetKind kind, std::size_t variantIndex, InteractionState state) const
        -> ResolvedStyle = 0;

    /**
     * @brief 带实例级样式覆盖的解析（级联层 3）。
     *
     * 与 `Resolve(kind, variantIndex, state)` 相同，但在其之上应用一个稀疏的
     * 实例级补丁。这是样式级联中的最高优先级层，当某个特定控件实例需要偏离
     * 其类级样式时使用。
     *
     * @param kind         控件类型。
     * @param variantIndex 变体索引。
     * @param state        当前交互状态。
     * @param instanceOverride 非空指针，指向实例覆盖。
     * @return 应用实例补丁后的解析样式。
     *
     * @see InstanceStyleOverride, §4.17
     */
    [[nodiscard]] virtual auto Resolve(
        WidgetKind kind, std::size_t variantIndex, InteractionState state, const InstanceStyleOverride& instanceOverride
    ) const -> ResolvedStyle = 0;

    // ========================================================================
    // 组件 Token 覆盖
    // ========================================================================

    /**
     * @brief 每个控件类的 Token 覆盖。
     *
     * 仅在控件需要偏离全局默认值时使用。例如，
     * `Dialog.radius = RadiusToken::Large` 覆盖全局的 `Default`。
     * 每个控件类注册一次，通常在模块初始化时进行。
     */
    struct ComponentOverride {
      WidgetKind kind;                          // 控件类型
      std::optional<RadiusToken> radius;        // 圆角半径
      std::optional<SpaceToken> paddingH;       // 水平内边距
      std::optional<SpaceToken> paddingV;       // 垂直内边距
      std::optional<SpaceToken> gap;            // 图标-文本间距、项目间距
      std::optional<SizeToken> minHeight;       // 最小高度
      std::optional<FontRole> font;             // 字体角色
      std::optional<ShadowToken> elevation;     // 阴影深度
      std::optional<TransitionDef> transition;  // 动画持续时间和缓动曲线
    };

    /**
     * @brief 注册组件级别的 Token 覆盖。
     *
     * 覆盖在下次 `SetTheme()` 调用时应用（如果主题已经激活则立即应用）。
     * 多次调用会累积；同一 `WidgetKind` 字段的后续注册会获胜。
     *
     * @param overrides 覆盖定义列表的 span。
     */
    virtual void RegisterComponentOverrides(std::span<const ComponentOverride> overrides) = 0;

    // ========================================================================
    // 动态 Token（插件扩展，ADR-015）
    // ========================================================================

    /**
     * @brief 插件注册的动态颜色 Token 的定义。
     *
     * 插件注册领域特定的颜色（例如 "FEA/MeshQualityBad"），这些颜色是主题感知的，
     * 但不属于核心 Token 词汇表。
     */
    struct DynamicColorDef {
      std::string_view key;  ///< 唯一键，例如 "FEA/MeshQualityBad"
      QColor lightValue;     ///< Light 主题下的颜色值
      QColor darkValue;      ///< Dark 主题下的颜色值
    };

    /**
     * @brief 插件注册的动态字体 Token 的定义。
     */
    struct DynamicFontDef {
      std::string_view key;  ///< 唯一键，例如 "CAD/PropertyGrid"
      FontSpec value;        ///< 字体规范（主题无关）
    };

    /**
     * @brief 插件注册的动态间距 Token 的定义。
     *
     * 存储的值是基础像素计数（密度缩放前）。
     */
    struct DynamicSpacingDef {
      std::string_view key;  ///< 唯一键，例如 "FEA/ResultMargin"
      int basePx;            ///< 基础像素值（查询时进行密度缩放）
    };

    /**
     * @brief 从插件注册动态颜色 Token。
     * @param defs 动态颜色定义列表的 span。
     */
    virtual void RegisterDynamicTokens(std::span<const DynamicColorDef> defs) = 0;

    /**
     * @brief 从插件注册动态字体 Token。
     */
    virtual void RegisterDynamicFonts(std::span<const DynamicFontDef> defs) = 0;

    /**
     * @brief 从插件注册动态间距 Token。
     */
    virtual void RegisterDynamicSpacings(std::span<const DynamicSpacingDef> defs) = 0;

    /**
     * @brief 按键查询动态颜色 Token。
     * @return 当前主题下解析出的颜色，如果未注册则返回 `std::nullopt`。
     */
    [[nodiscard]] virtual auto DynamicColor(std::string_view key) const -> std::optional<QColor> = 0;

    /**
     * @brief 按键查询动态字体 Token。
     * @return 字体规范，如果未注册则返回 `std::nullopt`。
     */
    [[nodiscard]] virtual auto DynamicFont(std::string_view key) const -> std::optional<FontSpec> = 0;

    /**
     * @brief 按键查询动态间距 Token（密度缩放后）。
     * @return 密度缩放后的像素值，如果未注册则返回 `std::nullopt`。
     */
    [[nodiscard]] virtual auto DynamicSpacingPx(std::string_view key) const -> std::optional<int> = 0;

    /**
     * @brief 注销之前注册的动态 Token（所有类型）。
     * @param keys 要移除的键字符串列表的 span。
     */
    virtual void UnregisterDynamicTokens(std::span<const std::string_view> keys) = 0;

    // ========================================================================
    // 测试支持
    // ========================================================================

    /**
     * @brief 覆盖所有动画持续时间以实现确定性测试。
     *
     * `WidgetTestFixture` 调用 `SetAnimationOverride(0)` 强制所有
     * `AnimationMs()` 查询返回 0ms，从而消除依赖时间的测试失败。
     *
     * @param forceMs 强制持续时间（毫秒）。传入 -1 恢复真实值。
     */
    virtual void SetAnimationOverride(int forceMs) = 0;

   Q_SIGNALS:
    /**
     * @brief 在 `SetTheme()` 完成 Token 重建后发出。
     *
     * 所有 `ThemeAware` 控件连接到此信号以重新获取它们的
     * `WidgetStyleSheet` 并调用 `OnThemeChanged()`。
     *
     * @param newTheme 新激活的主题名称。
     */
    void ThemeChanged(const QString& newTheme);

   protected:
    /// @brief 受保护的构造函数 —— 仅允许子类实例化。
    using QObject::QObject;
  };

  // ============================================================================
  // 全局主题访问器
  // ============================================================================

  /// @brief 设置全局主题服务实例。由 Application::Initialize() 调用一次。
  /// @param svc 指向主题服务的指针（必须比所有控件寿命更长）。nullptr 用于清除。
  MATCHA_EXPORT void SetThemeService(IThemeService* svc);

  /// @brief 获取全局主题服务实例。
  /// @return 对活动主题服务的引用。
  /// @pre 必须已使用非空指针调用 SetThemeService()。
  [[nodiscard]] MATCHA_EXPORT auto GetThemeService() -> IThemeService&;

  /// @brief 检查全局主题服务是否已设置。
  [[nodiscard]] MATCHA_EXPORT auto HasThemeService() -> bool;
}  // namespace matcha::gui
