#pragma once

/**
 * @file NyanBadge.h
 * @brief Theme-aware status badge (pill-shaped tag) with semantic variants.
 *
 * A compact QWidget that paints a rounded pill shape with variant-specific
 * background/foreground colors. Optionally shows a close (x) button.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Pill shape: fully rounded corners (height/2 radius)
 * - Variants map to semantic color tokens:
 *   Success=SuccessNormal, Warning=WarningNormal, Error=ErrorNormal,
 *   Info=PrimaryNormal, Neutral=Background4, Custom=user-specified
 * - Text: Foreground7 (light on dark variants) or Foreground1 (Neutral)
 * - Close button: small x painted at right edge
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Semantic variant for badge background color.
 */
enum class BadgeVariant : uint8_t {
    Success,  ///< Green success indicator
    Warning,  ///< Yellow/orange warning indicator
    Error,    ///< Red error/danger indicator
    Info,     ///< Blue/primary informational
    Neutral,  ///< Gray neutral tag
    Custom,   ///< User-specified custom color

    Count_
};

/**
 * @brief Theme-aware pill-shaped status badge.
 *
 * Displays a short text label in a colored pill. Optional close button
 * emits `Closed()` when clicked.
 */
class MATCHA_EXPORT NyanBadge : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a badge.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanBadge(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanBadge() override;

    NyanBadge(const NyanBadge&)            = delete;
    NyanBadge& operator=(const NyanBadge&) = delete;
    NyanBadge(NyanBadge&&)                 = delete;
    NyanBadge& operator=(NyanBadge&&)      = delete;

    /// @brief Set the badge text.
    void SetText(const QString& text);

    /// @brief Get the badge text.
    [[nodiscard]] auto Text() const -> QString;

    /// @brief Set the visual variant.
    void SetVariant(BadgeVariant variant);

    /// @brief Get the current variant.
    [[nodiscard]] auto Variant() const -> BadgeVariant;

    /// @brief Set custom background color (only used when variant == Custom).
    void SetCustomColor(const QColor& color);

    /// @brief Enable or disable the close (x) button.
    void SetClosable(bool closable);

    /// @brief Whether the close button is shown.
    [[nodiscard]] auto IsClosable() const -> bool;

    /// @brief Size hint based on text width + padding.
    [[nodiscard]] auto sizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the close button is clicked.
    void Closed();

protected:
    /// @brief Custom paint: pill shape + text + optional close icon.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle close button click.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Resolve background and foreground colors for current variant.
    struct ResolvedColors {
        QColor background;
        QColor foreground;
    };
    [[nodiscard]] auto ResolveColors() const -> ResolvedColors;

    static constexpr int kVPadding   = 2;   ///< Vertical padding
    static constexpr int kHPadding   = 8;   ///< Horizontal padding
    static constexpr int kCloseSize  = 10;  ///< Close button area size
    static constexpr int kCloseGap   = 4;   ///< Gap before close button
    static constexpr int kFixedHeight = 20; ///< Fixed badge height

    QString      _text;
    BadgeVariant _variant    = BadgeVariant::Neutral;
    QColor       _customColor;
    bool         _closable   = false;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
