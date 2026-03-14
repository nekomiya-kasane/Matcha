/**
 * @file ComboBoxEventFilter.cpp
 * @brief Implementation of ComboBoxEventFilter.
 */

#include "ComboBoxEventFilter.h"

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWidget>

namespace matcha::gui {

namespace cb = fw::fsm::combo_box;

ComboBoxEventFilter::ComboBoxEventFilter(QWidget* watched, fw::WidgetNode* owner)
    : QObject(watched)
    , _ctrl(cb::State::Closed, cb::kTransitions)
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

void ComboBoxEventFilter::NotifyPopupOpened()
{
    _ctrl.Process(cb::Event::ActivateOpen);
}

void ComboBoxEventFilter::NotifyPopupClosed()
{
    _ctrl.Process(cb::Event::ClickOutside);
}

void ComboBoxEventFilter::NotifyItemSelected()
{
    _ctrl.Process(cb::Event::SelectItem);
}

void ComboBoxEventFilter::NotifyEscape()
{
    _ctrl.Process(cb::Event::Escape);
}

auto ComboBoxEventFilter::eventFilter(QObject* obj, QEvent* event) -> bool
{
    switch (event->type()) {
    case QEvent::Enter:
    case QEvent::HoverEnter:
        _ctrl.Process(cb::Event::MouseEnter);
        break;

    case QEvent::Leave:
    case QEvent::HoverLeave:
        _ctrl.Process(cb::Event::MouseLeave);
        break;

    case QEvent::EnabledChange: {
        auto* widget = qobject_cast<QWidget*>(obj);
        if (widget != nullptr && widget->isEnabled()) {
            _ctrl.Process(cb::Event::Enable);
        } else {
            _ctrl.Process(cb::Event::Disable);
        }
        break;
    }

    case QEvent::KeyPress: {
        auto* ke = dynamic_cast<QKeyEvent*>(event);
        if (ke == nullptr) { break; }
        if (ke->key() == Qt::Key_Escape) {
            _ctrl.Process(cb::Event::Escape);
        } else if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
            _ctrl.Process(cb::Event::ArrowNavigate);
        }
        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}

} // namespace matcha::gui
