#pragma once

/**
 * @file NyanDoubleSpinBox.h
 * @brief Theme-aware double spin box with engineering precision.
 *
 * Inherits QDoubleSpinBox for Qt double spin box semantics and ThemeAware
 * for design token integration. Shares visual language with NyanLineEdit.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget sharing NyanLineEdit border/focus tokens.
 *
 * @par Visual consistency
 * Border = Border4 normal, PrimaryNormal focus, Background1 fill, height 26px.
 * Up/down buttons painted with Foreground3 arrows, matching NyanSpinBox.
 *
 * @see NyanSpinBox for shared painting patterns.
 * @see NyanLineEdit for shared border/focus styling.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QDoubleSpinBox>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Theme-aware double spin box with engineering precision.
 *
 * Provides SetRange/SetStep/SetPrecision convenience API on top of QDoubleSpinBox.
 * Custom-painted border and up/down buttons using design tokens.
 */
class MATCHA_EXPORT NyanDoubleSpinBox : public QDoubleSpinBox, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a double spin box.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanDoubleSpinBox(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanDoubleSpinBox() override;

    NyanDoubleSpinBox(const NyanDoubleSpinBox&)            = delete;
    NyanDoubleSpinBox& operator=(const NyanDoubleSpinBox&) = delete;
    NyanDoubleSpinBox(NyanDoubleSpinBox&&)                 = delete;
    NyanDoubleSpinBox& operator=(NyanDoubleSpinBox&&)      = delete;

    /// @brief Set the valid double range.
    void SetRange(double minimum, double maximum);

    /// @brief Set the single-step increment.
    void SetStep(double step);

    /// @brief Set decimal precision (1-15).
    void SetPrecision(int decimals);

    /// @brief Get the current precision.
    [[nodiscard]] auto Precision() const -> int;

    /// @brief Size hint: width from QDoubleSpinBox, height 26px.
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

    int _precision = 6;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
