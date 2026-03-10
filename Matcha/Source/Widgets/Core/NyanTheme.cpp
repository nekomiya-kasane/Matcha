/**
 * @file NyanTheme.cpp
 * @brief NyanTheme concrete implementation of IThemeService.
 */

#include <Matcha/Widgets/Core/ContrastChecker.h>
#include <Matcha/Widgets/Core/NyanTheme.h>
#include <Matcha/Widgets/Core/TonalPaletteGenerator.h>
#include "SvgIconProvider.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOperatingSystemVersion>

#include <cmath>
#include <optional>
#include <utility>

namespace matcha::gui {

// ============================================================================
// ColorToken name table (must match ColorToken enum order exactly)
// ============================================================================

namespace {

constexpr std::array<const char*, kColorTokenCount> kColorTokenNames = {
    // Neutral: Surface (5)
    "Surface", "SurfaceContainer", "SurfaceElevated", "SurfaceSunken", "Spotlight",
    // Neutral: Fill (4)
    "Fill", "FillHover", "FillActive", "FillMuted",
    // Neutral: Border (3)
    "BorderSubtle", "BorderDefault", "BorderStrong",
    // Neutral: Text (4)
    "TextPrimary", "TextSecondary", "TextTertiary", "TextDisabled",
    // Primary 10-step
    "PrimaryBg", "PrimaryBgHover", "PrimaryBorder", "PrimaryBorderHover", "PrimaryHover",
    "Primary", "PrimaryActive", "PrimaryTextHover", "PrimaryText", "PrimaryTextActive",
    // Success 10-step
    "SuccessBg", "SuccessBgHover", "SuccessBorder", "SuccessBorderHover", "SuccessHover",
    "Success", "SuccessActive", "SuccessTextHover", "SuccessText", "SuccessTextActive",
    // Warning 10-step
    "WarningBg", "WarningBgHover", "WarningBorder", "WarningBorderHover", "WarningHover",
    "Warning", "WarningActive", "WarningTextHover", "WarningText", "WarningTextActive",
    // Error 10-step
    "ErrorBg", "ErrorBgHover", "ErrorBorder", "ErrorBorderHover", "ErrorHover",
    "Error", "ErrorActive", "ErrorTextHover", "ErrorText", "ErrorTextActive",
    // Info 10-step
    "InfoBg", "InfoBgHover", "InfoBorder", "InfoBorderHover", "InfoHover",
    "Info", "InfoActive", "InfoTextHover", "InfoText", "InfoTextActive",
    // Special (8)
    "OnAccent", "OnAccentSecondary", "Focus", "Selection", "Link", "Scrim", "Overlay", "Shadow", "Separator",
};

// Index of first semantic hue token (PrimaryBg) in kColorTokenNames / ColorToken enum.
// 5 Surface + 4 Fill + 3 Border + 4 Text = 16
constexpr std::size_t kSemanticHueOffset = 16;

// Number of steps per hue (Ant Design 10-step model)
constexpr std::size_t kStepsPerHue = 10;

// Hue names in order matching the enum layout (5 hues x 10 levels)
constexpr std::array<const char*, 5> kHueNames = {
    "Primary", "Success", "Warning", "Error", "Info",
};

// FontRole JSON key table (must match FontRole enum order exactly)
constexpr std::array<const char*, kFontRoleCount> kFontRoleNames = {
    "Body", "BodyMedium", "BodyBold", "Caption", "Heading", "Monospace", "ToolTip",
};

} // anonymous namespace

// ============================================================================
// Constructor
// ============================================================================

NyanTheme::NyanTheme(QString palettePath, QObject* parent)
    : IThemeService(parent)
    , _palettePath(std::move(palettePath))
{
}

// ============================================================================
// Theme Lifecycle
// ============================================================================

void NyanTheme::SetTheme(const QString& name)
{
    _currentTheme = name;

    // Resolve mode: check registered themes, else default by name
    const auto regKey = name.toStdString();
    auto regIt = _registeredThemes.find(regKey);
    if (regIt != _registeredThemes.end()) {
        _currentMode = regIt->second.mode;
    } else if (name == kThemeDark) {
        _currentMode = ThemeMode::Dark;
    } else {
        _currentMode = ThemeMode::Light;
    }

    LoadPalette(name);
    BuildFonts();
    BuildShadows();
    BuildStyleSheets();
    ApplyComponentOverrides();
    InvalidateIconCache();

#ifndef NDEBUG
    // D2: Debug-build WCAG AA contrast validation for all widget kinds
    for (std::size_t k = 0; k < kWidgetKindCount; ++k) {
        const auto kind = static_cast<WidgetKind>(k);
        const auto& sheet = ResolveStyleSheet(kind);
        if (sheet.variants.empty()) {
            continue;
        }
        const auto& normal = sheet.variants[0].colors[std::to_underlying(
            fw::InteractionState::Normal)];
        const QColor fg = Color(normal.foreground);
        const QColor bg = Color(normal.background);
        const double ratio = ContrastChecker::Ratio(fg, bg);
        if (ratio < 4.5) {
            qWarning("A11y: WidgetKind(%zu) contrast ratio %.2f < 4.5:1 (AA)",
                     k, ratio);
        }
    }
#endif

    BuildGlobalStyleSheet();

    emit ThemeChanged(name);
}

auto NyanTheme::CurrentTheme() const -> const QString&
{
    return _currentTheme;
}

auto NyanTheme::CurrentMode() const -> ThemeMode
{
    return _currentMode;
}

auto NyanTheme::RegisterTheme(const QString& name,
                              const QString& jsonPath,
                              ThemeMode mode) -> bool
{
    if (name.isEmpty()) {
        return false;
    }
    if (!QFile::exists(jsonPath)) {
        return false;
    }
    _registeredThemes[name.toStdString()] = ThemeEntry {jsonPath, mode};
    return true;
}

// ============================================================================
// Palette Loading
// ============================================================================

void NyanTheme::LoadPalette(const QString& themeName)
{
    // Resolve the JSON file path for this theme
    QString filePath;
    const auto regKey = themeName.toStdString();
    const auto regIt = _registeredThemes.find(regKey);
    if (regIt != _registeredThemes.end()) {
        filePath = regIt->second.jsonPath;
    } else {
        // Built-in themes: look for <name>.json in palette directory
        const QString candidate = _palettePath + u'/' + themeName + QStringLiteral(".json");
        if (QFile::exists(candidate)) {
            filePath = candidate;
        } else {
            // Unknown theme — magenta fallback
            _colors.fill(QColor(255, 0, 255));
            return;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        _colors.fill(QColor(255, 0, 255));
        return;
    }

    const auto doc = QJsonDocument::fromJson(file.readAll());
    const auto root = doc.object();

    // Step 0: If "extends" is present, load the base theme first (one level only)
    const auto extendsName = root.value(QStringLiteral("extends")).toString();
    if (!extendsName.isEmpty()) {
        if (extendsName != themeName) {
            LoadPalette(extendsName);
            // _colors now holds the base theme; we overlay below
        }
    } else if (regIt != _registeredThemes.end()) {
        // Registered theme without "extends" starts from scratch
        _colors.fill(QColor(255, 0, 255));
    }

    // Step 1: Overlay explicit color entries (non-empty values only)
    const auto colors = root.value(QStringLiteral("colors")).toObject();
    if (!colors.isEmpty()) {
        for (std::size_t i = 0; i < kColorTokenCount; ++i) {
            const auto value = colors.value(QLatin1String(kColorTokenNames[i])).toString();
            if (!value.isEmpty()) {
                const QColor c = QColor::fromString(value);
                if (c.isValid()) {
                    _colors[i] = c;
                }
            } else if (extendsName.isEmpty()) {
                // No base theme and no value — magenta fallback for this slot
                _colors[i] = QColor(255, 0, 255);
            }
            // else: inherited from base (Step 0)
        }
    }

    // Step 2: If colorSeeds present, auto-generate the 36 semantic hue tokens
    const auto seeds = root.value(QStringLiteral("colorSeeds")).toObject();
    if (!seeds.isEmpty()) {
        for (std::size_t h = 0; h < kHueNames.size(); ++h) {
            const auto seedStr = seeds.value(QLatin1String(kHueNames[h])).toString();
            if (seedStr.isEmpty()) {
                continue;
            }

            const QColor seedColor = QColor::fromString(seedStr);
            if (!seedColor.isValid()) {
                continue;
            }

            const auto ramp = (_currentMode == ThemeMode::Light)
                ? TonalPaletteGenerator::GenerateLight(seedColor)
                : TonalPaletteGenerator::GenerateDark(seedColor);

            const std::size_t base = kSemanticHueOffset + (h * kStepsPerHue);
            for (std::size_t t = 0; t < kStepsPerHue; ++t) {
                _colors[base + t] = ramp[t];
            }
        }
    }

    // Step 3: Apply colorOverrides (individual token corrections after generation)
    const auto overrides = root.value(QStringLiteral("colorOverrides")).toObject();
    if (!overrides.isEmpty()) {
        for (std::size_t i = 0; i < kColorTokenCount; ++i) {
            const auto ovStr = overrides.value(QLatin1String(kColorTokenNames[i])).toString();
            if (!ovStr.isEmpty()) {
                const QColor ovColor = QColor::fromString(ovStr);
                if (ovColor.isValid()) {
                    _colors[i] = ovColor;
                }
            }
        }
    }

    // Step 4: Read optional fontScale from palette JSON
    if (root.contains(QStringLiteral("fontScale"))) {
        const auto fs = static_cast<float>(root.value(QStringLiteral("fontScale")).toDouble(1.0));
        _fontScale = std::clamp(fs, fw::kFontScaleMin, fw::kFontScaleMax);
    }

    // Step 5: Read optional spring dynamics parameters
    // JSON format: "spring": { "mass": 1.0, "stiffness": 200, "damping": 20 }
    const auto springObj = root.value(QStringLiteral("spring")).toObject();
    if (!springObj.isEmpty()) {
        if (springObj.contains(QStringLiteral("mass"))) {
            _springSpec.mass = static_cast<float>(springObj.value(QStringLiteral("mass")).toDouble(1.0));
        }
        if (springObj.contains(QStringLiteral("stiffness"))) {
            _springSpec.stiffness = static_cast<float>(springObj.value(QStringLiteral("stiffness")).toDouble(200.0));
        }
        if (springObj.contains(QStringLiteral("damping"))) {
            _springSpec.damping = static_cast<float>(springObj.value(QStringLiteral("damping")).toDouble(20.0));
        }
    }

    // Step 6: Read optional per-FontRole overrides (size, weight)
    // JSON format: "fonts": { "Body": { "size": 10, "weight": 500 }, ... }
    _fontOverrides = {}; // reset before overlay
    const auto fontsObj = root.value(QStringLiteral("fonts")).toObject();
    if (!fontsObj.isEmpty()) {
        for (std::size_t i = 0; i < kFontRoleCount; ++i) {
            const auto roleObj = fontsObj.value(QLatin1String(kFontRoleNames[i])).toObject();
            if (roleObj.isEmpty()) {
                continue;
            }
            if (roleObj.contains(QStringLiteral("size"))) {
                _fontOverrides[i].sizeInPt = roleObj.value(QStringLiteral("size")).toInt();
            }
            if (roleObj.contains(QStringLiteral("weight"))) {
                _fontOverrides[i].weight = roleObj.value(QStringLiteral("weight")).toInt();
            }
        }
    }
}

// ============================================================================
// Font Detection
// ============================================================================

void NyanTheme::BuildFonts()
{
    // Detect platform font family
    QString sansFamily;
    QString monoFamily;

#if defined(Q_OS_WIN)
    sansFamily = QStringLiteral("Segoe UI");
    monoFamily = QStringLiteral("Cascadia Code");
#elif defined(Q_OS_MACOS)
    sansFamily = QStringLiteral("SF Pro Text");
    monoFamily = QStringLiteral("SF Mono");
#else
    sansFamily = QStringLiteral("Noto Sans");
    monoFamily = QStringLiteral("Noto Sans Mono");
#endif

    // Verify family availability, fallback to system default
    const auto families = QFontDatabase::families();
    if (!families.contains(sansFamily)) {
        sansFamily = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
    }
    if (!families.contains(monoFamily)) {
        monoFamily = QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
    }

    // Helper: scale a base pt size by _fontScale, clamp to >= 6pt
    auto scaledPt = [this](int basePt) -> int {
        const int scaled = static_cast<int>(
            std::lroundf(static_cast<float>(basePt) * _fontScale));
        return std::max(scaled, 6); // minimum 6pt for readability
    };

    // Body: 9pt Normal
    _fonts[std::to_underlying(FontRole::Body)] = {
        .family = sansFamily, .sizeInPt = scaledPt(9), .weight = 400,
    };

    // BodyMedium: 9pt Medium
    _fonts[std::to_underlying(FontRole::BodyMedium)] = {
        .family = sansFamily, .sizeInPt = scaledPt(9), .weight = 500,
    };

    // BodyBold: 9pt Bold
    _fonts[std::to_underlying(FontRole::BodyBold)] = {
        .family = sansFamily, .sizeInPt = scaledPt(9), .weight = 700,
    };

    // Caption: 8pt Normal
    _fonts[std::to_underlying(FontRole::Caption)] = {
        .family = sansFamily, .sizeInPt = scaledPt(8), .weight = 400,
    };

    // Heading: 12pt DemiBold
    _fonts[std::to_underlying(FontRole::Heading)] = {
        .family = sansFamily, .sizeInPt = scaledPt(12), .weight = 600,
    };

    // Monospace: 9pt Normal mono
    _fonts[std::to_underlying(FontRole::Monospace)] = {
        .family = monoFamily, .sizeInPt = scaledPt(9), .weight = 400,
    };

    // ToolTip: 8pt Normal
    _fonts[std::to_underlying(FontRole::ToolTip)] = {
        .family = sansFamily, .sizeInPt = scaledPt(8), .weight = 400,
    };

    // Apply per-FontRole JSON overrides (size, weight) from palette
    for (std::size_t i = 0; i < kFontRoleCount; ++i) {
        const auto& ov = _fontOverrides[i];
        if (ov.sizeInPt) {
            _fonts[i].sizeInPt = scaledPt(*ov.sizeInPt);
        }
        if (ov.weight) {
            _fonts[i].weight = *ov.weight;
        }
    }
}

// ============================================================================
// Shadow Derivation
// ============================================================================

void NyanTheme::BuildShadows()
{
    // Flat: no shadow
    _shadows[std::to_underlying(ElevationToken::Flat)] = {
        .offsetX = 0, .offsetY = 0, .blurRadius = 0, .opacity = 0.0,
    };

    // Low: 2px blur, 6% opacity
    _shadows[std::to_underlying(ElevationToken::Low)] = {
        .offsetX = 0, .offsetY = 1, .blurRadius = 2, .opacity = 0.06,
    };

    // Medium: 4px blur, 10% opacity
    _shadows[std::to_underlying(ElevationToken::Medium)] = {
        .offsetX = 0, .offsetY = 2, .blurRadius = 4, .opacity = 0.10,
    };

    // High: 8px blur, 15% opacity
    _shadows[std::to_underlying(ElevationToken::High)] = {
        .offsetX = 0, .offsetY = 4, .blurRadius = 8, .opacity = 0.15,
    };

    // Window: 8px margin + multi-pass blur
    _shadows[std::to_underlying(ElevationToken::Window)] = {
        .offsetX = 0, .offsetY = 4, .blurRadius = 16, .opacity = 0.20,
    };
}

// ============================================================================
// WidgetStyleSheet Assembly
// ============================================================================

void NyanTheme::BuildDefaultVariants(WidgetKind kind)
{
    const auto idx = static_cast<std::size_t>(std::to_underlying(kind));

    // Default single-variant: Normal state uses neutral palette
    VariantStyle defaultVariant {};
    auto& c = defaultVariant.colors;

    c[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Surface,     ColorToken::TextPrimary,  ColorToken::BorderSubtle};
    c[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,   ColorToken::TextPrimary,  ColorToken::BorderDefault};
    c[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::FillActive,  ColorToken::TextPrimary,  ColorToken::BorderStrong};
    c[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceElevated, ColorToken::TextDisabled, ColorToken::BorderSubtle};
    c[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Surface,     ColorToken::TextPrimary,  ColorToken::Focus};
    c[std::to_underlying(InteractionState::Selected)] = {ColorToken::PrimaryBgHover, ColorToken::Primary,   ColorToken::Primary};
    c[std::to_underlying(InteractionState::Error)]    = {ColorToken::ErrorBgHover, ColorToken::Error,       ColorToken::Error};
    c[std::to_underlying(InteractionState::DragOver)] = {ColorToken::PrimaryBg,   ColorToken::Primary,     ColorToken::PrimaryHover};

    // Apply disabled state opacity and cursor defaults
    c[std::to_underlying(InteractionState::Disabled)].opacity = 0.45F;
    c[std::to_underlying(InteractionState::Disabled)].cursor  = fw::CursorToken::Forbidden;
    for (auto s : {InteractionState::Normal, InteractionState::Hovered, InteractionState::Pressed,
                   InteractionState::Focused, InteractionState::Selected}) {
        c[std::to_underlying(s)].cursor = fw::CursorToken::Pointer;
    }

    _variantStorage[idx] = {defaultVariant};

    // PushButton: 4 variants (Primary, Secondary, Ghost, Danger)
    if (kind == WidgetKind::PushButton) {
        VariantStyle primary {};
        primary.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Primary,        ColorToken::OnAccent, ColorToken::Primary};
        primary.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::PrimaryHover,   ColorToken::OnAccent, ColorToken::PrimaryHover};
        primary.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::PrimaryActive,  ColorToken::OnAccent, ColorToken::PrimaryActive};
        primary.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::PrimaryBorder,  ColorToken::TextDisabled, ColorToken::PrimaryBorder};
        primary.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Primary,        ColorToken::OnAccent, ColorToken::Focus};
        primary.colors[std::to_underlying(InteractionState::Selected)] = {ColorToken::Primary,        ColorToken::OnAccent, ColorToken::Primary};
        primary.colors[std::to_underlying(InteractionState::Error)]    = {ColorToken::Error,          ColorToken::OnAccent, ColorToken::Error};

        VariantStyle secondary {};
        secondary.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Surface,        ColorToken::Primary,      ColorToken::BorderDefault};
        secondary.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::PrimaryBgHover, ColorToken::PrimaryHover, ColorToken::PrimaryHover};
        secondary.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::PrimaryBgHover, ColorToken::PrimaryActive, ColorToken::PrimaryActive};
        secondary.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceElevated, ColorToken::TextDisabled, ColorToken::BorderSubtle};
        secondary.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Surface,        ColorToken::Primary,      ColorToken::Focus};
        secondary.colors[std::to_underlying(InteractionState::Selected)] = {ColorToken::PrimaryBgHover, ColorToken::Primary,      ColorToken::Primary};
        secondary.colors[std::to_underlying(InteractionState::Error)]    = {ColorToken::ErrorBgHover,   ColorToken::Error,        ColorToken::Error};

        VariantStyle ghost {};
        ghost.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Surface,    ColorToken::TextPrimary,  ColorToken::Surface};
        ghost.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,  ColorToken::TextPrimary,  ColorToken::FillHover};
        ghost.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::FillActive, ColorToken::TextPrimary,  ColorToken::FillActive};
        ghost.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::Surface,    ColorToken::TextDisabled,  ColorToken::Surface};
        ghost.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Surface,    ColorToken::TextPrimary,  ColorToken::Focus};
        ghost.colors[std::to_underlying(InteractionState::Selected)] = {ColorToken::PrimaryBgHover, ColorToken::Primary,  ColorToken::PrimaryBgHover};
        ghost.colors[std::to_underlying(InteractionState::Error)]    = {ColorToken::ErrorBgHover,   ColorToken::Error,    ColorToken::ErrorBgHover};

        VariantStyle danger {};
        danger.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Error,        ColorToken::OnAccent, ColorToken::Error};
        danger.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::ErrorHover,   ColorToken::OnAccent, ColorToken::ErrorHover};
        danger.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::ErrorActive,  ColorToken::OnAccent, ColorToken::ErrorActive};
        danger.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::ErrorBorder,  ColorToken::TextDisabled, ColorToken::ErrorBorder};
        danger.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Error,        ColorToken::OnAccent, ColorToken::Focus};
        danger.colors[std::to_underlying(InteractionState::Selected)] = {ColorToken::Error,        ColorToken::OnAccent, ColorToken::Error};
        danger.colors[std::to_underlying(InteractionState::Error)]    = {ColorToken::Error,        ColorToken::OnAccent, ColorToken::Error};

        _variantStorage[idx] = {primary, secondary, ghost, danger};
    }

    // CheckBox: 3 variants (Unchecked=0, Checked=1, Partial=2)
    if (kind == WidgetKind::CheckBox) {
        VariantStyle unchecked {};
        unchecked.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::SurfaceElevated, ColorToken::TextSecondary, ColorToken::BorderStrong};
        unchecked.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::SurfaceElevated, ColorToken::TextSecondary, ColorToken::PrimaryHover};
        unchecked.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::SurfaceElevated, ColorToken::TextSecondary, ColorToken::PrimaryActive};
        unchecked.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceElevated, ColorToken::TextDisabled,  ColorToken::BorderDefault, 0.45F};
        unchecked.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::SurfaceElevated, ColorToken::TextSecondary, ColorToken::Focus};

        VariantStyle checked {};
        checked.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Primary,       ColorToken::OnAccent, ColorToken::Primary};
        checked.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::PrimaryHover,  ColorToken::OnAccent, ColorToken::PrimaryHover};
        checked.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::PrimaryActive, ColorToken::OnAccent, ColorToken::PrimaryActive};
        checked.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::PrimaryBorder, ColorToken::TextDisabled, ColorToken::PrimaryBorder, 0.45F};
        checked.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Primary,       ColorToken::OnAccent, ColorToken::Focus};

        VariantStyle partial {};
        partial.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::SurfaceElevated, ColorToken::PrimaryActive, ColorToken::BorderStrong};
        partial.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::SurfaceElevated, ColorToken::PrimaryHover,  ColorToken::PrimaryHover};
        partial.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::SurfaceElevated, ColorToken::PrimaryActive, ColorToken::PrimaryActive};
        partial.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceElevated, ColorToken::TextDisabled,  ColorToken::BorderDefault, 0.45F};
        partial.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::SurfaceElevated, ColorToken::PrimaryActive, ColorToken::Focus};

        _variantStorage[idx] = {unchecked, checked, partial};
    }

    // RadioButton: 2 variants (Unchecked=0, Checked=1)
    if (kind == WidgetKind::RadioButton) {
        VariantStyle unchecked {};
        unchecked.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::SurfaceElevated, ColorToken::TextSecondary, ColorToken::BorderStrong};
        unchecked.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::SurfaceElevated, ColorToken::TextSecondary, ColorToken::PrimaryHover};
        unchecked.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceElevated, ColorToken::TextDisabled,  ColorToken::BorderDefault, 0.45F};
        unchecked.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::SurfaceElevated, ColorToken::TextSecondary, ColorToken::Focus};

        VariantStyle checked {};
        checked.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Primary,       ColorToken::OnAccent, ColorToken::Primary};
        checked.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::PrimaryHover,  ColorToken::OnAccent, ColorToken::PrimaryHover};
        checked.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::PrimaryBorder, ColorToken::TextDisabled, ColorToken::PrimaryBorder, 0.45F};
        checked.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Primary,       ColorToken::OnAccent, ColorToken::Focus};

        _variantStorage[idx] = {unchecked, checked};
    }

    // Toggle: 2 variants (Off=0, On=1)
    if (kind == WidgetKind::Toggle) {
        VariantStyle off {};
        off.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Fill,        ColorToken::SurfaceElevated, ColorToken::BorderStrong};
        off.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,   ColorToken::SurfaceElevated, ColorToken::BorderStrong};
        off.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::FillActive,  ColorToken::SurfaceElevated, ColorToken::BorderStrong};
        off.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::Fill,        ColorToken::FillActive,      ColorToken::BorderDefault, 0.45F};

        VariantStyle on {};
        on.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Primary,       ColorToken::OnAccent, ColorToken::Primary};
        on.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::PrimaryHover,  ColorToken::OnAccent, ColorToken::PrimaryHover};
        on.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::PrimaryActive, ColorToken::OnAccent, ColorToken::PrimaryActive};
        on.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::PrimaryBorder, ColorToken::TextDisabled, ColorToken::PrimaryBorder, 0.45F};

        _variantStorage[idx] = {off, on};
    }

    // ToolButton: 2 variants (Default=0, Active=1)
    if (kind == WidgetKind::ToolButton) {
        VariantStyle def {};
        def.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Surface,    ColorToken::TextPrimary, ColorToken::Surface};
        def.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,  ColorToken::TextPrimary, ColorToken::FillHover};
        def.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::FillActive, ColorToken::TextPrimary, ColorToken::FillActive};
        def.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::Surface,    ColorToken::TextDisabled, ColorToken::Surface, 0.45F};
        def.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Surface,    ColorToken::TextPrimary, ColorToken::Focus};

        VariantStyle active {};
        active.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::PrimaryBg,      ColorToken::Primary,      ColorToken::PrimaryBg};
        active.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::PrimaryBgHover, ColorToken::PrimaryHover, ColorToken::PrimaryBgHover};
        active.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::PrimaryBgHover, ColorToken::PrimaryActive, ColorToken::PrimaryBgHover};
        active.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::Surface,        ColorToken::TextDisabled,  ColorToken::Surface, 0.45F};
        active.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::PrimaryBg,      ColorToken::Primary,      ColorToken::Focus};

        _variantStorage[idx] = {def, active};
    }

    // LineEdit: 1 variant with focused=Primary border
    if (kind == WidgetKind::LineEdit || kind == WidgetKind::SpinBox) {
        VariantStyle input {};
        input.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Surface,         ColorToken::TextPrimary,  ColorToken::BorderDefault};
        input.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::Surface,         ColorToken::TextPrimary,  ColorToken::BorderStrong};
        input.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Surface,         ColorToken::TextPrimary,  ColorToken::Primary};
        input.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceElevated, ColorToken::TextDisabled,  ColorToken::BorderSubtle, 0.45F};
        input.colors[std::to_underlying(InteractionState::Error)]    = {ColorToken::ErrorBg,         ColorToken::TextPrimary,  ColorToken::Error};

        _variantStorage[idx] = {input};
    }

    // ComboBox: 1 variant with focused=Primary accent
    if (kind == WidgetKind::ComboBox) {
        VariantStyle combo {};
        combo.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::FillHover,       ColorToken::TextPrimary,   ColorToken::BorderDefault};
        combo.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,       ColorToken::TextPrimary,   ColorToken::BorderStrong};
        combo.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::SurfaceElevated, ColorToken::Primary,       ColorToken::PrimaryActive};
        combo.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::Fill,            ColorToken::TextDisabled,   ColorToken::BorderDefault, 0.45F};
        combo.colors[std::to_underlying(InteractionState::Error)]    = {ColorToken::ErrorBg,         ColorToken::TextPrimary,   ColorToken::Error};

        _variantStorage[idx] = {combo};
    }

    // TabWidget: 2 variants (Inactive=0, Active=1)
    if (kind == WidgetKind::TabWidget) {
        VariantStyle inactive {};
        inactive.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::SurfaceContainer, ColorToken::TextSecondary, ColorToken::BorderSubtle};
        inactive.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,        ColorToken::TextPrimary,   ColorToken::BorderDefault};
        inactive.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::FillActive,       ColorToken::TextPrimary,   ColorToken::BorderDefault};
        inactive.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceContainer, ColorToken::TextDisabled,  ColorToken::BorderSubtle, 0.45F};

        VariantStyle active {};
        active.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Surface,     ColorToken::Primary,      ColorToken::Primary};
        active.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::Surface,     ColorToken::PrimaryHover, ColorToken::PrimaryHover};
        active.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::Surface,     ColorToken::PrimaryActive, ColorToken::PrimaryActive};
        active.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::Surface,     ColorToken::TextDisabled,  ColorToken::BorderSubtle, 0.45F};

        _variantStorage[idx] = {inactive, active};
    }

    // DataTable: 3 variants (Default=0, Selected=1, Striped=2)
    if (kind == WidgetKind::DataTable) {
        VariantStyle def {};
        def.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Surface,          ColorToken::TextPrimary,  ColorToken::BorderSubtle};
        def.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,        ColorToken::TextPrimary,  ColorToken::BorderSubtle};
        def.colors[std::to_underlying(InteractionState::Selected)] = {ColorToken::PrimaryBg,        ColorToken::TextPrimary,  ColorToken::PrimaryBorder};
        def.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::SurfaceElevated,  ColorToken::TextDisabled,  ColorToken::BorderSubtle, 0.45F};

        VariantStyle selected {};
        selected.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::PrimaryBg,      ColorToken::Primary,      ColorToken::PrimaryBorder};
        selected.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::PrimaryBgHover, ColorToken::PrimaryHover, ColorToken::PrimaryBorderHover};
        selected.colors[std::to_underlying(InteractionState::Selected)] = {ColorToken::PrimaryBgHover, ColorToken::Primary,      ColorToken::Primary};

        VariantStyle striped {};
        striped.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::SurfaceContainer, ColorToken::TextPrimary,  ColorToken::BorderSubtle};
        striped.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::FillHover,        ColorToken::TextPrimary,  ColorToken::BorderSubtle};
        striped.colors[std::to_underlying(InteractionState::Selected)] = {ColorToken::PrimaryBg,        ColorToken::TextPrimary,  ColorToken::PrimaryBorder};

        _variantStorage[idx] = {def, selected, striped};
    }

    // ProgressBar: 3 variants (Primary=0, Success=1, Error=2)
    if (kind == WidgetKind::ProgressBar) {
        VariantStyle primary {};
        primary.colors[std::to_underlying(InteractionState::Normal)] = {ColorToken::Fill, ColorToken::Primary, ColorToken::BorderSubtle};

        VariantStyle success {};
        success.colors[std::to_underlying(InteractionState::Normal)] = {ColorToken::Fill, ColorToken::Success, ColorToken::BorderSubtle};

        VariantStyle error {};
        error.colors[std::to_underlying(InteractionState::Normal)] = {ColorToken::Fill, ColorToken::Error, ColorToken::BorderSubtle};

        _variantStorage[idx] = {primary, success, error};
    }

    // Slider: 1 variant (track=bg, thumb=fg, border=track border)
    if (kind == WidgetKind::Slider) {
        VariantStyle slider {};
        slider.colors[std::to_underlying(InteractionState::Normal)]   = {ColorToken::Fill,          ColorToken::Primary,      ColorToken::BorderDefault};
        slider.colors[std::to_underlying(InteractionState::Hovered)]  = {ColorToken::Fill,          ColorToken::PrimaryHover, ColorToken::BorderDefault};
        slider.colors[std::to_underlying(InteractionState::Pressed)]  = {ColorToken::Fill,          ColorToken::PrimaryActive, ColorToken::BorderDefault};
        slider.colors[std::to_underlying(InteractionState::Disabled)] = {ColorToken::Fill,          ColorToken::BorderDefault, ColorToken::BorderSubtle, 0.45F};
        slider.colors[std::to_underlying(InteractionState::Focused)]  = {ColorToken::Fill,          ColorToken::Primary,      ColorToken::Focus};

        _variantStorage[idx] = {slider};
    }
}

