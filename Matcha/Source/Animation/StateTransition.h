#pragma once

/**
 * @file StateTransition.h
 * @brief Framework-level auto-animator for InteractionState changes (S3.2).
 *
 * Each ThemeAware widget embeds a StateTransition instance. When the widget's
 * WidgetFsmController fires InteractionStateChanged, StateTransition interpolates
 * the widget's dynamic color properties (background, foreground, border)
 * from old to new state colors using IAnimationService.
 *
 * This class is internal to the Matcha library.
 */

#include "Matcha/Theming/Token/TokenEnums.h"

#include <cstdint>

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

class IAnimationService;
class IThemeService;
struct WidgetStyleSheet;

/**
 * @brief Animates widget color properties on InteractionState transitions.
 *
 * Lifecycle:
 * 1. Constructed with a target WidgetNode and its style sheet.
 * 2. OnStateChanged() called by the widget when InteractionStateChanged fires.
 * 3. Resolves old/new StateStyle from the WidgetStyleSheet's variant[0].
 * 4. Calls IAnimationService::Animate() for bg/fg/border properties.
 *
 * Thread safety: GUI thread only.
 */
class StateTransition {
public:
    /**
     * @param target   Widget to animate (resolved to QWidget* internally).
     * @param variant  Variant index into WidgetStyleSheet::variants.
     */
    explicit StateTransition(fw::WidgetNode* target, uint8_t variant = 0);

    /**
     * @brief Called when InteractionStateChanged fires.
     *
     * Resolves colors for old/new states from the style sheet and
     * starts color transition animations via the global IAnimationService.
     *
     * @param sheet    Current WidgetStyleSheet for the widget.
     * @param oldState Previous interaction state.
     * @param newState New interaction state.
     * @param duration Animation speed token from the style sheet.
     * @param easing   Easing curve token (default: OutCubic).
     */
    void OnStateChanged(const WidgetStyleSheet& sheet,
                        fw::InteractionState oldState,
                        fw::InteractionState newState,
                        fw::AnimationsToken duration,
                        fw::EasingToken easing = fw::EasingToken::OutCubic);

private:
    fw::WidgetNode* _target;
    uint8_t _variant;
};

} // namespace matcha::gui
