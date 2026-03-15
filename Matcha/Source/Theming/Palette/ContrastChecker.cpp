#include "Matcha/Theming/Palette/ContrastChecker.h"

#include <algorithm>
#include <cmath>

namespace matcha::gui {

namespace {

// sRGB linearization per WCAG 2.1 spec
auto Linearize(double srgb) -> double
{
    return (srgb <= 0.04045) ? srgb / 12.92
                             : std::pow((srgb + 0.055) / 1.055, 2.4);
}

} // anonymous namespace

auto ContrastChecker::RelativeLuminance(QColor color) -> double
{
    const double r = Linearize(color.redF());
    const double g = Linearize(color.greenF());
    const double b = Linearize(color.blueF());
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

auto ContrastChecker::Ratio(QColor fg, QColor bg) -> double
{
    const double lFg = RelativeLuminance(fg);
    const double lBg = RelativeLuminance(bg);
    const double lighter = std::max(lFg, lBg);
    const double darker  = std::min(lFg, lBg);
    return (lighter + 0.05) / (darker + 0.05);
}

auto ContrastChecker::MeetsAA(QColor fg, QColor bg) -> bool
{
    return Ratio(fg, bg) >= 4.5;
}

auto ContrastChecker::MeetsAAA(QColor fg, QColor bg) -> bool
{
    return Ratio(fg, bg) >= 7.0;
}

auto ContrastChecker::MeetsAALargeText(QColor fg, QColor bg) -> bool
{
    return Ratio(fg, bg) >= 3.0;
}

auto ContrastChecker::SuggestFix(QColor fg, QColor bg, double targetRatio) -> QColor
{
    if (Ratio(fg, bg) >= targetRatio) {
        return fg;
    }

    // Strategy: binary search on HSL lightness of fg.
    // Determine direction: if bg is dark, lighten fg; if bg is light, darken fg.
    const double bgLum = RelativeLuminance(bg);

    float hue = 0.0F;
    float sat = 0.0F;
    float light = 0.0F;
    fg.getHslF(&hue, &sat, &light);

    // Try both directions and pick the closer one that meets the target
    auto tryDirection = [&](bool lighten) -> QColor {
        float lo = lighten ? light : 0.0F;
        float hi = lighten ? 1.0F : light;

        QColor best = fg;
        for (int iter = 0; iter < 32; ++iter) {
            const float mid = (lo + hi) / 2.0F;
            QColor candidate = QColor::fromHslF(hue, sat, mid);
            const double ratio = Ratio(candidate, bg);
            if (ratio >= targetRatio) {
                best = candidate;
                if (lighten) {
                    hi = mid; // try closer to original
                } else {
                    lo = mid;
                }
            } else {
                if (lighten) {
                    lo = mid; // need more lightness
                } else {
                    hi = mid;
                }
            }
        }
        return best;
    };

    // If bg is dark (luminance < 0.5), prefer lightening fg
    if (bgLum < 0.5) {
        QColor lighter = tryDirection(true);
        if (Ratio(lighter, bg) >= targetRatio) { return lighter; }
        return tryDirection(false);
    }

    QColor darker = tryDirection(false);
    if (Ratio(darker, bg) >= targetRatio) { return darker; }
    return tryDirection(true);
}

} // namespace matcha::gui
