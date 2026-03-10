#pragma once

/**
 * @file NyanPanel.h
 * @brief Theme-aware generic styled container with elevation and border.
 *
 * Replaces old NyanPanel's PanelState(Panel0-6) with semantic ElevationToken.
 * Custom-painted background with optional border and elevation shadow.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanPanel.h`
 * - PanelState(Panel0-6) -> ElevationToken (Flat/Low/Medium/High)
 * - SetDisplayBorder(bool) -> SetBorderVisible(bool)
 *
 * @par Visual specification
 * - Background: Background1 (Flat), Background2 (Low/Medium/High)
 * - Border: Border2, 1px, rounded with RadiusToken::Default
 * - Shadow: per ElevationToken from ShadowSpec
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ElevationToken, ShadowSpec.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

/**
 * @brief Theme-aware generic container panel with elevation and border.
 *
 * Use as a styled container for grouping child widgets. Elevation
 * controls shadow depth; border is optional.
 */
class MATCHA_EXPORT NyanPanel : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a panel.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanPanel(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanPanel() override;

    NyanPanel(const NyanPanel&)            = delete;
    NyanPanel& operator=(const NyanPanel&) = delete;
    NyanPanel(NyanPanel&&)                 = delete;
    NyanPanel& operator=(NyanPanel&&)      = delete;

    /// @brief Set the elevation level (controls shadow depth).
    void SetElevation(ElevationToken elevation);

    /// @brief Get the current elevation.
    [[nodiscard]] auto Elevation() const -> ElevationToken;

    /// @brief Set whether the border is visible.
    void SetBorderVisible(bool visible);

    /// @brief Whether the border is visible.
    [[nodiscard]] auto BorderVisible() const -> bool;

    /// @brief Size hint: 200x150.
    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: background + border + shadow.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    ElevationToken _elevation = ElevationToken::Flat;
    bool _borderVisible = true;
};

} // namespace matcha::gui
