#include "Matcha/Tree/Controls/TreeWidgetNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanStructureTree.h"

#include <QItemSelectionModel>
#include <QStandardItemModel>

#include <algorithm>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(TreeWidgetNode, WidgetNode)

// ============================================================================
// Helpers — recursive TreeItemNode -> QStandardItem conversion
// ============================================================================

namespace {

auto BuildStandardItem(const TreeItemNode* node) -> QStandardItem*
{
    auto* item = new QStandardItem(QString::fromStdString(node->Text()));
    if (!node->IconPath().empty()) {
        item->setIcon(QIcon(QString::fromStdString(node->IconPath())));
    }
    for (int i = 0; i < node->ChildCount(); ++i) {
        item->appendRow(BuildStandardItem(node->Child(i)));
    }
    return item;
}

} // anonymous namespace

// ============================================================================
// Construction / destruction
// ============================================================================

TreeWidgetNode::TreeWidgetNode(std::string id)
    : WidgetNode(std::move(id), NodeType::TreeWidget)
{
}

TreeWidgetNode::~TreeWidgetNode() = default;

// ============================================================================
// Tree data API
// ============================================================================

auto TreeWidgetNode::AddRootItem(std::unique_ptr<TreeItemNode> item) -> TreeItemNode*
{
    if (!item) { return nullptr; }
    auto* raw = item.get();
    _rootItems.push_back(std::move(item));
    RebuildModel();
    return raw;
}

void TreeWidgetNode::RemoveRootItem(int index)
{
    if (index < 0 || index >= static_cast<int>(_rootItems.size())) { return; }
    _rootItems.erase(_rootItems.begin() + index);
    RebuildModel();
}

auto TreeWidgetNode::RootItem(int index) const -> TreeItemNode*
{
    if (index < 0 || index >= static_cast<int>(_rootItems.size())) {
        return nullptr;
    }
    return _rootItems[static_cast<size_t>(index)].get();
}

auto TreeWidgetNode::RootItemCount() const -> int
{
    return static_cast<int>(_rootItems.size());
}

void TreeWidgetNode::Clear()
{
    _rootItems.clear();
    RebuildModel();
}

void TreeWidgetNode::SetHeaderLabel(std::string_view label)
{
    _headerLabel.assign(label.data(), label.size());
    RebuildModel();
}

void TreeWidgetNode::SyncModel()
{
    RebuildModel();
}

// ============================================================================
// Selection
// ============================================================================

auto TreeWidgetNode::SelectedPath() const -> std::vector<int>
{
    auto* w = qobject_cast<gui::NyanStructureTree*>(_widget);
    if (w == nullptr) { return {}; }

    auto* selModel = w->TreeView()->selectionModel();
    if (selModel == nullptr || !selModel->hasSelection()) { return {}; }

    // Walk from selected index up to root, collecting row indices.
    std::vector<int> path;
    QModelIndex idx = selModel->currentIndex();
    while (idx.isValid()) {
        path.push_back(idx.row());
        idx = idx.parent();
    }
    std::ranges::reverse(path);
    return path;
}

// ============================================================================
// Display
// ============================================================================

void TreeWidgetNode::SetTitle(std::string_view title)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanStructureTree*>(_widget)) {
        w->SetTitle(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
    }
}

auto TreeWidgetNode::Title() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanStructureTree*>(_widget)) {
        return w->Title().toStdString();
    }
    return {};
}

void TreeWidgetNode::ExpandAll()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanStructureTree*>(_widget)) {
        w->TreeView()->expandAll();
    }
}

void TreeWidgetNode::CollapseAll()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanStructureTree*>(_widget)) {
        w->TreeView()->collapseAll();
    }
}

// ============================================================================
// Widget creation
// ============================================================================

auto TreeWidgetNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanStructureTree(parent);
    QObject::connect(w, &gui::NyanStructureTree::SelectionChanged, w,
                     [this](const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
                         SelectionChanged notif;
                         SendNotification(this, notif);
                     });
    QObject::connect(w, &gui::NyanStructureTree::ItemDoubleClicked, w,
                     [this](const QModelIndex& index) {
                         ItemDoubleClicked notif(index.row());
                         SendNotification(this, notif);
                     });
    QObject::connect(w, &gui::NyanStructureTree::CollapsedChanged, w, [this](bool collapsed) {
        CollapsedChanged notif(collapsed);
        SendNotification(this, notif);
    });
    QObject::connect(w, &gui::NyanStructureTree::ContextMenuRequested, w,
                     [this](const QPoint& globalPos, const QModelIndex& /*index*/) {
                         ContextMenuRequest notif(globalPos.x(), globalPos.y());
                         SendNotification(this, notif);
                     });

    // If items were added before widget creation, build model now.
    if (!_rootItems.empty()) {
        RebuildModel();
    }

    return w;
}

// ============================================================================
// Internal model sync
// ============================================================================

void TreeWidgetNode::RebuildModel()
{
    EnsureWidget();
    auto* w = qobject_cast<gui::NyanStructureTree*>(_widget);
    if (w == nullptr) { return; }

    // Get existing model or create a new one.
    auto* existingModel = qobject_cast<QStandardItemModel*>(w->Model());
    QStandardItemModel* model = existingModel;
    if (model == nullptr) {
        model = new QStandardItemModel(w);
    } else {
        model->clear();
    }

    if (!_headerLabel.empty()) {
        model->setHorizontalHeaderLabels({QString::fromStdString(_headerLabel)});
    }

    for (const auto& root : _rootItems) {
        model->appendRow(BuildStandardItem(root.get()));
    }

    if (model != existingModel) {
        w->SetModel(model);
    }
}

} // namespace matcha::fw
