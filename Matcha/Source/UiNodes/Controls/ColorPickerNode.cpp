#include "Matcha/UiNodes/Controls/ColorPickerNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanColorPicker.h"

#include <QColor>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ColorPickerNode, WidgetNode)

ColorPickerNode::ColorPickerNode(std::string id)
    : WidgetNode(std::move(id), NodeType::ColorPicker)
{
}

ColorPickerNode::~ColorPickerNode() = default;

void ColorPickerNode::SetColor(uint32_t rgba)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanColorPicker*>(_widget)) {
        w->SetColor(QColor::fromRgba(rgba));
    }
}

auto ColorPickerNode::Color() const -> uint32_t
{
    if (auto* w = qobject_cast<gui::NyanColorPicker*>(_widget)) {
        return w->Color().rgba();
    }
    return 0;
}

void ColorPickerNode::SetAlphaEnabled(bool enabled)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanColorPicker*>(_widget)) {
        w->SetAlphaEnabled(enabled);
    }
}

auto ColorPickerNode::IsAlphaEnabled() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanColorPicker*>(_widget)) {
        return w->IsAlphaEnabled();
    }
    return false;
}

auto ColorPickerNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanColorPicker(parent);
    QObject::connect(w, &gui::NyanColorPicker::ColorChanged, w, [this](const QColor& color) {
        ColorChanged notif(color.rgba());
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
