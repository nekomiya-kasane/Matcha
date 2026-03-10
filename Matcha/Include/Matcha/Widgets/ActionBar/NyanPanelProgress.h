#pragma once

/**
 * @file NyanPanelProgress.h
 * @brief Panel with embedded thin progress bar at top edge.
 *
 * Inherits NyanPanel for styled container behavior and adds a thin
 * progress bar rendered at the top edge of the panel.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanPanelProgress.h`
 * - SetMinimum/SetMaximum/SetValue -> SetProgress(min, max, value)
 * - Inherits NyanPanel (same as old)
 *
 * @par Visual specification
 * - Progress bar: 3px height at top edge, PrimaryNormal fill, Background4 track
 * - Inherits all NyanPanel visual properties (elevation, border, background)
 *
 * @see NyanPanel for base container.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/ActionBar/NyanPanel.h>

namespace matcha::gui {

/**
 * @brief Panel with embedded thin progress bar at top edge.
 */
class MATCHA_EXPORT NyanPanelProgress : public NyanPanel {
    Q_OBJECT

public:
    /**
     * @brief Construct a panel with progress bar.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanPanelProgress(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanPanelProgress() override;

    NyanPanelProgress(const NyanPanelProgress&)            = delete;
    NyanPanelProgress& operator=(const NyanPanelProgress&) = delete;
    NyanPanelProgress(NyanPanelProgress&&)                 = delete;
    NyanPanelProgress& operator=(NyanPanelProgress&&)      = delete;

    /// @brief Set progress range and current value in one call.
    void SetProgress(int minimum, int maximum, int value);

    /// @brief Set the minimum value.
    void SetMinimum(int minimum);

    /// @brief Set the maximum value.
    void SetMaximum(int maximum);

    /// @brief Set the current value.
    void SetValue(int value);

    /// @brief Get current value.
    [[nodiscard]] auto Value() const -> int;

    /// @brief Get maximum value.
    [[nodiscard]] auto Maximum() const -> int;

    /// @brief Get minimum value.
    [[nodiscard]] auto Minimum() const -> int;

protected:
    /// @brief Custom paint: panel + progress bar at top.
    void paintEvent(QPaintEvent* event) override;

private:
    static constexpr int kBarHeight = 3;

    int _minimum = 0;
    int _maximum = 100;
    int _value   = 0;
};

} // namespace matcha::gui