void NyanTheme::BuildStyleSheets()
{
    for (std::size_t i = 0; i < kWidgetKindCount; ++i) {
        const auto kind = static_cast<WidgetKind>(static_cast<uint8_t>(i));

        // Build variant color maps
        BuildDefaultVariants(kind);

        // Default geometry tokens
        _sheets[i] = WidgetStyleSheet {
            .radius    = RadiusToken::Default,
            .paddingH  = SpacingToken::Px4,
            .paddingV  = SpacingToken::Px4,
            .font      = FontRole::Body,
            .elevation = ElevationToken::Flat,
            .transition = {},
            .variants  = _variantStorage[i],
        };
    }

    // Per-kind geometry overrides (from architecture spec)
    auto& dialog = _sheets[std::to_underlying(WidgetKind::Dialog)];
    dialog.radius    = RadiusToken::Large;
    dialog.elevation = ElevationToken::High;

    auto& tooltip = _sheets[std::to_underlying(WidgetKind::Tooltip)];
    tooltip.radius    = RadiusToken::Small;
    tooltip.elevation = ElevationToken::High;
    tooltip.font      = FontRole::ToolTip;

    auto& actionBar = _sheets[std::to_underlying(WidgetKind::ActionBar)];
    actionBar.font    = FontRole::Caption;
    actionBar.paddingH = SpacingToken::Px2;
    actionBar.paddingV = SpacingToken::Px2;

    auto& contextMenu = _sheets[std::to_underlying(WidgetKind::ContextMenu)];
    contextMenu.elevation = ElevationToken::High;
    contextMenu.radius    = RadiusToken::Default;

    auto& popConfirm = _sheets[std::to_underlying(WidgetKind::PopConfirm)];
    popConfirm.elevation = ElevationToken::High;
    popConfirm.radius    = RadiusToken::Large;

    auto& comboBox = _sheets[std::to_underlying(WidgetKind::ComboBox)];
    comboBox.paddingH = SpacingToken::Px6;
    comboBox.paddingV = SpacingToken::Px6;

    auto& groupBox = _sheets[std::to_underlying(WidgetKind::GroupBox)];
    groupBox.radius    = RadiusToken::Large;
    groupBox.elevation = ElevationToken::Medium;

    auto& panel = _sheets[std::to_underlying(WidgetKind::Panel)];
    panel.elevation = ElevationToken::Low;

    auto& toolButton = _sheets[std::to_underlying(WidgetKind::ToolButton)];
    toolButton.font = FontRole::Caption;

    auto& statusBar = _sheets[std::to_underlying(WidgetKind::StatusBar)];
    statusBar.paddingH = SpacingToken::Px2;
    statusBar.paddingV = SpacingToken::Px2;
    statusBar.font    = FontRole::Caption;
}

