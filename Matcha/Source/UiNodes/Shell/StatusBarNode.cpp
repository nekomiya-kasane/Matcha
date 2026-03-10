#include "Matcha/UiNodes/Shell/StatusBarNode.h"

#include "Matcha/UiNodes/Shell/StatusItemNode.h"
#include "Matcha/Widgets/Controls/NyanLabel.h"
#include "Matcha/Widgets/Controls/NyanProgressBar.h"
#include "Matcha/Widgets/Shell/NyanStatusBar.h"

#include <QString>
#include <QWidget>

#include <memory>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(StatusBarNode, UiNode)

StatusBarNode::StatusBarNode(UiNode* parentHint)
    : UiNode("statusbar", NodeType::StatusBar)
    , _statusBar(new gui::NyanStatusBar(parentHint ? parentHint->Widget() : nullptr))
{
}

StatusBarNode::~StatusBarNode() = default;

auto StatusBarNode::StatusBar() -> gui::NyanStatusBar*
{
    return _statusBar;
}

auto StatusBarNode::Widget() -> QWidget*
{
    return _statusBar;
}

// ============================================================================
// Item management
// ============================================================================

auto StatusBarNode::AddLabel(std::string_view id, std::string_view text,
                             gui::StatusBarSide side) -> StatusItemNode*
{
    auto qid = QString::fromUtf8(id.data(), static_cast<int>(id.size()));

    // Create NyanLabel widget
    auto* label = new gui::NyanLabel(_statusBar);
    label->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Insert into NyanStatusBar widget
    if (_statusBar->AddItem(qid, label, side) == nullptr) {
        delete label;
        return nullptr;
    }

    // Create child node
    auto node = std::make_unique<StatusItemNode>(std::string(id),
                                                  StatusItemKind::Label, label);
    node->SetText(text);
    auto* raw = static_cast<StatusItemNode*>(AddNode(std::move(node)));
    return raw;
}

auto StatusBarNode::AddProgress(std::string_view id,
                                gui::StatusBarSide side) -> StatusItemNode*
{
    auto qid = QString::fromUtf8(id.data(), static_cast<int>(id.size()));

    auto* bar = new gui::NyanProgressBar(_statusBar);
    bar->setFixedWidth(120);
    bar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (_statusBar->AddItem(qid, bar, side) == nullptr) {
        delete bar;
        return nullptr;
    }

    auto node = std::make_unique<StatusItemNode>(std::string(id),
                                                  StatusItemKind::Progress, bar);
    auto* raw = static_cast<StatusItemNode*>(AddNode(std::move(node)));
    return raw;
}

auto StatusBarNode::AddWidget(std::string_view id, QWidget* widget,
                              gui::StatusBarSide side) -> StatusItemNode*
{
    if (widget == nullptr) { return nullptr; }

    auto qid = QString::fromUtf8(id.data(), static_cast<int>(id.size()));

    if (_statusBar->AddItem(qid, widget, side) == nullptr) {
        return nullptr;
    }

    auto node = std::make_unique<StatusItemNode>(std::string(id),
                                                  StatusItemKind::Custom, widget);
    auto* raw = static_cast<StatusItemNode*>(AddNode(std::move(node)));
    return raw;
}

auto StatusBarNode::RemoveItem(std::string_view id) -> bool
{
    auto* item = FindItem(id);
    if (item == nullptr) { return false; }

    auto qid = QString::fromUtf8(id.data(), static_cast<int>(id.size()));
    _statusBar->RemoveItem(qid);

    auto removed = RemoveNode(item);
    return removed != nullptr;
}

auto StatusBarNode::FindItem(std::string_view id) -> StatusItemNode*
{
    for (size_t i = 0; i < NodeCount(); ++i) {
        auto* child = NodeAt(i);
        if (child->Type() == NodeType::StatusBarItem && child->Id() == id) {
            return static_cast<StatusItemNode*>(child);
        }
    }
    return nullptr;
}

auto StatusBarNode::ItemCount() const -> int
{
    return _statusBar->ItemCount();
}

} // namespace matcha::fw
