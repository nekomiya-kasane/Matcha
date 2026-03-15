#include "Matcha/Tree/Controls/PushButtonNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanPushButton.h"
#include "Matcha/Theming/IThemeService.h"

#include <QIcon>
#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(PushButtonNode, WidgetNode)

PushButtonNode::PushButtonNode(std::string id)
    : WidgetNode(std::move(id), NodeType::PushButton)
{
}

PushButtonNode::~PushButtonNode() = default;

void PushButtonNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPushButton*>(_widget)) {
        w->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto PushButtonNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanPushButton*>(_widget)) {
        return w->text().toStdString();
    }
    return {};
}

void PushButtonNode::SetVariant(gui::ButtonVariant variant)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPushButton*>(_widget)) {
        w->SetVariant(variant);
    }
}

auto PushButtonNode::Variant() const -> gui::ButtonVariant
{
    if (auto* w = qobject_cast<gui::NyanPushButton*>(_widget)) {
        return w->Variant();
    }
    return gui::ButtonVariant::Secondary;
}

auto PushButtonNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanPushButton(parent);
    QObject::connect(w, &QPushButton::clicked, w, [this]() {
        Activated notif;
        SendNotification(this, notif);
    });
    QObject::connect(w, &QPushButton::pressed, w, [this]() {
        Pressed notif;
        SendNotification(this, notif);
    });
    QObject::connect(w, &QPushButton::released, w, [this]() {
        Released notif;
        SendNotification(this, notif);
    });
    return w;
}

void PushButtonNode::OnIconChanged()
{
    auto* w = qobject_cast<gui::NyanPushButton*>(_widget);
    if (w == nullptr || !gui::HasThemeService()) {
        return;
    }
    if (_iconId.empty()) {
        w->setIcon(QIcon());
        return;
    }
    const int sizePx = static_cast<int>(_iconSize);
    const QColor fg = gui::GetThemeService().Color(gui::ColorToken::TextPrimary);
    const QPixmap pm = gui::GetThemeService().ResolveIcon(_iconId, _iconSize, fg);
    if (!pm.isNull()) {
        w->setIcon(QIcon(pm));
        w->setIconSize(QSize(sizePx, sizePx));
    }
}

} // namespace matcha::fw
