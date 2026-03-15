#include "Matcha/Tree/Controls/PlainTextEditNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include <QFont>
#include <QPlainTextEdit>
#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(PlainTextEditNode, WidgetNode)

PlainTextEditNode::PlainTextEditNode(std::string id)
    : WidgetNode(std::move(id), NodeType::PlainTextEdit)
{
}

PlainTextEditNode::~PlainTextEditNode() = default;

void PlainTextEditNode::SetPlainText(std::string_view text)
{
    EnsureWidget();
    if (_edit) {
        _edit->setPlainText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto PlainTextEditNode::PlainText() const -> std::string
{
    if (_edit) {
        return _edit->toPlainText().toStdString();
    }
    return {};
}

void PlainTextEditNode::AppendPlainText(std::string_view text)
{
    EnsureWidget();
    if (_edit) {
        _edit->appendPlainText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

void PlainTextEditNode::Clear()
{
    if (_edit) {
        _edit->clear();
    }
}

void PlainTextEditNode::SetReadOnly(bool readOnly)
{
    EnsureWidget();
    if (_edit) {
        _edit->setReadOnly(readOnly);
    }
}

auto PlainTextEditNode::IsReadOnly() const -> bool
{
    if (_edit) {
        return _edit->isReadOnly();
    }
    return false;
}

void PlainTextEditNode::SetMaximumBlockCount(int count)
{
    EnsureWidget();
    if (_edit) {
        _edit->setMaximumBlockCount(count);
    }
}

void PlainTextEditNode::SetFont(std::string_view family, int pointSize)
{
    EnsureWidget();
    if (_edit) {
        _edit->setFont(QFont(QString::fromUtf8(family.data(), static_cast<int>(family.size())), pointSize));
    }
}

void PlainTextEditNode::SetStyleSheet(std::string_view css)
{
    EnsureWidget();
    if (_edit) {
        _edit->setStyleSheet(QString::fromUtf8(css.data(), static_cast<int>(css.size())));
    }
}

auto PlainTextEditNode::CreateWidget(QWidget* parent) -> QWidget*
{
    _edit = new QPlainTextEdit(parent);
    QObject::connect(_edit, &QPlainTextEdit::textChanged, _edit, [this]() {
        if (_edit && !_edit->isReadOnly()) {
            TextChanged notif(_edit->toPlainText().toStdString());
            SendNotification(this, notif);
        }
    });
    return _edit;
}

} // namespace matcha::fw
