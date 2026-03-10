#pragma once

/**
 * @file NyanToolButton.h
 * @brief Theme-aware toolbar button with flyout menu support.
 *
 * Inherits QToolButton for Qt toolbar semantics and ThemeAware for design
 * token integration. Supports flyout popup menus with configurable policy.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanToolBench.h` (buttons embedded in ToolBench)
 * - `old/NyanGuis/Gui/inc/NyanToolBenchStyle.h` (font 11px, icon 16x16)
 *
 * @par Visual preservation
 * Compact button with smaller padding than PushButton. Icon 16x16.
 * Uses Secondary-variant colors from PushButton palette by default:
 * Background4/Foreground1 normal, Background5 hover, Background6 press.
 *
 * @par New features
 * FlyoutPolicy (Simple/LastUsed/MostCommon) per SolidWorks 3-type model.
 * Right-click signal for context actions.
 *
 * @see NyanPushButton for shared painting patterns.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QToolButton>

class QMenu;

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Flyout popup behavior policy.
 *
 * Models the SolidWorks 3-type flyout paradigm:
 * - Simple: always shows the same menu on arrow click
 * - LastUsed: arrow shows menu, clicking button repeats last-used action
 * - MostCommon: arrow shows menu, clicking button triggers most-frequent action
 */
enum class FlyoutPolicy : uint8_t {
    Simple,     ///< Static flyout menu
    LastUsed,   ///< Repeat last-used action on button click
    MostCommon, ///< Repeat most-frequent action on button click

    Count_
};

/**
 * @brief Theme-aware toolbar button with flyout menu support.
 *
 * Compact button for toolbar use. Supports icon-only and icon+text modes.
 * Flyout menu shown via dropdown arrow when a QMenu is attached.
 * Emits `RightClicked()` on right mouse button press for context actions.
 */
class MATCHA_EXPORT NyanToolButton : public QToolButton, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a tool button.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanToolButton(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanToolButton() override;

    NyanToolButton(const NyanToolButton&)            = delete;
    NyanToolButton& operator=(const NyanToolButton&) = delete;
    NyanToolButton(NyanToolButton&&)                 = delete;
    NyanToolButton& operator=(NyanToolButton&&)      = delete;

    /// @brief Set the flyout popup menu (takes no ownership).
    void SetFlyoutMenu(QMenu* menu);

    /// @brief Get the attached flyout menu (may be null).
    [[nodiscard]] auto FlyoutMenu() const -> QMenu*;

    /// @brief Set the flyout behavior policy.
    void SetFlyoutPolicy(FlyoutPolicy policy);

    /// @brief Get the current flyout policy.
    [[nodiscard]] auto GetFlyoutPolicy() const -> FlyoutPolicy;

    /// @brief Compact size: 28x28 default for toolbar use.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Same as sizeHint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted on right mouse button press.
    void RightClicked();

protected:
    /// @brief Custom paint: compact rounded rect + icon using design tokens.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle right-click to emit RightClicked signal.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kDefaultSize  = 28;  ///< Default square size in px
    static constexpr int kIconSize     = 16;  ///< Icon render size in px
    static constexpr int kArrowWidth   = 8;   ///< Dropdown arrow area width
    static constexpr int kArrowSize    = 4;   ///< Arrow triangle half-size

    FlyoutPolicy _flyoutPolicy = FlyoutPolicy::Simple;
    QMenu*       _flyoutMenu   = nullptr;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