// ============================================================================
// Component Override Application
// ============================================================================

void NyanTheme::ApplyComponentOverrides()
{
    for (const auto& ov : _componentOverrides) {
        auto& sheet = _sheets[std::to_underlying(ov.kind)]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        if (ov.radius) {
            sheet.radius = *ov.radius;
        }
        if (ov.padding) {
            sheet.paddingH = *ov.padding;
            sheet.paddingV = *ov.padding;
        }
        if (ov.font) {
            sheet.font = *ov.font;
        }
        if (ov.elevation) {
            sheet.elevation = *ov.elevation;
        }
        if (ov.animation) {
            sheet.transition.duration = *ov.animation;
        }
    }
}

// ============================================================================
// ITokenRegistry Implementation
// ============================================================================

void NyanTheme::SetDensity(fw::DensityLevel level)
{
    if (_density == level) {
        return;
    }
    _density = level;
    BuildGlobalStyleSheet();
    emit ThemeChanged(_currentTheme);
}

auto NyanTheme::CurrentDensity() const -> fw::DensityLevel
{
    return _density;
}

auto NyanTheme::CurrentDensityScale() const -> float
{
    return fw::DensityScale(_density);
}

void NyanTheme::SetDirection(fw::TextDirection dir)
{
    if (_direction == dir) {
        return;
    }
    _direction = dir;
    InvalidateIconCache();
    emit ThemeChanged(_currentTheme);
}

