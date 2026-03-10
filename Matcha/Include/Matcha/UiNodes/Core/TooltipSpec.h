#pragma once

/**
 * @file TooltipSpec.h
 * @brief Tooltip specification value type (S9, fw layer, zero Qt).
 *
 * A tooltip is a property of the WidgetNode that triggers it, not a separate
 * managed resource. Application's internal event filter reads the spec and
 * shows/hides a single reused NyanRichTooltip instance.
 */

#include "Matcha/UiNodes/Core/TokenEnums.h"

#include <cstdint>
#include <string>

namespace matcha::fw {

/**
 * @brief Tooltip position hint relative to the trigger widget.
 */
enum class TooltipPosition : uint8_t {
    Auto  = 0, ///< Let the framework choose (default)
    Below = 1, ///< Prefer below the widget
    Above = 2, ///< Prefer above the widget
};

/**
 * @brief Tooltip specification — fw-layer, Qt-free.
 *
 * Stored on WidgetNode. The gui layer reads this to render the tooltip.
 */
struct TooltipSpec {
    std::string title;                               ///< Main tooltip text (required)
    std::string shortcut;                            ///< Keyboard shortcut hint (optional)
    std::string description;                         ///< Extended description (optional, tier 2)
    IconId      iconId;                               ///< Icon URI to show beside title (optional, empty = none)
    int         tier1DelayMs = 200;                  ///< Delay before showing tier 1 (title only)
    int         tier2DelayMs = 1000;                 ///< Delay before expanding to tier 2 (description)
    TooltipPosition position = TooltipPosition::Auto;
    uint32_t    previewKey  = 0;                     ///< Opaque key for rich preview (0 = none)

    [[nodiscard]] auto IsEmpty() const -> bool { return title.empty(); }
};

} // namespace matcha::fw
