#pragma once

/**
 * @file NyanPopConfirm.h
 * @brief Theme-aware confirmation popup / dialog with state icon and 3-button model.
 *
 * A frameless dialog that displays a title, optional state icon (Info/Question/
 * Warn/Error), message body, and up to three action buttons (Confirm/Deny/Cancel).
 * Can be used as a popup bubble anchored to a widget, or as a modal dialog
 * returning a result code.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanPopConfirm.h`
 * - `old/NyanGuis/Gui/inc/NyanPopConfirmPrivate.h` (eliminated)
 *
 * @par Visual specification (from specs/details/314-popconfirm.md)
 * - 12-position arrow pointing at anchor widget
 * - Level 3 shadow container
 * - Required: title text, container, action buttons (Confirm/Cancel)
 * - Optional: auxiliary text, icon prefix, deny button
 * - Active directional trigger
 *
 * @see ThemeAware for mixin lifecycle.
 * @see DesignTokens.h for ColorToken values.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QDialog>

namespace matcha::gui {

class NyanLabel;
class NyanPushButton;

/**
 * @brief Arrow position for the PopConfirm bubble.
 *
 * 12 positions matching the spec: 4 sides x 3 alignments each.
 */
enum class ArrowPosition : uint8_t {
    TopLeft,    TopCenter,    TopRight,
    BottomLeft, BottomCenter, BottomRight,
    LeftTop,    LeftCenter,   LeftBottom,
    RightTop,   RightCenter,  RightBottom,

    Count_
};

/**
 * @brief Severity / information state of the PopConfirm.
 *
 * Determines the prefix icon displayed next to the title.
 * Matches old `NyanPopConfirm::PopConfirmState`.
 */
enum class PopConfirmState : uint8_t {
    Info,       ///< Informational prompt (default)
    Question,   ///< Help / question prompt
    Warn,       ///< Warning prompt
    Error,      ///< Error / destructive prompt

    Count_
};

/**
 * @brief Identifies which button in the PopConfirm.
 */
enum class PopConfirmButton : uint8_t {
    Confirm,    ///< OK / Confirm button
    Deny,       ///< No / Deny button (hidden by default)
    Cancel,     ///< Cancel button

    Count_
};

/**
 * @brief Result code from modal PopConfirm execution.
 */
enum class PopConfirmCode : uint8_t {
    ConfirmCode,  ///< User clicked Confirm
    DenyCode,     ///< User clicked Deny
    CancelCode,   ///< User clicked Cancel or Close

    Count_
};

/**
 * @brief Theme-aware confirmation popup / dialog.
 *
 * Layout: [icon] [title] ----stretch---- [close]
 *                [message body]
 *                    ----stretch---- [confirm] [deny] [cancel]
 *
 * Supports both async (signal-based, ShowAt) and sync (modal, PopConfirm) usage.
 */
class MATCHA_EXPORT NyanPopConfirm : public QDialog, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a pop confirm dialog.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanPopConfirm(QWidget* parent = nullptr);

    /**
     * @brief Construct with title and state.
     * @param theme Theme service reference.
     * @param title Title text.
     * @param state Severity state for icon selection.
     * @param parent Optional parent widget.
     */
    NyanPopConfirm(const QString& title,
                   PopConfirmState state = PopConfirmState::Info,
                   QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanPopConfirm() override;

    NyanPopConfirm(const NyanPopConfirm&)            = delete;
    NyanPopConfirm& operator=(const NyanPopConfirm&) = delete;
    NyanPopConfirm(NyanPopConfirm&&)                 = delete;
    NyanPopConfirm& operator=(NyanPopConfirm&&)      = delete;

    /// @brief Set the title text.
    void SetTitle(const QString& title);

    /// @brief Get the title text.
    [[nodiscard]] auto Title() const -> QString;

    /// @brief Set the severity state (updates icon).
    void SetState(PopConfirmState state);

    /// @brief Get the severity state.
    [[nodiscard]] auto State() const -> PopConfirmState;

    /// @brief Set the message body text.
    void SetMessage(const QString& message);

    /// @brief Set the message body from multiple lines.
    void SetMessage(const QStringList& messages);

    /// @brief Get the message text.
    [[nodiscard]] auto Message() const -> QString;

    /// @brief Set text for a specific button.
    void SetButtonText(const QString& text, PopConfirmButton button);

    /// @brief Set visibility for a specific button.
    void SetButtonVisible(bool visible, PopConfirmButton button);

    /// @brief Set the arrow position relative to this popup.
    void SetArrowPosition(ArrowPosition pos);

    /// @brief Get the arrow position.
    [[nodiscard]] auto GetArrowPosition() const -> ArrowPosition;

    /// @brief Show the popup anchored to a target widget (async, signal-based).
    void ShowAt(QWidget* anchor);

    /// @brief Execute modally and return the result code.
    [[nodiscard]] auto PopConfirm() -> PopConfirmCode;

    /// @brief Size hint based on content.
    [[nodiscard]] auto sizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the confirm button is clicked.
    void Confirmed();

    /// @brief Emitted when the deny button is clicked.
    void Denied();

    /// @brief Emitted when the cancel or close button is clicked.
    void Cancelled();

protected:
    /// @brief Custom paint: bubble background + shadow + arrow.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    /// @brief Shared initialization called by all constructors.
    void InitLayout();

    /// @brief Update the state icon pixmap.
    void ApplyStateIcon();

    /// @brief Position this popup relative to the anchor widget.
    void PositionRelativeTo(QWidget* anchor);

    static constexpr int kArrowSize   = 8;
    static constexpr int kPadding     = 16;
    static constexpr int kButtonGap   = 8;
    static constexpr int kFixedWidth  = 312;
    static constexpr int kIconSize    = 16;

    PopConfirmState _state      = PopConfirmState::Info;
    PopConfirmCode  _resultCode = PopConfirmCode::CancelCode;
    ArrowPosition   _arrowPos   = ArrowPosition::TopCenter;

    NyanLabel*      _iconLabel    = nullptr;
    NyanLabel*      _titleLabel   = nullptr;
    NyanLabel*      _messageLabel = nullptr;
    NyanPushButton* _confirmBtn   = nullptr;
    NyanPushButton* _denyBtn      = nullptr;
    NyanPushButton* _cancelBtn    = nullptr;
    NyanPushButton* _closeBtn     = nullptr;
};

} // namespace matcha::gui
