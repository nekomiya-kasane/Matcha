/**
 * @file NyanTabItem.cpp
 * @brief NyanTabItem implementation — self-drawn single tab widget.
 */

#include <Matcha/Widgets/Shell/NyanTabItem.h>
#include <Matcha/Widgets/Shell/NyanTabBar.h>

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

namespace matcha::gui {

NyanTabItem::NyanTabItem(TabStyle style, fw::PageId pageId,
                         const QString& title, QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::DocumentBar)
    , _style(style)
    , _pageId(pageId)
    , _title(title)
{
    int w = (style == TabStyle::TitleBar) ? kTitleBarWidth : kFloatingWidth;
    int h = (style == TabStyle::TitleBar) ? kTitleBarHeight : kFloatingHeight;
    setFixedSize(w, h);
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::PointingHandCursor);
}

NyanTabItem::~NyanTabItem() = default;

void NyanTabItem::SetTitle(const QString& title)
{
    _title = title;
    update();
}

void NyanTabItem::SetActive(bool active)
{
    if (_active != active) {
        _active = active;
        update();
    }
}

// -- Size hints --

auto NyanTabItem::sizeHint() const -> QSize
{
    int w = (_style == TabStyle::TitleBar) ? kTitleBarWidth : kFloatingWidth;
    int h = (_style == TabStyle::TitleBar) ? kTitleBarHeight : kFloatingHeight;
    return {w, h};
}

auto NyanTabItem::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// -- Geometry --

auto NyanTabItem::CloseButtonRect() const -> QRect
{
    int x = width() - kCloseSize - 4;
    int y = (height() / 2) - (kCloseSize / 2);
    return {x, y, kCloseSize, kCloseSize};
}

// -- Paint --

void NyanTabItem::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (_style == TabStyle::TitleBar) {
        PaintTitleBar(p);
    } else {
        PaintFloating(p);
    }
}

void NyanTabItem::PaintTitleBar(QPainter& painter)
{
    QRect r = rect().adjusted(1, 2, -1, -2);

    if (_active) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 60));
        painter.drawRoundedRect(r, kRadius, kRadius);
    } else if (_hovered) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 45));
        painter.drawRoundedRect(r, kRadius, kRadius);
    } else {
        painter.setPen(QPen(QColor(255, 255, 255, 30), 1));
        painter.setBrush(QColor(255, 255, 255, 20));
        painter.drawRoundedRect(r, kRadius, kRadius);
    }

    // Text
    QRect textRect = r;
    textRect.setLeft(r.left() + kTextLeft);
    textRect.setRight(r.right() - kTextRight);

    QFont font = painter.font();
    if (_active) {
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255, 255));
    } else if (_hovered) {
        painter.setPen(QColor(255, 255, 255, 230));
    } else {
        painter.setPen(QColor(255, 255, 255, 180));
    }

    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(_title, Qt::ElideMiddle, textRect.width());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedText);

    if (_active) {
        font.setBold(false);
        painter.setFont(font);
    }

    // Close button (active or hovered)
    if (_active || _hovered) {
        PaintCloseButton(painter, CloseButtonRect(), /*darkMode=*/false);
    }
}

void NyanTabItem::PaintFloating(QPainter& painter)
{
    QRect r = rect().adjusted(1, 1, -1, -1);

    if (_active) {
        painter.setPen(QPen(QColor(180, 180, 180), 1));
        painter.setBrush(QColor(255, 255, 255, 255));
        painter.drawRoundedRect(r, 2, 2);
    } else if (_hovered) {
        painter.setPen(QPen(QColor(200, 200, 200), 1));
        painter.setBrush(QColor(245, 245, 245));
        painter.drawRoundedRect(r, 2, 2);
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(235, 235, 235));
        painter.drawRoundedRect(r, 2, 2);
    }

    // Text
    QRect textRect = r;
    textRect.setLeft(r.left() + kTextLeft);
    textRect.setRight(r.right() - kTextRight);

    QFont font = painter.font();
    if (_active) {
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(30, 30, 30));
    } else {
        painter.setPen(QColor(80, 80, 80));
    }

    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(_title, Qt::ElideMiddle, textRect.width());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedText);

    if (_active) {
        font.setBold(false);
        painter.setFont(font);
    }

    // Close button
    if (_active || _hovered) {
        PaintCloseButton(painter, CloseButtonRect(), /*darkMode=*/true);
    }
}

