#include <Matcha/Widgets/ActionBar/ActionBarFloatingFrame.h>
#include <Matcha/Widgets/ActionBar/NyanActionBar.h>

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

namespace matcha::gui {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ActionBarFloatingFrame::ActionBarFloatingFrame(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setMouseTracking(true);
    setMinimumSize(200, 60);

    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(1, kGripHeight + 1, 1, 1); // 1px border + grip area at top
    _layout->setSpacing(0);
}

ActionBarFloatingFrame::~ActionBarFloatingFrame() = default;

// ---------------------------------------------------------------------------
// ActionBar management
// ---------------------------------------------------------------------------

void ActionBarFloatingFrame::SetActionBar(NyanActionBar* bar)
{
    if (_actionBar == bar) { return; }

    // Remove previous
    if (_actionBar != nullptr) {
        _layout->removeWidget(_actionBar);
    }

    _actionBar = bar;

    if (_actionBar != nullptr) {
        _actionBar->setParent(this);
        _actionBar->SetOrientation(Qt::Horizontal);
        _layout->addWidget(_actionBar);
        _actionBar->show();
    }
}

auto ActionBarFloatingFrame::GetActionBar() const -> NyanActionBar*
{
    return _actionBar;
}

auto ActionBarFloatingFrame::TakeActionBar() -> NyanActionBar*
{
    NyanActionBar* bar = _actionBar;
    if (_actionBar != nullptr) {
        _layout->removeWidget(_actionBar);
        _actionBar->setParent(nullptr);
        _actionBar = nullptr;
    }
    return bar;
}

// ---------------------------------------------------------------------------
// Dock target
// ---------------------------------------------------------------------------

void ActionBarFloatingFrame::SetDockTarget(QWidget* container)
{
    _dockTarget = container;
}

auto ActionBarFloatingFrame::DockTarget() const -> QWidget*
{
    return _dockTarget;
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void ActionBarFloatingFrame::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background with border
    QColor bg(50, 50, 55, 240);
    QColor border(80, 80, 85);
    QColor grip(100, 100, 110);

    QPainterPath path;
    path.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5),
                        kBorderRadius, kBorderRadius);
    p.fillPath(path, bg);
    p.setPen(QPen(border, 1.0));
    p.drawPath(path);

    // Grip handle: centered dots/line at top
    int gripY = kGripHeight / 2 + 1;
    int gripW = 40;
    int gripX = (width() - gripW) / 2; // NOLINT(bugprone-integer-division)
    p.setPen(Qt::NoPen);
    p.setBrush(grip);
    p.drawRoundedRect(gripX, gripY, gripW, 3, 1.5, 1.5);
}

// ---------------------------------------------------------------------------
// Mouse events (grip drag)
// ---------------------------------------------------------------------------

auto ActionBarFloatingFrame::IsInGripArea(QPoint pos) const -> bool
{
    return pos.y() <= kGripHeight + 2;
}

void ActionBarFloatingFrame::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && IsInGripArea(event->pos())) {
        _dragging = true;
        _dragStartPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void ActionBarFloatingFrame::mouseMoveEvent(QMouseEvent* event)
{
    if (_dragging) {
        QPoint newPos = event->globalPosition().toPoint() - _dragStartPos;
        move(newPos);
        Q_EMIT DragNearEdge(event->globalPosition().toPoint());
        event->accept();
        return;
    }

    // Update cursor based on position
    if (IsInGripArea(event->pos())) {
        setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }

    QWidget::mouseMoveEvent(event);
}

void ActionBarFloatingFrame::mouseReleaseEvent(QMouseEvent* event)
{
    if (_dragging && event->button() == Qt::LeftButton) {
        _dragging = false;
        setCursor(Qt::ArrowCursor);
        Q_EMIT DragFinished(event->globalPosition().toPoint());
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

// ---------------------------------------------------------------------------
// Resize
// ---------------------------------------------------------------------------

void ActionBarFloatingFrame::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // ActionBar inside will auto-reflow via its layout
}

} // namespace matcha::gui
