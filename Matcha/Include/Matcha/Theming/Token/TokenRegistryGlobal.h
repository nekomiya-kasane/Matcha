#pragma once

/**
 * @file TokenRegistryGlobal.h
 * @brief Global ITokenRegistry accessor for the fw layer.
 *
 * Set once by Application at startup. Used by ContainerNode and other
 * fw-layer classes to resolve SpacingToken -> pixel values without
 * depending on matcha::gui.
 */

#include <Matcha/Core/Macros.h>

namespace matcha::fw {

class ITokenRegistry;

MATCHA_EXPORT void SetGlobalTokenRegistry(ITokenRegistry* registry);
[[nodiscard]] MATCHA_EXPORT auto GetGlobalTokenRegistry() -> ITokenRegistry*;

} // namespace matcha::fw