void NyanTabItem::PaintCloseButton(QPainter& painter, const QRect& closeRect,
                                   bool darkMode) const
{
    if (_closeHovered) {
        painter.setPen(Qt::NoPen);
        QColor bg = darkMode ? QColor(0, 0, 0, 30) : QColor(255, 255, 255, 50);
        painter.setBrush(bg);
        painter.drawRoundedRect(closeRect, closeRect.width() / 2.0,
                                closeRect.height() / 2.0);
    }

    QColor penColor = darkMode
        ? QColor(80, 80, 80, _closeHovered ? 255 : 160)
        : QColor(255, 255, 255, _closeHovered ? 255 : 160);
    painter.setPen(QPen(penColor, 1.5));

    int margin = 4;
    painter.drawLine(closeRect.left() + margin, closeRect.top() + margin,
                     closeRect.right() - margin, closeRect.bottom() - margin);
    painter.drawLine(closeRect.right() - margin, closeRect.top() + margin,
                     closeRect.left() + margin, closeRect.bottom() - margin);
}

// -- Mouse Events --

void NyanTabItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    // Check close button first
    if (CloseButtonRect().contains(event->pos())) {
        Q_EMIT CloseRequested(_pageId);
        return;
    }

    _dragArmed = true;
    _reordering = false;
    _dragStartPos = event->pos();
    _dragStartX = mapToGlobal(event->pos()).x();
    Q_EMIT Pressed(_pageId);

    event->accept(); // Consume event — prevent propagation to parent title bar
}

void NyanTabItem::mouseMoveEvent(QMouseEvent* event)
{
    // Close button hover
    bool newCloseHover = (_active || _hovered)
                         && CloseButtonRect().contains(event->pos());
    if (newCloseHover != _closeHovered) {
        _closeHovered = newCloseHover;
        update();
    }

    if (!_dragArmed) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - _dragStartPos).manhattanLength() < 10) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    auto* parentBar = qobject_cast<NyanTabBar*>(parentWidget());
    if (parentBar == nullptr) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    QPoint posInBar = mapToParent(event->pos());
    QRect barRect = parentBar->rect();
    barRect.adjust(0, -barRect.height(), 0, barRect.height());

    if (barRect.contains(posInBar)) {
        // Reorder mode: still within bar vertical bounds
        _reordering = true;
        parentBar->RequestReorder(this, mapToGlobal(event->pos()).x());
    } else {
        // Detach mode: left bar vertically -> start QDrag
        _dragArmed = false;
        _reordering = false;

        auto* drag = new QDrag(this);
        auto* mimeData = new QMimeData();
        mimeData->setData(NyanTabBar::kMimeType,
            QByteArray::number(static_cast<qulonglong>(_pageId.value)));
        drag->setMimeData(mimeData);

        Qt::DropAction result = drag->exec(Qt::MoveAction);

        // Delegate result handling to parent bar
        parentBar->HandleDragResult(_pageId, result);
    }

    QWidget::mouseMoveEvent(event);
}

void NyanTabItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (_reordering) {
        _reordering = false;
        // Reorder is already committed during mouseMoveEvent
    }
    _dragArmed = false;
    QWidget::mouseReleaseEvent(event);
}

void NyanTabItem::enterEvent(QEnterEvent* /*event*/)
{
    _hovered = true;
    update();
}

void NyanTabItem::leaveEvent(QEvent* event)
{
    _hovered = false;
    _closeHovered = false;
    update();
    QWidget::leaveEvent(event);
}

void NyanTabItem::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
