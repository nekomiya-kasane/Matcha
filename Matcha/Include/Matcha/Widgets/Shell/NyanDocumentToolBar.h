#pragma once

/**
 * @file NyanDocumentToolBar.h
 * @brief Document toolbar with module combo, tab bar, and global buttons.
 *
 * NyanDocumentToolBar is the second row of the shell header area.
 * It contains:
 * - ModuleCombo (left)
 * - Separator
 * - TabBar slot (stretch)
 * - GlobalButtonContainer (right)
 *
 * Extracted from the former Row 2 of NyanMainTitleBar.
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

class QComboBox;
class QFrame;
class QHBoxLayout;

namespace matcha::gui {

class NyanTabBar;

/**
 * @brief Document toolbar: ModuleCombo + TabBar + GlobalButtons.
 *
 * A11y role: ToolBar.
 */
class MATCHA_EXPORT NyanDocumentToolBar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanDocumentToolBar(QWidget* parent = nullptr);
    ~NyanDocumentToolBar() override;

    NyanDocumentToolBar(const NyanDocumentToolBar&)            = delete;
    NyanDocumentToolBar& operator=(const NyanDocumentToolBar&) = delete;
    NyanDocumentToolBar(NyanDocumentToolBar&&)                 = delete;
    NyanDocumentToolBar& operator=(NyanDocumentToolBar&&)      = delete;

    // -- Tab Bar --

    /// @brief Set the tab bar widget (document tabs).
    void SetTabBar(NyanTabBar* tabBar);

    /// @brief Get the tab bar widget.
    [[nodiscard]] auto GetTabBar() const -> NyanTabBar*;

    // -- Module Combo --

    /// @brief Set the module combo items.
    void SetModuleItems(const QStringList& items);

    /// @brief Get the current module name.
    [[nodiscard]] auto CurrentModule() const -> QString;

    /// @brief Set the current module by name.
    void SetCurrentModule(const QString& name);

    // -- Containers --

    /// @brief Get the global button container (right side).
    [[nodiscard]] auto GlobalButtonContainer() -> QWidget*;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when module selection changes.
    void ModuleChanged(const QString& moduleName);

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    void InitLayout();
    void UpdateStyles();

    static constexpr int kHeight = 36;

    QHBoxLayout* _layout                = nullptr;
    QComboBox*   _moduleCombo           = nullptr;
    QFrame*      _separator             = nullptr;
    NyanTabBar*  _tabBar                = nullptr;
    QWidget*     _globalButtonContainer = nullptr;
};

} // namespace matcha::gui
