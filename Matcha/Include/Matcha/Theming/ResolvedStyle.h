#pragma once

/**
 * @file ResolvedStyle.h
 * @brief Fully resolved widget style snapshot for paintEvent consumption.
 *
 * ResolvedStyle is the output of IThemeService::Resolve(). It contains
 * all visual attributes needed by a widget's paintEvent, fully resolved
 * from tokens to concrete Qt values. Widgets no longer need to dereference
 * tokens themselves.
 *
 * @see docs/07_Declarative_Style_RFC.md Section 5.5
 * @see IThemeService::Resolve()
 */

#include <Matcha/Theming/DesignTokens.h>

#include <QColor>
#include <QFont>

#include <optional>

namespace matcha::gui {

/**
 * @brief Fully resolved visual style for a widget at a specific (variant, state).
 *
 * Produced by `IThemeService::Resolve(kind, variantIndex, state)`.
 * All values are concrete: colors are QColor, geometry is density-scaled px,
 * font is a fully constructed QFont. Widgets consume this directly in paintEvent
 * without any further token lookups.
 *
 * Usage:
 * @code
 * auto style = Theme().Resolve(WidgetKind::PushButton,
 *                               std::to_underlying(_variant),
 *                               currentState());
 * p.setOpacity(style.opacity);
 * p.setBrush(style.background);
 * p.setPen(QPen(style.border, style.borderWidthPx));
 * p.drawRoundedRect(r, style.radiusPx, style.radiusPx);
 * p.setFont(style.font);
 * p.setPen(style.foreground);
 * p.drawText(textRect, Qt::AlignVCenter, text());
 * @endcode
 */
struct ResolvedStyle {
    // -- Resolved colors --
    QColor background;      ///< Background fill color
    QColor foreground;      ///< Text / icon color
    QColor border;          ///< Border stroke color

    // -- Resolved geometry (density-scaled) --
    int radiusPx      = 0;  ///< Corner radius in pixels
    int paddingHPx    = 0;  ///< Horizontal content padding in pixels
    int paddingVPx    = 0;  ///< Vertical content padding in pixels
    int gapPx         = 0;  ///< Icon-text gap in pixels
    int minHeightPx   = 0;  ///< Minimum component height in pixels
    int borderWidthPx = 0;  ///< Border stroke width in pixels

    // -- Resolved typography --
    QFont font;             ///< Fully constructed, platform-resolved font
    int   lineHeightPx = 0; ///< Line height in pixels (fontSizePx * lineHeightMultiplier)

    // -- Resolved visual --
    ShadowSpec shadow;      ///< Box shadow parameters
    float opacity = 1.0F;   ///< Widget opacity (0.0..1.0)

    // -- Resolved transition --
    int durationMs    = 0;  ///< Animation duration in milliseconds
    int easingType    = 0;  ///< QEasingCurve::Type for state transitions
};

/**
 * @brief Per-instance style override (cascade Layer 3 — highest priority).
 *
 * Allows a single widget instance to patch any field of a `ResolvedStyle`
 * after the base cascade resolution. Each `std::optional` field acts as a
 * sparse patch: if `has_value()`, it replaces the corresponding resolved
 * value; otherwise the base value is kept.
 *
 * Usage:
 * @code
 * InstanceStyleOverride ov;
 * ov.background = QColor("#FF0000");
 * ov.radiusPx   = 12;
 * auto style = Theme().Resolve(WidgetKind::PushButton, 0,
 *                               InteractionState::Normal, &ov);
 * @endcode
 *
 * @see §4.17 Style Cascade Resolution
 */
struct InstanceStyleOverride {
    // -- Color overrides --
    std::optional<QColor> background;
    std::optional<QColor> foreground;
    std::optional<QColor> border;

    // -- Geometry overrides (concrete px, already density-scaled by caller) --
    std::optional<int>    radiusPx;
    std::optional<int>    paddingHPx;
    std::optional<int>    paddingVPx;
    std::optional<int>    gapPx;
    std::optional<int>    minHeightPx;
    std::optional<int>    borderWidthPx;

    // -- Visual overrides --
    std::optional<float>  opacity;

    // -- Transition overrides --
    std::optional<int>    durationMs;

    /**
     * @brief Apply this override to a resolved style (in-place patch).
     * @param style The style to patch.
     */
    void ApplyTo(ResolvedStyle& style) const
    {
        if (background)    { style.background    = *background; }
        if (foreground)    { style.foreground     = *foreground; }
        if (border)        { style.border         = *border; }
        if (radiusPx)      { style.radiusPx       = *radiusPx; }
        if (paddingHPx)    { style.paddingHPx     = *paddingHPx; }
        if (paddingVPx)    { style.paddingVPx     = *paddingVPx; }
        if (gapPx)         { style.gapPx          = *gapPx; }
        if (minHeightPx)   { style.minHeightPx    = *minHeightPx; }
        if (borderWidthPx) { style.borderWidthPx  = *borderWidthPx; }
        if (opacity)       { style.opacity         = *opacity; }
        if (durationMs)    { style.durationMs      = *durationMs; }
    }
};

} // namespace matcha::gui
