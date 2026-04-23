#include "Matcha/Tree/UiNode.h"

#include <QWidget>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(UiNode, CommandNode)

UiNode::UiNode(std::string id, NodeType type, std::string name)
    : CommandNode(nullptr, std::move(id))
    , _name(name.empty() ? std::string(Id()) : std::move(name))
    , _type(type)
{
}

UiNode::~UiNode() = default;

auto UiNode::ParentNode() const -> UiNode*
{
    return static_cast<UiNode*>(Parent());
}

auto UiNode::AddNode(std::unique_ptr<UiNode> child) -> UiNode*
{
    if (!child) {
        return nullptr;
    }
    auto* raw = child.get();
    CommandNode::AddChild(std::move(child));
    return raw;
}

auto UiNode::RemoveNode(UiNode* child) -> std::unique_ptr<UiNode>
{
    auto owned = CommandNode::RemoveChild(child);
    // Safe downcast: in UiNode tree, all children are UiNodes
    return std::unique_ptr<UiNode>(static_cast<UiNode*>(owned.release()));
}

auto UiNode::NodeAt(size_t index) const -> UiNode*
{
    return static_cast<UiNode*>(Children()[index].get());
}

auto UiNode::FindById(std::string_view id) const -> UiNode*
{
    for (const auto& child : Children()) {
        auto* uiChild = static_cast<UiNode*>(child.get());
        if (uiChild->Id() == id) {
            return uiChild;
        }
        if (auto* found = uiChild->FindById(id)) {
            return found;
        }
    }
    return nullptr;
}

auto UiNode::FindByName(std::string_view name) const -> UiNode*
{
    for (const auto& child : Children()) {
        auto* uiChild = static_cast<UiNode*>(child.get());
        if (uiChild->_name == name) {
            return uiChild;
        }
        if (auto* found = uiChild->FindByName(name)) {
            return found;
        }
    }
    return nullptr;
}

auto UiNode::Descendants() -> std::generator<UiNode*>
{
    for (const auto& child : Children()) {
        auto* uiChild = static_cast<UiNode*>(child.get());
        co_yield uiChild;
        for (auto* desc : uiChild->Descendants()) {
            co_yield desc;
        }
    }
}

auto UiNode::DescendantsOfType(NodeType type) -> std::generator<UiNode*>
{
    for (auto* node : Descendants()) {
        if (node->_type == type) {
            co_yield node;
        }
    }
}

auto UiNode::ChildrenOfType(NodeType type) -> std::generator<UiNode*>
{
    for (const auto& child : Children()) {
        auto* uiChild = static_cast<UiNode*>(child.get());
        if (uiChild->_type == type) {
            co_yield uiChild;
        }
    }
}

void UiNode::TraverseDepthFirst(const std::function<void(UiNode*)>& visitor)
{
    visitor(this);
    for (const auto& child : Children()) {
        static_cast<UiNode*>(child.get())->TraverseDepthFirst(visitor);
    }
}

void UiNode::ForEachNode(std::function<void(UiNode*)> fn)
{
    TraverseDepthFirst(fn);
}

auto UiNode::ResolveParentWidget() -> QWidget*
{
    for (auto* p = ParentNode(); p != nullptr; p = p->ParentNode()) {
        if (auto* w = p->Widget()) {
            return w;
        }
    }
    return nullptr;
}

void UiNode::ReparentTo(UiNode* newParent)
{
    auto* w = Widget();
    if (w == nullptr) { return; }

    QWidget* target = newParent ? newParent->Widget() : nullptr;
    w->setParent(target);
}

auto UiNode::FindEnclosingFocusScope() -> UiNode*
{
    UiNode* node = this;
    while (node != nullptr) {
        if (node->IsFocusScope()) {
            return node;
        }
        node = node->ParentNode();
    }
    return nullptr;
}

} // namespace matcha::fw