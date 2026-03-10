#include <Matcha/Widgets/ActionBar/TrapezoidHandle.h>

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace matcha::gui {

TrapezoidHandle::TrapezoidHandle(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::ActionBar)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(sizeHint());
}

TrapezoidHandle::~TrapezoidHandle() = default;

void TrapezoidHandle::SetDockSide(DockSide side)
{
    if (_dockSide == side) { return; }
    _dockSide = side;
    setFixedSize(sizeHint());
    update();
}

auto TrapezoidHandle::GetDockSide() const -> DockSide
{
    return _dockSide;
}

auto TrapezoidHandle::sizeHint() const -> QSize
{
    // For top/bottom docking: wide horizontally, shallow vertically
    // For left/right docking: shallow horizontally, wide vertically
    switch (_dockSide) {
    case DockSide::Bottom:
    case DockSide::Top:
        return {kLongEdge, kDepth};
    case DockSide::Left:
    case DockSide::Right:
        return {kDepth, kLongEdge};
    }
    return {kLongEdge, kDepth};
}

auto TrapezoidHandle::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

auto TrapezoidHandle::BuildTrapezoidPath() const -> QPainterPath
{
    const qreal w = width();
    const qreal h = height();
    const qreal inset = static_cast<qreal>(kLongEdge - kShortEdge) / 2.0;

    QPainterPath path;

    switch (_dockSide) {
    case DockSide::Bottom:
        // Wider at bottom, narrower at top
        path.moveTo(0, h);
        path.lineTo(w, h);
        path.lineTo(w - inset, 0);
        path.lineTo(inset, 0);
        break;
    case DockSide::Top:
        // Wider at top, narrower at bottom
        path.moveTo(0, 0);
        path.lineTo(w, 0);
        path.lineTo(w - inset, h);
        path.lineTo(inset, h);
        break;
    case DockSide::Left:
        // Wider at left, narrower at right
        path.moveTo(0, 0);
        path.lineTo(0, h);
        path.lineTo(w, h - inset);
        path.lineTo(w, inset);
        break;
    case DockSide::Right:
        // Wider at right, narrower at left
        path.moveTo(w, 0);
        path.lineTo(w, h);
        path.lineTo(0, h - inset);
        path.lineTo(0, inset);
        break;
    }

    path.closeSubpath();
    return path;
}

void TrapezoidHandle::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();
    QColor bg = theme.Color(ColorToken::FillActive);
    QColor border = theme.Color(ColorToken::BorderDefault);

    if (_pressed) {
        bg = theme.Color(ColorToken::FillActive);
    } else if (_hovered) {
        bg = theme.Color(ColorToken::FillActive);
    }

    auto path = BuildTrapezoidPath();
    p.setPen(QPen(border, 1.0));
    p.setBrush(bg);
    p.drawPath(path);

    // Draw a small arrow/chevron in the center pointing away from the edge
    const QRectF r = path.boundingRect();
    const QPointF center = r.center();
    const qreal arrowHalf = 4.0;

    p.setPen(QPen(theme.Color(ColorToken::TextSecondary), 1.5));
    p.setBrush(Qt::NoBrush);

    QPainterPath arrow;
    switch (_dockSide) {
    case DockSide::Bottom:
        // Chevron pointing up (expand upward)
        arrow.moveTo(center.x() - arrowHalf, center.y() + 2);
        arrow.lineTo(center.x(), center.y() - 2);
        arrow.lineTo(center.x() + arrowHalf, center.y() + 2);
        break;
    case DockSide::Top:
        // Chevron pointing down
        arrow.moveTo(center.x() - arrowHalf, center.y() - 2);
        arrow.lineTo(center.x(), center.y() + 2);
        arrow.lineTo(center.x() + arrowHalf, center.y() - 2);
        break;
    case DockSide::Left:
        // Chevron pointing right
        arrow.moveTo(center.x() - 2, center.y() - arrowHalf);
        arrow.lineTo(center.x() + 2, center.y());
        arrow.lineTo(center.x() - 2, center.y() + arrowHalf);
        break;
    case DockSide::Right:
        // Chevron pointing left
        arrow.moveTo(center.x() + 2, center.y() - arrowHalf);
        arrow.lineTo(center.x() - 2, center.y());
        arrow.lineTo(center.x() + 2, center.y() + arrowHalf);
        break;
    }
    p.drawPath(arrow);
}

void TrapezoidHandle::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _pressed = true;
        update();
    }
    QWidget::mousePressEvent(event);
}

void TrapezoidHandle::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _pressed) {
        _pressed = false;
        update();
        if (rect().contains(event->pos())) {
            Q_EMIT Clicked();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void TrapezoidHandle::enterEvent(QEnterEvent* event)
{
    _hovered = true;
    update();
    QWidget::enterEvent(event);
}

void TrapezoidHandle::leaveEvent(QEvent* event)
{
    _hovered = false;
    _pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void TrapezoidHandle::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
