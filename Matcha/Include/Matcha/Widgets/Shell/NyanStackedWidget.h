#pragma once

/**
 * @file NyanStackedWidget.h
 * @brief Theme-aware stacked widget with optional cross-fade transition.
 *
 * Inherits QStackedWidget for Qt stacked semantics and ThemeAware for
 * design token integration. Optionally cross-fades between pages using
 * AnimationToken duration.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for AnimationToken.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QStackedWidget>

namespace matcha::gui {

/**
 * @brief Theme-aware stacked widget with optional cross-fade.
 *
 * When cross-fade is enabled, switching pages uses a QGraphicsOpacityEffect
 * animation with AnimationToken::Normal duration.
 */
class MATCHA_EXPORT NyanStackedWidget : public QStackedWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a stacked widget.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanStackedWidget(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanStackedWidget() override;

    NyanStackedWidget(const NyanStackedWidget&)            = delete;
    NyanStackedWidget& operator=(const NyanStackedWidget&) = delete;
    NyanStackedWidget(NyanStackedWidget&&)                 = delete;
    NyanStackedWidget& operator=(NyanStackedWidget&&)      = delete;

    /// @brief Enable or disable cross-fade transition between pages.
    void SetCrossFadeEnabled(bool enabled);

    /// @brief Whether cross-fade is enabled.
    [[nodiscard]] auto CrossFadeEnabled() const -> bool;

    /// @brief Switch to page at index, with optional cross-fade.
    void SetCurrentIndex(int index);

protected:
    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    bool _crossFade = false;
};

} // namespace matcha::gui