auto NyanTheme::CurrentDirection() const -> fw::TextDirection
{
    return _direction;
}

// ============================================================================
// Global Token Queries
// ============================================================================

auto NyanTheme::Color(ColorToken token) const -> QColor
{
    const auto idx = std::to_underlying(token);
    if (idx >= kColorTokenCount) return QColor(255, 0, 255);
    return _colors[idx];
}

auto NyanTheme::Color(ColorToken token, InteractionState /*state*/) const -> QColor
{
    // For now, state-adjusted colors delegate to the base color.
    // Full state-adjusted color derivation (brightness shift) is Phase 3+ work.
    return Color(token);
}

auto NyanTheme::Font(FontRole role) const -> const FontSpec&
{
    const auto idx = std::to_underlying(role);
    if (idx >= kFontRoleCount) return _fonts[0];
    return _fonts[idx];
}

void NyanTheme::SetFontScale(float factor)
{
    factor = std::clamp(factor, fw::kFontScaleMin, fw::kFontScaleMax);
    if (std::abs(factor - _fontScale) < 1e-5F) {
        return; // no change
    }
    _fontScale = factor;
    BuildFonts();
    emit ThemeChanged(_currentTheme);
}

auto NyanTheme::FontScale() const -> float
{
    return _fontScale;
}

