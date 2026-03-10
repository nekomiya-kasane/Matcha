#pragma once

/**
 * @file NyanTabWidget.h
 * @brief Theme-aware tabbed container with TabRole-based visual styling.
 *
 * Inherits QTabWidget for Qt tab semantics and ThemeAware for design
 * token integration. Supports closable tabs and drag-reorder.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Tab bar: Background2 fill, themed font, rounded top corners
 * - Active tab: Background1 fill, PrimaryNormal bottom accent line
 * - Inactive tab: Background3 fill, Foreground3 text
 * - Close button: small X on hover, Foreground5 color
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QTabWidget>

namespace matcha::gui {

/// @brief Visual role for tab widget -- determines styling variant.
enum class TabRole : uint8_t {
    Document,  ///< Document tabs (accent bottom line, close button)
    Settings,  ///< Settings/preferences tabs (no close button, compact)
    Panel,     ///< Panel tabs (minimal chrome, used in side panels)
    Count_
};

/**
 * @brief Theme-aware tabbed container.
 *
 * Wraps QTabWidget with TabRole-based visual styling, closable tabs,
 * and drag reorder support.
 */
class MATCHA_EXPORT NyanTabWidget : public QTabWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a tab widget.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanTabWidget(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanTabWidget() override;

    NyanTabWidget(const NyanTabWidget&)            = delete;
    NyanTabWidget& operator=(const NyanTabWidget&) = delete;
    NyanTabWidget(NyanTabWidget&&)                 = delete;
    NyanTabWidget& operator=(NyanTabWidget&&)      = delete;

    /// @brief Set the tab visual role.
    void SetTabRole(TabRole role);

    /// @brief Get the current tab role.
    [[nodiscard]] auto GetTabRole() const -> TabRole;

    /// @brief Set whether tabs are closable.
    void SetTabsClosable(bool closable);

    /// @brief Set whether tabs can be reordered by drag.
    void SetTabsMovable(bool movable);

protected:
    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void ApplyTabRoleStyle();

    TabRole _tabRole = TabRole::Document;
};

} // namespace matcha::gui
