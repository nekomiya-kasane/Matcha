#pragma once

/**
 * @file NyanSelectionInput.h
 * @brief Entity pick input with selection states.
 *
 * NyanSelectionInput provides:
 * - Visual state feedback during pick operations
 * - Single/Multiple selection modes
 * - Click-to-pick interaction
 *
 * @par Visual states (from old NyanTraitSelect)
 * - Idle: Default background (Background2)
 * - Picking (unselected): Warning yellow (WarningFocus/WarningLight)
 * - Selected: Success green (SuccessFocus/SuccessLight)
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Pick mode for entity selection.
 */
enum class PickMode : uint8_t {
    Disabled,  ///< Picking disabled
    Single,    ///< Single entity selection
    Multiple   ///< Multiple entity selection
};

/**
 * @brief Visual state during selection operation.
 *
 * Maps to old NyanTraitSelect states:
 * - Idle = DefaultRecognition/DisableAuto
 * - PickingEmpty = UnSelectActive (picking, nothing selected yet)
 * - PickingIdle = UnSelectClose (picking paused)
 * - Selected = SelectActive (has selection, active)
 * - SelectedIdle = SelectClose (has selection, inactive)
 */
enum class SelectionState : uint8_t {
    Idle,          ///< Default state, not picking
    PickingEmpty,  ///< Actively picking, no selection yet (yellow active)
    PickingIdle,   ///< Picking paused, no selection (yellow idle)
    Selected,      ///< Has selection, active (green active)
    SelectedIdle   ///< Has selection, inactive (green idle)
};

/**
 * @brief Entity pick input with selection states.
 *
 * A11y role: ComboBox (editable selection).
 */
class MATCHA_EXPORT NyanSelectionInput : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a selection input.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanSelectionInput(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanSelectionInput() override;

    NyanSelectionInput(const NyanSelectionInput&)            = delete;
    NyanSelectionInput& operator=(const NyanSelectionInput&) = delete;
    NyanSelectionInput(NyanSelectionInput&&)                 = delete;
    NyanSelectionInput& operator=(NyanSelectionInput&&)      = delete;

    // -- Configuration --

    /// @brief Set the pick mode.
    void SetMode(PickMode mode);

    /// @brief Get the pick mode.
    [[nodiscard]] auto Mode() const -> PickMode;

    /// @brief Set the prompt text (placeholder).
    void SetPrompt(const QString& prompt);

    /// @brief Get the prompt text.
    [[nodiscard]] auto Prompt() const -> QString;

    /// @brief Set the current selection display text.
    void SetSelection(const QString& selection);

    /// @brief Get the current selection display text.
    [[nodiscard]] auto Selection() const -> QString;

    /// @brief Set the visual state.
    void SetState(SelectionState state);

    /// @brief Get the visual state.
    [[nodiscard]] auto State() const -> SelectionState;

    /// @brief Clear the selection.
    void ClearSelection();

    /// @brief Check if there is a selection.
    [[nodiscard]] auto HasSelection() const -> bool;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the pick button is clicked.
    void PickRequested();

    /// @brief Emitted when the selection changes.
    void SelectionChanged(const QString& selection);

    /// @brief Emitted when the clear button is clicked.
    void ClearRequested();

protected:
    /// @brief Custom paint for themed selection input.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle mouse click for pick trigger.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void UpdateVisualState();
    void OnPickButtonClicked();
    void OnClearButtonClicked();

    [[nodiscard]] auto StateBackgroundColor() const -> QColor;
    [[nodiscard]] auto StateBorderColor() const -> QColor;

    static constexpr int kHeight   = 28;
    static constexpr int kRadius   = 3;
    static constexpr int kPadding  = 8;

    QHBoxLayout*   _layout       = nullptr;
    QLineEdit*     _displayEdit  = nullptr;
    QPushButton*   _pickButton   = nullptr;
    QPushButton*   _clearButton  = nullptr;

    PickMode       _mode         = PickMode::Single;
    SelectionState _state        = SelectionState::Idle;
    QString        _selection;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