auto NyanTheme::SpacingPx(fw::SpacingToken token) const -> int
{
    const int basePx = fw::ToPixels(token);
    const float scale = CurrentDensityScale();
    return static_cast<int>(std::lroundf(static_cast<float>(basePx) * scale));
}

auto NyanTheme::Radius(fw::RadiusToken token) const -> int
{
    return fw::ToPixels(token);
}

auto NyanTheme::Shadow(ElevationToken token) const -> const ShadowSpec&
{
    const auto idx = std::to_underlying(token);
    if (idx >= kElevationTokenCount) return _shadows[0];
    return _shadows[idx];
}

auto NyanTheme::AnimationMs(fw::AnimationToken speed) const -> int
{
    if (_animOverride >= 0) return _animOverride;
    const auto idx = std::to_underlying(speed);
    if (idx >= fw::kAnimationTokenCount) return 0;
    return fw::kDefaultAnimationMs[idx];
}

auto NyanTheme::ResolveStyleSheet(WidgetKind kind) const -> const WidgetStyleSheet&
{
    const auto idx = std::to_underlying(kind);
    if (idx >= kWidgetKindCount) return _sheets[0];
    return _sheets[idx];
}

// ============================================================================
// Declarative Style Resolution (RFC-07)
// ============================================================================

auto NyanTheme::Resolve(WidgetKind kind,
                        std::size_t variantIndex,
                        InteractionState state) const -> ResolvedStyle
{
    const auto& sheet = ResolveStyleSheet(kind);
    const float scale = CurrentDensityScale();

    // Resolve colors from variant/state matrix
    StateStyle ss; // default if out of range
    if (!sheet.variants.empty()) {
        const auto vi = std::min(variantIndex, sheet.variants.size() - 1);
        const auto si = static_cast<std::size_t>(std::to_underlying(state));
        if (si < kInteractionStateCount) {
            ss = sheet.variants[vi].colors[si];
        }
    }

    // Build resolved font with letterSpacing applied
    const auto& fontSpec = Font(sheet.font);
    QFont resolvedFont(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
    if (fontSpec.letterSpacing != 0.0) {
        resolvedFont.setLetterSpacing(QFont::AbsoluteSpacing, fontSpec.letterSpacing);
    }

    // Compute line height in pixels from font metrics and multiplier
    const QFontMetrics fm(resolvedFont);
    const int lineHeightPx = static_cast<int>(
        std::lround(static_cast<double>(fm.height()) * fontSpec.lineHeightMultiplier));

    // Resolve easing
    const int easingType = Easing(sheet.transition.easing);

    return ResolvedStyle {
        .background    = Color(ss.background),
        .foreground    = Color(ss.foreground),
        .border        = Color(ss.border),
        .radiusPx      = static_cast<int>(std::lroundf(static_cast<float>(Radius(sheet.radius)) * scale)),
        .paddingHPx    = SpacingPx(sheet.paddingH),
        .paddingVPx    = SpacingPx(sheet.paddingV),
        .gapPx         = SpacingPx(sheet.gap),
        .minHeightPx   = static_cast<int>(static_cast<float>(fw::ToPixels(sheet.minHeight)) * scale),
        .borderWidthPx = SpacingPx(ss.borderWidth),
        .font          = resolvedFont,
        .lineHeightPx  = lineHeightPx,
        .shadow        = [&]() {
            auto s = Shadow(sheet.elevation);
            s.offsetY   = static_cast<int>(std::lroundf(static_cast<float>(s.offsetY) * scale));
            s.blurRadius = static_cast<int>(std::lroundf(static_cast<float>(s.blurRadius) * scale));
            return s;
        }(),
        .opacity       = ss.opacity,
        .durationMs    = AnimationMs(sheet.transition.duration),
        .easingType    = easingType,
    };
}

// ============================================================================
// Component Overrides
// ============================================================================

void NyanTheme::RegisterComponentOverrides(std::span<const ComponentOverride> overrides)
{
    _componentOverrides.insert(_componentOverrides.end(),
                               overrides.begin(), overrides.end());
}

// ============================================================================
// Dynamic Tokens
// ============================================================================

void NyanTheme::RegisterDynamicTokens(std::span<const DynamicColorDef> defs)
{
    for (const auto& def : defs) {
        _dynamicColors[std::string(def.key)] = DynamicColorEntry {
            .lightValue = def.lightValue,
            .darkValue  = def.darkValue,
        };
    }
}

void NyanTheme::RegisterDynamicFonts(std::span<const DynamicFontDef> defs)
{
    for (const auto& def : defs) {
        _dynamicFonts[std::string(def.key)] = def.value;
    }
}

void NyanTheme::RegisterDynamicSpacings(std::span<const DynamicSpacingDef> defs)
{
    for (const auto& def : defs) {
        _dynamicSpacings[std::string(def.key)] = def.basePx;
    }
}

auto NyanTheme::DynamicColor(std::string_view key) const -> std::optional<QColor>
{
    const auto it = _dynamicColors.find(std::string(key));
    if (it == _dynamicColors.end()) {
        return std::nullopt;
    }
    return (_currentMode == ThemeMode::Light) ? it->second.lightValue : it->second.darkValue;
}

auto NyanTheme::DynamicFont(std::string_view key) const -> std::optional<FontSpec>
{
    const auto it = _dynamicFonts.find(std::string(key));
    if (it == _dynamicFonts.end()) {
        return std::nullopt;
    }
    // Apply global font scale so plugin fonts scale consistently with core fonts
    FontSpec result = it->second;
    const int scaled = static_cast<int>(
        std::lroundf(static_cast<float>(result.sizeInPt) * _fontScale));
    result.sizeInPt = std::max(scaled, 6);
    return result;
}

auto NyanTheme::DynamicSpacingPx(std::string_view key) const -> std::optional<int>
{
    const auto it = _dynamicSpacings.find(std::string(key));
    if (it == _dynamicSpacings.end()) {
        return std::nullopt;
    }
    const float scale = fw::DensityScale(_density);
    return static_cast<int>(std::lroundf(static_cast<float>(it->second) * scale));
}

void NyanTheme::UnregisterDynamicTokens(std::span<const std::string_view> keys)
{
    for (const auto& key : keys) {
        auto k = std::string(key);
        _dynamicColors.erase(k);
        _dynamicFonts.erase(k);
        _dynamicSpacings.erase(k);
    }
}

// ============================================================================
// Icon Resolution (asset:// URI)
// ============================================================================

auto NyanTheme::RegisterIconDirectory(std::string_view uriPrefix,
                                       const QString& dirPath) -> int
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        return 0;
    }
    int count = 0;
    const auto entries = dir.entryInfoList(QStringList {QStringLiteral("*.svg")},
                                           QDir::Files);
    for (const auto& entry : entries) {
        std::string uri(uriPrefix);
        uri += entry.baseName().toStdString();
        _iconRegistry[std::move(uri)] = entry.absoluteFilePath();
        ++count;
    }
    return count;
}

