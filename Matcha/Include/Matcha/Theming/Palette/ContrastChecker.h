#pragma once

/**
 * @file ContrastChecker.h
 * @brief WCAG 2.1 contrast ratio utilities (S5).
 *
 * Static utility class for computing luminance contrast ratios between
 * foreground and background colors per WCAG 2.1 guidelines.
 *
 * All methods are pure functions with no side effects.
 */

#include "Matcha/Core/Macros.h"

#include <QColor>

namespace matcha::gui {

/**
 * @brief WCAG 2.1 contrast ratio checker.
 *
 * Reference: https://www.w3.org/TR/WCAG21/#contrast-minimum
 *
 * Relative luminance formula:
 *   L = 0.2126 * R_lin + 0.7152 * G_lin + 0.0722 * B_lin
 * where R_lin = (R/255 <= 0.04045) ? R/255/12.92 : ((R/255+0.055)/1.055)^2.4
 *
 * Contrast ratio = (L_lighter + 0.05) / (L_darker + 0.05)
 */
class MATCHA_EXPORT ContrastChecker {
public:
    /**
     * @brief Compute relative luminance of a color (WCAG 2.1 formula).
     * @return Value in [0.0, 1.0]. 0 = black, 1 = white.
     */
    [[nodiscard]] static auto RelativeLuminance(QColor color) -> double;

    /**
     * @brief Compute the WCAG contrast ratio between two colors.
     * @return Value in [1.0, 21.0]. Higher = more contrast.
     */
    [[nodiscard]] static auto Ratio(QColor fg, QColor bg) -> double;

    /**
     * @brief Check if the pair meets WCAG AA for normal text (>= 4.5:1).
     */
    [[nodiscard]] static auto MeetsAA(QColor fg, QColor bg) -> bool;

    /**
     * @brief Check if the pair meets WCAG AAA for normal text (>= 7:1).
     */
    [[nodiscard]] static auto MeetsAAA(QColor fg, QColor bg) -> bool;

    /**
     * @brief Check if the pair meets WCAG AA for large text (>= 3:1).
     */
    [[nodiscard]] static auto MeetsAALargeText(QColor fg, QColor bg) -> bool;

    /**
     * @brief Suggest a modified foreground color that meets the target ratio.
     *
     * Adjusts lightness (HSL) of `fg` to achieve `targetRatio` against `bg`.
     * Returns the original `fg` if it already meets the target.
     *
     * @param fg Foreground color to adjust.
     * @param bg Background color (unchanged).
     * @param targetRatio Desired minimum contrast ratio (e.g. 4.5 for AA).
     * @return Adjusted foreground color.
     */
    [[nodiscard]] static auto SuggestFix(QColor fg, QColor bg, double targetRatio) -> QColor;
};

} // namespace matcha::gui
