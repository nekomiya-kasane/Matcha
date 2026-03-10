#include "Matcha/UiNodes/Controls/LineEditNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanLineEdit.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(LineEditNode, WidgetNode)

LineEditNode::LineEditNode(std::string id)
    : WidgetNode(std::move(id), NodeType::LineEdit)
{
}

LineEditNode::~LineEditNode() = default;

void LineEditNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        w->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto LineEditNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        return w->text().toStdString();
    }
    return {};
}

void LineEditNode::SetPlaceholder(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        w->setPlaceholderText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto LineEditNode::Placeholder() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        return w->placeholderText().toStdString();
    }
    return {};
}

void LineEditNode::SetReadOnly(bool readOnly)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        w->setReadOnly(readOnly);
    }
}

auto LineEditNode::IsReadOnly() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        return w->isReadOnly();
    }
    return false;
}

void LineEditNode::SetMaxLength(int length)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        w->setMaxLength(length);
    }
}

auto LineEditNode::MaxLength() const -> int
{
    if (auto* w = qobject_cast<gui::NyanLineEdit*>(_widget)) {
        return w->maxLength();
    }
    return 32767; // Qt default
}

auto LineEditNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanLineEdit(parent);
    QObject::connect(w, &QLineEdit::textChanged, w, [this](const QString& text) {
        TextChanged notif(text.toStdString());
        SendNotification(this, notif);
    });
    QObject::connect(w, &QLineEdit::editingFinished, w, [this]() {
        EditingFinished notif;
        SendNotification(this, notif);
    });
    QObject::connect(w, &QLineEdit::returnPressed, w, [this]() {
        ReturnPressed notif;
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
