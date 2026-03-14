#pragma once

/**
 * @file NyanRichTooltip.h
 * @brief Theme-aware two-tier rich tooltip replacing plain QToolTip.
 *
 * CAD toolbar buttons require rich tooltips with icon, title, description,
 * shortcut, and optional preview image/widget. Two-tier delay system:
 * - Tier 1: hover shows brief summary (title + shortcut) after ~200ms
 * - Tier 2: dwell ~1s expands to full detail (description + preview)
 *
 * @par Visual specification (spec4 ss 3.1)
 * - Hover shows brief summary, dwell 1s shows detailed info
 * - Preview image/animation slot reserved
 *
 * @par Old project reference
 * - No old equivalent. Replaces plain QToolTip usage.
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for AnimationToken values.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QPixmap>
#include <QWidget>

class QTimer;

namespace matcha::gui {

class SimpleWidgetEventFilter;

class NyanLabel;

/**
 * @brief Theme-aware rich tooltip with two-tier delay expansion.
 *
 * Tier 1 (brief):  [icon] Title                 Shortcut
 * Tier 2 (detail): [icon] Title                 Shortcut
 *                   Description text (multi-line)
 *                   [Preview image / widget]
 *
 * Usage: create once, call ShowForWidget(trigger) on hover,
 * call Hide() on mouse leave.
 */
class MATCHA_EXPORT NyanRichTooltip : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a rich tooltip.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanRichTooltip(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanRichTooltip() override;

    NyanRichTooltip(const NyanRichTooltip&)            = delete;
    NyanRichTooltip& operator=(const NyanRichTooltip&) = delete;
    NyanRichTooltip(NyanRichTooltip&&)                 = delete;
    NyanRichTooltip& operator=(NyanRichTooltip&&)      = delete;

    // -- Content setters --

    /// @brief Set the tooltip icon (displayed left of title).
    void SetIcon(const QPixmap& icon);

    /// @brief Set the title text (shown in Tier 1).
    void SetTitle(const QString& title);

    /// @brief Get the title text.
    [[nodiscard]] auto Title() const -> QString;

    /// @brief Set the description text (shown in Tier 2).
    void SetDescription(const QString& description);

    /// @brief Set the keyboard shortcut text (shown in Tier 1, right-aligned).
    void SetShortcut(const QString& shortcut);

    /// @brief Set a preview image (shown in Tier 2 below description).
    void SetPreviewImage(const QPixmap& image);

    /// @brief Set a preview widget (shown in Tier 2 below description).
    /// @note Takes ownership of the widget.
    void SetPreviewWidget(QWidget* widget);

    // -- Delay configuration --

    /// @brief Set Tier 1 delay in milliseconds (default: 200).
    void SetTier1Delay(int ms);

    /// @brief Get Tier 1 delay.
    [[nodiscard]] auto Tier1Delay() const -> int;

    /// @brief Set Tier 2 delay in milliseconds (default: 1000).
    void SetTier2Delay(int ms);

    /// @brief Get Tier 2 delay.
    [[nodiscard]] auto Tier2Delay() const -> int;

    // -- Show / Hide --

    /// @brief Start the show sequence anchored near a trigger widget.
    void ShowForWidget(QWidget* trigger);

    /// @brief Cancel timers and hide the tooltip.
    void Hide();

    /// @brief Size hint based on current tier and content.
    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: themed background, shadow, content.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Transition from hidden to Tier 1 display.
    void ShowTier1();

    /// @brief Expand from Tier 1 to Tier 2 display.
    void ExpandToTier2();

    /// @brief Position this tooltip relative to the trigger widget.
    void PositionNear(QWidget* trigger);

    /// @brief Update layout visibility for current tier.
    void UpdateTierVisibility();

    static constexpr int kPadding       = 12;
    static constexpr int kIconSize      = 20;
    static constexpr int kShadowInset   = 4;
    static constexpr int kMaxWidth      = 320;
    static constexpr int kDefaultTier1  = 200;
    static constexpr int kDefaultTier2  = 1000;

    enum class Tier : uint8_t { Hidden, Brief, Detail };

    Tier     _tier         = Tier::Hidden;
    int      _tier1DelayMs = kDefaultTier1;
    int      _tier2DelayMs = kDefaultTier2;

    QPixmap  _icon;
    QPixmap  _previewImage;
    QWidget* _previewWidget = nullptr;
    QWidget* _triggerWidget = nullptr;

    NyanLabel* _titleLabel       = nullptr;
    NyanLabel* _shortcutLabel    = nullptr;
    NyanLabel* _descriptionLabel = nullptr;

    QTimer*  _tier1Timer = nullptr;
    QTimer*  _tier2Timer = nullptr;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
