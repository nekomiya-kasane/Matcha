#pragma once

/**
 * @file NyanTheme.h
 * @brief Concrete IThemeService implementation with JSON palette loading.
 *
 * `NyanTheme` is the single concrete implementation of `IThemeService`.
 * It loads Light.json / Dark.json seed palettes (71 ColorToken values verbatim),
 * detects platform fonts, derives shadow specs from elevation levels, and builds
 * per-WidgetKind style sheets with default geometry tokens + variant color matrices.
 *
 * Storage is flat arrays for O(1) lookup. All const query methods are lock-free
 * reads -- arrays are immutable between `SetTheme()` calls.
 *
 * @see 05_Greenfield_Plan.md ss 2.6 for the NyanTheme design.
 * @see IThemeService.h for the abstract interface.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/IThemeService.h>

#include <QColor>
#include <QPixmap>
#include <QString>

#include <array>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace matcha::gui {

/**
 * @brief Concrete theme engine implementation.
 *
 * **Lifecycle**:
 * 1. Construct with palette search path (directory containing Light.json / Dark.json).
 * 2. Call `SetTheme(kThemeLight)` to perform initial token build.
 * 3. Widgets query tokens via the `IThemeService` interface.
 * 4. Call `SetTheme(kThemeDark)` to switch; all ThemeAware widgets auto-update.
 *
 * **Thread safety**: All const query methods are safe for concurrent reads.
 * `SetTheme()` must be called from the GUI thread only (emits signal).
 */
class MATCHA_EXPORT NyanTheme : public IThemeService {
    Q_OBJECT

public:
    /**
     * @brief Construct with palette directory path.
     * @param palettePath Directory containing Light.json and Dark.json.
     * @param parent Optional QObject parent.
     */
    explicit NyanTheme(QString palettePath, QObject* parent = nullptr);

    ~NyanTheme() override = default;

    NyanTheme(const NyanTheme&)            = delete;
    NyanTheme& operator=(const NyanTheme&) = delete;
    NyanTheme(NyanTheme&&)                 = delete;
    NyanTheme& operator=(NyanTheme&&)      = delete;

    // -- ITokenRegistry interface implementation --

    void SetDensity(fw::DensityLevel level) override;
    [[nodiscard]] auto CurrentDensity() const -> fw::DensityLevel override;
    [[nodiscard]] auto CurrentDensityScale() const -> float override;
    void SetDirection(fw::TextDirection dir) override;
    [[nodiscard]] auto CurrentDirection() const -> fw::TextDirection override;
    [[nodiscard]] auto SpacingPx(fw::SpacingToken token) const -> int override;
    [[nodiscard]] auto Radius(fw::RadiusToken token) const -> int override;
    [[nodiscard]] auto AnimationMs(fw::AnimationToken speed) const -> int override;
    [[nodiscard]] auto TimingMs(fw::TimingTokenId id) const -> int override;
    void SetTimingOverride(fw::TimingTokenId id, int ms) override;

    // -- IThemeService interface implementation --

    void SetTheme(const QString& name) override;

    [[nodiscard]] auto CurrentTheme() const -> const QString& override;

    [[nodiscard]] auto CurrentMode() const -> ThemeMode override;

    auto RegisterTheme(const QString& name,
                       const QString& jsonPath,
                       ThemeMode mode) -> bool override;

    [[nodiscard]] auto Color(ColorToken token) const -> QColor override;
    [[nodiscard]] auto Color(ColorToken token,
                             InteractionState state) const -> QColor override;
    [[nodiscard]] auto Font(FontRole role) const -> const FontSpec& override;
    void SetFontScale(float factor) override;
    [[nodiscard]] auto FontScale() const -> float override;
    [[nodiscard]] auto Shadow(ElevationToken token) const -> const ShadowSpec& override;
    [[nodiscard]] auto Easing(EasingToken easing) const -> int override;

    [[nodiscard]] auto ResolveStyleSheet(WidgetKind kind) const
        -> const WidgetStyleSheet& override;

    [[nodiscard]] auto Resolve(WidgetKind kind,
                               std::size_t variantIndex,
                               InteractionState state) const -> ResolvedStyle override;

    [[nodiscard]] auto Resolve(WidgetKind kind,
                               std::size_t variantIndex,
                               InteractionState state,
                               const InstanceStyleOverride& instanceOverride) const -> ResolvedStyle override;

    void RegisterComponentOverrides(std::span<const ComponentOverride> overrides) override;

    [[nodiscard]] auto ResolveIcon(const fw::IconId& iconId,
                                   fw::IconSize size,
                                   QColor color) const -> QPixmap override;
    void InvalidateIconCache() override;
    auto RegisterIconDirectory(std::string_view uriPrefix,
                               const QString& dirPath) -> int override;

    void RegisterDynamicTokens(std::span<const DynamicColorDef> defs) override;
    void RegisterDynamicFonts(std::span<const DynamicFontDef> defs) override;
    void RegisterDynamicSpacings(std::span<const DynamicSpacingDef> defs) override;
    [[nodiscard]] auto DynamicColor(std::string_view key) const
        -> std::optional<QColor> override;
    [[nodiscard]] auto DynamicFont(std::string_view key) const
        -> std::optional<FontSpec> override;
    [[nodiscard]] auto DynamicSpacingPx(std::string_view key) const
        -> std::optional<int> override;
    void UnregisterDynamicTokens(std::span<const std::string_view> keys) override;

