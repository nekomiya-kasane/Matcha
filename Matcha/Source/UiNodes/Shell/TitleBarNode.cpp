/**
 * @file TitleBarNode.cpp
 * @brief TitleBarNode base class implementation.
 */

#include "Matcha/UiNodes/Shell/TitleBarNode.h"

namespace matcha::fw {

TitleBarNode::TitleBarNode(std::string id)
    : UiNode(std::move(id), NodeType::TitleBar)
{
}

TitleBarNode::~TitleBarNode() = default;

} // namespace matcha::fw
