#include "InteractionEventFilter.h"

#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <QEvent>
#include <QWidget>

namespace matcha::gui {

InteractionEventFilter::InteractionEventFilter(QWidget* watched, fw::WidgetNode* owner)
    : QObject(watched)
    , _owner(owner)
{
    watched->installEventFilter(this);
    watched->setMouseTracking(true);
    watched->setAttribute(Qt::WA_Hover, true);
}

auto InteractionEventFilter::eventFilter(QObject* obj, QEvent* event) -> bool
{
    using fw::InteractionInput;

    InteractionInput input{};
    bool relevant = true;

    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::HoverEnter:
        input = InteractionInput::Enter;
        break;
    case QEvent::Leave:
    case QEvent::HoverLeave:
        input = InteractionInput::Leave;
        break;
    case QEvent::MouseButtonPress:
        input = InteractionInput::Press;
        break;
    case QEvent::MouseButtonRelease:
        input = InteractionInput::Release;
        break;
    case QEvent::FocusIn:
        input = InteractionInput::Focus;
        break;
    case QEvent::FocusOut:
        input = InteractionInput::Blur;
        break;
    case QEvent::EnabledChange: {
        auto* widget = qobject_cast<QWidget*>(obj);
        input = (widget != nullptr && widget->isEnabled())
                    ? InteractionInput::Enable
                    : InteractionInput::Disable;
        break;
    }
    default:
        relevant = false;
        break;
    }

    if (relevant) {
        const auto oldState = _fsm.CurrentState();
        const auto newState = _fsm.HandleInput(input);
        if (oldState != newState && _owner != nullptr) {
            fw::InteractionStateChanged notif(oldState, newState);
            _owner->SendNotification(_owner, notif);
        }
    }

    return QObject::eventFilter(obj, event);
}

} // namespace matcha::gui
