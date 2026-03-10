#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Widgets/Core/NyanTheme.h>

#include "QtAppGuard.h"

#include <QColor>

#include <array>
#include <string_view>
#include <utility>

using namespace matcha::gui;

// ============================================================================
// Runtime tests
// ============================================================================

TEST_SUITE("PaletteLoad") {

TEST_CASE("Light palette loads all valid colors") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    CHECK(theme.CurrentTheme() == kThemeLight);

    // Verify all ColorToken values are valid (not default-constructed QColor)
    for (std::size_t i = 0; i < kColorTokenCount; ++i) {
        const auto token = static_cast<ColorToken>(static_cast<uint8_t>(i));
        const auto color = theme.Color(token);
        CHECK_MESSAGE(color.isValid(), "ColorToken index ", i, " is invalid");
        // Should not be magenta fallback (missing palette indicator)
        CHECK_MESSAGE(color != QColor(255, 0, 255),
                      "ColorToken index ", i, " is magenta fallback");
    }
}

TEST_CASE("Dark palette loads all valid colors") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeDark);

    CHECK(theme.CurrentTheme() == kThemeDark);

    for (std::size_t i = 0; i < kColorTokenCount; ++i) {
        const auto token = static_cast<ColorToken>(static_cast<uint8_t>(i));
        const auto color = theme.Color(token);
        CHECK_MESSAGE(color.isValid(), "ColorToken index ", i, " is invalid");
    }
}

TEST_CASE("Light and Dark palettes differ") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

    theme.SetTheme(kThemeLight);
    const auto lightBg0 = theme.Color(ColorToken::Surface);

    theme.SetTheme(kThemeDark);
    const auto darkBg0 = theme.Color(ColorToken::Surface);

    CHECK(lightBg0 != darkBg0);
}

TEST_CASE("FontSpec family is non-empty after SetTheme") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    for (std::size_t i = 0; i < kFontRoleCount; ++i) {
        const auto role = static_cast<FontRole>(static_cast<uint8_t>(i));
        const auto& font = theme.Font(role);
        CHECK_MESSAGE(!font.family.isEmpty(),
                      "FontRole index ", i, " has empty family");
        CHECK(font.sizeInPt > 0);
        CHECK(font.weight > 0);
    }
}

TEST_CASE("ShadowSpec blur increases with elevation") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    const auto& flat = theme.Shadow(ElevationToken::Flat);
    const auto& low = theme.Shadow(ElevationToken::Low);
    const auto& med = theme.Shadow(ElevationToken::Medium);
    const auto& high = theme.Shadow(ElevationToken::High);

    CHECK(flat.blurRadius == 0);
    CHECK(low.blurRadius > flat.blurRadius);
    CHECK(med.blurRadius > low.blurRadius);
    CHECK(high.blurRadius > med.blurRadius);
    CHECK(flat.opacity == doctest::Approx(0.0));
    CHECK(low.opacity > 0.0);
}

TEST_CASE("AnimationMs returns default values") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    CHECK(theme.AnimationMs(AnimationToken::Instant) == 0);
    CHECK(theme.AnimationMs(AnimationToken::Quick) == 160);
    CHECK(theme.AnimationMs(AnimationToken::Normal) == 200);
    CHECK(theme.AnimationMs(AnimationToken::Slow) == 350);
}

TEST_CASE("SetAnimationOverride forces all to return override value") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    theme.SetAnimationOverride(0);
    CHECK(theme.AnimationMs(AnimationToken::Quick) == 0);
    CHECK(theme.AnimationMs(AnimationToken::Slow) == 0);

    theme.SetAnimationOverride(42);
    CHECK(theme.AnimationMs(AnimationToken::Normal) == 42);

    theme.SetAnimationOverride(-1); // restore
    CHECK(theme.AnimationMs(AnimationToken::Normal) == 200);
}

TEST_CASE("ResolveStyleSheet returns valid sheets") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    for (std::size_t i = 0; i < kWidgetKindCount; ++i) {
        const auto kind = static_cast<WidgetKind>(static_cast<uint8_t>(i));
        const auto& sheet = theme.ResolveStyleSheet(kind);
        CHECK_MESSAGE(!sheet.variants.empty(),
                      "WidgetKind index ", i, " has no variants");
    }
}

TEST_CASE("PushButton has 4 variants") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    const auto& sheet = theme.ResolveStyleSheet(WidgetKind::PushButton);
    CHECK(sheet.variants.size() == 4);
}

TEST_CASE("Dynamic token register, query, unregister") {
    matcha::test::QtAppGuard::Ensure();
    NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
    theme.SetTheme(kThemeLight);

    // Not registered yet
    CHECK(!theme.DynamicColor("Test/MyColor").has_value());

    // Register
    std::array defs = {
        IThemeService::DynamicColorDef {
            .key = "Test/MyColor",
            .lightValue = QColor(255, 0, 0),
            .darkValue = QColor(0, 0, 255),
        },
    };
    theme.RegisterDynamicTokens(defs);

    // Query in Light theme
    auto result = theme.DynamicColor("Test/MyColor");
    REQUIRE(result.has_value());
    CHECK(result->red() == 255);

    // Switch to Dark
    theme.SetTheme(kThemeDark);
    result = theme.DynamicColor("Test/MyColor");
    REQUIRE(result.has_value());
    CHECK(result->blue() == 255);

    // Unregister
    std::array<std::string_view, 1> keys = {"Test/MyColor"};
    theme.UnregisterDynamicTokens(keys);
    CHECK(!theme.DynamicColor("Test/MyColor").has_value());
}

} // TEST_SUITE

#ifdef __clang__
#pragma clang diagnostic pop
#endif
