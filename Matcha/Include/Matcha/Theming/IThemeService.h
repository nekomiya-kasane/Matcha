#pragma once

/**
 * @file IThemeService.h
 * @brief Abstract theme service interface for the Design Token System.
 *
 * `IThemeService` inherits `fw::ITokenRegistry` (Qt-free token queries) and
 * extends it with Qt-dependent capabilities:
 * - Theme lifecycle (`SetTheme` / `CurrentTheme`)
 * - Color queries (resolved to QColor)
 * - Font queries (resolved to FontSpec with QString)
 * - Shadow queries (ShadowSpec)
 * - Easing queries (QEasingCurve::Type)
 * - Per-widget composite style sheets (`ResolveStyleSheet`)
 * - Component token overrides for per-widget-class deviations
 * - Dynamic token registration for plugin extension (ADR-015)
 * - Animation override for deterministic test execution
 *
 * `NyanTheme` provides the concrete implementation of both interfaces.
 *
 * @see ITokenRegistry.h for the Qt-free base interface.
 * @see DesignTokens.h for token types.
 * @see WidgetStyleSheet.h for WidgetKind and WidgetStyleSheet.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/Token/ITokenRegistry.h>
#include <Matcha/Theming/DesignTokens.h>
#include <Matcha/Theming/ResolvedStyle.h>
#include <Matcha/Theming/WidgetStyleSheet.h>

#include <QColor>
#include <QObject>
#include <QPixmap>
#include <QString>

#include <optional>
#include <span>
#include <string_view>

namespace matcha::gui {

// ============================================================================
// ThemeId  (string-based, supports unlimited user themes)
// ============================================================================

/// @brief Theme identifier — a plain QString name (e.g. "Light", "MyCustomDark").
using ThemeId = QString;

/// @brief Built-in theme name constants.
inline const QString kThemeLight         = QStringLiteral("Light");
inline const QString kThemeDark          = QStringLiteral("Dark");
inline const QString kThemeHighContrast  = QStringLiteral("HighContrast");

/**
 * @brief System-level light/dark classification for a theme.
 *
 * Every theme belongs to either the Light or Dark family. This controls:
 * - `colorSeeds` tonal palette generation direction
 * - `DynamicColor` light/dark branch selection
 * - OS dark-mode integration (system theme auto-switch)
 */
enum class ThemeMode : uint8_t {
    Light = 0,
    Dark  = 1,
};

// ============================================================================
// IThemeService
// ============================================================================

/**
 * @brief Abstract interface for the Design Token System theme engine.
 *
 * All query methods are `[[nodiscard]]` and `const` -- they perform O(1) flat
 * array lookups into the token storage built by `SetTheme()`.
 *
 * Widgets access tokens via `IThemeService&` (obtained from `ThemeAware` mixin
 * or directly from the application shell).
 *
 * **Thread safety**: All const query methods are safe for concurrent reads.
 * `SetTheme()` must be called from the GUI thread only.
 */
class MATCHA_EXPORT IThemeService : public QObject, public fw::ITokenRegistry {
    Q_OBJECT

public:
    ~IThemeService() override = default;

    IThemeService(const IThemeService&)            = delete;
    IThemeService& operator=(const IThemeService&) = delete;
    IThemeService(IThemeService&&)                 = delete;
    IThemeService& operator=(IThemeService&&)      = delete;

    // ITokenRegistry::SpacingPx, Radius, AnimationMs, SetDensity, SetDirection
    // are inherited and must be implemented by NyanTheme.

    // ========================================================================
    // Theme Lifecycle
    // ========================================================================

    /**
     * @brief Switch to the specified theme.
     *
     * Rebuilds all token arrays (colors, fonts, shadows, style sheets) and
     * emits `ThemeChanged(name)`. Must be called from the GUI thread.
     *
     * @param name Theme name (must be a built-in or registered theme).
     */
    virtual void SetTheme(const QString& name) = 0;

    /**
     * @brief Query the currently active theme name.
     * @return Active theme name.
     */
    [[nodiscard]] virtual auto CurrentTheme() const -> const QString& = 0;

    /**
     * @brief Query the ThemeMode (Light/Dark) of the currently active theme.
     * @return ThemeMode of the active theme.
     */
    [[nodiscard]] virtual auto CurrentMode() const -> ThemeMode = 0;

