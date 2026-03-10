/**
 * @file WidgetPickerFilter.cpp
 * @brief QApplication-level event filter for widget picking.
 */

#include "WidgetPickerFilter.h"
#include "LayoutBoundsOverlay.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QWidget>

namespace nyancad {

WidgetPickerFilter::WidgetPickerFilter(LayoutBoundsOverlay* overlay, QObject* parent)
    : QObject(parent)
    , _overlay(overlay)
{
}

WidgetPickerFilter::~WidgetPickerFilter()
{
    Deactivate();
}

void WidgetPickerFilter::Activate()
{
    if (_active) { return; }
    _active = true;
    QApplication::instance()->installEventFilter(this);
}

void WidgetPickerFilter::Deactivate()
{
    if (!_active) { return; }
    _active = false;
    if (QApplication::instance() != nullptr) {
        QApplication::instance()->removeEventFilter(this);
    }
    if (_overlay != nullptr) {
        _overlay->SetPickedWidget(nullptr);
    }
}

auto WidgetPickerFilter::IsActive() const -> bool
{
    return _active;
}

bool WidgetPickerFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (!_active) {
        return QObject::eventFilter(obj, event);
    }

    if (event->type() == QEvent::MouseMove) {
        auto* me = static_cast<QMouseEvent*>(event);
        auto* widget = QApplication::widgetAt(me->globalPosition().toPoint());

        // Skip our own overlay and diagnostics window widgets
        if (widget != nullptr && widget->window() != _overlay) {
            if (_overlay != nullptr) {
                _overlay->SetPickedWidget(widget);
            }
            Q_EMIT Hovered(widget);
        }
        return false; // don't block the event
    }

    if (event->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton) {
            auto* widget = QApplication::widgetAt(me->globalPosition().toPoint());
            if (widget != nullptr && widget->window() != _overlay) {
                Q_EMIT Picked(widget);
                Deactivate();
                return true; // consume the click
            }
        }
        // Escape with right-click
        if (me->button() == Qt::RightButton) {
            Deactivate();
            return true;
        }
    }

    // Escape key cancels pick mode
    if (event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Escape) {
            Deactivate();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

} // namespace nyancad
