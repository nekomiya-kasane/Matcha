#pragma once

/**
 * @file ITokenRegistry.h
 * @brief Qt-free abstract interface for design token queries.
 *
 * ITokenRegistry lives in matcha::fw and has ZERO Qt dependency.
 * It provides density-aware spacing/radius queries that the fw layer
 * (ContainerNode, WidgetNode) can use without touching Qt types.
 *
 * The concrete implementation is NyanTheme (in matcha::gui), which
 * also implements IThemeService. Application exposes both interfaces:
 *   - Tokens() -> ITokenRegistry&   (fw-layer consumers)
 *   - Theme()  -> IThemeService&    (gui-layer consumers)
 *
 * Both return the same underlying NyanTheme object.
 *
 * @see plan.md Part VIII for the split rationale.
 * @see TokenEnums.h for the token enum definitions.
 */

#include "Matcha/UiNodes/Core/TokenEnums.h"

namespace matcha::fw {

/**
 * @brief Abstract interface for Qt-free design token queries.
 *
 * All query methods are [[nodiscard]] and const. They perform O(1)
 * lookups with density scaling applied transparently.
 *
 * Thread safety: const query methods are safe for concurrent reads.
 * SetDensity/SetDirection must be called from the GUI thread only.
 */
class ITokenRegistry {
public:
    virtual ~ITokenRegistry() = default;

    ITokenRegistry(const ITokenRegistry&)            = delete;
    ITokenRegistry& operator=(const ITokenRegistry&) = delete;
    ITokenRegistry(ITokenRegistry&&)                 = delete;
    ITokenRegistry& operator=(ITokenRegistry&&)      = delete;

    // ========================================================================
    // Density
    // ========================================================================

    /**
     * @brief Set the active density level.
     *
     * Triggers re-resolution of all density-dependent tokens.
     * GUI thread only.
     */
    virtual void SetDensity(DensityLevel level) = 0;

    /**
     * @brief Query the current density level.
     */
    [[nodiscard]] virtual auto CurrentDensity() const -> DensityLevel = 0;

    /**
     * @brief Query the current density scale factor.
     */
    [[nodiscard]] virtual auto CurrentDensityScale() const -> float = 0;

    // ========================================================================
    // Text Direction
    // ========================================================================

    /**
     * @brief Set the global text direction (LTR/RTL).
     */
    virtual void SetDirection(TextDirection dir) = 0;

    /**
     * @brief Query the current text direction.
     */
    [[nodiscard]] virtual auto CurrentDirection() const -> TextDirection = 0;

    // ========================================================================
    // Token Queries (O(1), density-scaled)
    // ========================================================================

    /**
     * @brief Query a spacing value in logical pixels (density-scaled).
     * @param token Spacing token.
     * @return basePx * densityScale, rounded to int.
     */
    [[nodiscard]] virtual auto SpacingPx(SpacingToken token) const -> int = 0;

    /**
     * @brief Query a corner radius value in logical pixels.
     * @param token Radius token.
     * @return Pixel value as int. RadiusToken::Round returns 255 (caller uses min(w,h)/2).
     */
    [[nodiscard]] virtual auto Radius(RadiusToken token) const -> int = 0;

    /**
     * @brief Query animation duration in milliseconds.
     *
     * When animation override is active (test mode), returns the override value.
     *
     * @param speed Animation speed preset.
     * @return Duration in milliseconds (0 = instant / test mode).
     */
    [[nodiscard]] virtual auto AnimationMs(AnimationToken speed) const -> int = 0;

protected:
    ITokenRegistry() = default;
};

} // namespace matcha::fw
