#pragma once

/**
 * @file NyanMenu.h
 * @brief Dropdown/submenu container with custom paint and animation.
 *
 * NyanMenu provides:
 * - Popup menu with slide animation (160ms OutCubic)
 * - Keyboard navigation (Up/Down/Enter/Escape)
 * - Submenu hover-expand with triangle safe zone
 * - Screen-edge clamping
 *
 * @par Old project reference
 * Replaces old NyanMenu (QMenu base) with standalone widget using NyanMenuItem.
 *
 * @par Visual specification
 * - Background: Background1 with Line4 border, 3px rounded corners
 * - Animation: slide from cursor direction, 160ms OutCubic
 *
 * @see NyanMenuItem, NyanMenuSeparator, NyanMenuCheckItem
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QTimer>
#include <QWidget>

class QVBoxLayout;
class QPropertyAnimation;

namespace matcha::gui {

class NyanMenuItem;
class NyanMenuSeparator;
class NyanMenuCheckItem;

/**
 * @brief Dropdown/submenu container with animation and keyboard navigation.
 *
 * A11y role: Menu.
 */
class MATCHA_EXPORT NyanMenu : public QWidget, public ThemeAware {
    Q_OBJECT
    Q_PROPERTY(int animationOffset READ AnimationOffset WRITE SetAnimationOffset)

public:
    /**
     * @brief Construct a menu.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanMenu(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanMenu() override;

    NyanMenu(const NyanMenu&)            = delete;
    NyanMenu& operator=(const NyanMenu&) = delete;
    NyanMenu(NyanMenu&&)                 = delete;
    NyanMenu& operator=(NyanMenu&&)      = delete;

    // -- Item Management --

    /// @brief Add a menu item.
    /// @return Pointer to the created item (owned by this menu).
    auto AddItem(const QString& text, const QIcon& icon = {}) -> NyanMenuItem*;

    /// @brief Add a separator.
    /// @return Pointer to the created separator (owned by this menu).
    auto AddSeparator() -> NyanMenuSeparator*;

    /// @brief Add a check item.
    /// @return Pointer to the created check item (owned by this menu).
    auto AddCheckItem(const QString& text, bool checked = false) -> NyanMenuCheckItem*;

    /// @brief Add a submenu.
    /// @return Pointer to the created submenu (owned by this menu).
    auto AddSubmenu(const QString& text, const QIcon& icon = {}) -> NyanMenu*;

    /// @brief Add an arbitrary widget into the menu layout.
    /// @param widget Widget to add (ownership transferred to this menu).
    void AddWidget(QWidget* widget);

    /// @brief Clear all items.
    void Clear();

    /// @brief Get the number of items.
    [[nodiscard]] auto ItemCount() const -> int;

    // -- Popup Control --

    /// @brief Show the menu at the specified global position.
    void Popup(const QPoint& globalPos);

    /// @brief Close the menu and any open submenus.
    void Close();

    /// @brief Check if the menu is currently visible.
    [[nodiscard]] auto IsOpen() const -> bool;

    /// @brief Whether this menu is a submenu (child of another NyanMenu).
    [[nodiscard]] auto IsSubmenu() const -> bool;

    /// @brief Get the parent menu (if this is a submenu), nullptr otherwise.
    [[nodiscard]] auto ParentMenu() const -> NyanMenu*;

    /// @brief Get the currently active (open) submenu, nullptr if none.
    [[nodiscard]] auto ActiveSubmenu() const -> NyanMenu*;

    /// @brief Handle mouse reentry from a child submenu.
    /// Called by MenuNode when mouse exits a submenu into this menu's area.
    /// @param globalPos Global cursor position.
    void HandleExternalMouseMove(const QPoint& globalPos);

    // -- Animation Property --

    [[nodiscard]] auto AnimationOffset() const -> int;
    void SetAnimationOffset(int offset);

Q_SIGNALS:
    /// @brief Emitted when the menu is about to show.
    void AboutToShow();

    /// @brief Emitted when the menu is about to hide.
    void AboutToHide();

    /// @brief Emitted when any item is triggered.
    void ItemTriggered(NyanMenuItem* item);

    /// @brief Emitted when mouse leaves this menu toward a global position.
    /// Used by MenuNode for UiNode-layer event routing to parent menu.
    void MouseExitedToward(QPoint globalPos);

protected:
    /// @brief Custom paint for themed menu.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle show event for animation.
    void showEvent(QShowEvent* event) override;

    /// @brief Handle hide event.
    void hideEvent(QHideEvent* event) override;

    /// @brief Handle keyboard navigation.
    void keyPressEvent(QKeyEvent* event) override;

    /// @brief Handle mouse move for submenu safe zone.
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief Handle focus out to close menu.
    void focusOutEvent(QFocusEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void StartShowAnimation(const QPoint& globalPos);
    void ClampToScreen(QPoint& pos);
    void UpdateHoveredItem(int index);
    void ActivateItem(int index);
    void ShowSubmenu(NyanMenu* submenu, NyanMenuItem* parentItem);
    void HideSubmenu();
    void OnItemTriggered();
    void OnSubmenuHoverTimeout();

    [[nodiscard]] auto ItemAt(int index) const -> QWidget*;
    [[nodiscard]] auto IndexOfItem(QWidget* item) const -> int;

    static constexpr int kAnimationDuration = 160;
    static constexpr int kBorderWidth       = 2;
    static constexpr int kRadius            = 3;
    static constexpr int kSubmenuDelay      = 200;  ///< ms before submenu opens

    QVBoxLayout*        _layout           = nullptr;
    QPropertyAnimation* _showAnimation    = nullptr;
    QTimer*             _submenuTimer     = nullptr;
    NyanMenu*           _activeSubmenu    = nullptr;
    NyanMenuItem*       _pendingSubmenu   = nullptr;
    int                 _hoveredIndex     = -1;
    int                 _animationOffset  = 0;
    QPoint              _lastMousePos;
    QPoint              _submenuAnchor;   ///< For triangle safe zone
    bool                _isSubmenu        = false;
    bool                _explicitClose    = false; ///< True when Close() called programmatically
    bool                _dismissingSubmenu = false; ///< Guard: suppress focusOut during HideSubmenu
};

} // namespace matcha::gui
