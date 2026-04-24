#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Theming/NyanTheme.h>
#include <Matcha/Theming/Palette/TonalPaletteGenerator.h>

#include <QColor>
#include <QDir>
#include <QFile>
#include <QGuiApplication>

using namespace matcha::gui;
using namespace matcha::fw;

// ============================================================================
// Helper: ensure QGuiApplication exists (shared singleton)
// ============================================================================

namespace {

auto ensureTonalApp() -> QGuiApplication*
{
    if (QCoreApplication::instance() != nullptr) {
        return nullptr; // already created by another TU
    }
    static int argc = 0;
    static QGuiApplication app(argc, nullptr);
    return &app;
}

// Helper to write a minimal palette JSON with optional colorSeeds/colorOverrides
void writeSeedPalette(const QString& dir, const QByteArray& seedsJson,
                      const QByteArray& overridesJson = {})
{
    QDir().mkpath(dir);
    QFile f(dir + "/Light.json");
    f.open(QIODevice::WriteOnly);
    QByteArray json = R"({
        "colors": {
            "Surface": "#FFFFFF", "SurfaceContainer": "#F8F8F8",
            "SurfaceElevated": "#FFFFFF", "SurfaceSunken": "#F0F0F0",
            "Spotlight": "#FFFFFF",
            "Fill": "#F0F0F0", "FillHover": "#E8E8E8",
            "FillActive": "#E0E0E0", "FillMuted": "#F5F5F5",
            "BorderSubtle": "#E0E0E0", "BorderDefault": "#CCCCCC",
            "BorderStrong": "#999999",
            "TextPrimary": "#1A1A1A", "TextSecondary": "#666666",
            "TextTertiary": "#999999", "TextDisabled": "#CCCCCC",
            "OnAccent": "#FFFFFF", "OnAccentSecondary": "#FFFFFFA6",
            "Focus": "#6366F1", "Selection": "#6366F126",
            "Link": "#6366F1", "Scrim": "#00000073",
            "Shadow": "#00000014", "Separator": "#0000000F"
        })";

    if (!seedsJson.isEmpty()) {
        json += ",\n        \"colorSeeds\": " + seedsJson;
    }
    if (!overridesJson.isEmpty()) {
        json += ",\n        \"colorOverrides\": " + overridesJson;
    }
    json += "\n    }";
    f.write(json);
    f.close();
}

} // anonymous namespace

// ============================================================================
// OKLCH round-trip tests
// ============================================================================

