#pragma once

/**
 * @file NyanProgressBar.h
 * @brief Theme-aware linear progress bar with custom-painted groove and fill.
 *
 * Inherits QProgressBar for Qt progress semantics and ThemeAware for design
 * token integration. Custom-painted groove and fill using design tokens.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanProgressBar.h` (QProgressBar base)
 * - `old/NyanGuis/Gui/inc/NyanProgressBarStyle.h` (Groove=Background4,
 *   Fill=PrimaryNormal, Text=Font1, Round groove)
 *
 * @par Visual specification
 * - Groove: Background4, fully rounded corners (height/2 radius)
 * - Fill: PrimaryNormal, same rounded corners
 * - Text: optional centered percentage overlay (Foreground1)
 * - Height: 8px default (no text), 20px with text
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QProgressBar>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware linear progress bar.
 *
 * Custom-painted groove and fill bar using design tokens.
 * Optional text overlay showing percentage.
 */
class MATCHA_EXPORT NyanProgressBar : public QProgressBar, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a progress bar.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanProgressBar(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanProgressBar() override;

    NyanProgressBar(const NyanProgressBar&)            = delete;
    NyanProgressBar& operator=(const NyanProgressBar&) = delete;
    NyanProgressBar(NyanProgressBar&&)                 = delete;
    NyanProgressBar& operator=(NyanProgressBar&&)      = delete;

    /// @brief Enable or disable the text percentage overlay.
    void SetTextVisible(bool visible);

    /// @brief Whether text overlay is shown.
    [[nodiscard]] auto IsTextVisible() const -> bool;

    /// @brief Size hint: width from QProgressBar, height depends on text mode.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: groove + fill + optional text.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kBarHeight     = 8;  ///< Bar height without text
    static constexpr int kBarHeightText = 20; ///< Bar height with text
    static constexpr int kDefaultWidth  = 200;

    bool _textVisible = false;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