    /**
     * @brief Register a custom theme backed by a JSON palette file.
     *
     * The JSON file may contain an `"extends"` key naming any registered
     * theme. When `SetTheme(name)` is called, the base theme is loaded
     * first, then the custom JSON's values are overlaid.
     *
     * There is no limit on the number of custom themes.
     *
     * @param name     Unique theme name (e.g. "OceanBlue"). Must not be empty.
     * @param jsonPath Absolute path to the palette JSON file.
     * @param mode     Light or Dark family classification.
     * @return true if registration succeeded, false if jsonPath does not exist.
     */
    virtual auto RegisterTheme(const QString& name,
                               const QString& jsonPath,
                               ThemeMode mode) -> bool = 0;

    // ========================================================================
    // Qt-Dependent Token Queries (Color, Font, Shadow, Easing)
    // SpacingPx, Radius, AnimationMs are inherited from ITokenRegistry.
    // ========================================================================

    /**
     * @brief Resolve a semantic color token to a concrete QColor.
     * @param token Semantic color slot.
     * @return Resolved color for the current theme.
     */
    [[nodiscard]] virtual auto Color(ColorToken token) const -> QColor = 0;

    /**
     * @brief Resolve a color token with interaction state overlay.
     *
     * @deprecated This overload currently delegates to `Color(token)` without
     * applying any state-dependent brightness/alpha shift. For correct
     * per-state colors, use `Resolve(kind, variantIndex, state)` which
     * reads from the WidgetStyleSheet variant/state color matrix.
     *
     * @param token Base semantic color slot.
     * @param state Widget interaction state (currently unused).
     * @return Base color for the current theme (state parameter is ignored).
     *
     * @see Resolve() for the recommended declarative style API.
     */
    [[nodiscard]] virtual auto Color(ColorToken token,
                                     InteractionState state) const -> QColor = 0;

    /**
     * @brief Query a font specification by role.
     * @param role Semantic font role.
     * @return Platform-specific FontSpec (family, size, weight, etc.).
     */
    [[nodiscard]] virtual auto Font(FontRole role) const -> const FontSpec& = 0;

    /**
     * @brief Set a global font scale factor applied to all FontRole base sizes.
     *
     * The factor is clamped to [kFontScaleMin, kFontScaleMax].
     * Triggers a font rebuild and emits ThemeChanged.
     *
     * @param factor Scale multiplier (1.0 = no scaling).
     */
    virtual void SetFontScale(float factor) = 0;

    /**
     * @brief Get the current global font scale factor.
     * @return Font scale factor (default 1.0).
     */
    [[nodiscard]] virtual auto FontScale() const -> float = 0;

    /**
     * @brief Convenience: set font scale from a preset.
     *
     * Equivalent to `SetFontScale(FontSizePresetScale(preset))`.
     */
    virtual void SetFontSizePreset(fw::FontSizePreset preset)
    {
        SetFontScale(fw::FontSizePresetScale(preset));
    }

    /**
     * @brief Query box shadow parameters by elevation level.
     * @param token Elevation token.
     * @return Shadow specification (offset, blur, opacity).
     */
    [[nodiscard]] virtual auto Shadow(ElevationToken token) const -> const ShadowSpec& = 0;

    /**
     * @brief Query an easing curve type by token.
     * @param easing Easing preset.
     * @return QEasingCurve::Type suitable for QPropertyAnimation.
     */
    [[nodiscard]] virtual auto Easing(EasingToken easing) const -> int = 0;

    /**
     * @brief Query the global default spring dynamics parameters.
     *
     * Used by AnimationService when EasingToken::Spring is selected
     * without an explicit SpringSpec override. Can be customized per
     * theme via the `"spring"` JSON object in the palette file.
     *
     * @return SpringSpec with mass, stiffness, damping values.
     */
    [[nodiscard]] virtual auto Spring() const -> const fw::SpringSpec& = 0;

    // ========================================================================
    // Icon Resolution (asset:// URI)
    // ========================================================================

    /**
     * @brief Resolve an icon URI to a rasterized QPixmap.
     *
     * The returned pixmap is theme-colorized (currentColor replaced with
     * the appropriate theme color), DPI-aware, and density-scaled.
     * Results are cached internally; repeated calls for the same
     * (iconId, size, color) triple return the cached pixmap.
     *
     * @param iconId Icon URI (e.g. "asset://matcha/icons/save"). Empty = null pixmap.
     * @param size   Target icon size.
     * @param color  Override colorization color. If invalid, uses TextPrimary.
     * @return Rasterized pixmap, or null QPixmap if iconId is empty or not found.
     */
    [[nodiscard]] virtual auto ResolveIcon(const fw::IconId& iconId,
                                           fw::IconSize size,
                                           QColor color) const -> QPixmap = 0;

