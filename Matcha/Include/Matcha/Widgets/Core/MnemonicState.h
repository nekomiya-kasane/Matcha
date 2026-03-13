#pragma once

/**
 * @file MnemonicState.h
 * @brief Global mnemonic (access key) visibility state and text rendering utilities.
 *
 * MnemonicState is a QObject singleton that:
 * 1. Tracks whether mnemonic underlines should be visible (Alt held, AlwaysShow, HighContrast)
 * 2. Parses '&' syntax from label strings to extract mnemonic characters
 * 3. Provides a DrawMnemonicText() helper for rendering text with optional underline
 *
 * Lifecycle: Created by Application::Initialize(), destroyed by Application::Shutdown().
 * Global accessor follows the same pattern as IThemeService / FocusManager.
 *
 * @see Matcha_Design_System_Specification.md Section 18.11 for full specification.
 */

#include <Matcha/Foundation/Macros.h>

#include <QObject>
#include <QString>

class QPainter;
class QRect;

namespace matcha::gui {

/**
 * @brief Parsed result from MnemonicState::Parse().
 */
struct MnemonicParseResult {
    QString displayText;    ///< Text with '&' markers removed (and '&&' collapsed to '&').
    QChar   mnemonicChar;   ///< The mnemonic character, or QChar::Null if none.
    int     underlineIndex; ///< Index of the mnemonic char in displayText, or -1 if none.
};

/**
 * @brief Global mnemonic underline visibility state and text utilities.
 *
 * Widget layer (matcha::gui) class. Depends on QPainter/QFontMetrics for rendering.
 * All widgets that render mnemonic text query ShouldShowUnderline() in paintEvent()
 * and connect to UnderlineVisibilityChanged for repaint triggers.
 */
class MATCHA_EXPORT MnemonicState : public QObject {
    Q_OBJECT

public:
    explicit MnemonicState(QObject* parent = nullptr);
    ~MnemonicState() override;

    MnemonicState(const MnemonicState&) = delete;
    auto operator=(const MnemonicState&) -> MnemonicState& = delete;
    MnemonicState(MnemonicState&&) = delete;
    auto operator=(MnemonicState&&) -> MnemonicState& = delete;

    // -- Static parsing (pure, no state dependency) --

    /**
     * @brief Parse a raw label string containing '&' mnemonic markers.
     *
     * Rules:
     * - '&X' marks X as the mnemonic character (first occurrence only).
     * - '&&' is an escaped literal '&' (no mnemonic).
     * - Trailing '&' at end of string is ignored.
     *
     * @param rawText Raw label text (e.g., "&File", "Save &As...", "Zoom && Pan").
     * @return Parsed result with display text, mnemonic char, and underline index.
     */
    [[nodiscard]] static auto Parse(const QString& rawText) -> MnemonicParseResult;

    // -- Global visibility state --

    /**
     * @brief Whether mnemonic underlines should currently be rendered.
     *
     * Returns true if any of: _altHeld, _alwaysShow is true.
     */
    [[nodiscard]] auto ShouldShowUnderline() const -> bool;

    /**
     * @brief Set whether the Alt key is currently held.
     *
     * Called by the Shell-level event filter on Alt press/release.
     * Emits UnderlineVisibilityChanged if the effective visibility changes.
     */
    void SetAltHeld(bool held);

    /**
     * @brief Query whether the Alt key is currently held.
     */
    [[nodiscard]] auto IsAltHeld() const -> bool;

    /**
     * @brief Enter Alt-tap activated mode (Alt pressed and released alone).
     *
     * In this mode, underlines remain visible until Deactivate() is called
     * (on Esc, mnemonic activation, or click-elsewhere). This matches the
     * Windows/Office behavior where Alt-tap enters KeyTip/mnemonic mode.
     */
    void SetAltActivated(bool activated);

    /**
     * @brief Query whether Alt-tap activated mode is active.
     */
    [[nodiscard]] auto IsAltActivated() const -> bool;

    /**
     * @brief Exit Alt-tap activated mode. Convenience for SetAltActivated(false).
     */
    void Deactivate();

    /**
     * @brief Set always-show mode (e.g., OS accessibility setting or HighContrast).
     *
     * When true, underlines are visible regardless of Alt state.
     * Emits UnderlineVisibilityChanged if the effective visibility changes.
     */
    void SetAlwaysShow(bool always);

    /**
     * @brief Query whether always-show mode is active.
     */
    [[nodiscard]] auto IsAlwaysShow() const -> bool;

    /**
     * @brief Query the OS "Always underline access keys" setting.
     *
     * On Windows, reads SPI_GETKEYBOARDCUES via SystemParametersInfo.
     * On other platforms, returns false.
     * Call once at startup and pass the result to SetAlwaysShow().
     */
    [[nodiscard]] static auto QueryOsKeyboardCues() -> bool;

    // -- Rendering helper --

    /**
     * @brief Draw label text with optional mnemonic underline.
     *
     * Parses the rawText for '&' markers, draws the display text, and
     * optionally draws a 1px underline beneath the mnemonic character.
     *
     * @param painter  Active QPainter (font and pen must be set by caller).
     * @param rect     Bounding rectangle for text layout.
     * @param flags    Qt::AlignmentFlag combination for text alignment.
     * @param rawText  Raw label text with '&' markers.
     * @param showUnderline  Whether to draw the mnemonic underline.
     */
    static void DrawMnemonicText(QPainter& painter, const QRect& rect,
                                  int flags, const QString& rawText,
                                  bool showUnderline);

Q_SIGNALS:
    /**
     * @brief Emitted when the effective underline visibility changes.
     *
     * All widgets rendering mnemonic text should connect this signal
     * to their update() slot for repaint.
     *
     * @param visible New visibility state.
     */
    void UnderlineVisibilityChanged(bool visible);

private:
    bool _altHeld      = false;
    bool _altActivated = false;
    bool _alwaysShow   = false;
};

// ============================================================================
// Global MnemonicState Accessor
// ============================================================================

/// @brief Set the global MnemonicState instance. Called once by Application::Initialize().
MATCHA_EXPORT void SetMnemonicState(MnemonicState* state);

/// @brief Get the global MnemonicState instance (may be nullptr before Initialize).
[[nodiscard]] MATCHA_EXPORT auto GetMnemonicState() -> MnemonicState*;

/// @brief Check if a global MnemonicState has been set.
[[nodiscard]] MATCHA_EXPORT auto HasMnemonicState() -> bool;

} // namespace matcha::gui
