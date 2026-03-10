/**
 * @file ThemeServiceGlobal.cpp
 * @brief Global IThemeService accessor implementation.
 */

#include "Matcha/Widgets/Core/IThemeService.h"

#include <cassert>

namespace matcha::gui {

namespace {
IThemeService* g_themeService = nullptr;
} // anonymous namespace

void SetThemeService(IThemeService* svc)
{
    g_themeService = svc;
}

auto GetThemeService() -> IThemeService&
{
    assert(g_themeService != nullptr && "SetThemeService() must be called before GetThemeService()");
    return *g_themeService;
}

auto HasThemeService() -> bool
{
    return g_themeService != nullptr;
}

} // namespace matcha::gui