TEST_SUITE("TonalPaletteGenerator") {

TEST_CASE("SrgbToOklch round-trip preserves color") {
    QColor red(255, 0, 0);
    auto oklch = TonalPaletteGenerator::SrgbToOklch(red);
    CHECK(oklch.L > 0.0f);
    CHECK(oklch.L < 1.0f);
    CHECK(oklch.C > 0.0f);

    QColor back = TonalPaletteGenerator::OklchToSrgb(oklch);
    CHECK(std::abs(back.red() - red.red()) <= 1);
    CHECK(std::abs(back.green() - red.green()) <= 1);
    CHECK(std::abs(back.blue() - red.blue()) <= 1);
}

TEST_CASE("SrgbToOklch round-trip for blue") {
    QColor blue(0, 0, 255);
    auto oklch = TonalPaletteGenerator::SrgbToOklch(blue);
    QColor back = TonalPaletteGenerator::OklchToSrgb(oklch);
    CHECK(std::abs(back.red() - blue.red()) <= 1);
    CHECK(std::abs(back.green() - blue.green()) <= 1);
    CHECK(std::abs(back.blue() - blue.blue()) <= 1);
}

TEST_CASE("SrgbToOklch round-trip for gray") {
    QColor gray(128, 128, 128);
    auto oklch = TonalPaletteGenerator::SrgbToOklch(gray);
    CHECK(oklch.C < 0.01f);
    QColor back = TonalPaletteGenerator::OklchToSrgb(oklch);
    CHECK(std::abs(back.red() - gray.red()) <= 1);
    CHECK(std::abs(back.green() - gray.green()) <= 1);
    CHECK(std::abs(back.blue() - gray.blue()) <= 1);
}

TEST_CASE("SrgbToOklch black and white extremes") {
    auto black = TonalPaletteGenerator::SrgbToOklch(QColor(0, 0, 0));
    CHECK(black.L == doctest::Approx(0.0f).epsilon(0.01));

    auto white = TonalPaletteGenerator::SrgbToOklch(QColor(255, 255, 255));
    CHECK(white.L == doctest::Approx(1.0f).epsilon(0.01));
}

// ============================================================================
// Tonal ramp generation tests
// ============================================================================

TEST_CASE("GenerateLight produces 10 tones with decreasing lightness") {
    QColor seed(99, 102, 241);
    auto ramp = TonalPaletteGenerator::GenerateLight(seed);
    CHECK(ramp.size() == 10);

    auto l0 = TonalPaletteGenerator::SrgbToOklch(ramp[0]);
    auto l9 = TonalPaletteGenerator::SrgbToOklch(ramp[9]);
    CHECK(l0.L > l9.L);

    for (int i = 0; i < 9; ++i) {
        auto li = TonalPaletteGenerator::SrgbToOklch(ramp[static_cast<std::size_t>(i)]);
        auto li1 = TonalPaletteGenerator::SrgbToOklch(ramp[static_cast<std::size_t>(i + 1)]);
        CHECK(li.L > li1.L);
    }
}

TEST_CASE("GenerateDark produces 10 tones with increasing lightness") {
    QColor seed(99, 102, 241);
    auto ramp = TonalPaletteGenerator::GenerateDark(seed);
    CHECK(ramp.size() == 10);

    auto l0 = TonalPaletteGenerator::SrgbToOklch(ramp[0]);
    auto l9 = TonalPaletteGenerator::SrgbToOklch(ramp[9]);
    CHECK(l0.L < l9.L);

    for (int i = 0; i < 9; ++i) {
        auto li = TonalPaletteGenerator::SrgbToOklch(ramp[static_cast<std::size_t>(i)]);
        auto li1 = TonalPaletteGenerator::SrgbToOklch(ramp[static_cast<std::size_t>(i + 1)]);
        CHECK(li.L < li1.L);
    }
}

TEST_CASE("All generated colors are valid sRGB") {
    QColor seed(34, 197, 94);
    auto lightRamp = TonalPaletteGenerator::GenerateLight(seed);
    auto darkRamp = TonalPaletteGenerator::GenerateDark(seed);

    for (const auto& c : lightRamp) {
        CHECK(c.isValid());
        CHECK(c.red() >= 0);
        CHECK(c.red() <= 255);
        CHECK(c.green() >= 0);
        CHECK(c.green() <= 255);
        CHECK(c.blue() >= 0);
        CHECK(c.blue() <= 255);
    }
    for (const auto& c : darkRamp) {
        CHECK(c.isValid());
    }
}

TEST_CASE("Hue is preserved across tonal ramp") {
    QColor seed(245, 158, 11);
    auto oklchSeed = TonalPaletteGenerator::SrgbToOklch(seed);
    auto ramp = TonalPaletteGenerator::GenerateLight(seed);

    for (const auto& c : ramp) {
        auto oklch = TonalPaletteGenerator::SrgbToOklch(c);
        float hueDiff = std::abs(oklch.H - oklchSeed.H);
        if (hueDiff > 180.0f) {
            hueDiff = 360.0f - hueDiff;
        }
        CHECK(hueDiff < 5.0f);
    }
}

// ============================================================================
// colorSeeds JSON integration test
// ============================================================================

TEST_CASE("LoadPalette with colorSeeds generates semantic hue tokens") {
    ensureTonalApp();

    QString testDir = QDir::temp().filePath("matcha_tonal_test");
    writeSeedPalette(testDir,
        R"({"Primary":"#6366F1","Success":"#22C55E","Warning":"#F59E0B","Error":"#EF4444","Info":"#1677FF"})");

    NyanTheme theme(testDir);
    theme.SetTheme(kThemeLight);

    CHECK(theme.Color(ColorToken::PrimaryBg) != QColor(255, 0, 255));
    CHECK(theme.Color(ColorToken::colorPrimary) != QColor(255, 0, 255));
    CHECK(theme.Color(ColorToken::PrimaryTextActive) != QColor(255, 0, 255));
    CHECK(theme.Color(ColorToken::Success) != QColor(255, 0, 255));
    CHECK(theme.Color(ColorToken::Warning) != QColor(255, 0, 255));
    CHECK(theme.Color(ColorToken::colorError) != QColor(255, 0, 255));

    auto lSubtlest = TonalPaletteGenerator::SrgbToOklch(theme.Color(ColorToken::PrimaryBg));
    auto lBoldest = TonalPaletteGenerator::SrgbToOklch(theme.Color(ColorToken::PrimaryTextActive));
    CHECK(lSubtlest.L > lBoldest.L);

    CHECK(theme.Color(ColorToken::Surface) == QColor::fromString("#FFFFFF"));

    QFile::remove(testDir + "/Light.json");
    QDir().rmdir(testDir);
}

TEST_CASE("colorOverrides overrides generated seed colors") {
    ensureTonalApp();

    QString testDir = QDir::temp().filePath("matcha_tonal_override_test");
    writeSeedPalette(testDir,
        R"({"Primary":"#6366F1"})",
        R"({"Primary":"#5B5FC7"})");

    NyanTheme theme(testDir);
    theme.SetTheme(kThemeLight);

    CHECK(theme.Color(ColorToken::colorPrimary) == QColor::fromString("#5B5FC7"));
    CHECK(theme.Color(ColorToken::PrimaryBg) != QColor(255, 0, 255));
    CHECK(theme.Color(ColorToken::PrimaryBg) != QColor::fromString("#5B5FC7"));

    QFile::remove(testDir + "/Light.json");
    QDir().rmdir(testDir);
}

// ============================================================================
// Dynamic Font / Spacing token tests
// ============================================================================

TEST_CASE("DynamicFont register, query, unregister") {
    ensureTonalApp();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    CHECK(!theme.DynamicFont("CAD/PropertyGrid").has_value());

    FontSpec spec;
    spec.family = QStringLiteral("Courier New");
    spec.sizeInPt = 11;
    spec.weight = 700;

    std::array defs = {
        IThemeService::DynamicFontDef {
            .key = "CAD/PropertyGrid",
            .value = spec,
        },
    };
    theme.RegisterDynamicFonts(defs);

    auto result = theme.DynamicFont("CAD/PropertyGrid");
    REQUIRE(result.has_value());
    CHECK(result->family == QStringLiteral("Courier New"));
    CHECK(result->sizeInPt == 11);
    CHECK(result->weight == 700);

    std::array<std::string_view, 1> keys = {"CAD/PropertyGrid"};
    theme.UnregisterDynamicTokens(keys);
    CHECK(!theme.DynamicFont("CAD/PropertyGrid").has_value());
}

TEST_CASE("DynamicSpacingPx register, query with density scaling") {
    ensureTonalApp();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    CHECK(!theme.DynamicSpacingPx("FEA/ResultMargin").has_value());

    std::array defs = {
        IThemeService::DynamicSpacingDef {
            .key = "FEA/ResultMargin",
            .basePx = 16,
        },
    };
    theme.RegisterDynamicSpacings(defs);

    // Default density (1.0x)
    theme.SetDensity(DensityLevel::Default);
    auto result = theme.DynamicSpacingPx("FEA/ResultMargin");
    REQUIRE(result.has_value());
    CHECK(*result == 16);

    // Compact density (0.875x): round(16 * 0.875) = 14
    theme.SetDensity(DensityLevel::Compact);
    result = theme.DynamicSpacingPx("FEA/ResultMargin");
    REQUIRE(result.has_value());
    CHECK(*result == 14);

    // Comfortable density (1.125x): round(16 * 1.125) = 18
    theme.SetDensity(DensityLevel::Comfortable);
    result = theme.DynamicSpacingPx("FEA/ResultMargin");
    REQUIRE(result.has_value());
    CHECK(*result == 18);

    std::array<std::string_view, 1> keys = {"FEA/ResultMargin"};
    theme.UnregisterDynamicTokens(keys);
    CHECK(!theme.DynamicSpacingPx("FEA/ResultMargin").has_value());
}

TEST_CASE("UnregisterDynamicTokens removes all types with same key") {
    ensureTonalApp();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    // Register color, font, and spacing with the same key prefix
    std::array colorDefs = {
        IThemeService::DynamicColorDef {
            .key = "Test/Multi",
            .lightValue = QColor(255, 0, 0),
            .darkValue = QColor(0, 0, 255),
        },
    };
    FontSpec fs;
    fs.family = QStringLiteral("Arial");
    fs.sizeInPt = 10;
    std::array fontDefs = {
        IThemeService::DynamicFontDef {.key = "Test/Multi", .value = fs},
    };
    std::array spacingDefs = {
        IThemeService::DynamicSpacingDef {.key = "Test/Multi", .basePx = 8},
    };

    theme.RegisterDynamicTokens(colorDefs);
    theme.RegisterDynamicFonts(fontDefs);
    theme.RegisterDynamicSpacings(spacingDefs);

    CHECK(theme.DynamicColor("Test/Multi").has_value());
    CHECK(theme.DynamicFont("Test/Multi").has_value());
    CHECK(theme.DynamicSpacingPx("Test/Multi").has_value());

    // Unregister removes all three
    std::array<std::string_view, 1> keys = {"Test/Multi"};
    theme.UnregisterDynamicTokens(keys);
    CHECK(!theme.DynamicColor("Test/Multi").has_value());
    CHECK(!theme.DynamicFont("Test/Multi").has_value());
    CHECK(!theme.DynamicSpacingPx("Test/Multi").has_value());
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
