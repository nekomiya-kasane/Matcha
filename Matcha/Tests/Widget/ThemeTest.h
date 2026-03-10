#pragma once

#include "WidgetTestFixture.h"

#include <Matcha/Widgets/Core/NyanTheme.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QSignalSpy>
#include <QTest>

#include <array>
#include <string_view>

extern matcha::test::WidgetTestFixture* gFixture;

// ============================================================================
// Mock ThemeAware subclass for testing callback delivery
// ============================================================================

class MockThemeAwareWidget : public matcha::gui::ThemeAware {
public:
    explicit MockThemeAwareWidget(matcha::gui::WidgetKind kind)
        : ThemeAware(kind)
    {
    }

    ~MockThemeAwareWidget() override = default;

    MockThemeAwareWidget(const MockThemeAwareWidget&)            = delete;
    MockThemeAwareWidget& operator=(const MockThemeAwareWidget&) = delete;
    MockThemeAwareWidget(MockThemeAwareWidget&&)                 = delete;
    MockThemeAwareWidget& operator=(MockThemeAwareWidget&&)      = delete;

    int callCount = 0;

    // Expose protected accessors for testing
    using ThemeAware::StyleSheet;
    using ThemeAware::Theme;

protected:
    void OnThemeChanged() override { ++callCount; }
};

// ============================================================================
// ThemeTest -- Qt Test class
// ============================================================================

class ThemeTest : public QObject {
    Q_OBJECT

private slots: // NOLINT(readability-redundant-access-specifiers)

    // -- S1: Theme switch updates colors --
    void themeSwitchUpdatesColors()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
        theme.SetTheme(kThemeLight);

        const auto lightBg0 = theme.Color(ColorToken::Surface);
        const auto lightFg1 = theme.Color(ColorToken::TextPrimary);

        theme.SetTheme(kThemeDark);

        const auto darkBg0 = theme.Color(ColorToken::Surface);
        const auto darkFg1 = theme.Color(ColorToken::TextPrimary);

