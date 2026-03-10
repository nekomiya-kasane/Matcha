#include "Matcha/UiNodes/Controls/PropertyGridNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanPropertyGrid.h"

#include <QString>
#include <QVariant>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(PropertyGridNode, WidgetNode)

PropertyGridNode::PropertyGridNode(std::string id)
    : WidgetNode(std::move(id), NodeType::PropertyGrid)
{
}

PropertyGridNode::~PropertyGridNode() = default;

void PropertyGridNode::AddProperty(std::string_view name, gui::PropertyType type,
                                   std::string_view defaultValue)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        auto qName = QString::fromUtf8(name.data(), static_cast<int>(name.size()));
        QVariant defVal;
        if (!defaultValue.empty()) {
            defVal = QString::fromUtf8(defaultValue.data(), static_cast<int>(defaultValue.size()));
        }
        w->AddProperty(qName, type, defVal);
    }
}

void PropertyGridNode::AddProperty(std::string_view name, gui::PropertyType type,
                                   double defaultValue)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        auto qName = QString::fromUtf8(name.data(), static_cast<int>(name.size()));
        w->AddProperty(qName, type, QVariant(defaultValue));
    }
}

void PropertyGridNode::AddProperty(std::string_view name, gui::PropertyType type,
                                   bool defaultValue)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        auto qName = QString::fromUtf8(name.data(), static_cast<int>(name.size()));
        w->AddProperty(qName, type, QVariant(defaultValue));
    }
}

void PropertyGridNode::AddProperty(std::string_view name, gui::PropertyType type,
                                   std::string_view defaultValue,
                                   std::span<const std::string> choices)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        auto qName = QString::fromUtf8(name.data(), static_cast<int>(name.size()));
        QVariant defVal;
        if (!defaultValue.empty()) {
            defVal = QString::fromUtf8(defaultValue.data(), static_cast<int>(defaultValue.size()));
        }
        QStringList qChoices;
        qChoices.reserve(static_cast<int>(choices.size()));
        for (const auto& c : choices) {
            qChoices.append(QString::fromStdString(c));
        }
        w->AddProperty(qName, type, defVal, qChoices);
    }
}

void PropertyGridNode::SetPropertyValue(std::string_view name, std::string_view value)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        auto qName = QString::fromUtf8(name.data(), static_cast<int>(name.size()));
        auto qValue = QString::fromUtf8(value.data(), static_cast<int>(value.size()));
        w->SetValue(qName, QVariant(qValue));
    }
}

auto PropertyGridNode::PropertyValue(std::string_view name) const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        auto qName = QString::fromUtf8(name.data(), static_cast<int>(name.size()));
        return w->Value(qName).toString().toStdString();
    }
    return {};
}

void PropertyGridNode::AddGroup(std::string_view name)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        w->AddGroup(QString::fromUtf8(name.data(), static_cast<int>(name.size())));
    }
}

void PropertyGridNode::Clear()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPropertyGrid*>(_widget)) {
        w->Clear();
    }
}

auto PropertyGridNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanPropertyGrid(parent);
    QObject::connect(w, &gui::NyanPropertyGrid::PropertyChanged, w,
                     [this](const QString& name, const QVariant& value) {
                         PropertyChanged notif(name.toStdString(), value.toString().toStdString());
                         SendNotification(this, notif);
                     });
    return w;
}

} // namespace matcha::fw
