#pragma once

/**
 * @file WidgetStyleSheet.h
 * @brief WidgetKind enum and WidgetStyleSheet value type.
 *
 * WidgetStyleSheet is the bridge between design tokens and widget painting.
 * It holds per-widget geometry tokens (radius, padding, font, elevation,
 * animation) plus a `std::span<const VariantStyle>` for variant-aware
 * color maps.
 *
 * @see 05_Greenfield_Plan.md ss 2.3 for the WidgetStyleSheet design.
 * @see DesignTokens.h for token types used in WidgetStyleSheet fields.
 */

#include <Matcha/Widgets/Core/DesignTokens.h>

#include <cstdint>
#include <span>

namespace matcha::gui {

// ============================================================================
// WidgetKind
// ============================================================================

/**
 * @brief Identifies each widget type for per-widget style sheet resolution.
 *
 * 35 entries matching the full Matcha widget bank (Tier 1/2/3) plus
 * MenuBar/Menu/MenuItem per VDL Appendix C errata E10.
 *
 * Used as index into `IThemeService::ResolveStyleSheet(WidgetKind)`.
 */
enum class WidgetKind : uint8_t {
    // -- Tier 1: Core Input Widgets --
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

    // -- Tier 2: Container & Layout Widgets --
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

    // -- Tier 3: Application-Level Widgets --
    DocumentBar,
    Legend,
    ProgressBar,
    ColorSwatch,
    Paginator,
    SelectionInput,
    StackedWidget,
    Tooltip,

    // -- Menu system (VDL Appendix C errata E10) --
    MenuBar,
    Menu,
    MenuItem,
    MenuSeparator,
    MenuCheckItem,

    // -- Notification system --
    Notification,

    // -- Title bar --
    MainTitleBar,

    // -- Dialog system --
    DialogTitleBar,
    DialogFootBar,
    FileDialog,

    // -- ActionBar system --
    ActionTab,
    ActionToolbar,

    // -- Phase 3b additions --
    Cascader,
    Transfer,
    FormLayout,

    // -- Phase 3c additions --
    Message,
    Alert,
    Avatar,

    // -- Shell split (TitleBar -> TitleBar + DocumentToolBar + LogoButton) --
    DocumentToolBar,
    LogoButton,

    Count_ ///< Sentinel for array sizing. Must be last.
};

/// @brief Compile-time count of WidgetKind values.
inline constexpr auto kWidgetKindCount = static_cast<std::size_t>(WidgetKind::Count_);

// ============================================================================
// WidgetStyleSheet
// ============================================================================

/**
 * @brief Per-widget-kind composite token snapshot for painting.
 *
 * Combines geometry tokens (shared across all variants of a widget kind)
 * with variant-aware color maps. Value-semantic, immutable after construction
 * by `NyanTheme::SetTheme()`.
 *
 * **Geometry fields** apply to all variants of this widget kind:
 * - `radius`      -- corner radius
 * - `paddingH/V`  -- horizontal / vertical content padding
 * - `gap`         -- icon-text gap, inter-item spacing
 * - `minHeight`   -- minimum component height (density-scaled)
 * - `borderWidth` -- default border stroke width
 *
 * **Typography**: `font` -- text font role
 *
 * **Visual**: `elevation`, `layer`
 *
 * **Transition**: `transition` -- animation duration + easing for state changes
 *
 * **Variant color maps** (`variants` span):
 * - Simple widgets (e.g., Label): 1 VariantStyle
 * - Multi-variant widgets (e.g., PushButton with Primary/Secondary/Ghost/Danger):
 *   N VariantStyle entries indexed by the widget's variant enum underlying value
 *
 * Widget painting example (RFC-07 target pattern):
 * @code
 * auto style = Theme().Resolve(WidgetKind::PushButton,
 *                               std::to_underlying(_variant),
 *                               currentState());
 * p.setBrush(style.background);
 * p.setPen(QPen(style.border, style.borderWidthPx));
 * p.drawRoundedRect(r, style.radiusPx, style.radiusPx);
 * @endcode
 *
 * @see docs/07_Declarative_Style_RFC.md Section 5.4
 */
struct WidgetStyleSheet {
    // -- Geometry (density-scaled by IThemeService::Resolve) --
    RadiusToken    radius      = RadiusToken::Default;   ///< Corner radius (3px default)
    SpacingToken   paddingH    = SpacingToken::Px4;      ///< Horizontal content padding
    SpacingToken   paddingV    = SpacingToken::Px4;      ///< Vertical content padding
    SpacingToken   gap         = SpacingToken::Px4;      ///< Icon-text gap, inter-item spacing
    SizeToken      minHeight   = SizeToken::Md;          ///< Minimum component height (32px default)
    SpacingToken   borderWidth = SpacingToken::Px1;      ///< Default border stroke width

    // -- Typography --
    FontRole       font        = FontRole::Body;         ///< Text font role

    // -- Visual --
    ElevationToken elevation   = ElevationToken::Flat;   ///< Box shadow level
    LayerToken     layer       = LayerToken::Base;       ///< Z-index stacking order

    // -- Transition --
    TransitionDef  transition;                           ///< Animation duration + easing

    /// @brief Variant color maps (non-owning view into NyanTheme storage).
    std::span<const VariantStyle> variants;
};

} // namespace matcha::gui
