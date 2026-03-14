/**
 * @file NyanColorSwatch.cpp
 * @brief Implementation of NyanColorSwatch themed clickable color chip.
 */

#include <Matcha/Widgets/Controls/NyanColorSwatch.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QToolTip>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanColorSwatch::NyanColorSwatch(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::ColorSwatch)
{
    setFixedSize(kSwatchSize, kSwatchSize);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanColorSwatch::~NyanColorSwatch() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanColorSwatch::SetColor(const QColor& color)
{
    _color = color;
    setToolTip(_color.isValid() ? _color.name(QColor::HexRgb).toUpper() : QString());
    update();
}

auto NyanColorSwatch::Color() const -> QColor { return _color; }

auto NyanColorSwatch::ColorHex() const -> QString
{
    return _color.isValid() ? _color.name(QColor::HexRgb).toUpper() : QString();
}

void NyanColorSwatch::SetTitle(const QString& title)
{
    _title = title;
    const int h = _title.isEmpty() ? kSwatchSize : (kSwatchSize + kTitleHeight);
    setFixedSize(kSwatchSize, h);
    updateGeometry();
    update();
}

auto NyanColorSwatch::Title() const -> QString { return _title; }

auto NyanColorSwatch::sizeHint() const -> QSize
{
    const int h = _title.isEmpty() ? kSwatchSize : (kSwatchSize + kTitleHeight);
    return {kSwatchSize, h};
}

auto NyanColorSwatch::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanColorSwatch::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = underMouse() ? InteractionState::Hovered : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::ColorSwatch, 0, istate);

    // -- Color rectangle --
    const QRect swatchRect(0, 0, kSwatchSize, kSwatchSize);

    p.setPen(QPen(style.border, style.borderWidthPx));

    if (_color.isValid()) {
        p.setBrush(_color);
    } else {
        p.setBrush(style.background);
    }
    p.drawRect(swatchRect.adjusted(0, 0, -1, -1));

    // -- Title text --
    if (!_title.isEmpty()) {
        QFont f = style.font;
        f.setPointSize(f.pointSize() - 1);
        p.setFont(f);
        p.setPen(style.foreground);
        const QRect titleRect(0, kSwatchSize, kSwatchSize, kTitleHeight);
        p.drawText(titleRect, Qt::AlignHCenter | Qt::AlignTop, _title);
    }
}

void NyanColorSwatch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit ColorClicked(_color);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void NyanColorSwatch::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui