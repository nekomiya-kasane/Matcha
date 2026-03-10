#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Core/UiNode.h"

namespace matcha::fw {

ContextMenuRequest::ContextMenuRequest(int globalX, int globalY)
    : _globalX(globalX), _globalY(globalY)
{
}

ContextMenuRequest::~ContextMenuRequest() = default;

ContextMenuRequest::ContextMenuRequest(ContextMenuRequest&&) noexcept = default;
auto ContextMenuRequest::operator=(ContextMenuRequest&&) noexcept -> ContextMenuRequest& = default;

void ContextMenuRequest::AddNode(std::unique_ptr<UiNode> node)
{
    if (node) {
        _nodes.push_back(std::move(node));
    }
}

auto ContextMenuRequest::NodeCount() const -> size_t
{
    return _nodes.size();
}

auto ContextMenuRequest::TakeNodes() -> std::vector<std::unique_ptr<UiNode>>
{
    return std::move(_nodes);
}

} // namespace matcha::fw
