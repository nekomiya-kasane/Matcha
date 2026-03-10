/**
 * @file ViewportHeaderBar.cpp
 * @brief Implementation of the viewport header bar widget.
 */

#include <Matcha/Widgets/Shell/ViewportHeaderBar.h>

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

namespace matcha::gui {

ViewportHeaderBar::ViewportHeaderBar(fw::ViewportId vpId, QWidget* parent)
    : QWidget(parent)
    , _vpId(vpId)
    , _label(QStringLiteral("Viewport"))
{
    setFixedHeight(kHeight);
    setMouseTracking(true);
    setCursor(Qt::OpenHandCursor);
}

ViewportHeaderBar::~ViewportHeaderBar() = default;

void ViewportHeaderBar::SetLabel(const QString& text)
{
    _label = text;
    update();
}

auto ViewportHeaderBar::Label() const -> QString
{
    return _label;
}

void ViewportHeaderBar::SetCloseButtonVisible(bool visible)
{
    _closeVisible = visible;
    update();
}

void ViewportHeaderBar::SetGhostMode(bool ghost)
{
    _ghostMode = ghost;
    update();
}

auto ViewportHeaderBar::CloseButtonRect() const -> QRect
{
    int x = width() - kCloseButtonSize - 4;
    int y = (height() - kCloseButtonSize) / 2;
    return {x, y, kCloseButtonSize, kCloseButtonSize};
}

auto ViewportHeaderBar::SplitHButtonRect() const -> QRect
{
    int x = width() - (kCloseButtonSize * 2) - 8;
    int y = (height() - kCloseButtonSize) / 2;
    return {x, y, kCloseButtonSize, kCloseButtonSize};
}

auto ViewportHeaderBar::SplitVButtonRect() const -> QRect
{
    int x = width() - (kCloseButtonSize * 3) - 12;
    int y = (height() - kCloseButtonSize) / 2;
    return {x, y, kCloseButtonSize, kCloseButtonSize};
}

void ViewportHeaderBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (_ghostMode) {
        p.setOpacity(0.4);
    }

    // Background
    QColor bgColor = _hovered ? QColor(60, 63, 65) : QColor(49, 51, 53);
    p.fillRect(rect(), bgColor);

    // Bottom border
    p.setPen(QColor(70, 73, 75));
    p.drawLine(0, height() - 1, width(), height() - 1);

    // Label
    p.setPen(QColor(187, 187, 187));
    QFont font = p.font();
    font.setPixelSize(11);
    p.setFont(font);
    int labelRight = _closeVisible ? width() - kCloseButtonSize - 10 : width() - 6;
    QRect labelRect(6, 0, labelRight - 6, height());
    p.drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft, _label);

    // Buttons (visible on hover)
    if (_hovered) {
        p.setPen(QColor(187, 187, 187));
        QFont btnFont = p.font();
        btnFont.setPixelSize(10);
        btnFont.setBold(true);
        p.setFont(btnFont);

        // Split V button (leftmost): draw "V" icon with horizontal line
        {
            QRect r = SplitVButtonRect();
            if (_splitVHovered) { p.fillRect(r, QColor(80, 83, 85)); }
            p.setPen(QColor(187, 187, 187));
            // Draw top-bottom split icon: a rectangle with horizontal divider
            int m = 3;
            p.drawRect(r.adjusted(m, m, -m, -m));
            int midY = r.top() + (r.height() / 2);
            p.drawLine(r.left() + m, midY, r.right() - m, midY);
        }

        // Split H button: draw "H" icon with vertical line
        {
            QRect r = SplitHButtonRect();
            if (_splitHHovered) { p.fillRect(r, QColor(80, 83, 85)); }
            p.setPen(QColor(187, 187, 187));
            // Draw left-right split icon: a rectangle with vertical divider
            int m = 3;
            p.drawRect(r.adjusted(m, m, -m, -m));
            int midX = r.left() + (r.width() / 2);
            p.drawLine(midX, r.top() + m, midX, r.bottom() - m);
        }

        // Close button (X)
        if (_closeVisible) {
            QRect cbRect = CloseButtonRect();
            if (_closeHovered) {
                p.fillRect(cbRect, QColor(80, 83, 85));
            }
            p.setPen(QColor(187, 187, 187));
            int margin = 4;
            p.drawLine(cbRect.left() + margin, cbRect.top() + margin,
                       cbRect.right() - margin, cbRect.bottom() - margin);
            p.drawLine(cbRect.right() - margin, cbRect.top() + margin,
                       cbRect.left() + margin, cbRect.bottom() - margin);
        }
    }
}

void ViewportHeaderBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if close button was clicked
        if (_closeVisible && CloseButtonRect().contains(event->pos())) {
            emit closeRequested(_vpId);
            return;
        }
        // Check split buttons
        if (_hovered && SplitHButtonRect().contains(event->pos())) {
            emit splitHRequested(_vpId);
            return;
        }
        if (_hovered && SplitVButtonRect().contains(event->pos())) {
            emit splitVRequested(_vpId);
            return;
        }
        _dragStartPos = event->pos();
        _dragging = false;
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void ViewportHeaderBar::mouseMoveEvent(QMouseEvent* event)
{
    // Update button hover states
    bool needsUpdate = false;
    if (_closeVisible) {
        bool was = _closeHovered;
        _closeHovered = CloseButtonRect().contains(event->pos());
        needsUpdate |= (was != _closeHovered);
    }
    if (_hovered) {
        bool wasH = _splitHHovered;
        _splitHHovered = SplitHButtonRect().contains(event->pos());
        needsUpdate |= (wasH != _splitHHovered);

        bool wasV = _splitVHovered;
        _splitVHovered = SplitVButtonRect().contains(event->pos());
        needsUpdate |= (wasV != _splitVHovered);
    }
    if (needsUpdate) {
        update();
    }

    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    if (!_dragging) {
        int dist = (event->pos() - _dragStartPos).manhattanLength();
        if (dist < kDragThreshold) {
            return;
        }
        _dragging = true;

        emit dragStarted(_vpId);

        // Start QDrag
        auto* drag = new QDrag(this);
        auto* mimeData = new QMimeData();
        mimeData->setData("application/x-matcha-viewport",
                          QByteArray::number(static_cast<qulonglong>(_vpId.value)));
        drag->setMimeData(mimeData);

        // Create a small pixmap for the drag cursor
        QPixmap pixmap(120, kHeight);
        pixmap.fill(QColor(49, 51, 53, 180));
        QPainter pixPainter(&pixmap);
        pixPainter.setPen(QColor(187, 187, 187));
        QFont font;
        font.setPixelSize(11);
        pixPainter.setFont(font);
        pixPainter.drawText(pixmap.rect(), Qt::AlignCenter, _label);
        pixPainter.end();
        drag->setPixmap(pixmap);

        auto result = drag->exec(Qt::MoveAction);

        emit dragEnded(_vpId, result == Qt::MoveAction);

        _dragging = false;
        setCursor(Qt::OpenHandCursor);
    }
}

void ViewportHeaderBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = false;
        setCursor(Qt::OpenHandCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void ViewportHeaderBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit maximizeToggled(_vpId);
    }
}

void ViewportHeaderBar::enterEvent(QEnterEvent* /*event*/)
{
    _hovered = true;
    update();
}

void ViewportHeaderBar::leaveEvent(QEvent* /*event*/)
{
    _hovered = false;
    _closeHovered = false;
    _splitHHovered = false;
    _splitVHovered = false;
    update();
}

} // namespace matcha::gui
