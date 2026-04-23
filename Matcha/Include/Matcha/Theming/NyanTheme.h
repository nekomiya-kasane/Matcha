#pragma once

/**
 * @file NyanTheme.h
 * @brief 带有 JSON 调色板加载功能的 IThemeService 具体实现。
 *
 * `NyanTheme` 是 `IThemeService` 的唯一具体实现。
 * 它加载 Light.json / Dark.json 种子调色板（直接包含 71 个 ColorToken 值），
 * 检测平台字体，根据高度级别推导阴影参数，并为每种 WidgetKind 构建
 * 带有默认几何 Token + 变体颜色矩阵的样式表。
 *
 * 存储使用平坦数组以实现 O(1) 查找。所有 const 查询方法都是无锁读取 ——
 * 数组在两次 `SetTheme()` 调用之间是不可变的。
 *
 * @see 05_Greenfield_Plan.md 第 2.6 节 NyanTheme 设计
 * @see IThemeService.h 抽象接口定义
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/IThemeService.h>

#include <QColor>
#include <QEasingCurve>
#include <QPixmap>
#include <QString>
#include <array>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace matcha::gui {

  /**
   * @brief 具体的主题引擎实现。
   *
   * **生命周期**：
   * 1. 使用调色板搜索路径（包含 Light.json / Dark.json 的目录）构造。
   * 2. 调用 `SetTheme(kThemeLight)` 执行初始 Token 构建。
   * 3. 控件通过 `IThemeService` 接口查询 Token。
   * 4. 调用 `SetTheme(kThemeDark)` 切换主题；所有 ThemeAware 控件自动更新。
   *
   * **线程安全**：所有 const 查询方法对于并发读取是安全的。
   * `SetTheme()` 必须仅从 GUI 线程调用（会发出信号）。
   */
  class MATCHA_EXPORT NyanTheme : public IThemeService {
    Q_OBJECT

   public:
    /**
     * @brief 使用调色板目录路径构造。
     * @param palettePath 包含 Light.json 和 Dark.json 的目录路径。
     * @param parent 可选的 QObject 父对象。
     */
    explicit NyanTheme(QString palettePath, QObject* parent = nullptr);

    ~NyanTheme() override = default;

    NyanTheme(const NyanTheme&) = delete;
    NyanTheme& operator=(const NyanTheme&) = delete;
    NyanTheme(NyanTheme&&) = delete;
    NyanTheme& operator=(NyanTheme&&) = delete;

    // -- ITokenRegistry 接口实现 --

    void SetDensity(fw::DensityLevel level) override;
    [[nodiscard]] auto CurrentDensity() const -> fw::DensityLevel override;
    [[nodiscard]] auto CurrentDensityScale() const -> float override;
    void SetDirection(fw::TextDirection dir) override;
    [[nodiscard]] auto CurrentDirection() const -> fw::TextDirection override;
    [[nodiscard]] auto SpacingPx(fw::SpaceToken token) const -> int override;
    [[nodiscard]] auto Radius(fw::RadiusToken token) const -> int override;
    [[nodiscard]] auto AnimationMs(fw::AnimationsToken speed) const -> int override;
    [[nodiscard]] auto TimingMs(fw::TimingTokenId id) const -> int override;
    void SetTimingOverride(fw::TimingTokenId id, int ms) override;

    // -- IThemeService 接口实现 --

    void SetTheme(const QString& name) override;

    [[nodiscard]] auto CurrentTheme() const -> const QString& override;

    [[nodiscard]] auto CurrentMode() const -> ThemeMode override;

    auto RegisterTheme(const QString& name, const QString& jsonPath, ThemeMode mode) -> bool override;

    [[nodiscard]] auto Color(ColorToken token) const -> QColor override;
    [[nodiscard]] auto Color(ColorToken token, InteractionState state) const -> QColor override;
    [[nodiscard]] auto Font(FontRole role) const -> const FontSpec& override;
    void SetFontScale(float factor) override;
    [[nodiscard]] auto FontScale() const -> float override;
    [[nodiscard]] auto Shadow(ShadowToken token) const -> const ShadowSpec& override;
    [[nodiscard]] auto Easing(EasingToken easing) const -> int override;

    [[nodiscard]] auto ResolveStyleSheet(WidgetKind kind) const -> const WidgetStyleSheet& override;

    [[nodiscard]] auto Resolve(WidgetKind kind, std::size_t variantIndex, InteractionState state) const
        -> ResolvedStyle override;

    [[nodiscard]] auto Resolve(
        WidgetKind kind, std::size_t variantIndex, InteractionState state, const InstanceStyleOverride& instanceOverride
    ) const -> ResolvedStyle override;

    void RegisterComponentOverrides(std::span<const ComponentOverride> overrides) override;

    [[nodiscard]] auto ResolveIcon(const fw::IconId& iconId, fw::IconToken size, QColor color) const
        -> QPixmap override;
    void InvalidateIconCache() override;
    auto RegisterIconDirectory(std::string_view uriPrefix, const QString& dirPath) -> int override;

    void RegisterDynamicTokens(std::span<const DynamicColorDef> defs) override;
    void RegisterDynamicFonts(std::span<const DynamicFontDef> defs) override;
    void RegisterDynamicSpacings(std::span<const DynamicSpacingDef> defs) override;
    [[nodiscard]] auto DynamicColor(std::string_view key) const -> std::optional<QColor> override;
    [[nodiscard]] auto DynamicFont(std::string_view key) const -> std::optional<FontSpec> override;
    [[nodiscard]] auto DynamicSpacingPx(std::string_view key) const -> std::optional<int> override;
    void UnregisterDynamicTokens(std::span<const std::string_view> keys) override;

    void SetAnimationOverride(int forceMs) override;
    [[nodiscard]] auto Spring() const -> const fw::SpringSpec& override;

   private:
    /// @brief 加载 JSON 调色板文件并填充颜色数组。
    void LoadPalette(const QString& themeName);

    /// @brief 检测平台字体并填充字体数组。
    void BuildFonts();

    /// @brief 根据高度级别推导阴影参数。
    void BuildShadows();

    /// @brief 为每种 WidgetKind 构建 WidgetStyleSheet。
    void BuildStyleSheets();

    /// @brief 应用已注册的组件覆盖到样式表。
    void ApplyComponentOverrides();

    /// @brief 从设计 Token 构建并应用全局 QSS 样式表。
    void BuildGlobalStyleSheet();

    /// @brief 查询操作系统的交互计时值（Windows SPI）。
    void QueryPlatformTimings();

    /// @brief 为某个控件种类构建默认的变体颜色映射。
    void BuildDefaultVariants(WidgetKind kind);

    // -- 存储 --

    /// @brief 文档8.9提及的内部成员
    /// 颜色 Token 存储 初始化 LoadPalette()
    std::array<QColor, kColorTokenCount> _colors{};
    /// @brief 字体 Token 存储 初始化 BuildFonts()
    std::array<FontSpec, kFontRoleCount> _fonts{};
    /// @brief 阴影 Token 存储 初始化 BuildShadows()
    std::array<ShadowSpec, kShadowTokenCount> _shadows{};
    /// @brief 补充:动画 Token 存储 Easing()函数可直接返回映射，
    /// 所以文档中提及的在构造函数中静态初始化是否需要保留
    std::array<QEasingCurve::Type, fw::kEasingTokenCount> _easings{};
    /// @brief 样式表 Token 存储 初始化 BuildDefaultVariants()
    std::array<WidgetStyleSheet, kWidgetKindCount> _styleSheets{};
    /// @brief 图标注册表：URI -> 文件系统路径（由 RegisterIconDirectory 自动填充）。
    std::unordered_map<std::string, QString> _iconRegistry;
    /// @brief 动态颜色 Token，按键字符串索引。
    struct DynamicColorEntry {
      QColor lightValue;
      QColor darkValue;
    };
    /// @brief 动态颜色 Token 存储 初始化 RegisterDynamicTokens()
    std::unordered_map<std::string, DynamicColorEntry> _dynamicColors;
    /// @brief 动态字体 Token 存储 初始化 RegisterDynamicFonts()
    std::unordered_map<std::string, FontSpec> _dynamicFonts;
    /// @brief 动态间距 Token 存储 初始化 RegisterDynamicSpacings()
    std::unordered_map<std::string, int> _dynamicSpacings;
    /// @brief 已注册的主题条目：JSON 路径 + 亮色/暗色分类
    struct ThemeEntry {
      QString jsonPath;
      ThemeMode mode;
    };
    /// @brief 动态主题注册表：主题名称 -> 主题条目 初始化 RegisterTheme()
    std::unordered_map<std::string, ThemeEntry> _themeRegistry;
    /// @brief 动态组件覆盖注册表：组件类型 -> 组件覆盖 初始化 RegisterComponentOverrides()
    std::vector<ComponentOverride> _overrideRegistry;
    /// @brief 当前应用的主题名称 初始化 SetTheme()
    QString _currentTheme = kThemeLight;
    /// @brief 当前应用的主题模式 初始化 SetTheme()
    ThemeMode _currentMode = ThemeMode::Light;
    /// @brief 当前应用的字体缩放系数 初始化在构造函数中，默认1.0F
    float _fontScale = 1.0F;  ///< 全局字体缩放系数
    /// @brief 当前应用的高度级别 初始化在构造函数中，有默认值
    fw::DensityLevel _density = fw::DensityLevel::Default;
    /// @brief 当前应用的文本方向 初始化在构造函数中，有默认值LTR
    fw::TextDirection _direction = fw::TextDirection::LTR;
    /// @brief 当前应用的默认弹簧参数 初始化 LoadPalette()
    fw::SpringSpec _springS{};  ///< 全局默认弹簧参数（JSON 可配置）
    /// @brief 当前应用的调色板文件路径 初始化在构造函数中
    QString _palettePath;


    /// @brief 仅在代码中出现的成员变量，暂未在文档中提及
    int _animOverride = -1;   ///< -1 = 使用真实值

    /// @brief 交互计时 Token 存储（§8.7）。初始化为 kDefaultTimingMs，
    /// 然后用操作系统平台查询结果修补。
    std::array<int, fw::kTimingTokenCount> _timingMs = fw::kDefaultTimingMs;

    /// @brief 每个 Token 的覆盖值。-1 = 无覆盖（使用 _timingMs 的值）。
    std::array<int, fw::kTimingTokenCount> _timingOverrides = []() consteval {
      std::array<int, fw::kTimingTokenCount> a{};
      for (auto& v : a) {
        v = -1;
      }
      return a;
    }();


    /// @brief 来自调色板的每 FontRole JSON 覆盖（大小、字重）。
    /// 由 LoadPalette() 填充，由 BuildFonts() 消费。
    struct FontOverride {
      std::optional<int> sizeInPt;
      std::optional<int> weight;
    };
    std::array<FontOverride, kFontRoleCount> _fontOverrides{};


    /// @brief 每个控件种类的变体存储（自有，样式表指向这些存储区）。
    std::array<std::vector<VariantStyle>, kWidgetKindCount> _variantStorage{};



    /// @brief 图标缓存：键 = (iconId + sizePx + colorRgb + rtlFlip) -> QPixmap。
    struct IconCacheKey {
      std::string iconId;
      int sizePx;
      QRgb colorRgb;
      bool rtlFlip;

      auto operator==(const IconCacheKey&) const -> bool = default;
    };
    struct IconCacheKeyHash {
      auto operator()(const IconCacheKey& k) const noexcept -> std::size_t {
        constexpr std::size_t kGolden = 0x9e3779b9ULL;
        std::size_t h = std::hash<std::string>{}(k.iconId);
        h ^= std::hash<int>{}(k.sizePx) + kGolden + (h << 6U) + (h >> 2U);
        h ^= std::hash<uint32_t>{}(static_cast<uint32_t>(k.colorRgb)) + kGolden + (h << 6U) + (h >> 2U);
        h ^= std::hash<bool>{}(k.rtlFlip) + kGolden + (h << 6U) + (h >> 2U);
        return h;
      }
    };
    mutable std::unordered_map<IconCacheKey, QPixmap, IconCacheKeyHash> _iconCache;
    mutable std::mutex _iconCacheMutex;


  };

}  // namespace matcha::gui