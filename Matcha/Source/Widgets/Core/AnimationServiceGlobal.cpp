#include "Matcha/Widgets/Core/IAnimationService.h"

#include <cassert>

namespace matcha::gui {

namespace {
IAnimationService* g_animationService = nullptr;
} // anonymous namespace

void SetAnimationService(IAnimationService* svc)
{
    g_animationService = svc;
}

auto GetAnimationService() -> IAnimationService*
{
    return g_animationService;
}

auto HasAnimationService() -> bool
{
    return g_animationService != nullptr;
}

} // namespace matcha::gui