    /// @brief Convenience overload using default theme foreground color.
    [[nodiscard]] auto ResolveIcon(const fw::IconId& iconId,
                                   fw::IconSize size) const -> QPixmap
    {
        return ResolveIcon(iconId, size, QColor {});
    }

    /**
     * @brief Register an icon directory for auto-scanning.
     *
     * All `.svg` files in the directory are registered under the given URI
     * prefix. For example, RegisterIconDirectory("asset://matcha/icons/", "/path/to/Icons")
     * registers `/path/to/Icons/save.svg` as `"asset://matcha/icons/save"`.
     *
     * @param uriPrefix URI prefix (e.g. "asset://matcha/icons/").
     * @param dirPath   Absolute filesystem path to the icon directory.
     * @return Number of icons registered.
     */
    virtual auto RegisterIconDirectory(std::string_view uriPrefix,
                                       const QString& dirPath) -> int = 0;

    /**
     * @brief Invalidate the icon cache (called internally on ThemeChanged).
     */
    virtual void InvalidateIconCache() = 0;

    // ========================================================================
    // Per-Widget Composite Style Sheet
    // ========================================================================

    /**
     * @brief Resolve the composite style sheet for a widget kind.
     *
     * Returns a reference to the pre-built `WidgetStyleSheet` containing
     * geometry tokens and variant color maps. This is the primary API for
     * widget `paintEvent` implementations.
     *
     * @param kind Widget type identifier.
     * @return Const reference to the style sheet (valid until next `SetTheme()`).
     */
    [[nodiscard]] virtual auto ResolveStyleSheet(WidgetKind kind) const
        -> const WidgetStyleSheet& = 0;

    // ========================================================================
    // Declarative Style Resolution (RFC-07)
    // ========================================================================

    /**
     * @brief Resolve a fully concrete style for a widget at a specific variant and state.
     *
     * This is the primary API for RFC-07 declarative widget painting. Returns a
     * `ResolvedStyle` with all tokens resolved to concrete Qt values: QColor,
     * density-scaled px, QFont, ShadowSpec, and transition parameters.
     *
     * Widgets call this once in paintEvent instead of multiple `Color()`/`Font()` calls.
     *
     * @param kind         Widget type.
     * @param variantIndex Index into `WidgetStyleSheet::variants` (0 for single-variant widgets).
     * @param state        Current interaction state.
     * @return Fully resolved style snapshot.
     *
     * @see docs/07_Declarative_Style_RFC.md Section 5.5
     */
    [[nodiscard]] virtual auto Resolve(WidgetKind kind,
                                       std::size_t variantIndex,
                                       InteractionState state) const -> ResolvedStyle = 0;

    /**
     * @brief Resolve with per-instance style override (cascade Layer 3).
     *
     * Same as `Resolve(kind, variantIndex, state)` but applies a sparse
     * instance-level patch on top. This is the highest priority layer in the
     * style cascade, used when a specific widget instance needs to deviate
     * from its class-level style.
     *
     * @param kind          Widget type.
     * @param variantIndex  Index into variants.
     * @param state         Current interaction state.
     * @param instanceOverride  Non-null pointer to instance override.
     * @return Resolved style with instance patches applied.
     *
     * @see InstanceStyleOverride, §4.17
     */
    [[nodiscard]] virtual auto Resolve(WidgetKind kind,
                                       std::size_t variantIndex,
                                       InteractionState state,
                                       const InstanceStyleOverride& instanceOverride) const -> ResolvedStyle = 0;

    // ========================================================================
    // Component Token Overrides
    // ========================================================================

    /**
     * @brief Per-widget-class token overrides.
     *
     * Only needed when a widget deviates from global defaults. For example,
     * `Dialog.radius = RadiusToken::Large` overrides the global `Default`.
     * Registered once per widget class, typically at module initialization.
     */
    struct ComponentOverride {
        WidgetKind kind {};                           ///< Target widget type
        std::optional<RadiusToken>    radius;        ///< Corner radius override
        std::optional<SpacingToken>   padding;       ///< Content padding override
        std::optional<SpacingToken>   margin;        ///< Outer margin override
        std::optional<FontRole>       font;          ///< Text font role override
        std::optional<ElevationToken> elevation;     ///< Box shadow override
        std::optional<AnimationToken> animation;     ///< Animation duration override
        std::optional<ColorToken>     backgroundToken; ///< Background color override
        std::optional<ColorToken>     foregroundToken; ///< Foreground color override
        std::optional<ColorToken>     borderToken;   ///< Border color override
    };

    /**
     * @brief Register component-level token overrides.
     *
     * Overrides are applied on the next `SetTheme()` call (or immediately
     * if a theme is already active). Multiple calls accumulate; later
     * registrations for the same `WidgetKind` field win.
     *
     * @param overrides Span of override definitions.
     */
    virtual void RegisterComponentOverrides(std::span<const ComponentOverride> overrides) = 0;

