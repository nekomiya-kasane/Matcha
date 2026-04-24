#pragma once

/**
 * @file WidgetStyleSheet.h
 * @brief WidgetKind 枚举及 WidgetStyleSheet 值类型。
 *
 * WidgetStyleSheet 是设计令牌与控件绘制之间的桥梁。
 * 它持有每个控件的几何令牌（圆角、内边距、字体、高度、动画），
 * 以及一个 `std::span<const VariantStyle>` 用于变体感知的颜色映射。
 *
 * @see COCAUI_Design_System_Specification.md 第 11 章 WidgetKind 注册表与组件覆盖
 * @see DesignTokens.h WidgetStyleSheet 字段中使用的令牌类型
 */

#include <Matcha/Theming/DesignTokens.h>

#include <cstdint>
#include <span>

namespace matcha::gui {

  // ============================================================================
  // WidgetKind 枚举
  // ============================================================================

  /**
   * @brief 标识每种控件类型，用于按控件解析样式表。
   *
   * 共 54 个条目（规范第 11.1 节），覆盖 Matcha 控件库的全部层级（第 1/2/3 层），
   * 外加 MenuBar/Menu/MenuItem 等菜单系统组件。
   *
   * 用作 `IThemeService::ResolveStyleSheet(WidgetKind)` 的索引。
   */
enum class WidgetKind : uint8_t {
  // -- 第 1 层：核心输入控件 --
    PushButton,
    ToolButton,
    LineEdit,
    SpinBox,
    ComboBox,
    CheckBox,
    RadioButton,
    Slider,
    Toggle,
    Label,
    Tag,

  // -- 第 2 层：容器与布局控件 --
    ScrollArea,
    ScrollBar,
    Panel,
    GroupBox,
    CollapsibleSection,
    TabWidget,
    Dialog,
    PopConfirm,
    ActionBar,
    DataTable,
    ListWidget,
    TableWidget,
    StructureTree,
    ContextMenu,
    StatusBar,
    Splitter,
    PropertyGrid,
    ColorPicker,

  // -- 第 3 层：应用程序级控件 --
    DocumentBar,
    Legend,
    ProgressBar,
    ColorSwatch,
    Paginator,
    SelectionInput,
    StackedWidget,
    Tooltip,

  // -- 菜单系统（规范第 11.1 节）--
    MenuBar,
    Menu,
    MenuItem,
    MenuSeparator,
    MenuCheckItem,

  // -- 通知系统 --
    Notification,

  // -- 标题栏 --
    MainTitleBar,

  // -- 对话框系统 --
    DialogTitleBar,
    DialogFootBar,
    FileDialog,

  // -- ActionBar 系统 --
    ActionTab,
    ActionToolbar,

  // -- 第 3b 阶段新增 --
    Cascader,
    Transfer,
    FormLayout,

  // -- 第 3c 阶段新增 --
    Message,
    Alert,
    Avatar,

  // -- Shell 拆分（TitleBar -> TitleBar + DocumentToolBar + LogoButton）--
    DocumentToolBar,
    LogoButton,

    Count_ ///< Sentinel for array sizing. Must be last.
};

  /// @brief WidgetKind 值的编译期数量
inline constexpr auto kWidgetKindCount = static_cast<std::size_t>(WidgetKind::Count_);

  // ============================================================================
  // WidgetStyleSheet 结构体
  // ============================================================================

  /**
   * @brief 用于绘制的每种控件的复合令牌快照。
   *
   * 将几何令牌（在控件的所有变体间共享）与变体感知的颜色映射组合在一起。
   * 值语义，由 `NyanTheme::SetTheme()` 构造后不可变。
   *
   * **几何字段** 适用于该控件种类的所有变体：
   * - `radius`      -- 圆角半径
   * - `paddingH/V`  -- 水平/垂直内容内边距
   * - `gap`         -- 图标-文本间距、项目间距
   * - `minHeight`   -- 最小组件高度（已密度缩放）
   * - `borderWidth` -- 默认边框描边宽度
   *
   * **字体排版**：`font` -- 文本字体角色
   *
   * **视觉效果**：`elevation`、`layer`
   *
   * **过渡动画**：`transition` -- 状态更改的动画持续时间 + 缓动曲线
   *
   * **变体颜色映射**（`variants` span）：
   * - 简单控件（例如 Label）：1 个 VariantStyle
   * - 多变体控件（例如具有 Primary/Secondary/Ghost/Danger 的 PushButton）：
   *   N 个 VariantStyle 条目，通过控件的变体枚举底层值索引
   *
   * 控件绘制示例（RFC-07 目标模式）：
   * @code
   * auto style = Theme().Resolve(WidgetKind::PushButton,
   *                               std::to_underlying(_variant),
   *                               currentState());
   * p.setBrush(style.background);
   * p.setPen(QPen(style.border, style.borderWidthPx));
   * p.drawRoundedRect(r, style.radiusPx, style.radiusPx);
   * @endcode
   *
   * @see COCAUI_Design_System_Specification.md §9.4 WidgetStyleSheet 结构体
   */
  struct WidgetStyleSheet {
    // -- 几何尺寸（由 IThemeService::Resolve 进行密度缩放）--
    RadiusToken    radius      = RadiusToken::borderRadius;   ///< 圆角半径（默认 3px）
    SpaceToken   paddingH    = SpaceToken::sizeStep;      ///< 水平内容内边距
    SpaceToken   paddingV    = SpaceToken::sizeStep;      ///< 垂直内容内边距
    SpaceToken   gap         = SpaceToken::sizeStep;      ///< 图标-文本间距、项目间距
    SizeToken      minHeight   = SizeToken::Md;          ///< 最小组件高度（默认 32px）
    SpaceToken   borderWidth = SpaceToken::marginXXXS;      ///< 默认边框描边宽度

    // -- 字体排版 --
    FontRole       font        = FontRole::fontSizeMD;         ///< 文本字体角色

    // -- 视觉效果 --
    ShadowToken elevation   = ShadowToken::shadow;   ///< 盒阴影级别
    LayerToken     layer       = LayerToken::zIndexBase;       ///< Z 轴堆叠顺序

    // -- 过渡动画 --
    TransitionDef  transition;                           ///< 动画持续时间 + 缓动曲线

    /// @brief 变体颜色映射（非拥有视图，指向 NyanTheme 存储区）
    std::span<const VariantStyle> variants;
  };

} // namespace matcha::gui
