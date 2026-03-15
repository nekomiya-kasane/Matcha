#pragma once

/**
 * @file ThemeAware.h
 * @brief Mixin class for theme-consuming widgets.
 *
 * All Matcha widgets that paint using design tokens inherit `ThemeAware`.
 * The mixin auto-connects to `IThemeService::ThemeChanged`, caches the
 * `WidgetStyleSheet` for the widget's `WidgetKind`, and exposes
 * `StyleSheet()` + `Theme()` accessors. Subclasses implement
 * `OnThemeChanged()` to trigger repaint.
 *
 * This is NOT a QObject -- it uses `QMetaObject::Connection` directly
 * and disconnects in the destructor.
 *
 * @see 05_Greenfield_Plan.md ss 2.5 for the ThemeAware design.
 * @see IThemeService.h for the theme service interface.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/IThemeService.h>
#include <Matcha/Theming/WidgetStyleSheet.h>

#include <QMetaObject>

namespace matcha::fw {
class WidgetNode;
} // namespace matcha::fw

namespace matcha::gui {

/**
 * @brief Mixin for widgets that consume design tokens from the theme engine.
 *
 * **Usage pattern** (Phase 3 widgets):
 * @code
 * class NyanPushButton : public QWidget, public ThemeAware {
 * public:
 *     explicit NyanPushButton(QWidget* parent = nullptr)
 *         : QWidget(parent)
 *         , ThemeAware(WidgetKind::PushButton)
 *     {}
 * protected:
 *     void OnThemeChanged() override { update(); }
 * };
 * @endcode
 *
 * **Lifecycle**:
 * 1. Constructor connects to `IThemeService::ThemeChanged` and fetches
 *    the initial `WidgetStyleSheet`.
 * 2. On theme switch, the connection callback refreshes the cached sheet
 *    and calls the subclass's `OnThemeChanged()`.
 * 3. Destructor disconnects the signal connection.
 */
class MATCHA_EXPORT ThemeAware {
public:
    /**
     * @brief Construct the mixin, connecting to the global theme service.
     * @param kind Widget kind for style sheet resolution.
     * @pre matcha::gui::SetThemeService() must have been called.
     */
    explicit ThemeAware(WidgetKind kind);

    /// @brief Destructor disconnects from ThemeChanged.
    virtual ~ThemeAware();

    ThemeAware(const ThemeAware&)            = delete;
    ThemeAware& operator=(const ThemeAware&) = delete;
    ThemeAware(ThemeAware&&)                 = delete;
    ThemeAware& operator=(ThemeAware&&)      = delete;

protected:
    /// @brief Access the cached style sheet for this widget's kind.
    [[nodiscard]] auto StyleSheet() const -> const WidgetStyleSheet& { return _sheet; }

    /// @brief Access the theme service (for direct token queries).
    [[nodiscard]] auto Theme() const -> const IThemeService& { return _theme; }

    /// @brief Access the theme service (non-const, for creating child widgets).
    [[nodiscard]] auto ThemeService() -> IThemeService& { return _theme; }

    /**
     * @brief Called when the theme changes.
     *
     * Subclasses implement this to trigger repaint (`update()`) or
     * recalculate layout-dependent values.
     */
    virtual void OnThemeChanged() = 0;

    /**
     * @brief Animate a property using this widget's WidgetStyleSheet transition config.
     *
     * Duration and easing are resolved from `StyleSheet().transition`.
     * Internally delegates to the global AnimationService.
     *
     * @param target   The WidgetNode to animate.
     * @param property Property to animate.
     * @param from     Start value.
     * @param to       End value.
     * @return Handle for the running animation (Invalid if service unavailable).
     */
    auto AnimateTransition(fw::WidgetNode* target,
                           fw::AnimationPropertyId property,
                           fw::AnimatableValue from,
                           fw::AnimatableValue to) -> fw::TransitionHandle;

    /**
     * @brief Animate a property with explicit duration and easing (ignoring style sheet).
     */
    auto AnimateTransition(fw::WidgetNode* target,
                           fw::AnimationPropertyId property,
                           fw::AnimatableValue from,
                           fw::AnimatableValue to,
                           fw::AnimationToken duration,
                           fw::EasingToken easing) -> fw::TransitionHandle;

    /**
     * @brief Paint a focus ring around a widget if it has keyboard focus.
     *
     * Call at the end of paintEvent() in widgets that should show a focus
     * indicator. Only paints when focus was gained via keyboard (Tab/Shift+Tab
     * or shortcut), not mouse click.
     *
     * @param widget The widget to paint the ring on.
     * @param theme  The theme service for color resolution.
     * @param radius Corner radius (default 4).
     */
    static void PaintFocusRing(QWidget* widget, const IThemeService& theme,
                               int radius = 4);

private:
    IThemeService&          _theme; ///< Non-owning reference to the theme service
    WidgetKind              _kind;  ///< Widget kind for style sheet resolution
    WidgetStyleSheet        _sheet; ///< Cached style sheet snapshot
    QMetaObject::Connection _conn;  ///< ThemeChanged signal connection
};

} // namespace matcha::gui
