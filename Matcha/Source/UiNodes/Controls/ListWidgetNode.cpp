#include "Matcha/UiNodes/Controls/ListWidgetNode.h"
#include "Matcha/UiNodes/Controls/LabelNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanListWidget.h"

#include <QListWidgetItem>
#include <QString>

namespace matcha::fw {

namespace {

constexpr int kDefaultItemHeight = 28;

} // anonymous namespace

MATCHA_IMPLEMENT_CLASS(ListWidgetNode, WidgetNode)

ListWidgetNode::ListWidgetNode(std::string id)
    : WidgetNode(std::move(id), NodeType::ListWidget)
{
}

ListWidgetNode::~ListWidgetNode() = default;

// ============================================================================
// UiNode-based Item API
// ============================================================================

void ListWidgetNode::AddItemNode(std::unique_ptr<UiNode> node)
{
    if (!node) { return; }
    EnsureWidget();
    auto* w = qobject_cast<gui::NyanListWidget*>(_widget);
    if (w == nullptr) { return; }

    // Trigger lazy widget creation via public Widget()
    (void)node->Widget();

    _itemNodes.push_back(std::move(node));
    const int idx = static_cast<int>(_itemNodes.size()) - 1;
    SyncItemWidget(idx);
}

void ListWidgetNode::InsertItemNode(int index, std::unique_ptr<UiNode> node)
{
    if (!node) { return; }
    EnsureWidget();
    auto* w = qobject_cast<gui::NyanListWidget*>(_widget);
    if (w == nullptr) { return; }

    const int clampedIdx = std::clamp(index, 0, static_cast<int>(_itemNodes.size()));
    (void)node->Widget();
    _itemNodes.insert(_itemNodes.begin() + clampedIdx, std::move(node));

    // Insert a placeholder QListWidgetItem at the position
    auto* lwItem = new QListWidgetItem();
    lwItem->setSizeHint(QSize(0, kDefaultItemHeight));
    w->insertItem(clampedIdx, lwItem);

    // Embed the widget
    auto* itemWidget = _itemNodes[static_cast<size_t>(clampedIdx)]->Widget();
    if (itemWidget != nullptr) {
        itemWidget->setParent(w);
        w->setItemWidget(lwItem, itemWidget);
    }
}

void ListWidgetNode::AddItem(std::string_view text)
{
    auto label = std::make_unique<LabelNode>(
        std::string(Id()) + "-item-" + std::to_string(_itemNodes.size()));
    label->SetText(std::string(text));
    AddItemNode(std::move(label));
}

void ListWidgetNode::AddItems(std::span<const std::string> items)
{
    for (const auto& item : items) {
        AddItem(item);
    }
}

// ============================================================================
// Item access / mutation
// ============================================================================

auto ListWidgetNode::ItemNode(int index) const -> UiNode*
{
    if (index < 0 || index >= static_cast<int>(_itemNodes.size())) {
        return nullptr;
    }
    return _itemNodes[static_cast<size_t>(index)].get();
}

void ListWidgetNode::RemoveItem(int index)
{
    if (index < 0 || index >= static_cast<int>(_itemNodes.size())) { return; }
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanListWidget*>(_widget)) {
        // Remove QListWidgetItem (also removes the embedded widget)
        delete w->takeItem(index);
    }
    _itemNodes.erase(_itemNodes.begin() + index);
}

auto ListWidgetNode::ItemCount() const -> int
{
    return static_cast<int>(_itemNodes.size());
}

// ============================================================================
// Selection
// ============================================================================

void ListWidgetNode::SetCurrentIndex(int index)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanListWidget*>(_widget)) {
        w->setCurrentRow(index);
    }
}

auto ListWidgetNode::CurrentIndex() const -> int
{
    if (auto* w = qobject_cast<gui::NyanListWidget*>(_widget)) {
        return w->currentRow();
    }
    return -1;
}

void ListWidgetNode::Clear()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanListWidget*>(_widget)) {
        w->clear();
    }
    _itemNodes.clear();
}

// ============================================================================
// Widget creation & sync
// ============================================================================

void ListWidgetNode::SyncItemWidget(int index)
{
    auto* w = qobject_cast<gui::NyanListWidget*>(_widget);
    if (w == nullptr) { return; }

    auto* lwItem = new QListWidgetItem();
    auto* itemWidget = _itemNodes[static_cast<size_t>(index)]->Widget();
    if (itemWidget != nullptr) {
        // Size hint from the item widget's preferred size
        const QSize hint = itemWidget->sizeHint();
        lwItem->setSizeHint(QSize(0, std::max(hint.height(), kDefaultItemHeight)));
        itemWidget->setParent(w);
    } else {
        lwItem->setSizeHint(QSize(0, kDefaultItemHeight));
    }

    w->addItem(lwItem);
    if (itemWidget != nullptr) {
        w->setItemWidget(lwItem, itemWidget);
    }
}

auto ListWidgetNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanListWidget(parent);
    QObject::connect(w, &QListWidget::currentRowChanged, w, [this](int row) {
        IndexChanged notif(row);
        SendNotification(this, notif);
    });
    QObject::connect(w, &QListWidget::itemDoubleClicked, w, [this](QListWidgetItem* item) {
        if (auto* lw = qobject_cast<QListWidget*>(_widget)) {
            ItemDoubleClicked notif(lw->row(item));
            SendNotification(this, notif);
        }
    });
    return w;
}

} // namespace matcha::fw
