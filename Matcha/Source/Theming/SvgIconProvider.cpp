/**
 * @file SvgIconProvider.cpp
 * @brief Internal SVG icon loading and theme colorization.
 */

#include "SvgIconProvider.h"

#include <QFile>
#include <QPainter>
#include <QSvgRenderer>

namespace matcha::gui::detail {

auto LoadAndColorizeSvg(const QString& svgPath,
                        int sizePx,
                        QColor color) -> QPixmap
{
    if (sizePx <= 0) {
        return {};
    }

    QFile file(svgPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QByteArray svgData = file.readAll();

    // Replace "currentColor" with the hex color for theme colorization
    QByteArray colorHex = color.name(QColor::HexRgb).toUtf8();
    svgData.replace("currentColor", colorHex);

    QSvgRenderer renderer(svgData);
    if (!renderer.isValid()) {
        return {};
    }

    QPixmap pixmap(sizePx, sizePx);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    return pixmap;
}

} // namespace matcha::gui::detail
