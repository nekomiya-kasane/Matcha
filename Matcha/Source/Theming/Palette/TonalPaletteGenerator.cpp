/**
 * @file TonalPaletteGenerator.cpp
 * @brief OKLCH-based tonal palette generation implementation.
 *
 * Conversion chain: sRGB -> Linear RGB -> XYZ D65 -> Oklab (L,a,b) -> OKLCH (L,C,H)
 * References:
 *   - Bjorn Ottosson, "A perceptual color space for image processing"
 *     https://bottosson.github.io/posts/oklab/
 */

#include <Matcha/Theming/Palette/TonalPaletteGenerator.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace matcha::gui {

namespace {

// ============================================================================
// sRGB <-> Linear RGB
// ============================================================================

[[nodiscard]] auto SrgbToLinear(float c) -> float
{
    if (c <= 0.04045f) {
        return c / 12.92f;
    }
    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}

[[nodiscard]] auto LinearToSrgb(float c) -> float
{
    if (c <= 0.0031308f) {
        return c * 12.92f;
    }
    return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}

// ============================================================================
// Linear RGB -> Oklab (L, a, b)
// Ottosson's direct conversion (bypasses XYZ for efficiency)
// ============================================================================

struct OklabColor {
    float L = 0.0f;
    float a = 0.0f;
    float b = 0.0f;
};

[[nodiscard]] auto LinearRgbToOklab(float r, float g, float b) -> OklabColor
{
    // Linear RGB -> LMS (approximate, Ottosson's M1 matrix)
    float l = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
    float m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
    float s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;

    // Cube root
    float l_ = std::cbrt(l);
    float m_ = std::cbrt(m);
    float s_ = std::cbrt(s);

    // LMS^(1/3) -> Oklab (Ottosson's M2 matrix)
    return {
        .L = 0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
        .a = 1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
        .b = 0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_,
    };
}

[[nodiscard]] auto OklabToLinearRgb(OklabColor lab) -> std::array<float, 3>
{
    // Oklab -> LMS^(1/3) (inverse of M2)
    float l_ = lab.L + 0.3963377774f * lab.a + 0.2158037573f * lab.b;
    float m_ = lab.L - 0.1055613458f * lab.a - 0.0638541728f * lab.b;
    float s_ = lab.L - 0.0894841775f * lab.a - 1.2914855480f * lab.b;

    // Cube
    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    // LMS -> Linear RGB (inverse of M1)
    float r =  4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
    float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
    float b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

    return {r, g, b};
}

// ============================================================================
// Oklab <-> OKLCH
// ============================================================================

[[nodiscard]] auto OklabToOklch(OklabColor lab) -> OklchColor
{
    float C = std::sqrt(lab.a * lab.a + lab.b * lab.b);
    float H = std::atan2(lab.b, lab.a) * (180.0f / std::numbers::pi_v<float>);
    if (H < 0.0f) {
        H += 360.0f;
    }
    return {.L = lab.L, .C = C, .H = H};
}

[[nodiscard]] auto OklchToOklab(OklchColor lch) -> OklabColor
{
    float hRad = lch.H * (std::numbers::pi_v<float> / 180.0f);
    return {
        .L = lch.L,
        .a = lch.C * std::cos(hRad),
        .b = lch.C * std::sin(hRad),
    };
}

// ============================================================================
// Gamut clamping: reduce chroma until sRGB-representable
// ============================================================================

[[nodiscard]] auto IsInGamut(float r, float g, float b) -> bool
{
    constexpr float kEps = -0.001f; // small tolerance
    constexpr float kMax = 1.001f;
    return (r >= kEps && r <= kMax &&
            g >= kEps && g <= kMax &&
            b >= kEps && b <= kMax);
}

[[nodiscard]] auto ClampToGamut(OklchColor lch) -> OklchColor
{
    // Binary search on chroma to find max in-gamut value
    float lo = 0.0f;
    float hi = lch.C;

    // First check if already in gamut
    auto lab = OklchToOklab(lch);
    auto [r, g, b] = OklabToLinearRgb(lab);
    if (IsInGamut(r, g, b)) {
        return lch;
    }

    // Binary search (12 iterations gives ~0.0001 precision)
    for (int i = 0; i < 12; ++i) {
        float mid = (lo + hi) * 0.5f;
        OklchColor test = {.L = lch.L, .C = mid, .H = lch.H};
        auto testLab = OklchToOklab(test);
        auto [tr, tg, tb] = OklabToLinearRgb(testLab);
        if (IsInGamut(tr, tg, tb)) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    return {.L = lch.L, .C = lo, .H = lch.H};
}

// ============================================================================
// Lightness distributions for 10 tonal levels (Ant Design model)
// ============================================================================

// Light theme: step 1 is very light, step 10 is dark (Ant Design model)
constexpr std::array<float, 10> kLightThemeLightness = {
    0.96f, // [0] Bg           (step 1)
    0.92f, // [1] BgHover      (step 2)
    0.86f, // [2] Border       (step 3)
    0.78f, // [3] BorderHover  (step 4)
    0.68f, // [4] Hover        (step 5)
    0.60f, // [5] Base/seed    (step 6)
    0.52f, // [6] Active       (step 7)
    0.44f, // [7] TextHover    (step 8)
    0.36f, // [8] Text         (step 9)
    0.30f, // [9] TextActive   (step 10)
};

// Dark theme: step 1 is very dark, step 10 is bright (Ant Design model)
constexpr std::array<float, 10> kDarkThemeLightness = {
    0.20f, // [0] Bg           (step 1)
    0.26f, // [1] BgHover      (step 2)
    0.33f, // [2] Border       (step 3)
    0.40f, // [3] BorderHover  (step 4)
    0.50f, // [4] Hover        (step 5)
    0.60f, // [5] Base         (step 6)
    0.68f, // [6] Active       (step 7)
    0.76f, // [7] TextHover    (step 8)
    0.84f, // [8] Text         (step 9)
    0.90f, // [9] TextActive   (step 10)
};

[[nodiscard]] auto GenerateRamp(QColor seed, const std::array<float, 10>& lightnessMap)
    -> std::array<QColor, 10>
{
    // Convert seed to OKLCH to extract chroma and hue
    auto oklch = TonalPaletteGenerator::SrgbToOklch(seed);

    std::array<QColor, 10> result {};
    for (int i = 0; i < 10; ++i) {
        OklchColor tone = {.L = lightnessMap[static_cast<std::size_t>(i)],
                           .C = oklch.C,
                           .H = oklch.H};
        tone = ClampToGamut(tone);
        result[static_cast<std::size_t>(i)] = TonalPaletteGenerator::OklchToSrgb(tone);
    }
    return result;
}

} // anonymous namespace

// ============================================================================
// Public API
// ============================================================================

auto TonalPaletteGenerator::SrgbToOklch(QColor color) -> OklchColor
{
    float r = SrgbToLinear(static_cast<float>(color.redF()));
    float g = SrgbToLinear(static_cast<float>(color.greenF()));
    float b = SrgbToLinear(static_cast<float>(color.blueF()));  // NOLINT(bugprone-narrowing-conversions)

    auto lab = LinearRgbToOklab(r, g, b);
    return OklabToOklch(lab);
}

auto TonalPaletteGenerator::OklchToSrgb(OklchColor oklch) -> QColor
{
    auto lab = OklchToOklab(oklch);
    auto [lr, lg, lb] = OklabToLinearRgb(lab);

    float sr = LinearToSrgb(std::clamp(lr, 0.0f, 1.0f));
    float sg = LinearToSrgb(std::clamp(lg, 0.0f, 1.0f));
    float sb = LinearToSrgb(std::clamp(lb, 0.0f, 1.0f));

    return QColor::fromRgbF(static_cast<double>(std::clamp(sr, 0.0f, 1.0f)),
                            static_cast<double>(std::clamp(sg, 0.0f, 1.0f)),
                            static_cast<double>(std::clamp(sb, 0.0f, 1.0f)));
}

auto TonalPaletteGenerator::GenerateLight(QColor seed) -> std::array<QColor, kToneCount>
{
    return GenerateRamp(seed, kLightThemeLightness);
}

auto TonalPaletteGenerator::GenerateDark(QColor seed) -> std::array<QColor, kToneCount>
{
    return GenerateRamp(seed, kDarkThemeLightness);
}

} // namespace matcha::gui