    // ========================================================================
    // Dynamic Tokens (Plugin Extension, ADR-015)
    // ========================================================================

    /**
     * @brief Definition for a plugin-registered dynamic color token.
     *
     * Plugins register domain-specific colors (e.g., "FEA/MeshQualityBad")
     * that are theme-aware but not part of the core token vocabulary.
     */
    struct DynamicColorDef {
        std::string_view key;        ///< Unique key, e.g. "FEA/MeshQualityBad"
        QColor           lightValue; ///< Color value for Light theme
        QColor           darkValue;  ///< Color value for Dark theme
    };

    /**
     * @brief Definition for a plugin-registered dynamic font token.
     */
    struct DynamicFontDef {
        std::string_view key;    ///< Unique key, e.g. "CAD/PropertyGrid"
        FontSpec         value;  ///< Font specification (theme-independent)
    };

    /**
     * @brief Definition for a plugin-registered dynamic spacing token.
     *
     * The stored value is a base pixel count (before density scaling).
     */
    struct DynamicSpacingDef {
        std::string_view key;    ///< Unique key, e.g. "FEA/ResultMargin"
        int              basePx; ///< Base pixel value (density-scaled on query)
    };

    /**
     * @brief Register dynamic color tokens from plugins.
     * @param defs Span of dynamic color definitions.
     */
    virtual void RegisterDynamicTokens(std::span<const DynamicColorDef> defs) = 0;

    /**
     * @brief Register dynamic font tokens from plugins.
     */
    virtual void RegisterDynamicFonts(std::span<const DynamicFontDef> defs) = 0;

    /**
     * @brief Register dynamic spacing tokens from plugins.
     */
    virtual void RegisterDynamicSpacings(std::span<const DynamicSpacingDef> defs) = 0;

    /**
     * @brief Query a dynamic color token by key.
     * @return Resolved color for the current theme, or `std::nullopt` if not registered.
     */
    [[nodiscard]] virtual auto DynamicColor(std::string_view key) const
        -> std::optional<QColor> = 0;

    /**
     * @brief Query a dynamic font token by key.
     * @return Font spec, or `std::nullopt` if not registered.
     */
    [[nodiscard]] virtual auto DynamicFont(std::string_view key) const
        -> std::optional<FontSpec> = 0;

    /**
     * @brief Query a dynamic spacing token by key (density-scaled).
     * @return Density-scaled pixel value, or `std::nullopt` if not registered.
     */
    [[nodiscard]] virtual auto DynamicSpacingPx(std::string_view key) const
        -> std::optional<int> = 0;

    /**
     * @brief Unregister previously registered dynamic tokens (all types).
     * @param keys Span of key strings to remove.
     */
    virtual void UnregisterDynamicTokens(std::span<const std::string_view> keys) = 0;

    // ========================================================================
    // Test Support
    // ========================================================================

    /**
     * @brief Override all animation durations for deterministic testing.
     *
     * `WidgetTestFixture` calls `SetAnimationOverride(0)` to force all
     * `AnimationMs()` queries to return 0ms, eliminating timing-dependent
     * test failures.
     *
     * @param forceMs Forced duration in ms. Pass -1 to restore real values.
     */
    virtual void SetAnimationOverride(int forceMs) = 0;

Q_SIGNALS:
    /**
     * @brief Emitted after `SetTheme()` completes token rebuild.
     *
     * All `ThemeAware` widgets connect to this signal to re-fetch their
     * `WidgetStyleSheet` and call `OnThemeChanged()`.
     *
     * @param newTheme The newly activated theme name.
     */
    void ThemeChanged(const QString& newTheme);

protected:
    /// @brief Protected constructor -- only subclasses can instantiate.
    using QObject::QObject;
};

// ============================================================================
// Global Theme Accessor
// ============================================================================

/// @brief Set the global theme service instance. Called once by Application::Initialize().
/// @param svc Pointer to the theme service (must outlive all widgets). nullptr to clear.
MATCHA_EXPORT void SetThemeService(IThemeService* svc);

/// @brief Get the global theme service instance.
/// @return Reference to the active theme service.
/// @pre SetThemeService() must have been called with a non-null pointer.
[[nodiscard]] MATCHA_EXPORT auto GetThemeService() -> IThemeService&;

/// @brief Check if a global theme service has been set.
[[nodiscard]] MATCHA_EXPORT auto HasThemeService() -> bool;

} // namespace matcha::gui