auto NyanTheme::ResolveIcon(const fw::IconId& iconId,
                            fw::IconSize size,
                            QColor color) const -> QPixmap
{
    if (iconId.empty()) {
        return {};
    }

    // Default colorization: TextPrimary
    if (!color.isValid()) {
        color = Color(ColorToken::TextPrimary);
    }

    const int basePx = static_cast<int>(std::to_underlying(size));
    const float scale = fw::DensityScale(_density);
    const int sizePx = static_cast<int>(std::lroundf(static_cast<float>(basePx) * scale));

    const bool rtlFlip = (_direction == fw::TextDirection::RTL
                          && fw::IsRtlFlippable(iconId));
    const IconCacheKey key {iconId, sizePx, color.rgba(), rtlFlip};

    {
        std::lock_guard lock(_iconCacheMutex);
        auto it = _iconCache.find(key);
        if (it != _iconCache.end()) {
            return it->second;
        }
    }

    // Lookup filesystem path from registry
    auto regIt = _iconRegistry.find(iconId);
    if (regIt == _iconRegistry.end()) {
        return {};
    }

    QPixmap pixmap = detail::LoadAndColorizeSvg(regIt->second, sizePx, color);

    // RTL: horizontally mirror directional icons
    if (rtlFlip && !pixmap.isNull()) {
        pixmap = pixmap.transformed(QTransform().scale(-1, 1));
    }

    {
        std::lock_guard lock(_iconCacheMutex);
        _iconCache[key] = pixmap;
    }

    return pixmap;
}

void NyanTheme::InvalidateIconCache()
{
    std::lock_guard lock(_iconCacheMutex);
    _iconCache.clear();
}

// ============================================================================
// Test Support
// ============================================================================

void NyanTheme::SetAnimationOverride(int forceMs)
{
    _animOverride = forceMs;
}

auto NyanTheme::Spring() const -> const fw::SpringSpec&
{
    return _springSpec;
}

auto NyanTheme::Easing(EasingToken easing) const -> int
{
    // Map EasingToken to QEasingCurve::Type integer values
    switch (easing) {
    case EasingToken::Linear:     return 0;  // QEasingCurve::Linear
    case EasingToken::OutCubic:   return 4;  // QEasingCurve::OutCubic
    case EasingToken::InOutCubic: return 5;  // QEasingCurve::InOutCubic
    case EasingToken::Spring:     return 44; // QEasingCurve::OutElastic (spring approx)
    default:                      return 4;  // OutCubic fallback (includes Count_)
    }
}

// ============================================================================
// Global QSS Stylesheet (Design Token driven)
// ============================================================================

