#pragma once

/**
 * @file DefaultPalette.h
 * @brief Compile-time default color palettes for Light and Dark themes.
 *
 * These constexpr arrays provide the built-in neutral + special color values
 * and seed colors for Light and Dark themes. They eliminate the hard dependency
 * on JSON palette files: NyanTheme can initialize from these defaults, then
 * optionally overlay values from JSON.
 *
 * Semantic hue tokens (5 hues x 10 steps = 50 colors) are NOT stored here.
 * They are always generated at runtime by TonalPaletteGenerator from the
 * seed colors in kLightSeeds / kDarkSeeds.
 *
 * Color format: 0xAARRGGBB (Qt QRgb format).
 *
 * @see docs/07_Declarative_Style_RFC.md Section 5.7
 * @see NyanTheme::LoadPalette() for the overlay logic.
 */

#include <Matcha/Theming/DesignTokens.h>

#include <array>
#include <cstdint>

namespace matcha::gui::defaults {

// ============================================================================
// Neutral + Special Color Defaults (24 tokens)
// ============================================================================

/// @brief Index constants matching ColorToken enum order for neutral + special.
/// Semantic hue tokens (index 16..65) are generated from seeds, not stored here.

/// @brief Default Light theme neutral + special colors (24 entries).
/// Indexed by: Surface=0, SurfaceContainer=1, ..., Separator=23.
/// These map to ColorToken values 0..15 (neutral) and 66..73 (special).
inline constexpr std::array<uint32_t, 24> kLightNeutralColors = {
    // -- Neutral: Surface (5) --
    0xFFFFFFFF,  // Surface
    0xFFF7F8FA,  // SurfaceContainer
    0xFFFFFFFF,  // SurfaceElevated
    0xFFF3F3F5,  // SurfaceSunken
    0xFFFFFFFF,  // Spotlight

    // -- Neutral: Fill (4) --
    0xFFF0F0F0,  // Fill
    0xFFE8E8E8,  // FillHover
    0xFFE0E0E0,  // FillActive
    0xFFF5F5F5,  // FillMuted

    // -- Neutral: Border (3) --
    0xFFE7E7E9,  // BorderSubtle
    0xFFDCDCDE,  // BorderDefault
    0xFFC5C5C8,  // BorderStrong

    // -- Neutral: Text (4) --
    0xE01F1F20,  // TextPrimary
    0xC03C3C3E,  // TextSecondary
    0x99575759,  // TextTertiary
    0x668B8B8E,  // TextDisabled

    // -- Special (8) --
    0xFFFFFFFF,  // OnAccent
    0xA6FFFFFF,  // OnAccentSecondary
    0xFFA4C4FE,  // Focus
    0x260066FF,  // Selection
    0xFF0066FF,  // Link
    0x73000000,  // Scrim
    0x14000000,  // Shadow
    0x0F000000,  // Separator
};

/// @brief Default Dark theme neutral + special colors (24 entries).
inline constexpr std::array<uint32_t, 24> kDarkNeutralColors = {
    // -- Neutral: Surface (5) --
    0xFF1F1F20,  // Surface
    0xFF2A2A2C,  // SurfaceContainer
    0xFF313133,  // SurfaceElevated
    0xFF171718,  // SurfaceSunken
    0xFF3C3C3E,  // Spotlight

    // -- Neutral: Fill (4) --
    0xFF48484A,  // Fill
    0xFF3C3C3E,  // FillHover
    0xFF575759,  // FillActive
    0xFF2A2A2C,  // FillMuted

    // -- Neutral: Border (3) --
    0xFF3C3C3E,  // BorderSubtle
    0xFF48484A,  // BorderDefault
    0xFF575759,  // BorderStrong

    // -- Neutral: Text (4) --
    0xE0F7F8FA,  // TextPrimary
    0xC0EEEEEF,  // TextSecondary
    0x99DCDCDE,  // TextTertiary
    0x66A6A6A9,  // TextDisabled

    // -- Special (8) --
    0xFFFFFFFF,  // OnAccent
    0xA6FFFFFF,  // OnAccentSecondary
    0xFF3366B2,  // Focus
    0x263385FF,  // Selection
    0xFF3385FF,  // Link
    0xB3000000,  // Scrim
    0x33000000,  // Shadow
    0x0FFFFFFF,  // Separator
};

// ============================================================================
// Seed Color Defaults (5 hues per theme)
// ============================================================================

/// @brief Seed colors for OKLCH tonal palette generation (Light theme).
/// Order: Primary, Success, Warning, Error, Info.
inline constexpr std::array<uint32_t, 5> kLightSeeds = {
    0xFF0066FF,  // Primary
    0xFF32CE99,  // Success
    0xFFED7B2F,  // Warning
    0xFFE34D59,  // Error
    0xFF1677FF,  // Info
};

/// @brief Seed colors for OKLCH tonal palette generation (Dark theme).
inline constexpr std::array<uint32_t, 5> kDarkSeeds = {
    0xFF3385FF,  // Primary
    0xFF4DD9A8,  // Success
    0xFFF09249,  // Warning
    0xFFE86B6B,  // Error
    0xFF1677FF,  // Info
};

// ============================================================================
// Font Family Defaults
// ============================================================================

/// @brief Default font family names per platform.
inline constexpr const char* kFontFamilyWin   = "Segoe UI";
inline constexpr const char* kFontFamilyMac   = "SF Pro Text";
inline constexpr const char* kFontFamilyLinux = "Noto Sans";
inline constexpr const char* kFontFamilyMono  = "Cascadia Code";

} // namespace matcha::gui::defaults
