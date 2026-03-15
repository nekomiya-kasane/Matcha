/**
 * @file TokenRegistryGlobal.cpp
 * @brief Global ITokenRegistry accessor implementation.
 */

#include "Matcha/Theming/Token/TokenRegistryGlobal.h"

#include <cassert>

namespace matcha::fw {

namespace {
ITokenRegistry* g_tokenRegistry = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
} // namespace

void SetGlobalTokenRegistry(ITokenRegistry* registry)
{
    g_tokenRegistry = registry;
}

auto GetGlobalTokenRegistry() -> ITokenRegistry*
{
    return g_tokenRegistry;
}

} // namespace matcha::fw