void NyanTheme::BuildGlobalStyleSheet()
{
    if (QApplication::instance() == nullptr) {
        return;
    }

    // Helper: QColor -> "#RRGGBB"
    auto c = [this](ColorToken t) -> QString { return Color(t).name(QColor::HexRgb); };

    const int radius = Radius(RadiusToken::Default);
    const int radiusLg = Radius(RadiusToken::Large);
    const auto& bodyFont = Font(FontRole::Body);
    const auto& captionFont = Font(FontRole::Caption);

    // Build one large stylesheet string from Design Tokens
    QString qss;
    qss.reserve(8192);

    // -- QWidget base --
    qss += QStringLiteral(
        "QWidget { font-family: '%1'; font-size: %2pt; color: %3; }\n")
        .arg(bodyFont.family, QString::number(bodyFont.sizeInPt), c(ColorToken::TextPrimary));

    // -- QPushButton --
    qss += QStringLiteral(
        "QPushButton {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: %4px; padding: 4px 12px;"
        "}\n"
        "QPushButton:hover {"
        "  background-color: %5; border-color: %6;"
        "}\n"
        "QPushButton:pressed {"
        "  background-color: %7;"
        "}\n"
        "QPushButton:disabled {"
        "  background-color: %8; color: %9; border-color: %10;"
        "}\n")
        .arg(c(ColorToken::Fill), c(ColorToken::TextPrimary), c(ColorToken::BorderDefault),
             QString::number(radius),
             c(ColorToken::FillHover), c(ColorToken::BorderStrong),
             c(ColorToken::FillActive),
             c(ColorToken::FillMuted), c(ColorToken::TextDisabled), c(ColorToken::BorderSubtle));

    // -- QLineEdit --
    qss += QStringLiteral(
        "QLineEdit {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: %4px; padding: 3px 6px;"
        "  selection-background-color: %5; selection-color: %6;"
        "}\n"
        "QLineEdit:focus {"
        "  border-color: %7;"
        "}\n"
        "QLineEdit:disabled {"
        "  background-color: %8; color: %9;"
        "}\n")
        .arg(c(ColorToken::SurfaceElevated), c(ColorToken::TextPrimary), c(ColorToken::BorderDefault),
             QString::number(radius),
             c(ColorToken::Selection), c(ColorToken::OnAccent),
             c(ColorToken::Primary),
             c(ColorToken::FillMuted), c(ColorToken::TextDisabled));

    // -- QTextEdit / QPlainTextEdit --
    qss += QStringLiteral(
        "QTextEdit, QPlainTextEdit {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: %4px; selection-background-color: %5;"
        "}\n"
        "QTextEdit:focus, QPlainTextEdit:focus {"
        "  border-color: %6;"
        "}\n")
        .arg(c(ColorToken::SurfaceElevated), c(ColorToken::TextPrimary), c(ColorToken::BorderDefault),
             QString::number(radius), c(ColorToken::Selection), c(ColorToken::Primary));

    // -- QSpinBox / QDoubleSpinBox --
    qss += QStringLiteral(
        "QSpinBox, QDoubleSpinBox {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: %4px; padding: 2px 4px;"
        "}\n"
        "QSpinBox:focus, QDoubleSpinBox:focus {"
        "  border-color: %5;"
        "}\n"
        "QSpinBox::up-button, QDoubleSpinBox::up-button,"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "  background-color: %6; border: none; width: 16px;"
        "}\n"
        "QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,"
        "QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {"
        "  background-color: %7;"
        "}\n"
        "QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {"
        "  image: none; border-left: 4px solid transparent; border-right: 4px solid transparent;"
        "  border-bottom: 5px solid %8; width: 0; height: 0;"
        "}\n"
        "QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {"
        "  image: none; border-left: 4px solid transparent; border-right: 4px solid transparent;"
        "  border-top: 5px solid %8; width: 0; height: 0;"
        "}\n"
        "QSpinBox:disabled, QDoubleSpinBox:disabled {"
        "  background-color: %9; color: %10;"
        "}\n")
        .arg(c(ColorToken::SurfaceElevated), c(ColorToken::TextPrimary), c(ColorToken::BorderDefault),
             QString::number(radius), c(ColorToken::Primary),
             c(ColorToken::Fill), c(ColorToken::FillHover),
             c(ColorToken::TextSecondary),
             c(ColorToken::FillMuted), c(ColorToken::TextDisabled));

    // -- QComboBox --
    qss += QStringLiteral(
        "QComboBox {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: %4px; padding: 3px 8px 3px 6px;"
        "}\n"
        "QComboBox:hover { border-color: %5; }\n"
        "QComboBox:focus { border-color: %6; }\n"
        "QComboBox::drop-down {"
        "  border: none; width: 20px; background: transparent;"
        "}\n"
        "QComboBox::down-arrow {"
        "  image: none; border-left: 4px solid transparent; border-right: 4px solid transparent;"
        "  border-top: 5px solid %7; width: 0; height: 0;"
        "}\n"
        "QComboBox QAbstractItemView {"
        "  background-color: %8; color: %2; border: 1px solid %3;"
        "  selection-background-color: %9; selection-color: %10;"
        "  outline: none;"
        "}\n"
        "QComboBox:disabled { background-color: %11; color: %12; }\n")
        .arg(c(ColorToken::SurfaceElevated), c(ColorToken::TextPrimary), c(ColorToken::BorderDefault),
             QString::number(radius), c(ColorToken::BorderStrong), c(ColorToken::Primary),
             c(ColorToken::TextSecondary),
             c(ColorToken::SurfaceElevated),
             c(ColorToken::PrimaryBg), c(ColorToken::TextPrimary),
             c(ColorToken::FillMuted), c(ColorToken::TextDisabled));

    // -- QCheckBox --
    qss += QStringLiteral(
        "QCheckBox { spacing: 6px; color: %1; }\n"
        "QCheckBox::indicator {"
        "  width: 16px; height: 16px; border-radius: 3px;"
        "  border: 1px solid %2; background-color: %3;"
        "}\n"
        "QCheckBox::indicator:hover { border-color: %4; }\n"
        "QCheckBox::indicator:checked {"
        "  background-color: %5; border-color: %5;"
        "}\n"
        "QCheckBox::indicator:checked:hover {"
        "  background-color: %6; border-color: %6;"
        "}\n"
        "QCheckBox:disabled { color: %7; }\n"
        "QCheckBox::indicator:disabled {"
        "  background-color: %8; border-color: %9;"
        "}\n")
        .arg(c(ColorToken::TextPrimary),
             c(ColorToken::BorderDefault), c(ColorToken::SurfaceElevated),
             c(ColorToken::Primary),
             c(ColorToken::Primary), c(ColorToken::PrimaryHover),
             c(ColorToken::TextDisabled),
             c(ColorToken::FillMuted), c(ColorToken::BorderSubtle));

    // -- QRadioButton --
    qss += QStringLiteral(
        "QRadioButton { spacing: 6px; color: %1; }\n"
        "QRadioButton::indicator {"
        "  width: 16px; height: 16px; border-radius: 8px;"
        "  border: 1px solid %2; background-color: %3;"
        "}\n"
        "QRadioButton::indicator:hover { border-color: %4; }\n"
        "QRadioButton::indicator:checked {"
        "  background-color: %5; border-color: %5;"
        "}\n"
        "QRadioButton:disabled { color: %6; }\n"
        "QRadioButton::indicator:disabled {"
        "  background-color: %7; border-color: %8;"
        "}\n")
        .arg(c(ColorToken::TextPrimary),
             c(ColorToken::BorderDefault), c(ColorToken::SurfaceElevated),
             c(ColorToken::Primary),
             c(ColorToken::Primary),
             c(ColorToken::TextDisabled),
             c(ColorToken::FillMuted), c(ColorToken::BorderSubtle));

    // -- QSlider (horizontal) --
    qss += QStringLiteral(
        "QSlider::groove:horizontal {"
        "  height: 4px; background: %1; border-radius: 2px;"
        "}\n"
        "QSlider::handle:horizontal {"
        "  width: 14px; height: 14px; margin: -5px 0;"
        "  background: %2; border: 2px solid %3; border-radius: 7px;"
        "}\n"
        "QSlider::handle:horizontal:hover {"
        "  background: %4; border-color: %5;"
        "}\n"
        "QSlider::sub-page:horizontal { background: %3; border-radius: 2px; }\n"
        "QSlider::add-page:horizontal { background: %1; border-radius: 2px; }\n"
        "QSlider:disabled::groove:horizontal { background: %6; }\n"
        "QSlider:disabled::handle:horizontal {"
        "  background: %7; border-color: %6;"
        "}\n")
        .arg(c(ColorToken::BorderDefault),
             c(ColorToken::SurfaceElevated), c(ColorToken::Primary),
             c(ColorToken::PrimaryBgHover), c(ColorToken::PrimaryHover),
             c(ColorToken::FillMuted), c(ColorToken::FillMuted));

    // -- QSlider (vertical) --
    qss += QStringLiteral(
        "QSlider::groove:vertical {"
        "  width: 4px; background: %1; border-radius: 2px;"
        "}\n"
        "QSlider::handle:vertical {"
        "  width: 14px; height: 14px; margin: 0 -5px;"
        "  background: %2; border: 2px solid %3; border-radius: 7px;"
        "}\n"
        "QSlider::handle:vertical:hover {"
        "  background: %4; border-color: %5;"
        "}\n")
        .arg(c(ColorToken::BorderDefault),
             c(ColorToken::SurfaceElevated), c(ColorToken::Primary),
             c(ColorToken::PrimaryBgHover), c(ColorToken::PrimaryHover));

    // QScrollBar: no global QSS — NyanScrollBar uses custom paintEvent.
    // Widgets that need themed scrollbars should install NyanScrollBar explicitly.

    // -- QScrollArea --
    qss += QStringLiteral(
        "QScrollArea { background: transparent; border: none; }\n");

    // -- QTabWidget / QTabBar --
    qss += QStringLiteral(
        "QTabWidget::pane {"
        "  border: 1px solid %1; background: %2;"
        "  border-radius: %3px;"
        "}\n"
        "QTabBar::tab {"
        "  background: %4; color: %5; border: 1px solid %1;"
        "  padding: 5px 12px; margin-right: 1px;"
        "  border-top-left-radius: %3px; border-top-right-radius: %3px;"
        "}\n"
        "QTabBar::tab:selected {"
        "  background: %2; color: %6; border-bottom-color: %2;"
        "}\n"
        "QTabBar::tab:hover:!selected {"
        "  background: %7;"
        "}\n")
        .arg(c(ColorToken::BorderDefault), c(ColorToken::SurfaceElevated),
             QString::number(radius),
             c(ColorToken::SurfaceSunken), c(ColorToken::TextSecondary),
             c(ColorToken::TextPrimary), c(ColorToken::FillHover));

    // -- QGroupBox --
    qss += QStringLiteral(
        "QGroupBox {"
        "  border: 1px solid %1; border-radius: %2px;"
        "  margin-top: 8px; padding-top: 12px;"
        "  background: transparent;"
        "}\n"
        "QGroupBox::title {"
        "  subcontrol-origin: margin; subcontrol-position: top left;"
        "  padding: 0 4px; color: %3; font-weight: bold;"
        "}\n")
        .arg(c(ColorToken::BorderDefault), QString::number(radius),
             c(ColorToken::TextPrimary));

    // -- QProgressBar --
    qss += QStringLiteral(
        "QProgressBar {"
        "  background: %1; border: 1px solid %2; border-radius: %3px;"
        "  text-align: center; color: %4; min-height: 18px;"
        "}\n"
        "QProgressBar::chunk {"
        "  background: %5; border-radius: %3px;"
        "}\n")
        .arg(c(ColorToken::FillMuted), c(ColorToken::BorderDefault),
             QString::number(radius),
             c(ColorToken::TextPrimary), c(ColorToken::Primary));

    // -- QTreeView / QListView / QTableView --
    qss += QStringLiteral(
        "QTreeView, QListView, QTableView {"
        "  background-color: %1; color: %2;"
        "  border: 1px solid %3; border-radius: %4px;"
        "  selection-background-color: %5; selection-color: %6;"
        "  outline: none;"
        "}\n"
        "QTreeView::item:hover, QListView::item:hover, QTableView::item:hover {"
        "  background-color: %7;"
        "}\n"
        "QTreeView::item:selected, QListView::item:selected, QTableView::item:selected {"
        "  background-color: %5; color: %6;"
        "}\n"
        "QTreeView::branch { background: transparent; }\n"
        "QHeaderView::section {"
        "  background-color: %8; color: %2; border: none;"
        "  border-bottom: 1px solid %3; padding: 4px 8px;"
        "  font-weight: 600;"
        "}\n"
        "QHeaderView::section:hover { background-color: %9; }\n")
        .arg(c(ColorToken::SurfaceContainer), c(ColorToken::TextPrimary),
             c(ColorToken::BorderDefault), QString::number(radius),
             c(ColorToken::PrimaryBg), c(ColorToken::PrimaryText),
             c(ColorToken::FillHover),
             c(ColorToken::SurfaceSunken), c(ColorToken::FillHover));

    // -- QToolTip --
    qss += QStringLiteral(
        "QToolTip {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: %4px; padding: 4px 8px;"
        "  font-size: %5pt;"
        "}\n")
        .arg(c(ColorToken::Spotlight), c(ColorToken::OnAccent), c(ColorToken::BorderDefault),
             QString::number(radius), QString::number(captionFont.sizeInPt));

    // -- QMenu --
    qss += QStringLiteral(
        "QMenu {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: %4px; padding: 4px 0;"
        "}\n"
        "QMenu::item {"
        "  padding: 5px 28px 5px 12px;"
        "}\n"
        "QMenu::item:selected {"
        "  background-color: %5; color: %6;"
        "}\n"
        "QMenu::item:disabled { color: %7; }\n"
        "QMenu::separator {"
        "  height: 1px; background: %3; margin: 4px 8px;"
        "}\n")
        .arg(c(ColorToken::SurfaceElevated), c(ColorToken::TextPrimary),
             c(ColorToken::BorderDefault), QString::number(radiusLg),
             c(ColorToken::PrimaryBg), c(ColorToken::TextPrimary),
             c(ColorToken::TextDisabled));

    // -- QMenuBar --
    qss += QStringLiteral(
        "QMenuBar {"
        "  background: transparent; color: %1; border: none;"
        "}\n"
        "QMenuBar::item { padding: 4px 8px; background: transparent; }\n"
        "QMenuBar::item:selected { background-color: %2; }\n")
        .arg(c(ColorToken::TextPrimary), c(ColorToken::FillHover));

    // -- QSplitter --
    qss += QStringLiteral(
        "QSplitter::handle { background: %1; }\n"
        "QSplitter::handle:horizontal { width: 2px; }\n"
        "QSplitter::handle:vertical { height: 2px; }\n"
        "QSplitter::handle:hover { background: %2; }\n")
        .arg(c(ColorToken::BorderSubtle), c(ColorToken::Primary));

    // -- QStatusBar --
    qss += QStringLiteral(
        "QStatusBar {"
        "  background: %1; color: %2; border-top: 1px solid %3;"
        "}\n"
        "QStatusBar::item { border: none; }\n")
        .arg(c(ColorToken::SurfaceSunken), c(ColorToken::TextSecondary),
             c(ColorToken::BorderSubtle));

    // -- QLabel (just inherits base font/color; no border/bg) --
    qss += QStringLiteral(
        "QLabel { background: transparent; }\n");

    // -- QToolButton --
    qss += QStringLiteral(
        "QToolButton {"
        "  background: transparent; color: %1; border: none;"
        "  border-radius: %2px; padding: 3px;"
        "}\n"
        "QToolButton:hover { background: %3; }\n"
        "QToolButton:pressed { background: %4; }\n"
        "QToolButton:disabled { color: %5; }\n")
        .arg(c(ColorToken::TextPrimary), QString::number(radius),
             c(ColorToken::FillHover), c(ColorToken::FillActive),
             c(ColorToken::TextDisabled));

    // -- QDialog --
    qss += QStringLiteral(
        "QDialog { background: %1; }\n")
        .arg(c(ColorToken::SurfaceContainer));

    // -- QFrame --
    qss += QStringLiteral(
        "QFrame[frameShape=\"4\"], QFrame[frameShape=\"5\"] {"
        "  color: %1;"
        "}\n")
        .arg(c(ColorToken::Separator));

    // Apply the complete stylesheet to the application
    qApp->setStyleSheet(qss);
}

} // namespace matcha::gui
