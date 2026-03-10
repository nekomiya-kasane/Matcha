#include "Matcha/UiNodes/Controls/ComboBoxNode.h"
#include "Matcha/UiNodes/Controls/LabelNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanComboBox.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QString>

namespace matcha::fw {

namespace {

constexpr int kDefaultItemHeight = 28;

} // anonymous namespace

MATCHA_IMPLEMENT_CLASS(ComboBoxNode, WidgetNode)

ComboBoxNode::ComboBoxNode(std::string id)
    : WidgetNode(std::move(id), NodeType::ComboBox)
{
}

ComboBoxNode::~ComboBoxNode() = default;

// ============================================================================
// UiNode-based Item API
// ============================================================================

void ComboBoxNode::AddItemNode(std::unique_ptr<UiNode> node)
{
    if (!node) { return; }
    EnsureWidget();
    auto* w = qobject_cast<gui::NyanComboBox*>(_widget);
    if (w == nullptr) { return; }

    // Trigger lazy widget creation via public Widget()
    (void)node->Widget();

    _itemNodes.push_back(std::move(node));

    // Add a placeholder text item to QComboBox model so count() stays in sync.
    // The actual visual is embedded in the popup QListWidget.
    w->addItem(QString());

    const int idx = static_cast<int>(_itemNodes.size()) - 1;
    SyncItemWidget(idx);
}

void ComboBoxNode::InsertItemNode(int index, std::unique_ptr<UiNode> node)
{
    if (!node) { return; }
    EnsureWidget();
    auto* w = qobject_cast<gui::NyanComboBox*>(_widget);
    if (w == nullptr) { return; }

    const int clampedIdx = std::clamp(index, 0, static_cast<int>(_itemNodes.size()));
    (void)node->Widget();
    _itemNodes.insert(_itemNodes.begin() + clampedIdx, std::move(node));

    w->insertItem(clampedIdx, QString());

    // Re-sync the inserted item into popup QListWidget
    auto* popupList = qobject_cast<QListWidget*>(w->view());
    if (popupList != nullptr) {
        auto* lwItem = popupList->item(clampedIdx);
        if (lwItem != nullptr) {
            auto* itemWidget = _itemNodes[static_cast<size_t>(clampedIdx)]->Widget();
            if (itemWidget != nullptr) {
                const QSize hint = itemWidget->sizeHint();
                lwItem->setSizeHint(QSize(0, std::max(hint.height(), kDefaultItemHeight)));
                itemWidget->setParent(popupList);
                popupList->setItemWidget(lwItem, itemWidget);
            }
        }
    }
}

void ComboBoxNode::AddItem(std::string_view text)
{
    auto label = std::make_unique<LabelNode>(
        std::string(Id()) + "-item-" + std::to_string(_itemNodes.size()));
    label->SetText(std::string(text));
    AddItemNode(std::move(label));
}

void ComboBoxNode::AddItems(std::span<const std::string> items)
{
    for (const auto& item : items) {
        AddItem(item);
    }
}

// ============================================================================
// Item access / mutation
// ============================================================================

auto ComboBoxNode::ItemNode(int index) const -> UiNode*
{
    if (index < 0 || index >= static_cast<int>(_itemNodes.size())) {
        return nullptr;
    }
    return _itemNodes[static_cast<size_t>(index)].get();
}

void ComboBoxNode::RemoveItem(int index)
{
    if (index < 0 || index >= static_cast<int>(_itemNodes.size())) { return; }
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        w->removeItem(index);
    }
    _itemNodes.erase(_itemNodes.begin() + index);
}

auto ComboBoxNode::ItemCount() const -> int
{
    return static_cast<int>(_itemNodes.size());
}

// ============================================================================
// Selection & Config
// ============================================================================

void ComboBoxNode::SetCurrentIndex(int index)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        w->setCurrentIndex(index);
    }
}

auto ComboBoxNode::CurrentIndex() const -> int
{
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        return w->currentIndex();
    }
    return -1;
}

auto ComboBoxNode::CurrentText() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        return w->currentText().toStdString();
    }
    return {};
}

void ComboBoxNode::SetEditable(bool editable)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        w->SetEditableMode(editable);
    }
}

auto ComboBoxNode::IsEditable() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        return w->IsEditableMode();
    }
    return false;
}

void ComboBoxNode::SetPlaceholder(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        w->SetPlaceholder(QString::fromUtf8(text.data(),
                                            static_cast<int>(text.size())));
    }
}

void ComboBoxNode::Clear()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanComboBox*>(_widget)) {
        w->clear();
    }
    _itemNodes.clear();
}

// ============================================================================
// Widget creation & sync
// ============================================================================

void ComboBoxNode::SyncItemWidget(int index)
{
    auto* w = qobject_cast<gui::NyanComboBox*>(_widget);
    if (w == nullptr) { return; }

    // Ensure the popup view is a QListWidget (replace default QListView once)
    auto* popupList = qobject_cast<QListWidget*>(w->view());
    if (popupList == nullptr) {
        popupList = new QListWidget(w);
        w->setModel(popupList->model());
        w->setView(popupList);
    }

    auto* lwItem = popupList->item(index);
    if (lwItem == nullptr) { return; }

    auto* itemWidget = _itemNodes[static_cast<size_t>(index)]->Widget();
    if (itemWidget != nullptr) {
        const QSize hint = itemWidget->sizeHint();
        lwItem->setSizeHint(QSize(0, std::max(hint.height(), kDefaultItemHeight)));
        itemWidget->setParent(popupList);
        popupList->setItemWidget(lwItem, itemWidget);

        // Set display text for the closed combo state (extracted from LabelNode if possible)
        if (auto* labelNode = dynamic_cast<LabelNode*>(_itemNodes[static_cast<size_t>(index)].get())) {
            w->setItemText(index, QString::fromStdString(labelNode->Text()));
        }
    } else {
        lwItem->setSizeHint(QSize(0, kDefaultItemHeight));
    }
}

auto ComboBoxNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanComboBox(parent);

    // Replace default popup view with QListWidget for setItemWidget() support
    auto* popupList = new QListWidget(w);
    w->setModel(popupList->model());
    w->setView(popupList);

    QObject::connect(w, &QComboBox::currentIndexChanged, w, [this](int index) {
        IndexChanged notif(index);
        SendNotification(this, notif);
    });
    QObject::connect(w, &QComboBox::textActivated, w, [this](const QString& text) {
        TextActivated notif(text.toStdString());
        SendNotification(this, notif);
    });
    return w;
}

} // namespace matcha::fw
