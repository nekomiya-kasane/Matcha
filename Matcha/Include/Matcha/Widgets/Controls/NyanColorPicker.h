#pragma once

/**
 * @file NyanColorPicker.h
 * @brief Theme-aware full-featured color selection panel.
 *
 * HSV wheel/square + RGB/Hex input + optional alpha slider.
 * Preset swatches row. Eyedropper pick from screen.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanColorPanel.h` (simple display chip)
 * - This is a full picker -- significantly expanded scope.
 *
 * @par Visual specification
 * - HSV wheel: 150x150 custom-painted
 * - Value slider: vertical, 20px wide
 * - RGB inputs: 3 NyanLineEdit (0-255)
 * - Hex input: NyanLineEdit (#RRGGBB)
 * - Alpha slider: horizontal, optional
 * - Preset swatches: 8 colors in a row
 * - Eyedropper button: pick from screen
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

#include <array>

class QLineEdit;
class QSlider;
class QPushButton;
class QLabel;

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware full-featured color selection panel.
 *
 * A11y role: Dialog.
 */
class MATCHA_EXPORT NyanColorPicker : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a color picker.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanColorPicker(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanColorPicker() override;

    NyanColorPicker(const NyanColorPicker&)            = delete;
    NyanColorPicker& operator=(const NyanColorPicker&) = delete;
    NyanColorPicker(NyanColorPicker&&)                 = delete;
    NyanColorPicker& operator=(NyanColorPicker&&)      = delete;

    /// @brief Set the current color.
    void SetColor(const QColor& color);

    /// @brief Get the current color.
    [[nodiscard]] auto Color() const -> QColor;

    /// @brief Enable or disable alpha channel editing.
    void SetAlphaEnabled(bool enabled);

    /// @brief Whether alpha channel is enabled.
    [[nodiscard]] auto IsAlphaEnabled() const -> bool;

    /// @brief Size hint.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Minimum size hint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when color changes (live preview).
    void ColorChanged(const QColor& color);

    /// @brief Emitted when user confirms color (OK button or double-click).
    void ColorConfirmed(const QColor& color);

protected:
    /// @brief Custom paint for HSV wheel.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle mouse for HSV wheel interaction.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Handle mouse drag for HSV wheel.
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief Handle mouse release.
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void SetupUi();
    void UpdateFromHsv();
    void UpdateFromRgb();
    void UpdateFromHex();
    void UpdateUiFromColor();
    void StartEyedropper();
    void PaintHsvWheel(QPainter& p);
    void PaintValueSlider(QPainter& p);
    void PaintPreview(QPainter& p);
    [[nodiscard]] auto HsvWheelRect() const -> QRect;
    [[nodiscard]] auto ValueSliderRect() const -> QRect;
    [[nodiscard]] auto PreviewRect() const -> QRect;
    [[nodiscard]] auto HitTestHsvWheel(const QPoint& pos) const -> bool;
    [[nodiscard]] auto HitTestValueSlider(const QPoint& pos) const -> bool;

    static constexpr int kWheelSize = 150;
    static constexpr int kSliderWidth = 20;
    static constexpr int kPreviewSize = 40;
    static constexpr int kSwatchSize = 24;
    static constexpr int kSwatchCount = 8;

    // HSV values (0-359 hue, 0-255 sat/val).
    int _hue = 0;
    int _saturation = 255;
    int _value = 255;
    int _alpha = 255;

    bool _alphaEnabled = false;
    bool _draggingWheel = false;
    bool _draggingValue = false;

    // UI elements.
    QLineEdit* _hexEdit = nullptr;
    QLineEdit* _rEdit = nullptr;
    QLineEdit* _gEdit = nullptr;
    QLineEdit* _bEdit = nullptr;
    QSlider* _alphaSlider = nullptr;
    QLabel* _alphaLabel = nullptr;
    QPushButton* _eyedropperBtn = nullptr;
    QPushButton* _okBtn = nullptr;

    std::array<QColor, kSwatchCount> _presetColors;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
