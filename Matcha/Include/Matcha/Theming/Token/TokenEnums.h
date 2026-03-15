#pragma once

/**
 * @file TokenEnums.h
 * @brief Qt-free design token enums for the matcha::fw layer.
 *
 * These enums are shared between fw (UiNode layer) and gui (Widget layer).
 * They have ZERO Qt dependency — only <cstdint> and <utility>.
 *
 * Enums relocated from gui::DesignTokens.h:
 *   SpacingToken, RadiusToken, AnimationToken, EasingToken, InteractionState
 *
 * New enums added for the Design System expansion:
 *   DensityLevel, TextDirection, IconId, IconSize, CursorToken, LayerToken
 *
 * @see plan.md Part VIII Section 8.6 for the relocation rationale.
 */

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace matcha::fw {

// ============================================================================
// Spacing Tokens
// ============================================================================

/**
 * @brief Spacing values in logical pixels (base values before density scaling).
 *
 * The underlying value IS the base pixel value:
 * `static_cast<int>(SpacingToken::Px8) == 8`.
 *
 * Actual pixel output = basePx * densityScale (applied by ITokenRegistry).
 */
enum class SpacingToken : uint8_t {
    None = 0,   ///< 0px
    Px1  = 1,   ///< 1px  -- border width, single-pixel adjustments
    Px2  = 2,   ///< 2px  -- menu panel padding, icon-text gap
    Px3  = 3,   ///< 3px  -- corner radius, separator height, checkbox inset
    Px4  = 4,   ///< 4px  -- standard padding, icon offset
    Px5  = 5,   ///< 5px  -- NyanLine thickness
    Px6  = 6,   ///< 6px  -- ComboBox container margin, ring stroke
    Px7  = 7,   ///< 7px  -- ProgressBar thin groove height
    Px8  = 8,   ///< 8px  -- window shadow margin, row padding
    Px16 = 16,  ///< 16px -- icon standard size, section gaps
    Px12 = 12,  ///< 12px -- compact section gap, dialog inner padding
    Px20 = 20,  ///< 20px -- submenu arrow margin
    Px24 = 24,  ///< 24px -- drawer side strip, title row height
    Px32 = 32,  ///< 32px -- button width, tab close reserve
    Px48 = 48,  ///< 48px -- large section gap, dialog outer padding
    Px64 = 64,  ///< 64px -- page-level section spacing

    Count_ = 16 ///< Number of defined spacing values. NOT a pixel value.
};

/**
 * @brief Convert SpacingToken to its base pixel value (before density scaling).
 * @param s Spacing token.
 * @return Base pixel value as int.
 */
[[nodiscard]] constexpr auto ToPixels(SpacingToken s) noexcept -> int
{
    return static_cast<int>(std::to_underlying(s));
}

// ============================================================================
// Radius Tokens
// ============================================================================

/**
 * @brief Corner radius values in logical pixels.
 *
 * The underlying value IS the pixel value (except Count_).
 */
enum class RadiusToken : uint8_t {
    None    = 0,   ///< 0px  -- sharp corners (DataTable cells)
    Small   = 2,   ///< 2px  -- indicator, expand bar
    Default = 3,   ///< 3px  -- THE standard radius (all current widgets)
    Medium  = 4,   ///< 4px  -- reserved (future use)
    Large   = 8,   ///< 8px  -- prominent (Dialog, Card, GroupBox)
    Round   = 255, ///< pill shape -- ProgressBar groove, Toggle, Badge

    Count_ = 6 ///< Number of defined radius values. NOT a pixel value.
};

/**
 * @brief Convert RadiusToken to integer pixel value.
 * @param r RadiusToken.
 * @return Pixel value as int. For Round (255), caller should use min(width, height)/2.
 */
[[nodiscard]] constexpr auto ToPixels(RadiusToken r) noexcept -> int
{
    return static_cast<int>(std::to_underlying(r));
}

// ============================================================================
// Elevation Tokens
// ============================================================================

/**
 * @brief Elevation levels determining box shadow intensity.
 */
enum class ElevationToken : uint8_t {
    Flat   = 0, ///< No shadow -- most widgets
    Low    = 1, ///< 2px blur, 6% opacity -- panel hover lift
    Medium = 2, ///< 4px blur, 10% opacity -- Card, GroupBox
    High   = 3, ///< 8px blur, 15% opacity -- Dropdown, PopConfirm
    Window = 4, ///< 8px margin + custom multi-pass blur -- main window shadow

    Count_ = 5 ///< Number of defined elevation values.
};

inline constexpr auto kElevationTokenCount = static_cast<std::size_t>(ElevationToken::Count_);

// ============================================================================
// Animation Tokens
// ============================================================================

/**
 * @brief Animation duration presets.
 */
enum class AnimationToken : uint8_t {
    Instant = 0, ///< 0ms   -- forced by test fixture
    Quick   = 1, ///< 160ms -- menu popup slide
    Normal  = 2, ///< 200ms -- drawer slide, ActionBar indicator
    Slow    = 3, ///< 350ms -- page transitions

    Count_ = 4 ///< Number of defined animation values.
};

inline constexpr auto kAnimationTokenCount = static_cast<std::size_t>(AnimationToken::Count_);

/**
 * @brief Default animation durations in milliseconds, indexed by AnimationToken.
 */
inline constexpr std::array<int, kAnimationTokenCount> kDefaultAnimationMs = {
    0,   // Instant
    160, // Quick
    200, // Normal
    350, // Slow
};

// ============================================================================
// Easing Tokens
// ============================================================================

/**
 * @brief Animation easing curve presets.
 *
 * Resolved to QEasingCurve::Type by IThemeService (gui layer).
 */
enum class EasingToken : uint8_t {
    Linear     = 0, ///< Linear interpolation
    OutCubic   = 1, ///< Decelerate (default for most transitions)
    InOutCubic = 2, ///< Accelerate then decelerate (page transitions)
    Spring     = 3, ///< Spring dynamics (see SpringSpec)

    Count_ = 4 ///< Number of defined easing values.
};

inline constexpr auto kEasingTokenCount = static_cast<std::size_t>(EasingToken::Count_);

/**
 * @brief Spring dynamics parameters for EasingToken::Spring.
 *
 * Models a critically/under-damped harmonic oscillator.
 * Used by the Animation Engine when EasingToken::Spring is selected.
 */
struct SpringSpec {
    float mass      = 1.0F;
    float stiffness = 200.0F;
    float damping   = 20.0F;
};

// ============================================================================
// Interaction State
// ============================================================================

/**
 * @brief Widget interaction states for state-dependent color lookup.
 */
enum class InteractionState : uint8_t {
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Focused,
    Selected,
    Error,
    DragOver, ///< Drop target highlight

    Count_ ///< Sentinel for array sizing. Must be last.
};

inline constexpr auto kInteractionStateCount = static_cast<std::size_t>(InteractionState::Count_);

// ============================================================================
// Density Level (NEW)
// ============================================================================

/**
 * @brief User-selectable density levels for spacing/font/icon scaling.
 *
 * DensityLevel::Default has scale factor 1.0.
 * Compact = 0.875, Comfortable = 1.125.
 */
enum class DensityLevel : uint8_t {
    Compact     = 0, ///< 0.875x -- reduced spacing for power users
    Default     = 1, ///< 1.0x   -- standard
    Comfortable = 2, ///< 1.125x -- increased spacing for touch/accessibility

    Count_ = 3
};

inline constexpr auto kDensityLevelCount = static_cast<std::size_t>(DensityLevel::Count_);

/**
 * @brief Density scale factors indexed by DensityLevel.
 */
inline constexpr std::array<float, kDensityLevelCount> kDensityScaleFactors = {
    0.875F, // Compact
    1.000F, // Default
    1.125F, // Comfortable
};

/**
 * @brief Get the scale factor for a density level.
 */
[[nodiscard]] constexpr auto DensityScale(DensityLevel level) noexcept -> float
{
    const auto idx = std::to_underlying(level);
    if (idx >= kDensityLevelCount) { return 1.0F; }
    return kDensityScaleFactors[idx];
}

// ============================================================================
// Font Size Preset
// ============================================================================

/**
 * @brief Convenience presets for global font scaling.
 *
 * Each preset maps to a predefined scale factor applied to all FontRole
 * base point sizes. Users may also set an arbitrary fontScaleFactor directly
 * (clamped to [kFontScaleMin, kFontScaleMax]).
 *
 * The preset multiplier is applied multiplicatively with the density scale:
 *   actualPt = basePt * fontScaleFactor * densityScale
 */
enum class FontSizePreset : uint8_t {
    Small   = 0, ///< 0.875x -- compact text for dense UIs
    Medium  = 1, ///< 1.0x   -- default
    Large   = 2, ///< 1.25x  -- accessibility / large displays

    Count_ = 3
};

inline constexpr auto kFontSizePresetCount = static_cast<std::size_t>(FontSizePreset::Count_);

/**
 * @brief Scale factors for each FontSizePreset.
 */
inline constexpr std::array<float, kFontSizePresetCount> kFontSizePresetFactors = {
    0.875F, // Small
    1.000F, // Medium
    1.250F, // Large
};

/**
 * @brief Get the font scale factor for a preset.
 */
[[nodiscard]] constexpr auto FontSizePresetScale(FontSizePreset preset) noexcept -> float
{
    const auto idx = std::to_underlying(preset);
    if (idx >= kFontSizePresetCount) { return 1.0F; }
    return kFontSizePresetFactors[idx];
}

/// @brief Minimum allowed fontScaleFactor (prevents unreadably small text).
inline constexpr float kFontScaleMin = 0.5F;

/// @brief Maximum allowed fontScaleFactor (prevents layout overflow).
inline constexpr float kFontScaleMax = 3.0F;

// ============================================================================
// Orientation
// ============================================================================

/**
 * @brief Layout orientation (Qt-free replacement for Qt::Orientation).
 */
enum class Orientation : uint8_t {
    Horizontal = 0,
    Vertical   = 1,
};

// ============================================================================
// Text Direction (NEW)
// ============================================================================

/**
 * @brief Text and layout direction for i18n/RTL support.
 */
enum class TextDirection : uint8_t {
    LTR = 0, ///< Left-to-right (default)
    RTL = 1, ///< Right-to-left (Arabic, Hebrew, etc.)
};

// ============================================================================
// Icon Identification (asset:// URI)
// ============================================================================

/**
 * @brief Icon identifier — an asset URI string.
 *
 * Format: `"asset://<authority>/<path>"`.
 * - Framework built-in icons: `"asset://matcha/icons/<name>"`
 *   (e.g. `"asset://matcha/icons/save"`, `"asset://matcha/icons/close"`).
 * - Application icons: `"asset://nyancad/icons/<name>"`.
 * - Plugin icons: `"asset://<plugin-id>/icons/<name>"`.
 *
 * An empty string represents "no icon" (equivalent to old IconToken::None).
 *
 * Icons are registered via IThemeService::RegisterIconDirectory() and resolved
 * via IThemeService::ResolveIcon(). Applications and plugins can register
 * arbitrary icons at runtime without modifying framework code.
 */
using IconId = std::string;

/**
 * @brief Framework built-in icon URI prefix.
 *
 * All framework-shipped SVG icons are registered under this prefix.
 * The icon name is the SVG filename without extension (kebab-case).
 */
inline constexpr std::string_view kMatchaIconPrefix = "asset://matcha/icons/";

/**
 * @brief Well-known framework icon URIs.
 *
 * These constants provide compile-time safety for commonly used framework
 * icons. Applications may use string literals directly for custom icons.
 */
namespace icons {

// Window chrome
inline constexpr std::string_view Close          = "asset://matcha/icons/close";
inline constexpr std::string_view Minimize       = "asset://matcha/icons/minimize";
inline constexpr std::string_view Maximize       = "asset://matcha/icons/maximize";
inline constexpr std::string_view Restore        = "asset://matcha/icons/restore";
inline constexpr std::string_view Pin            = "asset://matcha/icons/pin";
inline constexpr std::string_view Unpin          = "asset://matcha/icons/unpin";

// Navigation
inline constexpr std::string_view ChevronLeft    = "asset://matcha/icons/chevron-left";
inline constexpr std::string_view ChevronRight   = "asset://matcha/icons/chevron-right";
inline constexpr std::string_view ChevronUp      = "asset://matcha/icons/chevron-up";
inline constexpr std::string_view ChevronDown    = "asset://matcha/icons/chevron-down";
inline constexpr std::string_view ArrowLeft      = "asset://matcha/icons/arrow-left";
inline constexpr std::string_view ArrowRight     = "asset://matcha/icons/arrow-right";
inline constexpr std::string_view ArrowUp        = "asset://matcha/icons/arrow-up";
inline constexpr std::string_view ArrowDown      = "asset://matcha/icons/arrow-down";
inline constexpr std::string_view Home           = "asset://matcha/icons/home";
inline constexpr std::string_view Back           = "asset://matcha/icons/back";
inline constexpr std::string_view Forward        = "asset://matcha/icons/forward";

// Data operations
inline constexpr std::string_view Search         = "asset://matcha/icons/search";
inline constexpr std::string_view Filter         = "asset://matcha/icons/filter";
inline constexpr std::string_view Sort           = "asset://matcha/icons/sort";
inline constexpr std::string_view SortAsc        = "asset://matcha/icons/sort-asc";
inline constexpr std::string_view SortDesc       = "asset://matcha/icons/sort-desc";

// CRUD
inline constexpr std::string_view Add            = "asset://matcha/icons/add";
inline constexpr std::string_view Remove         = "asset://matcha/icons/remove";
inline constexpr std::string_view Edit           = "asset://matcha/icons/edit";
inline constexpr std::string_view Delete         = "asset://matcha/icons/delete";
inline constexpr std::string_view Copy           = "asset://matcha/icons/copy";
inline constexpr std::string_view Paste          = "asset://matcha/icons/paste";
inline constexpr std::string_view Cut            = "asset://matcha/icons/cut";
inline constexpr std::string_view Undo           = "asset://matcha/icons/undo";
inline constexpr std::string_view Redo           = "asset://matcha/icons/redo";

// Document
inline constexpr std::string_view Save           = "asset://matcha/icons/save";
inline constexpr std::string_view SaveAs         = "asset://matcha/icons/save-as";
inline constexpr std::string_view Open           = "asset://matcha/icons/open";
inline constexpr std::string_view NewFile        = "asset://matcha/icons/new-file";
inline constexpr std::string_view NewFolder      = "asset://matcha/icons/new-folder";

// Viewport
inline constexpr std::string_view ZoomIn         = "asset://matcha/icons/zoom-in";
inline constexpr std::string_view ZoomOut        = "asset://matcha/icons/zoom-out";
inline constexpr std::string_view ZoomFit        = "asset://matcha/icons/zoom-fit";
inline constexpr std::string_view FullScreen     = "asset://matcha/icons/fullscreen";
inline constexpr std::string_view RotateLeft     = "asset://matcha/icons/rotate-left";
inline constexpr std::string_view RotateRight    = "asset://matcha/icons/rotate-right";

// Status
inline constexpr std::string_view Info           = "asset://matcha/icons/info";
inline constexpr std::string_view Warning        = "asset://matcha/icons/warning";
inline constexpr std::string_view Error          = "asset://matcha/icons/error";
inline constexpr std::string_view Success        = "asset://matcha/icons/success";
inline constexpr std::string_view Help           = "asset://matcha/icons/help";
inline constexpr std::string_view Question       = "asset://matcha/icons/question";

// Misc
inline constexpr std::string_view Settings       = "asset://matcha/icons/settings";
inline constexpr std::string_view Menu           = "asset://matcha/icons/menu";
inline constexpr std::string_view MoreHorizontal = "asset://matcha/icons/more-horizontal";
inline constexpr std::string_view MoreVertical   = "asset://matcha/icons/more-vertical";
inline constexpr std::string_view Check          = "asset://matcha/icons/check";
inline constexpr std::string_view Cross          = "asset://matcha/icons/cross";
inline constexpr std::string_view Refresh        = "asset://matcha/icons/refresh";
inline constexpr std::string_view Download       = "asset://matcha/icons/download";
inline constexpr std::string_view Upload         = "asset://matcha/icons/upload";
inline constexpr std::string_view Eye            = "asset://matcha/icons/eye";
inline constexpr std::string_view EyeOff         = "asset://matcha/icons/eye-off";
inline constexpr std::string_view Lock           = "asset://matcha/icons/lock";
inline constexpr std::string_view Unlock         = "asset://matcha/icons/unlock";

} // namespace icons

/**
 * @brief Check if an icon should be horizontally mirrored in RTL mode.
 *
 * Directional icons (arrows, chevrons, back/forward) are mirrored so that
 * "left" becomes "right" and vice versa when the UI direction is RTL.
 *
 * @param iconId The icon URI to check.
 * @return true if the icon should be flipped in RTL layouts.
 */
[[nodiscard]] inline auto IsRtlFlippable(std::string_view iconId) noexcept -> bool
{
    return iconId == icons::ChevronLeft  || iconId == icons::ChevronRight
        || iconId == icons::ArrowLeft    || iconId == icons::ArrowRight
        || iconId == icons::Back         || iconId == icons::Forward;
}

/**
 * @brief Standard icon sizes.
 */
enum class IconSize : uint8_t {
    Xs  = 12, ///< 12px -- inline indicators
    Sm  = 16, ///< 16px -- toolbar, menu items (default)
    Md  = 20, ///< 20px -- buttons with icon
    Lg  = 24, ///< 24px -- headers, navigation
    Xl  = 32, ///< 32px -- empty states, feature icons

    Count_ = 5
};

// ============================================================================
// Cursor Tokens (NEW)
// ============================================================================

/**
 * @brief Standardized cursor vocabulary.
 *
 * Resolved to QCursor by IThemeService in the gui layer.
 */
enum class CursorToken : uint8_t {
    Default = 0,
    Pointer,        ///< Hand pointer (clickable)
    Text,           ///< I-beam (text input)
    Wait,           ///< Busy spinner
    Crosshair,      ///< Precision select
    Move,           ///< 4-way move
    ResizeH,        ///< Horizontal resize
    ResizeV,        ///< Vertical resize
    ResizeNESW,     ///< Diagonal NE-SW resize
    ResizeNWSE,     ///< Diagonal NW-SE resize
    Forbidden,      ///< Not allowed
    Grab,           ///< Open hand (draggable)
    Grabbing,       ///< Closed hand (dragging)
    ZoomIn,         ///< Magnify +
    ZoomOut,        ///< Magnify -

    Count_
};

// ============================================================================
// Size Tokens (RFC-07)
// ============================================================================

/**
 * @brief Standard component height presets (base values before density scaling).
 *
 * Replaces all hardcoded ButtonSize, kFixedHeight, kIndicatorSize etc.
 * Actual pixel output = kBaseSizePx[token] * densityScale.
 *
 * @see docs/07_Declarative_Style_RFC.md Section 5.2
 */
enum class SizeToken : uint8_t {
    Xs = 0,  ///< 20px base -- badge, indicator, small icon button
    Sm = 1,  ///< 24px base -- compact button, tag, checkbox indicator
    Md = 2,  ///< 32px base -- standard button, input (DEFAULT)
    Lg = 3,  ///< 40px base -- prominent button, header input
    Xl = 4,  ///< 48px base -- hero button, touch target

    Count_ = 5
};

inline constexpr auto kSizeTokenCount = static_cast<std::size_t>(SizeToken::Count_);

/**
 * @brief Base pixel values for each SizeToken (before density scaling).
 */
inline constexpr std::array<int, kSizeTokenCount> kBaseSizePx = {
    20, // Xs
    24, // Sm
    32, // Md
    40, // Lg
    48, // Xl
};

/**
 * @brief Convert SizeToken to its base pixel value (before density scaling).
 */
[[nodiscard]] constexpr auto ToPixels(SizeToken s) noexcept -> int
{
    const auto idx = std::to_underlying(s);
    if (idx >= kSizeTokenCount) { return 32; }
    return kBaseSizePx[idx];
}

// ============================================================================
// Layer Tokens
// ============================================================================

/**
 * @brief Z-index stacking order for overlays.
 *
 * The underlying value is the z-index priority.
 */
enum class LayerToken : uint16_t {
    Base          = 0,     ///< Normal content
    Elevated      = 100,   ///< Cards, panels
    Sticky        = 200,   ///< Sticky headers
    Dropdown      = 300,   ///< Dropdown menus
    Modal         = 400,   ///< Modal dialogs
    Popover       = 500,   ///< Popovers, tooltips
    Notification  = 600,   ///< Toast / snackbar (reserved)
    Overlay       = 700,   ///< Full-screen overlays
    Maximum       = 9999,  ///< Debug overlays
};

// ============================================================================
// Interaction Timing Tokens (§8.7)
// ============================================================================

/**
 * @brief Named interaction timing tokens for non-animation intervals.
 *
 * These govern when interactions trigger, debounce, or timeout — distinct
 * from AnimationToken which controls visual motion duration.
 *
 * Default values are provided in kDefaultTimingMs[]. On Windows, some
 * tokens are overridden by OS queries (SystemParametersInfo) at startup.
 *
 * @see Matcha_Design_System_Specification.md §8.7
 */
enum class TimingTokenId : uint8_t {
    HoverDelay           = 0,   ///< 200ms — delay before hover state activates
    TooltipDelay         = 1,   ///< 500ms — delay before tooltip appears
    TooltipDismissDelay  = 2,   ///< 100ms — grace period leaving tooltip trigger
    LongPressThreshold   = 3,   ///< 500ms — long-press recognition (touch/pen)
    DoubleClickWindow    = 4,   ///< 400ms — max interval for double-click
    DebounceSearch       = 5,   ///< 300ms — search-as-you-type debounce
    DebounceResize       = 6,   ///< 100ms — window resize layout debounce
    AutoSaveInterval     = 7,   ///< 30000ms — auto-save interval
    IdleTimeout          = 8,   ///< 60000ms — UI idle mode threshold
    RepeatKeyInitial     = 9,   ///< 500ms — delay before key repeat starts
    RepeatKeyInterval    = 10,  ///< 33ms  — interval between key repeats (~30/s)
    DragInitDelay        = 11,  ///< 150ms — hold before drag initiates
    ToastDismissTimeout  = 12,  ///< 5000ms — auto-dismiss for Toast
    MenuOpenDelay        = 13,  ///< 200ms — submenu open on hover
    MenuCloseDelay       = 14,  ///< 300ms — grace before submenu closes

    Count_ = 15
};

inline constexpr auto kTimingTokenCount = static_cast<std::size_t>(TimingTokenId::Count_);

/**
 * @brief Default timing values in milliseconds, indexed by TimingTokenId.
 *
 * Platform-specific overrides (Windows SystemParametersInfo, etc.) replace
 * selected entries at ITokenRegistry initialization time.
 */
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
// Transition Definition (RFC-07)
// ============================================================================

/**
 * @brief Declares the animation transition for entering a state.
 *
 * Stored in WidgetStyleSheet to define how state changes animate.
 * Widgets no longer need to hardcode QVariantAnimation parameters.
 *
 * @see docs/07_Declarative_Style_RFC.md Section 5.3
 */
struct TransitionDef {
    AnimationToken duration = AnimationToken::Normal;
    EasingToken    easing   = EasingToken::OutCubic;
};

// ============================================================================
// Animation Property Identification (RFC-08)
// ============================================================================

/**
 * @brief Identifies an animatable property on a WidgetNode.
 *
 * Qt-free enum mapped to QByteArray property names internally by
 * AnimationService. Predefined IDs cover all current widget animation
 * targets. Plugin authors use the UserDefined range (>= 1000).
 */
enum class AnimationPropertyId : uint16_t {
    // -- Generic widget properties --
    Opacity         = 0,
    Position        = 1,
    Size            = 2,
    MinimumHeight   = 3,
    MaximumHeight   = 4,
    Geometry        = 5,

    // -- Color properties (StateTransition) --
    BackgroundColor = 10,
    ForegroundColor = 11,
    BorderColor     = 12,

    // -- Widget-specific properties --
    ArrowRotation   = 100,   ///< CollapsibleSection, GroupBox chevron angle
    SlideOffset     = 101,   ///< Notification slide-in, Menu popup offset
    ContentHeight   = 102,   ///< CollapsibleSection content region

    // -- Extension range --
    UserDefined     = 1000,  ///< Plugin authors start here
};

// ============================================================================
// Animatable Value (RFC-08)
// ============================================================================

/**
 * @brief Qt-free tagged union for animation start/end values.
 *
 * Supports the four value types needed by current widget animations:
 * double (opacity, rotation), int (height, offset), RGBA color, 2D point.
 * Converted to QVariant internally by AnimationService.
 */
struct Point2D {
    int x = 0;
    int y = 0;
};

struct AnimatableValue {
    enum class Type : uint8_t {
        Double,
        Int,
        Rgba,
        Point2D,
    };

    Type type = Type::Double;
    double   asDouble = 0.0;      ///< Active when type == Double
    int      asInt    = 0;        ///< Active when type == Int
    uint32_t asRgba   = 0;        ///< Active when type == Rgba (0xAARRGGBB)
    Point2D  asPoint  = {};       ///< Active when type == Point2D

    static constexpr auto FromDouble(double v) -> AnimatableValue {
        AnimatableValue val;
        val.type = Type::Double;
        val.asDouble = v;
        return val;
    }

    static constexpr auto FromInt(int v) -> AnimatableValue {
        AnimatableValue val;
        val.type = Type::Int;
        val.asInt = v;
        return val;
    }

    static constexpr auto FromRgba(uint8_t r, uint8_t g, uint8_t b,
                                   uint8_t a = 255) -> AnimatableValue {
        AnimatableValue val;
        val.type = Type::Rgba;
        val.asRgba = (static_cast<uint32_t>(a) << 24U)
                   | (static_cast<uint32_t>(r) << 16U)
                   | (static_cast<uint32_t>(g) << 8U)
                   | static_cast<uint32_t>(b);
        return val;
    }

    static constexpr auto FromPoint(int x, int y) -> AnimatableValue {
        AnimatableValue val;
        val.type = Type::Point2D;
        val.asPoint = {x, y};
        return val;
    }
};

// ============================================================================
// Transition Handle (RFC-08)
// ============================================================================

/**
 * @brief Opaque handle for a running animation, usable from fw layer.
 *
 * Returned by WidgetNode::AnimateProperty(). Can be passed to
 * CancelAnimation(). Maps internally to AnimationService's tracking ID.
 */
enum class TransitionHandle : uint64_t { Invalid = 0 };

} // namespace matcha::fw
