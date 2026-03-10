#pragma once

/**
 * @file NyanStatusBar.h
 * @brief Item-based application bottom status bar.
 *
 * NyanStatusBar is a horizontal container that holds StatusBarItem children.
 * Every item is clamped to the bar's fixed height (24px). Items are arranged
 * into left-aligned and right-aligned groups separated by a stretch spacer.
 *
 * @see StatusBarNode for the UiNode-level API.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/WidgetEnums.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <vector>

#include <QWidget>

class QHBoxLayout;

namespace matcha::gui {

/**
 * @brief Item-based application bottom status bar.
 *
 * All children are QWidget-based items clamped to 24px height.
 * A11y role: StatusBar.
 */
class MATCHA_EXPORT NyanStatusBar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /// @brief Construct the status bar.
    /// @param parent Optional parent widget.
    explicit NyanStatusBar(QWidget* parent = nullptr);

    ~NyanStatusBar() override;

    NyanStatusBar(const NyanStatusBar&)            = delete;
    NyanStatusBar& operator=(const NyanStatusBar&) = delete;
    NyanStatusBar(NyanStatusBar&&)                 = delete;
    NyanStatusBar& operator=(NyanStatusBar&&)      = delete;

    // -- Item management --

    /// @brief Add a widget item to the status bar.
    /// @param id Unique string identifier for lookup/removal.
    /// @param widget The widget to insert (reparented to this). Height clamped.
    /// @param side Left or Right alignment group.
    /// @return The widget pointer, or nullptr if id already exists.
    auto AddItem(const QString& id, QWidget* widget, StatusBarSide side) -> QWidget*;

    /// @brief Remove an item by id. The widget is hidden and deleted later.
    /// @return true if found and removed.
    auto RemoveItem(const QString& id) -> bool;

    /// @brief Find an item widget by id.
    /// @return The widget, or nullptr if not found.
    [[nodiscard]] auto FindItem(const QString& id) const -> QWidget*;

    /// @brief Number of items currently in the bar.
    [[nodiscard]] auto ItemCount() const -> int;

    /// @brief Fixed bar height constant.
    static constexpr int kHeight = 24;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    struct ItemEntry {
        QString   id;
        QWidget*  widget = nullptr;
        StatusBarSide side = StatusBarSide::Left;
    };

    static constexpr int kHPadding = 8;

    QHBoxLayout*          _layout      = nullptr;
    int                   _stretchIndex = -1;
    std::vector<ItemEntry> _items;

    void RebuildLayout();
};

} // namespace matcha::gui
