/**
 * @file ControlBar.cpp
 * @brief Implementation of ControlBar UiNode.
 */

#include <Matcha/Tree/Composition/Shell/ControlBar.h>

namespace matcha::fw {

ControlBar::ControlBar(std::string name)
    : UiNode("control-bar", NodeType::ControlBar, std::move(name))
{
}

ControlBar::~ControlBar() = default;

void ControlBar::Clear()
{
    while (NodeCount() > 0) {
        RemoveNode(NodeAt(0));
    }
}

} // namespace matcha::fw
