#pragma once

/**
 * @file TokenEnums.h
 * @brief matcha::fw 层的无 Qt 依赖设计 Token 枚举定义
 *
 * 本文件中的枚举在 fw（UiNode 层）和 gui（Widget 层）之间共享。
 * 它们不依赖任何 Qt 类型 —— 仅使用 <cstdint> 和 <utility>。
 *
 * 定义的枚举包括：
 *   SpaceToken, RadiusToken, ShadowToken, LineToken, IconSizeToken,
 *   ControlHeightToken, PopupWidthToken, LayerToken, AnimationsToken,
 *   InteractionState, DensityLevel, TextDirection, EasingToken,
 *   SpringSpec, FontSizePreset, SizeToken, TimingTokenId, AnimationPropertyId,
 *   AnimatableValue, TransitionHandle, TransitionDef.
 *
 * @see COCAUI_Design_System_Specification.md 第4、5章及附录A。
 */

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace matcha::fw {

  // ============================================================================
  // 交互状态（不属于Token，文档第 1.6 节表格内有提到）
  // ============================================================================

  enum class InteractionState : uint8_t {
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Focused,
    Selected,
    Error,
    DragOver,

    Count_
  };

  inline constexpr auto kInteractionStateCount = static_cast<std::size_t>(InteractionState::Count_);

  // ============================================================================
  // 字体缩放系统（文档第 3.4 节）
  // ============================================================================

  enum class FontSizePreset : uint8_t {
    Small = 0,
    Medium = 1,
    Large = 2,

    Count_ = 3
  };

  inline constexpr auto kFontSizePresetCount = static_cast<std::size_t>(FontSizePreset::Count_);

  /// 字体缩放因子数组，按 FontSizePreset 索引
  inline constexpr std::array<float, kFontSizePresetCount> kFontSizePresetFactors = {
    0.875F,
    1.000F,
    1.250F,
  };

  /**
   * @brief 获取字体缩放系统对应的缩放因子
   * @param preset 字体缩放系统
   * @return 字体缩放系统对应的缩放因子
   */
  [[nodiscard]] constexpr auto FontSizePresetScale(FontSizePreset preset) noexcept -> float {
    const auto idx = std::to_underlying(preset);
    if (idx >= kFontSizePresetCount) {
      return 1.0F;
    }
    return kFontSizePresetFactors[idx];
  }

  /// 字体缩放因子最小值
  inline constexpr float kFontScaleMin = 0.5F;
  /// 字体缩放因子最大值
  inline constexpr float kFontScaleMax = 3.0F;

  // ============================================================================
  // 间距 Token（文档第 4.1 节）
  // ============================================================================

  /**
   * @brief 以逻辑像素为单位的间距值（密度缩放前的基础值）
   */
  enum class SpaceToken : uint8_t {
    sizeStep = 4,  // Seed token
    sizeUnit = 4,  // Seed token
    spaceNone = 0,
    marginXXXS = 2,
    marginXXS = 4,
    marginXS = 8,
    marginSM = 12,
    marginMS = 16,
    marginMD = 20,
    marginLG = 24,
    marginXL = 28,
    marginXXL = 32,
    paddingXXXS = 2,
    paddingXXS = 4,
    paddingXS = 8,
    paddingSM = 12,
    paddingMS = 16,
    paddingMD = 20,
    paddingLG = 24,
    paddingXL = 28,
    paddingXXL = 32,

    Count_ = 19  ///< Number of defined spacing values. NOT a pixel value.
  };

  /**
   * @brief 将 SpaceToken 转换为其基础像素值(在密度缩放前).
   * @param s Space Token.
   * @return 基础像素值为int.
   */
  [[nodiscard]] constexpr auto ToPixels(SpaceToken s) noexcept -> int {
    return static_cast<int>(std::to_underlying(s));
  }

  // ============================================================================
  // 密度级别（不属于Token，文档第 4.2 节提到,能影响其他Token）
  // ============================================================================

  enum class DensityLevel : uint8_t {
    Compact = 0,
    Default = 1,
    Comfortable = 2,

    Count_ = 3
  };

  inline constexpr auto kDensityLevelCount = static_cast<std::size_t>(DensityLevel::Count_);

  /// 密度缩放因子数组，按 DensityLevel 索引
  inline constexpr std::array<float, kDensityLevelCount> kDensityScaleFactors = {
    0.875F,  // Compact
    1.000F,  // Default
    1.125F,  // Comfortable
  };

  /**
   * @brief 获取密度级别对应的缩放因子
   * @param level 密度级别
   * @return 密度级别对应的缩放因子
   */
  [[nodiscard]] constexpr auto DensityScale(DensityLevel level) noexcept -> float {
    const auto idx = std::to_underlying(level);
    if (idx >= kDensityLevelCount) {
      return 1.0F;
    }
    return kDensityScaleFactors[idx];
  }

  // ============================================================================
  // 圆角 Token（文档第 4.3 节）
  // ============================================================================

  /**
   * @brief 以逻辑像素为单位的圆角半径值
   *
   * 枚举底层值即为像素值（Count_ 除外）
   */
  enum class RadiusToken : uint8_t {
    borderRadius = 3,  // Seed token
    borderRadiusNone = 0,
    borderRadiusSM = 1,
    borderRadiusMD = 3,
    borderRadiusLG = 6,
    borderRadiusRound = 255,  // Capsule shape

    Count_ = 6
  };

  /**
   * @brief 将 RadiusToken 转换为整数像素值.
   * @param r Radius Token.
   * @return像素值为int。对于圆形（255）,应使用最小（宽度，高度）/2。
   */
  [[nodiscard]] constexpr auto ToPixels(RadiusToken r) noexcept -> int {
    return static_cast<int>(std::to_underlying(r));
  }

  // ============================================================================
  // 线条 Token（文档第 4.4 节）
  // ============================================================================

  /**
   * @brief Line width values in logical pixels.
   */
  enum class LineToken : uint8_t {
    lineStyle = 10,  // seedToken
    lineStyleSolid,
    lineStyleDashed,
    lineStyleDotted,
    lineWidth = 1,  // seedToken
    lineWidth0 = 0,
    lineWidth1 = 1,
    lineWidth2 = 2,
    lineWidth3 = 3,
    lineWidth4 = 4,

    Count_ = 10  ///< Number of defined line values. NOT a pixel value.
  };
  inline constexpr auto kLineTokenCount = static_cast<std::size_t>(LineToken::Count_);

  // ============================================================================
  // 图标尺寸 Token / 图标标识（文档第 4.5 节、第 16 章）
  // ============================================================================

  /**
   * @brief 图标标识符 —— 一个资源 URI 字符串
   *
   * 格式：`"asset://<authority>/<path>"`。
   * - 框架内置图标：`"asset://matcha/icons/<name>"`（例如 `"asset://matcha/icons/save"`）
   * - 应用程序图标：`"asset://nyancad/icons/<name>"`
   * - 插件图标：`"asset://<plugin-id>/icons/<name>"`
   *
   * 空字符串表示“无图标”。
   */
  using IconId = std::string;

  /**
   * @brief 框架内置图标的 URI 前缀
   */
  inline constexpr std::string_view kMatchaIconPrefix = "asset://matcha/icons/";

  /**
   * @brief 常用框架图标 URI 常量
   *
   * 提供编译期安全的图标引用。应用程序可直接使用字符串字面量定义自定义图标。
   */
  namespace icons {

    // 窗口装饰
    inline constexpr std::string_view Close = "asset://matcha/icons/close";
    inline constexpr std::string_view Minimize = "asset://matcha/icons/minimize";
    inline constexpr std::string_view Maximize = "asset://matcha/icons/maximize";
    inline constexpr std::string_view Restore = "asset://matcha/icons/restore";
    inline constexpr std::string_view Pin = "asset://matcha/icons/pin";
    inline constexpr std::string_view Unpin = "asset://matcha/icons/unpin";

    // 导航
    inline constexpr std::string_view ChevronLeft = "asset://matcha/icons/chevron-left";
    inline constexpr std::string_view ChevronRight = "asset://matcha/icons/chevron-right";
    inline constexpr std::string_view ChevronUp = "asset://matcha/icons/chevron-up";
    inline constexpr std::string_view ChevronDown = "asset://matcha/icons/chevron-down";
    inline constexpr std::string_view ArrowLeft = "asset://matcha/icons/arrow-left";
    inline constexpr std::string_view ArrowRight = "asset://matcha/icons/arrow-right";
    inline constexpr std::string_view ArrowUp = "asset://matcha/icons/arrow-up";
    inline constexpr std::string_view ArrowDown = "asset://matcha/icons/arrow-down";
    inline constexpr std::string_view Home = "asset://matcha/icons/home";
    inline constexpr std::string_view Back = "asset://matcha/icons/back";
    inline constexpr std::string_view Forward = "asset://matcha/icons/forward";

    // 数据操作
    inline constexpr std::string_view Search = "asset://matcha/icons/search";
    inline constexpr std::string_view Filter = "asset://matcha/icons/filter";
    inline constexpr std::string_view Sort = "asset://matcha/icons/sort";
    inline constexpr std::string_view SortAsc = "asset://matcha/icons/sort-asc";
    inline constexpr std::string_view SortDesc = "asset://matcha/icons/sort-desc";

    // 增删改查
    inline constexpr std::string_view Add = "asset://matcha/icons/add";
    inline constexpr std::string_view Remove = "asset://matcha/icons/remove";
    inline constexpr std::string_view Edit = "asset://matcha/icons/edit";
    inline constexpr std::string_view Delete = "asset://matcha/icons/delete";
    inline constexpr std::string_view Copy = "asset://matcha/icons/copy";
    inline constexpr std::string_view Paste = "asset://matcha/icons/paste";
    inline constexpr std::string_view Cut = "asset://matcha/icons/cut";
    inline constexpr std::string_view Undo = "asset://matcha/icons/undo";
    inline constexpr std::string_view Redo = "asset://matcha/icons/redo";

    // 文档
    inline constexpr std::string_view Save = "asset://matcha/icons/save";
    inline constexpr std::string_view SaveAs = "asset://matcha/icons/save-as";
    inline constexpr std::string_view Open = "asset://matcha/icons/open";
    inline constexpr std::string_view NewFile = "asset://matcha/icons/new-file";
    inline constexpr std::string_view NewFolder = "asset://matcha/icons/new-folder";

    // 视口
    inline constexpr std::string_view ZoomIn = "asset://matcha/icons/zoom-in";
    inline constexpr std::string_view ZoomOut = "asset://matcha/icons/zoom-out";
    inline constexpr std::string_view ZoomFit = "asset://matcha/icons/zoom-fit";
    inline constexpr std::string_view FullScreen = "asset://matcha/icons/fullscreen";
    inline constexpr std::string_view RotateLeft = "asset://matcha/icons/rotate-left";
    inline constexpr std::string_view RotateRight = "asset://matcha/icons/rotate-right";

    // 状态
    inline constexpr std::string_view Info = "asset://matcha/icons/info";
    inline constexpr std::string_view Warning = "asset://matcha/icons/warning";
    inline constexpr std::string_view Error = "asset://matcha/icons/error";
    inline constexpr std::string_view Success = "asset://matcha/icons/success";
    inline constexpr std::string_view Help = "asset://matcha/icons/help";
    inline constexpr std::string_view Question = "asset://matcha/icons/question";

    // 杂项
    inline constexpr std::string_view Settings = "asset://matcha/icons/settings";
    inline constexpr std::string_view Menu = "asset://matcha/icons/menu";
    inline constexpr std::string_view MoreHorizontal = "asset://matcha/icons/more-horizontal";
    inline constexpr std::string_view MoreVertical = "asset://matcha/icons/more-vertical";
    inline constexpr std::string_view Check = "asset://matcha/icons/check";
    inline constexpr std::string_view Cross = "asset://matcha/icons/cross";
    inline constexpr std::string_view Refresh = "asset://matcha/icons/refresh";
    inline constexpr std::string_view Download = "asset://matcha/icons/download";
    inline constexpr std::string_view Upload = "asset://matcha/icons/upload";
    inline constexpr std::string_view Eye = "asset://matcha/icons/eye";
    inline constexpr std::string_view EyeOff = "asset://matcha/icons/eye-off";
    inline constexpr std::string_view Lock = "asset://matcha/icons/lock";
    inline constexpr std::string_view Unlock = "asset://matcha/icons/unlock";

  }  // namespace icons

  /**
   * @brief 判断图标在 RTL 模式下是否应水平翻转
   *
   * 方向性图标（箭头、返回/前进）在 UI 方向为 RTL 时会镜像，使得“左”变为“右”。
   *
   * @param iconId 要检查的图标 URI
   * @return 如果应在 RTL 布局中翻转则返回 true
   */
  [[nodiscard]] inline auto IsRtlFlippable(std::string_view iconId) noexcept -> bool {
    return iconId == icons::ChevronLeft || iconId == icons::ChevronRight || iconId == icons::ArrowLeft
           || iconId == icons::ArrowRight || iconId == icons::Back || iconId == icons::Forward;
  }

  /**
   * @brief 以逻辑像素为单位的图标尺寸值
   */
  enum class IconToken : uint8_t {
    iconSize = 20,  // seedToken
    iconSizeXXXXXS = 4,
    iconSizeXXXXS = 6,
    iconSizeXXXS = 8,
    iconSizeXXS = 10,
    iconSizeXS = 12,
    iconSizeSM = 16,
    iconSizeMS = 20,
    iconSizeMD = 24,
    iconSizeLG = 32,
    iconSizeXL = 40,
    iconSizeXXL = 56,
    iconSizeXXXL = 64,

    Count_ = 13  ///< Number of defined icon size values. NOT a pixel value.
  };

  /**
   * @brief 将 IconSizeToken 转换为整数像素值.
   * @param i IconToken Token.
   * @return 像素值为int。
   */
  [[nodiscard]] constexpr auto ToPixels(IconToken i) noexcept -> int {
    return static_cast<int>(std::to_underlying(i));
  }

  // ============================================================================
  // 控件高度 Token（文档第 4.6 节）
  // ============================================================================

  /**
   * @brief 以逻辑像素为单位的控件高度值
   */
  enum class ControlHeightToken : uint8_t {
    controlHeight = 24,  // seedToken
    controlHeightXS = 20,
    controlHeightSM = 24,
    controlHeightMS = 28,
    controlHeightMD = 32,
    controlHeightLG = 36,
    controlHeightXL = 40,
    controlHeightXXL = 44,
    controlHeightXXXL = 48,

    Count_ = 9  ///< Number of defined control height values. NOT a pixel value.
  };

  /**
   * @brief 将 ControlHeightToken 转换为整数像素值.
   * @param h ControlHeight Token.
   * @return 像素值为int。
   */
  [[nodiscard]] constexpr auto ToPixels(ControlHeightToken h) noexcept -> int {
    return static_cast<int>(std::to_underlying(h));
  }

  // ============================================================================
  // 尺寸 Token（文档第 4.6 节、附录 A
  // ============================================================================

  /// 抽象尺寸级别，用于控件高度、弹出宽度等
  enum class SizeToken : uint8_t {
    Xs = 0,
    Sm = 1,
    Md = 2,
    Lg = 3,
    Xl = 4,

    Count_ = 5
  };

  inline constexpr auto kSizeTokenCount = static_cast<std::size_t>(SizeToken::Count_);

  /// SizeToken 对应的基础像素值（密度缩放前）
  inline constexpr std::array<int, kSizeTokenCount> kBaseSizePx = {
    20,  // Xs
    24,  // Sm
    32,  // Md
    40,  // Lg
    48,  // Xl
  };

  [[nodiscard]] constexpr auto ToPixels(SizeToken s) noexcept -> int {
    const auto idx = std::to_underlying(s);
    if (idx >= kSizeTokenCount) {
      return 32;
    }
    return kBaseSizePx[idx];
  }

  // ============================================================================
  // 弹出容器宽度 Token（文档第 4.7 节）
  // ============================================================================

  /**
   * @brief 弹出容器宽度 values in logical pixels.
   */
  enum class PopupWidthToken : uint16_t {
    controlColumnWidth = 72,  // Seed token
    controlGutterWidth = 8,   // Seed token
    controlPopupWidthXS = controlColumnWidth * 2 + controlGutterWidth * 1,
    controlPopupWidthSM = controlColumnWidth * 3 + controlGutterWidth * 2,
    controlPopupWidthMS = controlColumnWidth * 4 + controlGutterWidth * 3,
    controlPopupWidthMD = controlColumnWidth * 6 + controlGutterWidth * 5,
    controlPopupWidthLG = controlColumnWidth * 8 + controlGutterWidth * 7,
    controlPopupWidthXL = controlColumnWidth * 12 + controlGutterWidth * 11,

    Count_ = 8  ///< Number of defined popup width values. NOT a pixel value.
  };

  /**
   * @brief 将 PopupWidthToken 转换为整数像素值.
   * @param w PopupWidth Token.
   * @return 像素值为int。
   */
  [[nodiscard]] constexpr auto ToPixels(PopupWidthToken w) noexcept -> int {
    return static_cast<int>(std::to_underlying(w));
  }

  // ============================================================================
  // 阴影 Token（文档第 4.8 节）
  // ============================================================================

  /**
   *  @brief 阴影强度级别
   */
  enum class ShadowToken : uint8_t {
    shadow,  // Seed token
    boxShadow,
    boxShadowSecondary,
    boxShadowTertiary,

    Count_ = 4  ///< Number of defined shadow values.
  };

  inline constexpr auto kShadowTokenCount = static_cast<std::size_t>(ShadowToken::Count_);

  // ============================================================================
  // 层级 (Z-index) Token（文档第 4.9 节）
  // ============================================================================
  /**
   * @brief 层级 values in logical pixels.
   */
  enum class LayerToken : uint16_t {
    zIndexBase = 0,          // Seed token
    zIndexPopupBase = 1000,  // Seed token
    zIndexElevated = 1010,
    zIndexSticky = 1020,
    zIndexDropdown = 1030,
    zIndexModal = 1040,
    zIndexTooltip = 1050,
    zIndexNotification = 1060,
    zIndexLoading = 1070,
    zIndexMaximum = 9999,

    Count_ = 10  ///< Number of defined layer values. NOT a pixel value.
  };

  inline constexpr auto kZIndexTokenCount = static_cast<std::size_t>(LayerToken::Count_);

  // ============================================================================
  // 动画 Token（文档第 5.1 节）
  // ============================================================================

  /**
   * @brief 动画时长预设
   */
  enum class AnimationsToken : uint16_t {
    motionBase = 0,  ///< 动画基础时长                  //seedToken
    motionDurationFast = 150,
    motionDurationDefault = 200,
    motionDurationSlow = 350,

    Count_ = 4  ///< Number of defined animation values.
  };

  inline constexpr auto kAnimationsTokenCount = static_cast<std::size_t>(AnimationsToken::Count_);

  /**
   * @brief Default animation durations in milliseconds, indexed by AnimationsToken.
   *        动画基础时长为 0ms.其他靠基础时长计算.
   */
  inline constexpr std::array<int, kAnimationsTokenCount> kDefaultAnimationMs = {
    0,    // Motion
    150,  // motionDurationFast
    200,  // motionDurationDefault
    350,  // motionDurationSlow
  };

  // ============================================================================
  // 缓动曲线 Token（文档第 5.2 节）
  // ============================================================================

  enum class EasingToken : uint8_t {
    Linear = 0,      ///< QEasingCurve::Linear 线性缓动曲线
    OutCubic = 1,    ///< QEasingCurve::OutCubic 出缓动曲线
    InOutCubic = 2,  ///< QEasingCurve::InOutCubic 先OutCubic 缓动曲线
    Spring = 3,      ///< QEasingCurve::Spring 弹簧缓动曲线  (这个先不考虑)

    Count_ = 4
  };

  inline constexpr auto kEasingTokenCount = static_cast<std::size_t>(EasingToken::Count_);

  struct SpringSpec {
    float mass = 1.0F;
    float stiffness = 200.0F;
    float damping = 20.0F;
  };
  // ============================================================================
  // 过渡定义（文档第 5.4 节）
  // ============================================================================

  /// 将时长和缓动曲线组合为一个可复用的动画配置
  struct TransitionDef {
    AnimationsToken duration = AnimationsToken::motionDurationDefault;
    EasingToken easing = EasingToken::OutCubic;
  };

  // ============================================================================
  // 交互计时 Token（文档第 5 章、附录 A）
  // ============================================================================

  enum class TimingTokenId : uint8_t {
    HoverDelay = 0,
    TooltipDelay = 1,
    TooltipDismissDelay = 2,
    LongPressThreshold = 3,
    DoubleClickWindow = 4,
    DebounceSearch = 5,
    DebounceResize = 6,
    AutoSaveInterval = 7,
    IdleTimeout = 8,
    RepeatKeyInitial = 9,
    RepeatKeyInterval = 10,
    DragInitDelay = 11,
    ToastDismissTimeout = 12,
    MenuOpenDelay = 13,
    MenuCloseDelay = 14,

    Count_ = 15
  };

  inline constexpr auto kTimingTokenCount = static_cast<std::size_t>(TimingTokenId::Count_);

  /// 默认交互计时值（毫秒）
  inline constexpr std::array<int, kTimingTokenCount> kDefaultTimingMs = {
    200,    // HoverDelay
    500,    // TooltipDelay
    100,    // TooltipDismissDelay
    500,    // LongPressThreshold
    400,    // DoubleClickWindow
    300,    // DebounceSearch
    100,    // DebounceResize
    30000,  // AutoSaveInterval
    60000,  // IdleTimeout
    500,    // RepeatKeyInitial
    33,     // RepeatKeyInterval
    150,    // DragInitDelay
    5000,   // ToastDismissTimeout
    200,    // MenuOpenDelay
    300,    // MenuCloseDelay
  };

  // ============================================================================
  // 动画属性标识（文档第 12.3 节）
  // ============================================================================

  /// 标识可动画化的 Widget 视觉属性
  enum class AnimationPropertyId : uint16_t {
    Opacity = 0,        // 不透明度
    Position = 1,       // 位置
    Size = 2,           // 尺寸
    MinimumHeight = 3,  // 最小高度
    MaximumHeight = 4,  // 最大高度
    Geometry = 5,       // 几何（位置+尺寸）

    BackgroundColor = 10,  // 背景色
    ForegroundColor = 11,  // 前景色（文本/图标）
    BorderColor = 12,      // 边框色

    ArrowRotation = 100,  // 箭头旋转角度
    SlideOffset = 101,    // 滑动偏移量
    ContentHeight = 102,  // 内容高度

    UserDefined = 1000,  // 用户自定义属性起始值
  };

  // ============================================================================
  // 可动画化值（文档第 12.4 节）
  // ============================================================================

  /// 无 Qt 依赖的二维点
  struct Point2D {
    int x = 0;
    int y = 0;
  };

  /// 标签联合体，表示动画系统可插值的值类型
  struct AnimatableValue {
    enum class Type : uint8_t {
      Double,   // 双精度浮点
      Int,      // 整数
      Rgba,     // 打包的 RGBA 颜色
      Point2D,  // 二维点
    };

    Type type = Type::Double;
    double asDouble = 0.0;
    int asInt = 0;
    uint32_t asRgba = 0;  // 格式：0xAARRGGBB
    Point2D asPoint = {};

    /// 从双精度浮点数创建可动画化值
    static constexpr auto FromDouble(double v) -> AnimatableValue {
      AnimatableValue val;
      val.type = Type::Double;
      val.asDouble = v;
      return val;
    }

    /// 从整数创建可动画化值
    static constexpr auto FromInt(int v) -> AnimatableValue {
      AnimatableValue val;
      val.type = Type::Int;
      val.asInt = v;
      return val;
    }

    /// 从 RGBA 颜色创建可动画化值
    static constexpr auto FromRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) -> AnimatableValue {
      AnimatableValue val;
      val.type = Type::Rgba;
      val.asRgba = (static_cast<uint32_t>(a) << 24U) | (static_cast<uint32_t>(r) << 16U)
                   | (static_cast<uint32_t>(g) << 8U) | static_cast<uint32_t>(b);
      return val;
    }

    /// 从二维点创建可动画化值
    static constexpr auto FromPoint(int x, int y) -> AnimatableValue {
      AnimatableValue val;
      val.type = Type::Point2D;
      val.asPoint = {x, y};
      return val;
    }
  };

  // ============================================================================
  // 过渡句柄（文档第 12.5 节）
  // ============================================================================

  /// 不透明的动画句柄，用于取消或查询动画状态
  enum class TransitionHandle : uint64_t { Invalid = 0 };

  // ============================================================================
  // 光标 Token（文档第 17.1 节）
  // ============================================================================

  /**
   * @brief 标准光标词汇。
   *
   * 由 IThemeService 中解析为 QCursor。
   */
  enum class CursorToken : uint8_t {
    Default = 0,
    Pointer,    ///< 手形指针（可点击元素）
    Text,       ///< I 形光标（文本输入）
    Wait,       ///< 忙碌旋转光标
    Crosshair,  ///< 十字准星（精确选择）
    Move,       ///< 四向移动光标
    SplitH,     ///< 水平分割光标
    SplitV,     ///< 垂直分割光标
    ResizeN,    ///< 向上调整大小
    ResizeE,    ///< 向右调整大小
    ResizeNE,   ///< 向上右调整大小
    ResizeNW,   ///< 向上左调整大小
    Forbidden,  ///< 禁止操作光标
    Grab,       ///< 准备抓取（可拖拽）
    Grabbing,   ///< 正在抓取（拖拽中）

    Count_
  };

  // ============================================================================
  // 文本方向（文档第 19.1 节提到）
  // ============================================================================

  enum class TextDirection : uint8_t {
    LTR = 0,
    RTL = 1,
  };
  // ============================================================================
  // Orientation
  // ============================================================================

  /**
   * @brief 布局方向（Qt：：Orientation的无Qt替代）。
   */
  enum class Orientation : uint8_t {
    Horizontal = 0,
    Vertical = 1,
  };
}  // namespace matcha::fw
