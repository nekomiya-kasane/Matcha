#include "Matcha/Tree/Controls/SearchBoxNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanSearchBox.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(SearchBoxNode, WidgetNode)

SearchBoxNode::SearchBoxNode(std::string id)
    : WidgetNode(std::move(id), NodeType::SearchBox)
{
}

SearchBoxNode::~SearchBoxNode() = default;

void SearchBoxNode::SetText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSearchBox*>(_widget)) {
        w->SetText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

void SearchBoxNode::Clear()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSearchBox*>(_widget)) {
        w->Clear();
    }
}

void SearchBoxNode::SetPlaceholder(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSearchBox*>(_widget)) {
        w->SetPlaceholder(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

void SearchBoxNode::SetSearchMode(gui::SearchMode mode)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanSearchBox*>(_widget)) {
        w->SetSearchMode(mode);
    }
}

auto SearchBoxNode::Text() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanSearchBox*>(_widget)) {
        return w->Text().toStdString();
    }
    return {};
}

auto SearchBoxNode::Placeholder() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanSearchBox*>(_widget)) {
        return w->Placeholder().toStdString();
    }
    return {};
}

auto SearchBoxNode::SearchMode() const -> gui::SearchMode
{
    if (auto* w = qobject_cast<gui::NyanSearchBox*>(_widget)) {
        return w->GetSearchMode();
    }
    return gui::SearchMode::Instant;
}

auto SearchBoxNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanSearchBox(parent);
    QObject::connect(w, &gui::NyanSearchBox::SearchChanged, w, [this](const QString& text) {
        TextChanged notif(text.toStdString());
        SendNotification(this, notif);
    });
    QObject::connect(w, &gui::NyanSearchBox::SearchSubmitted, w, [this](const QString& text) {
        SearchSubmitted notif(text.toStdString());
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
