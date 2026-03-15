#pragma once

/**
 * @file GridConstants.h
 * @brief Constexpr 24-column grid system for consistent UI layout.
 *
 * All values are logical pixels (DPI-independent). The grid provides
 * column widths, gutters, margins, and vertical spacing constants
 * used across all Matcha layout code.
 */

#include <array>

namespace matcha::fw {

/**
 * @brief Constexpr grid constants for the 24-column layout system.
 */
struct GridConstants {
    // -- Grid dimensions --

    static constexpr int kColumnCount   = 24;
    static constexpr int kColumnWidth   = 72;   ///< px per column
    static constexpr int kGutterWidth   = 8;    ///< px between columns
    static constexpr int kMargin        = 4;    ///< px outer margin

    // -- Vertical spacing levels --

    static constexpr int kSpacingXs     = 4;    ///< Extra-small
    static constexpr int kSpacingSm     = 8;    ///< Small
    static constexpr int kSpacingMd     = 12;   ///< Medium
    static constexpr int kSpacingLg     = 16;   ///< Large

    // -- Derived constants --

    /// @brief Total grid width (all columns + gutters + margins).
    static constexpr int kTotalWidth =
        (2 * kMargin) + (kColumnCount * kColumnWidth) + ((kColumnCount - 1) * kGutterWidth);

    // -- Helper functions --

    /// @brief Width for N adjacent columns including internal gutters.
    /// @param columns Number of columns (1..24).
    /// @return Width in logical pixels, or 0 if columns <= 0.
    [[nodiscard]] static constexpr auto ColumnSpan(int columns) -> int {
        if (columns <= 0) {
            return 0;
        }
        if (columns > kColumnCount) {
            columns = kColumnCount;
        }
        return (columns * kColumnWidth) + ((columns - 1) * kGutterWidth);
    }

    /// @brief Width of one equal division of a container.
    /// @param totalWidth Container width in px.
    /// @param divisions Number of equal divisions (2/3/4/6/8/12).
    /// @return Width per division, or 0 if divisions <= 0.
    [[nodiscard]] static constexpr auto EqualDivision(int totalWidth, int divisions) -> int {
        if (divisions <= 0) {
            return 0;
        }
        return (totalWidth - ((divisions - 1) * kGutterWidth)) / divisions;
    }

    /// @brief Vertical spacing for a given level (0=Xs, 1=Sm, 2=Md, 3=Lg).
    /// @param level Spacing level 0..3. Clamped to valid range.
    [[nodiscard]] static constexpr auto VerticalSpacing(int level) -> int {
        constexpr std::array kSpacing = {kSpacingXs, kSpacingSm, kSpacingMd, kSpacingLg};
        if (level < 0) {
            return kSpacing[0];
        }
        if (level > 3) {
            return kSpacing[3];
        }
        return kSpacing[level];
    }
};

} // namespace matcha::fw
