#pragma once

/**
 * @file NyanActionTab.h
 * @brief Tab widget holding toolbars for ActionBar.
 *
 * NyanActionTab provides:
 * - Container for multiple NyanActionToolbar widgets
 * - Persistence policy tag for workbench switching
 * - Horizontal scrollable layout
 *
 * @see NyanActionToolbar for button groups.
 * @see NyanActionBar for the main ribbon container.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

#include <vector>

class QBoxLayout;

namespace matcha::gui {

class NyanActionToolbar;

/**
 * @brief Tab persistence policy.
 *
 * Used by business layer to manage tab lifecycle during workbench switching.
 */
enum class TabPersistence : uint8_t {
    General,    ///< Always visible (e.g., File, Edit)
    Extension,  ///< Visible when extension is active
    Workshop    ///< Visible only in specific workshop
};

/**
 * @brief Tab widget holding toolbars for ActionBar.
 */
class MATCHA_EXPORT NyanActionTab : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct an action tab.
     * @param theme Theme service reference.
     * @param parent Optional parent widget.
     */
    explicit NyanActionTab(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanActionTab() override;

    NyanActionTab(const NyanActionTab&)            = delete;
    NyanActionTab& operator=(const NyanActionTab&) = delete;
    NyanActionTab(NyanActionTab&&)                 = delete;
    NyanActionTab& operator=(NyanActionTab&&)      = delete;

    // -- Toolbars --

    /// @brief Add a toolbar.
    /// @param toolbar Toolbar to add (takes ownership).
    void AddToolbar(NyanActionToolbar* toolbar);

    /// @brief Remove a toolbar.
    void RemoveToolbar(NyanActionToolbar* toolbar);

    /// @brief Get toolbar count.
    [[nodiscard]] auto ToolbarCount() const -> int;

    /// @brief Get toolbar at index.
    [[nodiscard]] auto ToolbarAt(int index) -> NyanActionToolbar*;

    // -- Label --

    /// @brief Set the tab display label.
    void SetLabel(const QString& label);

    /// @brief Get the tab display label.
    [[nodiscard]] auto Label() const -> QString;

    // -- Persistence --

    /// @brief Set persistence policy.
    void SetPersistence(TabPersistence persistence);

    /// @brief Get persistence policy.
    [[nodiscard]] auto Persistence() const -> TabPersistence;

    // -- Orientation --

    /// @brief Set layout orientation (Horizontal or Vertical).
    /// Propagates to child toolbars.
    void SetOrientation(Qt::Orientation orientation);

    /// @brief Get current orientation.
    [[nodiscard]] auto Orientation() const -> Qt::Orientation;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Forwarded from toolbars.
    void ButtonClicked(const QString& toolbarId, const QString& buttonId, bool checked);

protected:
    /// @brief Custom paint for themed background.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void ConnectToolbar(NyanActionToolbar* toolbar);

    static constexpr int kHeight  = 56;
    static constexpr int kSpacing = 8;

    QBoxLayout*                     _layout      = nullptr;
    Qt::Orientation                  _orientation = Qt::Horizontal;
    std::vector<NyanActionToolbar*> _toolbars;
    QString                         _label;
    TabPersistence                  _persistence = TabPersistence::General;
};

} // namespace matcha::gui
