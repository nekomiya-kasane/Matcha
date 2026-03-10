#pragma once

/**
 * @file NyanActionBar.h
 * @brief QTabWidget-based ribbon container with 4-layer hierarchy.
 *
 * NyanActionBar provides:
 * - Tab-based ribbon container (ActionBar -> Tab -> Toolbar -> Button)
 * - Display modes: IconOnly, IconText, WideSpacing
 * - Dock sides: Bottom, Left, Right, Top
 * - Tab persistence policies for workbench switching
 *
 * @par Hierarchy
 * - ActionBar contains multiple ActionTab widgets
 * - ActionTab contains multiple ActionToolbar widgets
 * - ActionToolbar contains multiple NyanToolButton widgets
 *
 * @see NyanActionTab for tab container.
 * @see NyanActionToolbar for button groups.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/ActionBar/NyanActionTab.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QTabWidget>

#include <map>

class QToolButton;

namespace matcha::gui {

/**
 * @brief ActionBar display mode.
 */
enum class ActionBarDisplayMode : uint8_t {
    IconOnly,    ///< Show icons only
    IconText,    ///< Show icons with text below
    WideSpacing  ///< Wide spacing between buttons
};

/**
 * @brief ActionBar dock side.
 */
enum class DockSide : uint8_t {
    Bottom,
    Left,
    Right,
    Top
};

class NyanVerticalTabBar;
class TrapezoidHandle;

/**
 * @brief QTabWidget-based ribbon container.
 *
 * Collapse behavior (3 states):
 * - Docked + expanded: CollapseButton at left of TabRow. Full bar visible.
 * - Docked + collapsed: Entire ActionBar hidden. TrapezoidHandle (external)
 *   centered on docked edge is the only visible element.
 * - Undocked + expanded: CollapseButton at left of TabRow. Full bar visible.
 * - Undocked + collapsed: Shrinks to mini-button (64x16). Click to expand.
 */
class MATCHA_EXPORT NyanActionBar : public QTabWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct an action bar.
     * @param theme Theme service reference.
     * @param parent Optional parent widget.
     */
    explicit NyanActionBar(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanActionBar() override;

    NyanActionBar(const NyanActionBar&)            = delete;
    NyanActionBar& operator=(const NyanActionBar&) = delete;
    NyanActionBar(NyanActionBar&&)                 = delete;
    NyanActionBar& operator=(NyanActionBar&&)      = delete;

    // -- Tabs --

    /// @brief Add a tab.
    /// @param name Tab name.
    /// @param persistence Persistence policy.
    /// @return Tab ID, or empty string on failure.
    [[nodiscard]] auto AddTab(const QString& name, TabPersistence persistence = TabPersistence::General) -> QString;

    /// @brief Add an externally-created tab (node layer ownership).
    /// @param tab Tab widget (not owned by ActionBar -- caller manages lifetime).
    /// @return Tab ID assigned by ActionBar.
    auto AddTab(NyanActionTab* tab) -> QString;

    /// @brief Remove a tab by ID.
    void RemoveTab(const QString& tabId);

    /// @brief Remove an externally-created tab (does NOT delete the widget).
    void RemoveTab(NyanActionTab* tab);

    /// @brief Remove tabs by persistence policy.
    void RemoveTabsByPersistence(TabPersistence persistence);

    /// @brief Get tab by ID.
    [[nodiscard]] auto Tab(const QString& tabId) -> NyanActionTab*;

    /// @brief Get current tab ID.
    [[nodiscard]] auto CurrentTabId() const -> QString;

    /// @brief Switch to tab by ID.
    void SwitchTab(const QString& tabId);

    /// @brief Get tab count.
    [[nodiscard]] auto TabCount() const -> int;

    // -- Display --

    /// @brief Set display mode.
    void SetDisplayMode(ActionBarDisplayMode mode);

    /// @brief Get display mode.
    [[nodiscard]] auto DisplayMode() const -> ActionBarDisplayMode;

    /// @brief Set dock side. Updates tab bar shape and orientation.
    void SetDockSide(DockSide side);

    /// @brief Get dock side.
    [[nodiscard]] auto GetDockSide() const -> DockSide;

    // -- Orientation --

    /// @brief Set layout orientation (Horizontal or Vertical).
    /// Propagates to tab bar and all child tabs/toolbars.
    void SetOrientation(Qt::Orientation orientation);

    /// @brief Get current orientation.
    [[nodiscard]] auto Orientation() const -> Qt::Orientation;

    /// @brief Get the custom vertical tab bar.
    [[nodiscard]] auto VerticalTabBar() const -> NyanVerticalTabBar*;

    // -- Dock state --

    /// @brief Set whether the ActionBar is docked to a WorkspaceFrame edge.
    void SetDocked(bool docked);

    /// @brief Whether the ActionBar is currently docked.
    [[nodiscard]] auto IsDocked() const -> bool;

    // -- Collapse --

    /// @brief Set collapsed state. Behavior depends on docked state.
    void SetCollapsed(bool collapsed);

    /// @brief Check if collapsed.
    [[nodiscard]] auto IsCollapsed() const -> bool;

    /// @brief Get the collapse button (left of TabRow). Always exists.
    [[nodiscard]] auto CollapseButton() const -> QToolButton*;

    /// @brief Get the mini-button widget (shown when undocked + collapsed).
    [[nodiscard]] auto MiniButton() const -> QToolButton*;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when tab is switched.
    void TabSwitched(const QString& tabId);

    /// @brief Emitted when a button is clicked.
    void ButtonClicked(const QString& tabId, const QString& toolbarId, const QString& buttonId, bool checked);

    /// @brief Emitted when collapsed state changes.
    void CollapsedChanged(bool collapsed);

protected:
    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitStyle();
    void RebuildStyleSheet();
    void InitCollapseButton();
    void InitMiniButton();
    void UpdateCollapseVisuals();
    void ConnectTab(const QString& tabId, NyanActionTab* tab);
    [[nodiscard]] auto GenerateTabId() -> QString;

    static constexpr int kTabBarHeight    = 26;
    static constexpr int kContentHeight   = 56;
    static constexpr int kMiniButtonWidth = 64;
    static constexpr int kMiniButtonHeight = 16;

    std::map<QString, NyanActionTab*> _tabs;
    ActionBarDisplayMode              _displayMode  = ActionBarDisplayMode::IconOnly;
    DockSide                          _dockSide     = DockSide::Bottom;
    Qt::Orientation                   _orientation  = Qt::Horizontal;
    bool                              _collapsed    = false;
    bool                              _docked       = false;
    int                               _nextTabId    = 0;
    QToolButton*                      _collapseBtn  = nullptr;
    QToolButton*                      _miniButton   = nullptr;
    NyanVerticalTabBar*               _vertTabBar   = nullptr;
};

} // namespace matcha::gui
