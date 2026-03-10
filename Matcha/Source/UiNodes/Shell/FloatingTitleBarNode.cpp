/**
 * @file FloatingTitleBarNode.cpp
 * @brief FloatingTitleBarNode implementation -- wraps NyanFloatingTitleBar.
 */

#include "Matcha/UiNodes/Shell/FloatingTitleBarNode.h"
#include "Matcha/Widgets/Shell/NyanFloatingTitleBar.h"

#include <QString>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(FloatingTitleBarNode, TitleBarNode)

FloatingTitleBarNode::FloatingTitleBarNode(std::string id, UiNode* parentHint)
    : TitleBarNode(std::move(id))
    , _titleBar(new gui::NyanFloatingTitleBar(parentHint ? parentHint->Widget() : nullptr))
{
}

FloatingTitleBarNode::~FloatingTitleBarNode() = default;

void FloatingTitleBarNode::SetTitle(std::string_view title)
{
    _titleBar->SetTitle(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
}

auto FloatingTitleBarNode::Title() const -> std::string
{
    return _titleBar->Title().toStdString();
}

auto FloatingTitleBarNode::Widget() -> QWidget*
{
    return _titleBar;
}

auto FloatingTitleBarNode::FloatingTitleBar() -> gui::NyanFloatingTitleBar*
{
    return _titleBar;
}

} // namespace matcha::fw
