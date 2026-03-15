#pragma once

/**
 * @file NyanSpinBox.h
 * @brief Theme-aware integer spin box with custom-painted buttons.
 *
 * Inherits QSpinBox for Qt spin box semantics and ThemeAware for design
 * token integration. Shares visual language with NyanLineEdit.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget sharing NyanLineEdit border/focus tokens.
 *
 * @par Visual consistency
 * Border = Border4 normal, PrimaryNormal focus, Background1 fill, height 26px.
 * Up/down buttons painted with Foreground3 arrows, Background4 bg, Background5 hover.
 *
 * @see NyanLineEdit for shared border/focus styling patterns.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QSpinBox>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware integer spin box.
 *
 * Provides SetRange/SetStep convenience API on top of QSpinBox.
 * Custom-painted border and up/down buttons using design tokens.
 */
class MATCHA_EXPORT NyanSpinBox : public QSpinBox, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct an integer spin box.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanSpinBox(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanSpinBox() override;

    NyanSpinBox(const NyanSpinBox&)            = delete;
    NyanSpinBox& operator=(const NyanSpinBox&) = delete;
    NyanSpinBox(NyanSpinBox&&)                 = delete;
    NyanSpinBox& operator=(NyanSpinBox&&)      = delete;

    /// @brief Set the valid integer range.
    void SetRange(int minimum, int maximum);

    /// @brief Set the single-step increment.
    void SetStep(int step);

    /// @brief Size hint: width from QSpinBox, height 26px.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: themed border + up/down button arrows.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kFixedHeight = 26; ///< Fixed height matching NyanLineEdit
    static constexpr int kButtonWidth = 16; ///< Up/down button area width
    static constexpr int kArrowSize   = 4;  ///< Arrow triangle half-size
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
