#pragma once

/**
 * @file DesignTokens.h
 * @brief Design Token System: Color, Font, and Qt-dependent value structs.
 *
 * Qt-free token enums (Spacing, Radius, Elevation, Animation, Easing,
 * InteractionState, Density, Direction, Icon, Cursor, Layer) are defined
 * in matcha::fw::TokenEnums.h and imported here via using declarations.
 *
 * This file retains Qt-dependent types: ColorToken (resolved to QColor),
 * FontSpec (contains QString), ShadowSpec, StateStyle, VariantStyle.
 *
 * @see TokenEnums.h for the Qt-free enum definitions.
 * @see 05_Greenfield_Plan.md ss 2.2 for the full token taxonomy.
 */

#include <Matcha/UiNodes/Core/TokenEnums.h>

#include <QString>
#include <QtTypes>

#include <array>
#include <cstdint>
#include <utility>

namespace matcha::gui {

// Import fw-layer token enums into gui namespace for source compatibility.
using matcha::fw::SpacingToken;
using matcha::fw::RadiusToken;
using matcha::fw::ElevationToken;
using matcha::fw::AnimationToken;
using matcha::fw::EasingToken;
using matcha::fw::InteractionState;
using matcha::fw::DensityLevel;
using matcha::fw::FontSizePreset;
using matcha::fw::TextDirection;
using matcha::fw::IconId;
using matcha::fw::IconSize;
using matcha::fw::CursorToken;
using matcha::fw::LayerToken;
using matcha::fw::SizeToken;
using matcha::fw::TransitionDef;

// Import constants.
using matcha::fw::kElevationTokenCount;
using matcha::fw::kAnimationTokenCount;
using matcha::fw::kDefaultAnimationMs;
using matcha::fw::kEasingTokenCount;
using matcha::fw::kInteractionStateCount;
using matcha::fw::kDensityLevelCount;
using matcha::fw::ToPixels;

// ============================================================================
// 1. Color Tokens
// ============================================================================

/**
 * @brief Semantic color slots for the entire design system.
 *
 * 74 tokens following the Ant Design v5 three-layer model:
 * - Neutral: 5 surface + 4 fill + 3 border + 4 text = 16
 * - Semantic: 5 hues x 10 Ant Design steps = 50
 * - Special: 8 purpose tokens
 *
 * Lookup is O(1) via flat array: `colors[std::to_underlying(token)]`.
 *
 * @see docs/06_Design_Token_Redesign.md for the full redesign rationale.
 */
enum class ColorToken : uint16_t {
    // -- Neutral: Surface (5) --
    Surface,              ///< App base background (Ant colorBgBase)
    SurfaceContainer,     ///< Card, panel, sidebar (Ant colorBgContainer)
    SurfaceElevated,      ///< Popup, dropdown, dialog (Ant colorBgElevated)
    SurfaceSunken,        ///< Recessed well, status bar (Ant colorBgLayout)
    Spotlight,            ///< Tooltip, floating highlight (Ant colorBgSpotlight)

    // -- Neutral: Fill (4) --
    Fill,                 ///< Interactive component default bg (Ant colorFill)
    FillHover,            ///< Component hovered bg (Ant colorFillSecondary)
    FillActive,           ///< Component pressed/active bg (Ant colorFillTertiary)
    FillMuted,            ///< Subtle/disabled fill (Ant colorFillQuaternary)

    // -- Neutral: Border (3) --
    BorderSubtle,         ///< Separator, container edge (Ant colorBorderSecondary)
    BorderDefault,        ///< Input/button resting border (Ant colorBorder)
    BorderStrong,         ///< Hovered border, focus ring

    // -- Neutral: Text (4) --
    TextPrimary,          ///< High-contrast body text (Ant colorText)
    TextSecondary,        ///< Description, secondary (Ant colorTextSecondary)
    TextTertiary,         ///< Placeholder, hint (Ant colorTextTertiary)
    TextDisabled,         ///< Disabled text, watermark (Ant colorTextQuaternary)

    // -- Primary hue: Ant Design 10-step --
    PrimaryBg,            ///< Step 1: lightest background
    PrimaryBgHover,       ///< Step 2: background hover
    PrimaryBorder,        ///< Step 3: colored border
    PrimaryBorderHover,   ///< Step 4: border hover
    PrimaryHover,         ///< Step 5: component hover
    Primary,              ///< Step 6: base / seed color
    PrimaryActive,        ///< Step 7: component pressed/active
    PrimaryTextHover,     ///< Step 8: text hover on neutral bg
    PrimaryText,          ///< Step 9: text on neutral bg
    PrimaryTextActive,    ///< Step 10: text pressed

    // -- Success hue: Ant Design 10-step --
    SuccessBg, SuccessBgHover, SuccessBorder, SuccessBorderHover, SuccessHover,
    Success, SuccessActive, SuccessTextHover, SuccessText, SuccessTextActive,

    // -- Warning hue: Ant Design 10-step --
    WarningBg, WarningBgHover, WarningBorder, WarningBorderHover, WarningHover,
    Warning, WarningActive, WarningTextHover, WarningText, WarningTextActive,

