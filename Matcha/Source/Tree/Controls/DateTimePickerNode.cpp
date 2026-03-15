#include "Matcha/Tree/Controls/DateTimePickerNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanDateTimePicker.h"

#include <QDateTime>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(DateTimePickerNode, WidgetNode)

DateTimePickerNode::DateTimePickerNode(std::string id)
    : WidgetNode(std::move(id), NodeType::DateTimePicker)
{
}

DateTimePickerNode::~DateTimePickerNode() = default;

void DateTimePickerNode::SetMode(uint8_t mode)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDateTimePicker*>(_widget)) {
        w->SetMode(static_cast<gui::DateTimeMode>(mode));
    }
}

auto DateTimePickerNode::Mode() const -> uint8_t
{
    if (auto* w = qobject_cast<gui::NyanDateTimePicker*>(_widget)) {
        return static_cast<uint8_t>(w->Mode());
    }
    return 0;
}

void DateTimePickerNode::SetDateTimeMsecs(int64_t msecsSinceEpoch)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDateTimePicker*>(_widget)) {
        w->setDateTime(QDateTime::fromMSecsSinceEpoch(msecsSinceEpoch));
    }
}

auto DateTimePickerNode::DateTimeMsecs() const -> int64_t
{
    if (auto* w = qobject_cast<gui::NyanDateTimePicker*>(_widget)) {
        return w->dateTime().toMSecsSinceEpoch();
    }
    return 0;
}

auto DateTimePickerNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanDateTimePicker(parent);
    QObject::connect(w, &QDateTimeEdit::dateTimeChanged, w,
        [this](const QDateTime& dt) {
            DateTimeChanged notif(dt.toMSecsSinceEpoch());
            SendNotification(this, notif);
        });
    return w;
}

} // namespace matcha::fw