        QVERIFY(lightBg0 != darkBg0);
        QVERIFY(lightFg1 != darkFg1);
    }

    // -- S1: Theme switch updates fonts --
    void themeSwitchUpdatesFont()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

        theme.SetTheme(kThemeLight);
        const auto& lightBody = theme.Font(FontRole::Body);
        QVERIFY(!lightBody.family.isEmpty());
        QVERIFY(lightBody.sizeInPt > 0);

        theme.SetTheme(kThemeDark);
        const auto& darkBody = theme.Font(FontRole::Body);
        QVERIFY(!darkBody.family.isEmpty());
        // Font specs should be identical across themes (platform fonts don't change)
        QCOMPARE(darkBody.family, lightBody.family);
        QCOMPARE(darkBody.sizeInPt, lightBody.sizeInPt);
    }

    // -- S1: Theme switch updates shadows --
    void themeSwitchUpdatesShadow()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

        theme.SetTheme(kThemeLight);
        const auto& lightHigh = theme.Shadow(ElevationToken::High);
        QVERIFY(lightHigh.blurRadius > 0);

        theme.SetTheme(kThemeDark);
        const auto& darkHigh = theme.Shadow(ElevationToken::High);
        QVERIFY(darkHigh.blurRadius > 0);
        // Shadow specs are theme-independent (same elevation = same blur)
        QCOMPARE(darkHigh.blurRadius, lightHigh.blurRadius);
    }

    // -- S1: ThemeChanged signal emitted --
    void themeChangedSignalEmitted()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

        QSignalSpy spy(&theme, &IThemeService::ThemeChanged);
        QVERIFY(spy.isValid());

        theme.SetTheme(kThemeLight);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<ThemeId>(), kThemeLight);

        theme.SetTheme(kThemeDark);
        QCOMPARE(spy.count(), 2);
        QCOMPARE(spy.at(1).at(0).value<ThemeId>(), kThemeDark);
    }

    // -- S2: ThemeAware receives callback --
    void themeAwareReceivesCallback()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
        theme.SetTheme(kThemeLight);
        SetThemeService(&theme);

        MockThemeAwareWidget mock(WidgetKind::PushButton);
        QCOMPARE(mock.callCount, 0);

        theme.SetTheme(kThemeDark);
        QCOMPARE(mock.callCount, 1);

        theme.SetTheme(kThemeLight);
        QCOMPARE(mock.callCount, 2);
    }

    // -- S2: ThemeAware StyleSheet refreshed --
    void themeAwareStyleSheetRefreshed()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
        theme.SetTheme(kThemeLight);
        SetThemeService(&theme);

        MockThemeAwareWidget mock(WidgetKind::PushButton);

        // StyleSheet should be populated (PushButton has 4 variants)
        QCOMPARE(mock.StyleSheet().variants.size(), std::size_t{4});

        theme.SetTheme(kThemeDark);
        // After theme switch, sheet is refreshed -- still 4 variants
        QCOMPARE(mock.StyleSheet().variants.size(), std::size_t{4});
    }

    // -- S3: Animation override forces zero --
    void animationOverrideForcesZero()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
        theme.SetTheme(kThemeLight);

        // Default values
        QCOMPARE(theme.AnimationMs(AnimationToken::Quick), 160);
        QCOMPARE(theme.AnimationMs(AnimationToken::Slow), 350);

        // Override to 0 (test mode)
        theme.SetAnimationOverride(0);
        QCOMPARE(theme.AnimationMs(AnimationToken::Quick), 0);
        QCOMPARE(theme.AnimationMs(AnimationToken::Normal), 0);
        QCOMPARE(theme.AnimationMs(AnimationToken::Slow), 0);

        // Restore
        theme.SetAnimationOverride(-1);
        QCOMPARE(theme.AnimationMs(AnimationToken::Quick), 160);
    }

    // -- S4: Component override applied --
    void componentOverrideApplied()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));

        // Register override: Dialog with radius=Small
        IThemeService::ComponentOverride ov;
        ov.kind   = WidgetKind::Dialog;
        ov.radius = RadiusToken::Small;
        std::array overrides = {ov};
        theme.RegisterComponentOverrides(overrides);

        // SetTheme applies overrides
        theme.SetTheme(kThemeLight);
        const auto& sheet = theme.ResolveStyleSheet(WidgetKind::Dialog);
        QCOMPARE(sheet.radius, RadiusToken::Small);
    }

    // -- S5: Dynamic token register + query --
    void dynamicTokenRegisterQuery()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
        theme.SetTheme(kThemeLight);

        // Not registered
        QVERIFY(!theme.DynamicColor("Test/Widget").has_value());

        // Register
        std::array defs = {
            IThemeService::DynamicColorDef {
                .key = "Test/Widget",
                .lightValue = QColor(10, 20, 30),
                .darkValue = QColor(30, 20, 10),
            },
        };
        theme.RegisterDynamicTokens(defs);

        auto result = theme.DynamicColor("Test/Widget");
        QVERIFY(result.has_value());
        QCOMPARE(result->red(), 10);

        // Switch theme -- dark value returned
        theme.SetTheme(kThemeDark);
        result = theme.DynamicColor("Test/Widget");
        QVERIFY(result.has_value());
        QCOMPARE(result->red(), 30);
    }

    // -- S5: Dynamic token unregister --
    void dynamicTokenUnregister()
    {
        using namespace matcha::gui;
        NyanTheme theme(QStringLiteral(MATCHA_TEST_PALETTE_DIR));
        theme.SetTheme(kThemeLight);

        std::array defs = {
            IThemeService::DynamicColorDef {
                .key = "Test/Remove",
                .lightValue = QColor(1, 2, 3),
                .darkValue = QColor(3, 2, 1),
            },
        };
        theme.RegisterDynamicTokens(defs);
        QVERIFY(theme.DynamicColor("Test/Remove").has_value());

        std::array<std::string_view, 1> keys = {"Test/Remove"};
        theme.UnregisterDynamicTokens(keys);
        QVERIFY(!theme.DynamicColor("Test/Remove").has_value());
    }
};
