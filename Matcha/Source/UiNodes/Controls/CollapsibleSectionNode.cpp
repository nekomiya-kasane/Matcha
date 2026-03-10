#include "Matcha/UiNodes/Controls/CollapsibleSectionNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanCollapsibleSection.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(CollapsibleSectionNode, WidgetNode)

CollapsibleSectionNode::CollapsibleSectionNode(std::string id)
    : WidgetNode(std::move(id), NodeType::CollapsibleSection)
{
}

CollapsibleSectionNode::~CollapsibleSectionNode() = default;

void CollapsibleSectionNode::SetTitle(std::string_view title)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanCollapsibleSection*>(_widget)) {
        w->SetTitle(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
    }
}

auto CollapsibleSectionNode::Title() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanCollapsibleSection*>(_widget)) {
        return w->Title().toStdString();
    }
    return {};
}

void CollapsibleSectionNode::SetExpanded(bool expanded)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanCollapsibleSection*>(_widget)) {
        w->SetExpanded(expanded);
    }
}

auto CollapsibleSectionNode::IsExpanded() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanCollapsibleSection*>(_widget)) {
        return w->IsExpanded();
    }
    return true;
}

void CollapsibleSectionNode::SetContentNode(UiNode* contentNode)
{
    EnsureWidget();
    if (contentNode == nullptr) { return; }
    if (auto* w = qobject_cast<gui::NyanCollapsibleSection*>(_widget)) {
        w->SetContent(contentNode->Widget());
    }
}

auto CollapsibleSectionNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanCollapsibleSection(parent);
    QObject::connect(w, &gui::NyanCollapsibleSection::ExpandToggled, w,
        [this](bool expanded) {
            ExpandToggled notif(expanded);
            SendNotification(this, notif);
        });
    return w;
}

} // namespace matcha::fw
