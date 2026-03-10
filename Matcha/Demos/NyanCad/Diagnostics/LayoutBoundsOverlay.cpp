/**
 * @file LayoutBoundsOverlay.cpp
 * @brief Transparent overlay painting red bounds around all visible widgets.
 */

#include "LayoutBoundsOverlay.h"

#include <QEvent>
#include <QPainter>
#include <QTimer>

namespace nyancad {

LayoutBoundsOverlay::LayoutBoundsOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    _refreshTimer = new QTimer(this);
    _refreshTimer->setInterval(66); // ~15 fps
    connect(_refreshTimer, &QTimer::timeout, this, [this]() {
        Reposition();
        update();
    });
}

LayoutBoundsOverlay::~LayoutBoundsOverlay() = default;

void LayoutBoundsOverlay::SetTargetWindow(QWidget* target)
{
    if (_target) {
        _target->removeEventFilter(this);
    }
    _target = target;
    if (_target) {
        _target->installEventFilter(this);
        Reposition();
    }
}

void LayoutBoundsOverlay::SetOverlayVisible(bool visible)
{
    if (visible) {
        Reposition();
        show();
        raise();
        _refreshTimer->start();
    } else {
        hide();
        _refreshTimer->stop();
    }
}

auto LayoutBoundsOverlay::IsOverlayVisible() const -> bool
{
    return isVisible();
}

void LayoutBoundsOverlay::SetPickedWidget(QWidget* widget)
{
    _picked = widget;
    if (isVisible()) {
        update();
    }
}

void LayoutBoundsOverlay::paintEvent(QPaintEvent* /*event*/)
{
    if (_target == nullptr || !_target->isVisible()) {
        return;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Collect all visible child widgets of the target
    const auto children = _target->findChildren<QWidget*>();

    // Red pen for layout bounds
    QPen boundsPen(QColor(255, 60, 60, 160), 1, Qt::SolidLine);

    for (auto* child : children) {
        if (!child->isVisible() || child == this || child->width() == 0 || child->height() == 0) {
            continue;
        }
        // Skip widgets that are children of the overlay itself
        if (child->window() == this) {
            continue;
        }

        // Map child's rect to overlay coordinates
        const QPoint topLeft = child->mapToGlobal(QPoint(0, 0));
        const QPoint local = mapFromGlobal(topLeft);
        const QRect r(local, child->size());

        if (child == _picked) {
            // Picked widget: blue highlight
            p.fillRect(r, QColor(66, 133, 244, 40));
            p.setPen(QPen(QColor(66, 133, 244, 220), 2, Qt::SolidLine));
            p.drawRect(r);

            // Label
            const QString label = QString::fromUtf8(child->metaObject()->className())
                + QStringLiteral(" %1x%2").arg(child->width()).arg(child->height());
            QFont f = p.font();
            f.setPixelSize(10);
            p.setFont(f);
            const QRect labelRect(r.left(), r.top() - 14, r.width(), 14);
            p.fillRect(labelRect, QColor(66, 133, 244, 200));
            p.setPen(Qt::white);
            p.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter,
                       QStringLiteral(" ") + label);
        } else {
            // Normal: red bounds
            p.setPen(boundsPen);
            p.drawRect(r);
        }
    }
}

bool LayoutBoundsOverlay::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == _target) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            Reposition();
        }
        if (event->type() == QEvent::Hide || event->type() == QEvent::Close) {
            hide();
        }
        if (event->type() == QEvent::Show && _refreshTimer->isActive()) {
            Reposition();
            show();
            raise();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void LayoutBoundsOverlay::Reposition()
{
    if (_target == nullptr) {
        return;
    }
    const QPoint globalPos = _target->mapToGlobal(QPoint(0, 0));
    setGeometry(globalPos.x(), globalPos.y(), _target->width(), _target->height());
}

} // namespace nyancad
