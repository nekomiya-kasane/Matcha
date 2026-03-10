#include "Matcha/UiNodes/Core/WidgetWrapper.h"

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(WidgetWrapper, UiNode)

WidgetWrapper::WidgetWrapper(std::string id, QWidget* widget)
    : UiNode(std::move(id), NodeType::WidgetWrapper)
    , _widget(widget)
{
}

WidgetWrapper::~WidgetWrapper() = default;

} // namespace matcha::fw
