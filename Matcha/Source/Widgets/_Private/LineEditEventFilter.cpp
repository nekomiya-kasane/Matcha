/**
 * @file LineEditEventFilter.cpp
 * @brief Implementation of LineEditEventFilter.
 */

#include "LineEditEventFilter.h"

#include "Matcha/Tree/WidgetNode.h"

#include <QEvent>
#include <QWidget>

namespace matcha::gui {

namespace le = fw::fsm::line_edit;

LineEditEventFilter::LineEditEventFilter(QWidget* watched, fw::WidgetNode* owner)
    : QObject(watched)
    , _ctrl(le::State::Normal, le::kTransitions)
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

void LineEditEventFilter::NotifyValidationFail()
{
    _ctrl.Process(le::Event::ValidationFail);
}

void LineEditEventFilter::NotifyValidationPass()
{
    _ctrl.Process(le::Event::ValidationPass);
}

void LineEditEventFilter::NotifyTextChanged()
{
    _ctrl.Process(le::Event::TextChanged);
}

void LineEditEventFilter::SetReadOnly(bool readOnly)
{
    _ctrl.Process(readOnly ? le::Event::SetReadOnly : le::Event::ClearReadOnly);
}

auto LineEditEventFilter::eventFilter(QObject* obj, QEvent* event) -> bool
{
    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::HoverEnter:
        _ctrl.Process(le::Event::MouseEnter);
        break;

    case QEvent::Leave:
    case QEvent::HoverLeave:
        _ctrl.Process(le::Event::MouseLeave);
        break;

    case QEvent::MouseButtonPress:
        _ctrl.Process(le::Event::Click);
        break;

    case QEvent::FocusIn:
        _ctrl.Process(le::Event::TabFocus);
        break;

    case QEvent::FocusOut:
        _ctrl.Process(le::Event::FocusLost);
        break;

    case QEvent::EnabledChange: {
        auto* widget = qobject_cast<QWidget*>(obj);
        if (widget != nullptr && widget->isEnabled()) {
            _ctrl.Process(le::Event::Enable);
        } else {
            _ctrl.Process(le::Event::Disable);
        }
        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

} // namespace matcha::gui
