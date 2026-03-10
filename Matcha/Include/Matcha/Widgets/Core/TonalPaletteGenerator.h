#pragma once

/**
 * @file TonalPaletteGenerator.h
 * @brief OKLCH-based tonal palette generation from seed colors.
 *
 * Generates 10-level tonal ramps from a single seed color using the OKLCH
 * perceptually uniform color space. Follows the Ant Design v5 10-step model.
 *
 * Algorithm:
 *   1. Convert seed QColor (sRGB) to OKLCH (L, C, H)
 *   2. Distribute lightness across 10 levels:
 *      - Light theme: L from 0.96 (step 1) to 0.30 (step 10)
 *      - Dark  theme: L from 0.20 (step 1) to 0.90 (step 10)
 *   3. Maintain constant chroma and hue from seed
 *   4. Clamp to sRGB gamut (reduce chroma if necessary)
 *   5. Convert back to QColor
 *
 * The 10 levels map to the Ant Design token naming:
 *   [0] Bg,       [1] BgHover,    [2] Border,     [3] BorderHover, [4] Hover,
 *   [5] Base,     [6] Active,     [7] TextHover,  [8] Text,        [9] TextActive
 *
 * @see plan.md S11 for the design rationale.
 * @see https://bottosson.github.io/posts/oklab/ for OKLCH specification.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/DesignTokens.h>

#include <QColor>

#include <array>

namespace matcha::gui {

/**
 * @brief OKLCH color representation (Lightness, Chroma, Hue).
 */
struct OklchColor {
    float L = 0.0f; ///< Lightness [0, 1]
    float C = 0.0f; ///< Chroma [0, ~0.37]
    float H = 0.0f; ///< Hue in degrees [0, 360)
};

/**
 * @brief Static utility for generating tonal palettes from seed colors.
 */
class MATCHA_EXPORT TonalPaletteGenerator {
public:
    static constexpr int kToneCount = 10;

    /**
     * @brief Generate a 10-level tonal ramp for a light theme.
     * @param seed Seed color (sRGB).
     * @return Array of 10 QColors: step 1 (lightest) to step 10 (darkest).
     */
    [[nodiscard]] static auto GenerateLight(QColor seed) -> std::array<QColor, kToneCount>;

    /**
     * @brief Generate a 10-level tonal ramp for a dark theme.
     * @param seed Seed color (sRGB).
     * @return Array of 10 QColors: step 1 (darkest) to step 10 (lightest).
     */
    [[nodiscard]] static auto GenerateDark(QColor seed) -> std::array<QColor, kToneCount>;

    // -- Color space conversions (public for testing) --

    /**
     * @brief Convert sRGB QColor to OKLCH.
     */
    [[nodiscard]] static auto SrgbToOklch(QColor color) -> OklchColor;

    /**
     * @brief Convert OKLCH to sRGB QColor, clamping to gamut.
     */
    [[nodiscard]] static auto OklchToSrgb(OklchColor oklch) -> QColor;
};

} // namespace matcha::gui
