#include "Matcha/UiNodes/Controls/TreeItemNode.h"

#include <algorithm>

namespace matcha::fw {

TreeItemNode::TreeItemNode(std::string text)
    : _text(std::move(text))
{
}

TreeItemNode::~TreeItemNode() = default;

void TreeItemNode::SetText(std::string_view text)
{
    _text.assign(text.data(), text.size());
}

auto TreeItemNode::Text() const -> const std::string&
{
    return _text;
}

void TreeItemNode::SetIconPath(std::string_view path)
{
    _iconPath.assign(path.data(), path.size());
}

auto TreeItemNode::IconPath() const -> const std::string&
{
    return _iconPath;
}

auto TreeItemNode::AddChild(std::unique_ptr<TreeItemNode> child) -> TreeItemNode*
{
    if (!child) { return nullptr; }
    child->_parent = this;
    _children.push_back(std::move(child));
    return _children.back().get();
}

auto TreeItemNode::InsertChild(int index, std::unique_ptr<TreeItemNode> child) -> TreeItemNode*
{
    if (!child) { return nullptr; }
    const int clampedIdx = std::clamp(index, 0, static_cast<int>(_children.size()));
    child->_parent = this;
    auto it = _children.insert(_children.begin() + clampedIdx, std::move(child));
    return it->get();
}

void TreeItemNode::RemoveChild(int index)
{
    if (index < 0 || index >= static_cast<int>(_children.size())) { return; }
    _children[static_cast<size_t>(index)]->_parent = nullptr;
    _children.erase(_children.begin() + index);
}

auto TreeItemNode::ChildCount() const -> int
{
    return static_cast<int>(_children.size());
}

auto TreeItemNode::Child(int index) const -> TreeItemNode*
{
    if (index < 0 || index >= static_cast<int>(_children.size())) {
        return nullptr;
    }
    return _children[static_cast<size_t>(index)].get();
}

auto TreeItemNode::Parent() const -> TreeItemNode*
{
    return _parent;
}

auto TreeItemNode::IndexInParent() const -> int
{
    if (_parent == nullptr) { return -1; }
    const auto& siblings = _parent->_children;
    for (int i = 0; i < static_cast<int>(siblings.size()); ++i) {
        if (siblings[static_cast<size_t>(i)].get() == this) {
            return i;
        }
    }
    return -1;
}

} // namespace matcha::fw
