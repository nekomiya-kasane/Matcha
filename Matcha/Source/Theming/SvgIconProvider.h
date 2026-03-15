#pragma once

/**
 * @file SvgIconProvider.h
 * @brief Internal SVG icon loading and theme colorization.
 *
 * INTERNAL to NyanTheme. Not a public header.
 * Loads an SVG file from disk, replaces "currentColor" with
 * the specified theme color, and rasterizes to QPixmap at the target size.
 */

#include <QColor>
#include <QPixmap>
#include <QString>

namespace matcha::gui::detail {

/**
 * @brief Load and colorize an SVG icon file, rasterizing to the given pixel size.
 *
 * @param svgPath  Absolute path to the .svg file.
 * @param sizePx   Target pixel size (square).
 * @param color    Color to replace "currentColor" with in the SVG.
 * @return Rasterized QPixmap, or null QPixmap on failure.
 */
[[nodiscard]] auto LoadAndColorizeSvg(const QString& svgPath,
                                      int sizePx,
                                      QColor color) -> QPixmap;

} // namespace matcha::gui::detail
