/**
 * @file SimpleWidgetEventFilter.cpp
 * @brief Implementation of SimpleWidgetEventFilter.
 */

#include "SimpleWidgetEventFilter.h"

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <QEvent>
#include <QWidget>

namespace matcha::gui {

namespace sw = fw::fsm::simple_widget;

SimpleWidgetEventFilter::SimpleWidgetEventFilter(QWidget* watched, fw::WidgetNode* owner)
    : QObject(watched)
    , _ctrl(sw::State::Normal, sw::kTransitions)
    , _owner(owner)
{
    watched->installEventFilter(this);
    watched->setMouseTracking(true);
    watched->setAttribute(Qt::WA_Hover, true);

    _ctrl.OnInteractionStateChanged([this](fw::InteractionState oldIS, fw::InteractionState newIS) {
        if (_owner != nullptr) {
            fw::InteractionStateChanged notif(oldIS, newIS);
            _owner->SendNotification(_owner, notif);
        }
    });
}

auto SimpleWidgetEventFilter::eventFilter(QObject* obj, QEvent* event) -> bool
{
    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::HoverEnter:
        _ctrl.Process(sw::Event::MouseEnter);
        break;

    case QEvent::Leave:
    case QEvent::HoverLeave:
        _ctrl.Process(sw::Event::MouseLeave);
        break;

    case QEvent::MouseButtonPress:
        _ctrl.Process(sw::Event::MouseDown);
        break;

    case QEvent::MouseButtonRelease:
        _ctrl.Process(sw::Event::MouseUp);
        break;

    case QEvent::FocusIn:
        _ctrl.Process(sw::Event::FocusIn);
        break;

    case QEvent::FocusOut:
        _ctrl.Process(sw::Event::FocusOut);
        break;

    case QEvent::EnabledChange: {
        auto* widget = qobject_cast<QWidget*>(obj);
        if (widget != nullptr && widget->isEnabled()) {
            _ctrl.Process(sw::Event::Enable);
        } else {
            _ctrl.Process(sw::Event::Disable);
        }
        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

} // namespace matcha::gui
