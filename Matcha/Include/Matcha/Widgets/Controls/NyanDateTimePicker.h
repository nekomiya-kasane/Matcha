#pragma once

/**
 * @file NyanDateTimePicker.h
 * @brief Theme-aware date/time input with themed calendar popup.
 *
 * Inherits QDateTimeEdit for Qt date/time editing and ThemeAware for
 * design token integration. Supports Date-only, Time-only, and DateTime
 * modes with a themed calendar popup.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Height: 26px (matches NyanLineEdit)
 * - Border: Border4 normal, PrimaryNormal focus
 * - Background: Background1
 * - Calendar popup: themed via palette override
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QDateTimeEdit>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Display mode for NyanDateTimePicker.
 */
enum class DateTimeMode : uint8_t {
    Date,     ///< Date only (yyyy-MM-dd)
    Time,     ///< Time only (HH:mm:ss)
    DateTime, ///< Full date+time

    Count_
};

/**
 * @brief Theme-aware date/time picker.
 *
 * Wraps QDateTimeEdit with design token painting and a themed calendar
 * popup. Mode controls which sections are visible and the default
 * display format.
 */
class MATCHA_EXPORT NyanDateTimePicker : public QDateTimeEdit, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a date/time picker.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanDateTimePicker(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanDateTimePicker() override;

    NyanDateTimePicker(const NyanDateTimePicker&)            = delete;
    NyanDateTimePicker& operator=(const NyanDateTimePicker&) = delete;
    NyanDateTimePicker(NyanDateTimePicker&&)                 = delete;
    NyanDateTimePicker& operator=(NyanDateTimePicker&&)      = delete;

    /// @brief Set the display mode (Date, Time, DateTime).
    void SetMode(DateTimeMode mode);

    /// @brief Get the current display mode.
    [[nodiscard]] auto Mode() const -> DateTimeMode;

    /// @brief Size hint: width from QDateTimeEdit, height 26px.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: themed border + background.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Apply format string and section visibility for current mode.
    void ApplyMode();

    static constexpr int kFixedHeight = 26;

    DateTimeMode _mode = DateTimeMode::DateTime;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
