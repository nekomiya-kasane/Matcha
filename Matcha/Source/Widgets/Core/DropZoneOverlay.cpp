/**
 * @file DropZoneOverlay.cpp
 * @brief Implementation of the drop zone overlay widget.
 */

#include <Matcha/Widgets/Core/DropZoneOverlay.h>

#include <QDebug>
#include <QPainter>

namespace matcha::gui {

DropZoneOverlay::DropZoneOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);
}

DropZoneOverlay::~DropZoneOverlay() = default;

void DropZoneOverlay::SetActiveZone(fw::DropZone zone)
{
    _zone = zone;
    _hasZone = true;

    static const char* kZoneNames[] = {"Center", "Top", "Bottom", "Left", "Right"};
    auto zr = ZoneRect(zone);
    qDebug().noquote() << QString("[DropZone] zone=%1 rect=(%2,%3 %4x%5) overlay=(%6x%7)")
        .arg(kZoneNames[static_cast<int>(zone)])
        .arg(zr.x()).arg(zr.y()).arg(zr.width()).arg(zr.height())
        .arg(width()).arg(height());

    update();
}

void DropZoneOverlay::ClearZone()
{
    _hasZone = false;
    update();
}

auto DropZoneOverlay::ZoneAtPoint(const QPoint& pos) const -> fw::DropZone
{
    int w = width();
    int h = height();
    if (w <= 0 || h <= 0) {
        return fw::DropZone::Center;
    }

    // Normalize to [0, 1]
    double nx = static_cast<double>(pos.x()) / w;
    double ny = static_cast<double>(pos.y()) / h;

    // Edge threshold: 30% from each edge
    constexpr double kEdge = 0.30;

    if (ny < kEdge) { return fw::DropZone::Top; }
    if (ny > (1.0 - kEdge)) { return fw::DropZone::Bottom; }
    if (nx < kEdge) { return fw::DropZone::Left; }
    if (nx > (1.0 - kEdge)) { return fw::DropZone::Right; }
    return fw::DropZone::Center;
}

auto DropZoneOverlay::ZoneRect(fw::DropZone zone) const -> QRect
{
    int w = width();
    int h = height();
    int halfW = w / 2;
    int halfH = h / 2;

    switch (zone) {
    case fw::DropZone::Top:
        return {0, 0, w, halfH};
    case fw::DropZone::Bottom:
        return {0, halfH, w, h - halfH};
    case fw::DropZone::Left:
        return {0, 0, halfW, h};
    case fw::DropZone::Right:
        return {halfW, 0, w - halfW, h};
    case fw::DropZone::Center:
        return {0, 0, w, h};
    }
    return {};
}

void DropZoneOverlay::paintEvent(QPaintEvent* /*event*/)
{
    if (!_hasZone) {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Dim the entire viewport
    p.fillRect(rect(), QColor(0, 0, 0, 40));

    // Highlight the active zone
    QRect zr = ZoneRect(_zone);
    QColor highlight(66, 133, 244, 80); // Blue tint
    QColor border(66, 133, 244, 180);

    p.fillRect(zr, highlight);
    p.setPen(QPen(border, 2));
    p.drawRect(zr.adjusted(1, 1, -1, -1));
}

} // namespace matcha::gui