    // -- Error hue: Ant Design 10-step --
    ErrorBg, ErrorBgHover, ErrorBorder, ErrorBorderHover, ErrorHover,
    Error, ErrorActive, ErrorTextHover, ErrorText, ErrorTextActive,

    // -- Info hue: Ant Design 10-step (NEW) --
    InfoBg, InfoBgHover, InfoBorder, InfoBorderHover, InfoHover,
    Info, InfoActive, InfoTextHover, InfoText, InfoTextActive,

    // -- Special purpose (9) --
    OnAccent,             ///< Text/icon on solid accent bg (white in light theme)
    OnAccentSecondary,    ///< Secondary text on solid accent bg
    Focus,                ///< Focus ring color
    Selection,            ///< Selection highlight (text, table row)
    Link,                 ///< Hyperlink color
    Scrim,                ///< Modal backdrop dim (dialog/drawer overlay)
    Overlay,              ///< Non-modal overlay bg (popover, dropdown, tooltip backdrop)
    Shadow,               ///< Box-shadow base color
    Separator,            ///< Horizontal/vertical rule

    Count_ ///< Sentinel for array sizing. Must be last.
};

/// @brief Compile-time count of ColorToken values (excluding Count_).
inline constexpr auto kColorTokenCount = static_cast<std::size_t>(ColorToken::Count_);

// InteractionState is now in matcha::fw::TokenEnums.h (imported above).

// ============================================================================
// 3. Typography Tokens
// ============================================================================

/**
 * @brief Semantic font roles mapping to platform-specific FontSpec values.
 *
 * `IThemeService::Font(FontRole)` returns `const FontSpec&`.
 */
enum class FontRole : uint8_t {
    Body,       ///< 9pt Normal       -- default widget text
    BodyMedium, ///< 9pt Medium       -- NyanLabel NameState
    BodyBold,   ///< 9pt Medium+Bold  -- NyanLabel TitleState
    Caption,    ///< 8pt Normal       -- ToolButton text, ActionBar tabs
    Heading,    ///< 12pt DemiBold    -- dialog titles, section headers
    Monospace,  ///< 9pt Normal mono  -- code display, LineEdit numeric
    ToolTip,    ///< 8pt Normal       -- tooltip overlays

    Count_ ///< Sentinel for array sizing. Must be last.
};

/// @brief Compile-time count of FontRole values.
inline constexpr auto kFontRoleCount = static_cast<std::size_t>(FontRole::Count_);

/**
 * @brief Concrete font specification resolved per platform.
 *
 * Platform font family is selected at startup via `QFontDatabase::systemFont`.
 * Sizes are DPI-independent point sizes.
 */
struct FontSpec {
    QString family;                    ///< e.g. "Segoe UI" (Win), "SF Pro" (macOS)
    int     sizeInPt      = 9;        ///< DPI-independent point size
    int     weight        = 400;      ///< QFont::Weight enum value (Normal=400)
    bool    italic        = false;    ///< Italic style flag
    qreal   lineHeightMultiplier = 1.4;  ///< Line height as multiplier of font size
    qreal   letterSpacing = 0.0;      ///< Letter spacing in pixels (0 = default)
};

// Spacing, Radius, Elevation tokens are now in matcha::fw::TokenEnums.h (imported above).

/**
 * @brief Concrete shadow parameters derived from ElevationToken.
 *
 * The algorithm in NyanTheme derives offsetY, blurRadius, and opacity
 * from the elevation enum level.
 */
struct ShadowSpec {
    int   offsetX    = 0;   ///< Horizontal shadow offset (typically 0)
    int   offsetY    = 0;   ///< Vertical shadow offset
    int   blurRadius = 0;   ///< Gaussian blur radius
    qreal opacity    = 0.0; ///< Shadow color opacity over Background0
};

// Animation tokens are now in matcha::fw::TokenEnums.h (imported above).

// ============================================================================
// 8. State Colors & Variant Style (Color Mapping)
// ============================================================================

/**
 * @brief Per-state visual style: colors + opacity + border width + cursor.
 *
 * Used inside VariantStyle to define widget appearance for each InteractionState.
 * Extends the former StateColors with non-color visual attributes needed for
 * declarative widget styling (RFC-07).
 *
 * @see docs/07_Declarative_Style_RFC.md Section 5.1
 */
struct StateStyle {
    ColorToken   background  = ColorToken::Surface;
    ColorToken   foreground  = ColorToken::TextPrimary;
    ColorToken   border      = ColorToken::BorderSubtle;
    float        opacity     = 1.0F;              ///< 0.0..1.0, applied to entire widget
    SpacingToken borderWidth = SpacingToken::Px1;  ///< Border stroke width
    CursorToken  cursor      = CursorToken::Default;
};

/**
 * @brief Per-variant color map: 7 InteractionState entries.
 *
 * A simple widget has 1 VariantStyle. Multi-variant widgets (e.g., PushButton
 * with Primary/Secondary/Ghost/Danger) have N VariantStyle entries indexed by
 * their variant enum.
 */
struct VariantStyle {
    std::array<StateStyle, kInteractionStateCount> colors = {};
};

// Easing tokens are now in matcha::fw::TokenEnums.h (imported above).

} // namespace matcha::gui
