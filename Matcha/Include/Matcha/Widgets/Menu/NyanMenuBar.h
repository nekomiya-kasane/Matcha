#pragma once

/**
 * @file NyanMenuBar.h
 * @brief Horizontal menu bar holding top-level menus.
 *
 * NyanMenuBar provides:
 * - Horizontal layout of menu buttons
 * - Alt-key mnemonic activation
 * - Left/Right keyboard navigation
 * - Auto-open adjacent menu on hover-while-open
 *
 * @par Visual specification (from spec3.md)
 * - Height: 24px
 * - Background: white (upper title bar)
 * - Menu item spacing: 12px
 *
 * @see NyanMenu for dropdown menus.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QElapsedTimer>
#include <QWidget>

class QHBoxLayout;

namespace matcha::gui {

class NyanMenu;

/**
 * @brief Horizontal menu bar holding top-level menus.
 *
 * A11y role: MenuBar.
 */
class MATCHA_EXPORT NyanMenuBar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a menu bar.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanMenuBar(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanMenuBar() override;

    NyanMenuBar(const NyanMenuBar&)            = delete;
    NyanMenuBar& operator=(const NyanMenuBar&) = delete;
    NyanMenuBar(NyanMenuBar&&)                 = delete;
    NyanMenuBar& operator=(NyanMenuBar&&)      = delete;

    // -- Menu Management --

    /// @brief Add a menu with the given title.
    /// @param title Menu title (use & for mnemonic, e.g., "&File").
    /// @return Pointer to the created menu (owned by this menu bar).
    auto AddMenu(const QString& title) -> NyanMenu*;

    /// @brief Remove a menu by pointer.
    void RemoveMenu(NyanMenu* menu);

    /// @brief Get the menu at the given index.
    [[nodiscard]] auto MenuAt(int index) const -> NyanMenu*;

    /// @brief Get the number of menus.
    [[nodiscard]] auto MenuCount() const -> int;

    // -- Size hints --

    /// @brief Size hint: height 24px.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Minimum size hint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when a menu is about to show.
    void MenuAboutToShow(NyanMenu* menu);

protected:
    /// @brief Custom paint for themed menu bar.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle keyboard navigation.
    void keyPressEvent(QKeyEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

    /// @brief Event filter for Alt-key activation.
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct MenuEntry {
        QString title;
        QString mnemonic;  ///< Single character after &
        NyanMenu* menu = nullptr;
        QWidget* button = nullptr;
    };

    void InitLayout();
    void CreateMenuButton(MenuEntry& entry);
    void OnMenuButtonClicked(int index);
    void OnMenuButtonHovered(int index);
    void OpenMenu(int index);
    void CloseActiveMenu();
    void NavigateMenu(int delta);
    [[nodiscard]] auto ExtractMnemonic(const QString& title) -> QPair<QString, QString>;

    static constexpr int kHeight   = 24;
    static constexpr int kSpacing  = 12;
    static constexpr int kPaddingH = 8;

    QHBoxLayout*         _layout       = nullptr;
    QVector<MenuEntry>   _menus;
    int                  _activeIndex    = -1;
    bool                 _menuOpen       = false;
    bool                 _altPressed     = false;
    bool                 _switchingMenu  = false;
    QElapsedTimer        _dismissTimer;    ///< Tracks popup auto-dismiss for click-to-toggle.
    int                  _dismissedIndex  = -1; ///< Which menu was just auto-dismissed.
};

} // namespace matcha::gui
