#include "Matcha/Tree/Controls/ColorSwatchNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanColorSwatch.h"

#include <QColor>
#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ColorSwatchNode, WidgetNode)

ColorSwatchNode::ColorSwatchNode(std::string id)
    : WidgetNode(std::move(id), NodeType::ColorSwatch)
{
}

ColorSwatchNode::~ColorSwatchNode() = default;

void ColorSwatchNode::SetColor(uint32_t rgba)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanColorSwatch*>(_widget)) {
        w->SetColor(QColor::fromRgba(rgba));
    }
}

auto ColorSwatchNode::Color() const -> uint32_t
{
    if (auto* w = qobject_cast<gui::NyanColorSwatch*>(_widget)) {
        return w->Color().rgba();
    }
    return 0;
}

void ColorSwatchNode::SetTitle(std::string_view title)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanColorSwatch*>(_widget)) {
        w->SetTitle(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
    }
}

auto ColorSwatchNode::Title() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanColorSwatch*>(_widget)) {
        return w->Title().toStdString();
    }
    return {};
}

void ColorSwatchNode::SetSwatchSize(int w, int h)
{
    EnsureWidget();
    if (auto* swatch = qobject_cast<gui::NyanColorSwatch*>(_widget)) {
        swatch->setFixedSize(w, h);
    }
}

auto ColorSwatchNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanColorSwatch(parent);
    QObject::connect(w, &gui::NyanColorSwatch::ColorClicked, w, [this](const QColor& color) {
        ColorChanged cnotif(color.rgba());
        SendNotification(this, cnotif);
        Activated anotif;
        SendNotification(this, anotif);
    });
    return w;
}

} // namespace matcha::fw
