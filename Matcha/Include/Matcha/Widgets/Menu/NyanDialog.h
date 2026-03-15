#pragma once

/**
 * @file NyanDialog.h
 * @brief Framework-managed dialog with modality support.
 *
 * NyanDialog provides:
 * - 3 modality types: Modal, SemiModal, Modeless
 * - Title bar with icon and window buttons
 * - Footer bar with Confirm/Apply/Cancel buttons
 * - Content area for custom widgets
 * - Free-drag, optional resize
 *
 * @par Modality types
 * - Modal: Blocks all windows
 * - SemiModal: Blocks parent + siblings, allows viewport interaction
 * - Modeless: Allows concurrent interaction
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QDialog>

class QScrollArea;
class QVBoxLayout;

namespace matcha::gui {

class NyanDialogTitleBar;
class NyanDialogFootBar;

/**
 * @brief Dialog modality type.
 */
enum class DialogModality : uint8_t {
    Modal,      ///< Blocks all windows
    SemiModal,  ///< Blocks parent + siblings, allows viewport interaction
    Modeless    ///< Allows concurrent interaction
};

/**
 * @brief Dialog result code.
 */
enum class DialogResult : uint8_t {
    Accepted,   ///< User confirmed
    Rejected,   ///< User cancelled
    Cancelled   ///< Dialog was closed without explicit action
};

/**
 * @brief Framework-managed dialog with modality support.
 *
 * A11y role: Dialog.
 */
class MATCHA_EXPORT NyanDialog : public QDialog, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a dialog.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanDialog(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanDialog() override;

    NyanDialog(const NyanDialog&)            = delete;
    NyanDialog& operator=(const NyanDialog&) = delete;
    NyanDialog(NyanDialog&&)                 = delete;
    NyanDialog& operator=(NyanDialog&&)      = delete;

    // -- Title Bar --

    /// @brief Get the title bar.
    [[nodiscard]] auto TitleBar() -> NyanDialogTitleBar*;

    /// @brief Set the dialog title.
    void SetTitle(const QString& title);

    /// @brief Get the dialog title.
    [[nodiscard]] auto Title() const -> QString;

    /// @brief Set the dialog icon.
    void SetIcon(const QIcon& icon);

    // -- Footer Bar --

    /// @brief Get the footer bar.
    [[nodiscard]] auto FootBar() -> NyanDialogFootBar*;

    /// @brief Set footer bar visible.
    void SetFootBarVisible(bool visible);

    /// @brief Check if footer bar is visible.
    [[nodiscard]] auto IsFootBarVisible() const -> bool;

    // -- Content --

    /// @brief Set the content widget.
    void SetContent(QWidget* content);

    /// @brief Get the content widget.
    [[nodiscard]] auto Content() const -> QWidget*;

    // -- Modality --

    /// @brief Set the dialog modality.
    void SetDialogModality(DialogModality modality);

    /// @brief Get the dialog modality.
    [[nodiscard]] auto Modality() const -> DialogModality;

    // -- Show --

    /// @brief Show the dialog as modal and return result.
    [[nodiscard]] auto ShowModal() -> DialogResult;

    /// @brief Show the dialog as modeless.
    void ShowModeless();

    /// @brief Switch to embedded mode (child widget, no OS window frame).
    /// Must be called before show(). Reparents to the given container.
    void SetEmbedded(QWidget* container);

    /// @brief Check if in embedded mode.
    [[nodiscard]] auto IsEmbedded() const -> bool;

    // -- Result --

    /// @brief Get the dialog result.
    [[nodiscard]] auto Result() const -> DialogResult;

    /// @brief Accept the dialog (Confirm clicked).
    void accept() override;

    /// @brief Reject the dialog (Cancel clicked).
    void reject() override;

    // -- Resize --

    /// @brief Set resizable.
    void SetResizable(bool resizable);

    /// @brief Check if resizable.
    [[nodiscard]] auto IsResizable() const -> bool;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when dialog is closed.
    void Closed(DialogResult result);

    /// @brief Emitted when confirm button is clicked.
    void ConfirmClicked();

    /// @brief Emitted when apply button is clicked.
    void ApplyClicked();

    /// @brief Emitted when cancel button is clicked.
    void CancelClicked();

protected:
    /// @brief Paint dialog background.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle close event.
    void closeEvent(QCloseEvent* event) override;

    /// @brief Trap Tab/Shift+Tab within the dialog (focus scope).
    void keyPressEvent(QKeyEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void ConnectSignals();
    void AdjustSizeToContent();
    void OnTitleBarDragStarted(const QPoint& globalPos);
    void OnTitleBarDragMoved(const QPoint& globalPos);

    static constexpr int kMinWidth  = 300;
    static constexpr int kMinHeight = 150;
    static constexpr double kMaxHeightFraction = 0.85;

    QVBoxLayout*        _layout     = nullptr;
    NyanDialogTitleBar* _titleBar   = nullptr;
    QScrollArea*        _scrollArea = nullptr;
    QWidget*            _content    = nullptr;
    NyanDialogFootBar*  _footBar    = nullptr;

    DialogModality      _modality   = DialogModality::Modal;
    DialogResult        _result     = DialogResult::Cancelled;
    bool                _resizable  = true;
    bool                _embedded   = false;

    QPoint              _dragOffset;
};

} // namespace matcha::gui
