/**
 * @file LogoButtonNode.cpp
 * @brief LogoButtonNode implementation -- wraps NyanLogoButton.
 */

#include "Matcha/Tree/Composition/Shell/LogoButtonNode.h"

#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Widgets/Shell/NyanLogoButton.h"

#include <QColor>
#include <QPixmap>
#include <QWidget>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(LogoButtonNode, UiNode)

LogoButtonNode::LogoButtonNode(std::string id, UiNode* parentHint)
    : UiNode(std::move(id), NodeType::LogoButton)
    , _logoButton(new gui::NyanLogoButton(parentHint ? parentHint->Widget() : nullptr))
{
    QObject::connect(_logoButton, &gui::NyanLogoButton::Clicked,
        [this]() {
            Activated notif;
            SendNotification(this, notif);
        });
}

LogoButtonNode::~LogoButtonNode() = default;

auto LogoButtonNode::Widget() -> QWidget*
{
    return _logoButton;
}

auto LogoButtonNode::LogoButton() -> gui::NyanLogoButton*
{
    return _logoButton;
}

void LogoButtonNode::SetLogoFromData(std::span<const uint8_t> pngData)
{
    QPixmap pm;
    pm.loadFromData(pngData.data(), static_cast<uint>(pngData.size()));
    _logoButton->SetLogo(pm);
}

void LogoButtonNode::SetSplitY(int row1Height)
{
    _logoButton->SetSplitY(row1Height);
}

void LogoButtonNode::SetColors(uint32_t topArgb, uint32_t bottomArgb)
{
    _logoButton->SetTopColor(QColor::fromRgba(topArgb));
    _logoButton->SetBottomColor(QColor::fromRgba(bottomArgb));
}

} // namespace matcha::fw
