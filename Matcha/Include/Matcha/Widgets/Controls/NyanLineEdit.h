#pragma once

/**
 * @file NyanLineEdit.h
 * @brief Theme-aware single-line text/numeric input with unit suffix.
 *
 * Inherits QLineEdit for Qt text editing and ThemeAware for design token
 * integration. Supports Text, Integer, and Double input modes with built-in
 * validation. Unit suffix is display-only (no conversion logic).
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanLineEdit.h` (180-value Unit enum = business logic)
 * - `old/NyanGuis/Gui/src/NyanLineEdit.cpp` (sizeHint height=26, InputMethod enum)
 * - `old/NyanGuis/Gui/inc/NyanLineEditStyle.h` (Border=Line4, Focus=PrimaryClick,
 *   Background=Background1)
 *
 * @par Visual preservation
 * Border = Border4 normal, PrimaryNormal focus, Background1 fill, height 26px.
 * Unit suffix painted as right-aligned label inside the edit frame.
 *
 * @par Business logic removal
 * Old 180-value `Unit` enum removed. Replaced by `SetUnitSuffix(QString)`
 * which accepts a display-only string. Business layer handles conversion
 * via `IUnitConverter` (not part of Matcha).
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QLineEdit>

#include <optional>

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Input mode for NyanLineEdit.
 *
 * Maps from old `NyanLineEdit::InputMethod`:
 * - Text    = old Text
 * - Integer = old Int
 * - Double  = old Double
 */
enum class InputMode : uint8_t {
    Text,    ///< Free-form text input
    Integer, ///< Integer-only (QIntValidator)
    Double,  ///< Double with precision (QDoubleValidator)

    Count_
};

/**
 * @brief Border style for NyanLineEdit.
 *
 * Controls the visual border rendering. Upper-layer container widgets
 * (e.g. DataTable) select the appropriate style when embedding the edit.
 *
 * Maps from old `NyanLineEdit::UseTo`:
 * - Full    = old InWidget (rounded rect border on all sides)
 * - Minimal = old InTable  (top/bottom lines only, no rounded corners)
 */
enum class LineEditBorder : uint8_t {
    Full,    ///< Rounded rect border on all sides (default)
    Minimal, ///< Top/bottom horizontal lines only

    Count_
};

/**
 * @brief Theme-aware single-line text/numeric input with unit suffix.
 *
 * Features:
 * - InputMode with automatic QValidator setup
 * - Range constraints for numeric modes
 * - Precision control for Double mode
 * - Display-only unit suffix (no business logic)
 * - Custom-painted border/background via design tokens
 */
class MATCHA_EXPORT NyanLineEdit : public QLineEdit, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a line edit.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanLineEdit(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanLineEdit() override;

    NyanLineEdit(const NyanLineEdit&)            = delete;
    NyanLineEdit& operator=(const NyanLineEdit&) = delete;
    NyanLineEdit(NyanLineEdit&&)                 = delete;
    NyanLineEdit& operator=(NyanLineEdit&&)      = delete;

    /// @brief Set the input mode (updates validator automatically).
    void SetInputMode(InputMode mode);

    /// @brief Get the current input mode.
    [[nodiscard]] auto GetInputMode() const -> InputMode;

    /// @brief Set the valid range for Integer/Double modes.
    void SetRange(double lower, double upper);

    /// @brief Get the lower bound of the range.
    [[nodiscard]] auto LowerValue() const -> double;

    /// @brief Get the upper bound of the range.
    [[nodiscard]] auto UpperValue() const -> double;

    /// @brief Set decimal precision for Double mode (1-15).
    void SetPrecision(int decimals);

    /// @brief Get the current precision.
    [[nodiscard]] auto Precision() const -> int;

    /// @brief Set display-only unit suffix (e.g. "mm", "kg/m3").
    void SetUnitSuffix(const QString& suffix);

    /// @brief Get the current unit suffix.
    [[nodiscard]] auto UnitSuffix() const -> QString;

    /// @brief Get the text with the unit suffix stripped (numeric content only).
    [[nodiscard]] auto TextWithoutSuffix() const -> QString;

    /// @brief Set the border style.
    void SetBorderStyle(LineEditBorder border);

    /// @brief Get the current border style.
    [[nodiscard]] auto BorderStyle() const -> LineEditBorder;

    /// @brief Get the current text as a numeric value (empty if not numeric).
    [[nodiscard]] auto NumericValue() const -> std::optional<double>;

    /// @brief Size hint: width from QLineEdit + suffix, height 26px.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    /// @brief Custom paint: themed border + background + unit suffix.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Rebuild the validator for the current InputMode and range.
    void RebuildValidator();

    static constexpr int kFixedHeight  = 26;  ///< Fixed height matching old sizeHint
    static constexpr int kSuffixGap    = 4;   ///< Gap before suffix text
    static constexpr int kHPadding     = 6;   ///< Horizontal padding

    InputMode      _inputMode    = InputMode::Text;
    LineEditBorder _borderStyle  = LineEditBorder::Full;
    double         _lowerValue = -1e15;
    double         _upperValue = 1e15;
    int            _precision  = 6;
    QString        _unitSuffix;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