    void SetAnimationOverride(int forceMs) override;
    [[nodiscard]] auto Spring() const -> const fw::SpringSpec& override;

private:
    /// @brief Load a JSON palette file and populate the color array.
    void LoadPalette(const QString& themeName);

    /// @brief Detect platform fonts and populate the font array.
    void BuildFonts();

    /// @brief Derive shadow specs from elevation levels.
    void BuildShadows();

    /// @brief Build WidgetStyleSheet for each WidgetKind.
    void BuildStyleSheets();

    /// @brief Apply registered component overrides to style sheets.
    void ApplyComponentOverrides();

    /// @brief Build and apply a global QSS stylesheet from Design Tokens.
    void BuildGlobalStyleSheet();

    /// @brief Query OS platform for interaction timing values (Windows SPI).
    void QueryPlatformTimings();

    /// @brief Build default variant color map for a widget kind.
    void BuildDefaultVariants(WidgetKind kind);

    // -- Storage --
    QString           _palettePath;
    QString           _currentTheme = kThemeLight;
    ThemeMode         _currentMode  = ThemeMode::Light;
    fw::DensityLevel  _density      = fw::DensityLevel::Default;
    fw::TextDirection _direction    = fw::TextDirection::LTR;
    float             _fontScale    = 1.0F; ///< Global font scale factor
    int               _animOverride = -1; ///< -1 = use real values

    /// @brief Interaction timing token storage (§8.7). Initialized from
    /// kDefaultTimingMs, then patched with OS platform queries.
    std::array<int, fw::kTimingTokenCount> _timingMs = fw::kDefaultTimingMs;

    /// @brief Per-token overrides. -1 = no override (use _timingMs value).
    std::array<int, fw::kTimingTokenCount> _timingOverrides = []() consteval {
        std::array<int, fw::kTimingTokenCount> a {};
        for (auto& v : a) { v = -1; }
        return a;
    }();

    fw::SpringSpec    _springSpec {};     ///< Global default spring params (JSON-configurable)

    std::array<QColor, kColorTokenCount>          _colors {};
    std::array<FontSpec, kFontRoleCount>           _fonts {};
    std::array<ShadowSpec, kElevationTokenCount>   _shadows {};

    /// @brief Per-FontRole JSON overrides from palette (size, weight).
    /// Populated by LoadPalette(), consumed by BuildFonts().
    struct FontOverride {
        std::optional<int> sizeInPt;
        std::optional<int> weight;
    };
    std::array<FontOverride, kFontRoleCount> _fontOverrides {};
    std::array<WidgetStyleSheet, kWidgetKindCount> _sheets {};

    /// @brief Per-widget-kind variant storage (owned, sheets point into these).
    std::array<std::vector<VariantStyle>, kWidgetKindCount> _variantStorage {};

    /// @brief Registered component overrides (accumulated across calls).
    std::vector<ComponentOverride> _componentOverrides;

    /// @brief Dynamic color tokens keyed by string.
    struct DynamicColorEntry {
        QColor lightValue;
        QColor darkValue;
    };
    std::unordered_map<std::string, DynamicColorEntry> _dynamicColors;

    /// @brief Dynamic font tokens keyed by string.
    std::unordered_map<std::string, FontSpec> _dynamicFonts;

    /// @brief Dynamic spacing tokens keyed by string (base px, density-scaled on query).
    std::unordered_map<std::string, int> _dynamicSpacings;

    /// @brief Icon registry: URI -> filesystem path (auto-populated by RegisterIconDirectory).
    std::unordered_map<std::string, QString> _iconRegistry;

    /// @brief Icon cache: key = (iconId + sizePx + colorRgb + rtlFlip) -> QPixmap.
    struct IconCacheKey {
        std::string   iconId;
        int           sizePx;
        QRgb          colorRgb;
        bool          rtlFlip;

        auto operator==(const IconCacheKey&) const -> bool = default;
    };
    struct IconCacheKeyHash {
        auto operator()(const IconCacheKey& k) const noexcept -> std::size_t {
            constexpr std::size_t kGolden = 0x9e3779b9ULL;
            std::size_t h = std::hash<std::string>{}(k.iconId);
            h ^= std::hash<int>{}(k.sizePx) + kGolden + (h << 6U) + (h >> 2U);
            h ^= std::hash<uint32_t>{}(static_cast<uint32_t>(k.colorRgb))
                 + kGolden + (h << 6U) + (h >> 2U);
            h ^= std::hash<bool>{}(k.rtlFlip)
                 + kGolden + (h << 6U) + (h >> 2U);
            return h;
        }
    };
    mutable std::unordered_map<IconCacheKey, QPixmap, IconCacheKeyHash> _iconCache;
    mutable std::mutex _iconCacheMutex;

    /// @brief Registered theme entry: JSON path + light/dark classification.
    struct ThemeEntry {
        QString   jsonPath;
        ThemeMode mode;
    };
    /// @brief Registered themes keyed by name (includes built-in + user themes).
    std::unordered_map<std::string, ThemeEntry> _registeredThemes;
};

} // namespace matcha::gui
