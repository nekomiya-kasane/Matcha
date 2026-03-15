#include "Matcha/Tree/Controls/PaginatorNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanPaginator.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(PaginatorNode, WidgetNode)

PaginatorNode::PaginatorNode(std::string id)
    : WidgetNode(std::move(id), NodeType::Paginator)
{
}

PaginatorNode::~PaginatorNode() = default;

void PaginatorNode::SetCount(int count)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPaginator*>(_widget)) {
        w->SetCount(count);
    }
}

auto PaginatorNode::Count() const -> int
{
    if (auto* w = qobject_cast<gui::NyanPaginator*>(_widget)) {
        return w->Count();
    }
    return 0;
}

void PaginatorNode::SetCurrent(int page)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPaginator*>(_widget)) {
        w->SetCurrent(page);
    }
}

auto PaginatorNode::Current() const -> int
{
    if (auto* w = qobject_cast<gui::NyanPaginator*>(_widget)) {
        return w->Current();
    }
    return -1;
}

void PaginatorNode::SetResetButtonVisible(bool visible)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanPaginator*>(_widget)) {
        w->SetResetButtonVisible(visible);
    }
}

auto PaginatorNode::IsResetButtonVisible() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanPaginator*>(_widget)) {
        return w->IsResetButtonVisible();
    }
    return false;
}

auto PaginatorNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanPaginator(parent);
    QObject::connect(w, &gui::NyanPaginator::PageChanged, w,
        [this](int page) {
            PageChanged notif(page);
            SendNotification(this, notif);
        });
    QObject::connect(w, &gui::NyanPaginator::ResetClicked, w,
        [this]() {
            ResetClicked notif;
            SendNotification(this, notif);
        });
    return w;
}

} // namespace matcha::fw
