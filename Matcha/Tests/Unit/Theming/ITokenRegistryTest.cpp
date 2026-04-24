#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Theming/NyanTheme.h>

#include "QtAppGuard.h"

using namespace matcha::gui;
using namespace matcha::fw;

// ============================================================================
// ITokenRegistry tests via NyanTheme concrete implementation
// ============================================================================

TEST_SUITE("ITokenRegistry") {

TEST_CASE("Default density is Default with scale 1.0") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    CHECK(theme.CurrentDensity() == DensityLevel::Default);
    CHECK(theme.CurrentDensityScale() == doctest::Approx(1.0f));
}

TEST_CASE("SetDensity changes density level and scale") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetDensity(DensityLevel::Compact);
    CHECK(theme.CurrentDensity() == DensityLevel::Compact);
    CHECK(theme.CurrentDensityScale() == doctest::Approx(0.875f));

    theme.SetDensity(DensityLevel::Comfortable);
    CHECK(theme.CurrentDensity() == DensityLevel::Comfortable);
    CHECK(theme.CurrentDensityScale() == doctest::Approx(1.125f));
}

TEST_CASE("SpacingPx applies density scaling") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    // Default density (1.0x): SpacingPx == base value
    theme.SetDensity(DensityLevel::Default);
    CHECK(theme.SpacingPx(SpaceToken::Px8) == 8);
    CHECK(theme.SpacingPx(SpaceToken::Px16) == 16);
    CHECK(theme.SpacingPx(SpaceToken::None) == 0);

    // Compact density (0.875x): Px8 -> round(8*0.875) = round(7.0) = 7
    theme.SetDensity(DensityLevel::Compact);
    CHECK(theme.SpacingPx(SpaceToken::Px8) == 7);
    // Px16 -> round(16*0.875) = round(14.0) = 14
    CHECK(theme.SpacingPx(SpaceToken::Px16) == 14);

    // Comfortable density (1.125x): Px8 -> round(8*1.125) = round(9.0) = 9
    theme.SetDensity(DensityLevel::Comfortable);
    CHECK(theme.SpacingPx(SpaceToken::Px8) == 9);
}

TEST_CASE("Default direction is LTR") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    CHECK(theme.CurrentDirection() == TextDirection::LTR);
}

TEST_CASE("SetDirection changes direction") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetDirection(TextDirection::RTL);
    CHECK(theme.CurrentDirection() == TextDirection::RTL);

    theme.SetDirection(TextDirection::LTR);
    CHECK(theme.CurrentDirection() == TextDirection::LTR);
}

TEST_CASE("Radius is not affected by density") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetDensity(DensityLevel::Compact);
    CHECK(theme.Radius(RadiusToken::Default) == 3);
    CHECK(theme.Radius(RadiusToken::Large) == 8);

    theme.SetDensity(DensityLevel::Comfortable);
    CHECK(theme.Radius(RadiusToken::Default) == 3);
}

TEST_CASE("AnimationMs respects override across density changes") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetAnimationOverride(0);
    CHECK(theme.AnimationMs(AnimationsToken::Normal) == 0);
    CHECK(theme.AnimationMs(AnimationsToken::Slow) == 0);

    theme.SetAnimationOverride(-1);
    CHECK(theme.AnimationMs(AnimationsToken::Normal) == 200);
    CHECK(theme.AnimationMs(AnimationsToken::Slow) == 350);
}

TEST_CASE("ITokenRegistry pointer from IThemeService") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    ITokenRegistry* registry = &theme;
    CHECK(registry->CurrentDensity() == DensityLevel::Default);
    CHECK(registry->SpacingPx(SpaceToken::Px4) == 4);

    registry->SetDensity(DensityLevel::Compact);
    CHECK(theme.CurrentDensity() == DensityLevel::Compact);
}

TEST_CASE("kThemeHighContrast falls back to magenta palette") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

    // HighContrast.json doesn't exist yet, so colors should be magenta fallback
    theme.SetTheme(kThemeHighContrast);
    CHECK(theme.Color(ColorToken::Surface) == QColor(255, 0, 255));
}

TEST_CASE("RegisterTheme rejects empty name") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

    CHECK_FALSE(theme.RegisterTheme(QString(), QStringLiteral(MATCHA_TEST_FIXTURE_DIR "/HighContrast.json"), ThemeMode::Light));
}

TEST_CASE("RegisterTheme rejects nonexistent file") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

    CHECK_FALSE(theme.RegisterTheme(QStringLiteral("Custom0"), QStringLiteral("/nonexistent/path.json"), ThemeMode::Light));
}

TEST_CASE("RegisterTheme + SetTheme loads registered palette") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

    CHECK(theme.RegisterTheme(kThemeHighContrast,
        QStringLiteral(MATCHA_TEST_FIXTURE_DIR "/HighContrast.json"), ThemeMode::Light));

    theme.SetTheme(kThemeHighContrast);
    // HighContrast.json sets BgBase = #FFFFFF
    CHECK(theme.Color(ColorToken::Surface) == QColor(255, 255, 255));
    // HighContrast.json sets BgComponent = #000000
    CHECK(theme.Color(ColorToken::Fill) == QColor(0, 0, 0));
}

TEST_CASE("Extends: custom theme inherits from base and overlays") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

    // Register a theme that extends Dark and overrides only BgBase and TextPrimary
    CHECK(theme.RegisterTheme(QStringLiteral("Custom0"),
        QStringLiteral(MATCHA_TEST_FIXTURE_DIR "/CustomExtends.json"), ThemeMode::Dark));

    // Load Dark first to know its values
    theme.SetTheme(kThemeDark);
    const QColor darkBgElevated = theme.Color(ColorToken::colorPrimaryBg);
    const QColor darkBorderSubtle = theme.Color(ColorToken::BorderSubtle);

    // Switch to custom theme
    theme.SetTheme(QString("Custom0"));

    // Overridden: BgBase = #FF0000
    CHECK(theme.Color(ColorToken::Surface) == QColor(255, 0, 0));
    // Overridden: TextPrimary = #FF00FF00 (ARGB: alpha=0xFF, R=0, G=0xFF, B=0)
    CHECK(theme.Color(ColorToken::colorText) == QColor::fromString(QStringLiteral("#FF00FF00")));
    // Inherited from Dark: BgElevated unchanged
    CHECK(theme.Color(ColorToken::colorPrimaryBg) == darkBgElevated);
    // Inherited from Dark: BorderSubtle unchanged
    CHECK(theme.Color(ColorToken::BorderSubtle) == darkBorderSubtle);
}

TEST_CASE("Unregistered Custom theme falls back to magenta") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

    // Custom1 not registered — should fall back to magenta
    theme.SetTheme(QString("Custom1"));
    CHECK(theme.Color(ColorToken::Surface) == QColor(255, 0, 255));
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
