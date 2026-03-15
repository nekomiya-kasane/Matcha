/**
 * @file PushButtonEventFilter.cpp
 * @brief Implementation of PushButtonEventFilter.
 */

#include "PushButtonEventFilter.h"

#include "Matcha/Tree/WidgetNode.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWidget>

namespace matcha::gui {

namespace pb = fw::fsm::push_button;

PushButtonEventFilter::PushButtonEventFilter(QWidget* watched, fw::WidgetNode* owner)
    : QObject(watched)
    , _ctrl(pb::State::Normal, pb::kTransitions)
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

auto PushButtonEventFilter::eventFilter(QObject* obj, QEvent* event) -> bool
{
    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::HoverEnter:
        _mouseInside = true;
        _ctrl.Process(pb::Event::MouseEnter);
        break;

    case QEvent::Leave:
    case QEvent::HoverLeave:
        _mouseInside = false;
        _ctrl.Process(pb::Event::MouseLeave);
        break;

    case QEvent::MouseButtonPress:
        _ctrl.Process(pb::Event::MouseDown);
        break;

    case QEvent::MouseButtonRelease:
        if (_mouseInside) {
            _ctrl.Process(pb::Event::MouseUpInside);
        } else {
            _ctrl.Process(pb::Event::MouseUpOutside);
        }
        break;

    case QEvent::FocusIn:
        _ctrl.Process(pb::Event::TabFocus);
        break;

    case QEvent::FocusOut:
        _ctrl.Process(pb::Event::TabAway);
        break;

    case QEvent::KeyPress: {
        const auto* ke = dynamic_cast<QKeyEvent*>(event);
        if (ke == nullptr) { break; }
        if (ke->key() == Qt::Key_Space) {
            _ctrl.Process(pb::Event::SpaceDown);
        } else if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            _ctrl.Process(pb::Event::EnterDown);
        }
        break;
    }

    case QEvent::KeyRelease: {
        const auto* ke = dynamic_cast<QKeyEvent*>(event);
        if (ke == nullptr) { break; }
        if (ke->key() == Qt::Key_Space) {
            // Space key-up: transition back and fire Activated
            _ctrl.Process(pb::Event::MouseUpInside);
        }
        break;
    }

    case QEvent::EnabledChange: {
        auto* widget = qobject_cast<QWidget*>(obj);
        if (widget != nullptr && widget->isEnabled()) {
            _ctrl.Process(pb::Event::Enable);
        } else {
            _ctrl.Process(pb::Event::Disable);
        }
        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

} // namespace matcha::gui
