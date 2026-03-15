/**
 * @file NyanLine.cpp
 * @brief Implementation of NyanLine 1px themed separator.
 */

#include <Matcha/Widgets/Controls/NyanLine.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

NyanLine::NyanLine(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Label) // Label kind (no dedicated separator kind)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(1);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanLine::~NyanLine() = default;

void NyanLine::SetOrientation(Qt::Orientation orientation)
{
    _orientation = orientation;
    if (_orientation == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(1);
        setMinimumWidth(0);
    } else {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        setFixedWidth(1);
        setMinimumHeight(0);
    }
    updateGeometry();
    update();
}

auto NyanLine::Orientation() const -> Qt::Orientation
{
    return _orientation;
}

void NyanLine::SetColor(ColorToken token)
{
    _colorToken = token;
    update();
}

auto NyanLine::Color() const -> ColorToken
{
    return _colorToken;
}

auto NyanLine::sizeHint() const -> QSize
{
    if (_orientation == Qt::Horizontal) {
        return {40, 1};
    }
    return {1, 40};
}

void NyanLine::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setPen(Theme().Color(_colorToken));

    if (_orientation == Qt::Horizontal) {
        p.drawLine(0, 0, width() - 1, 0);
    } else {
        p.drawLine(0, 0, 0, height() - 1);
    }
}

void NyanLine::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui