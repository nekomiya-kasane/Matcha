/**
 * @file NyanTabWidget.cpp
 * @brief Implementation of NyanTabWidget themed tabbed container.
 */

#include <Matcha/Widgets/Shell/NyanTabWidget.h>

#include <QFont>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanTabWidget::NyanTabWidget(QWidget* parent)
    : QTabWidget(parent)
    , ThemeAware(WidgetKind::TabWidget)
{
    ApplyTabRoleStyle();
}

NyanTabWidget::~NyanTabWidget() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanTabWidget::SetTabRole(TabRole role)
{
    _tabRole = role;
    ApplyTabRoleStyle();
}

auto NyanTabWidget::GetTabRole() const -> TabRole
{
    return _tabRole;
}

void NyanTabWidget::SetTabsClosable(bool closable)
{
    setTabsClosable(closable);
}

void NyanTabWidget::SetTabsMovable(bool movable)
{
    setMovable(movable);
}

// ============================================================================
// Theme
// ============================================================================

void NyanTabWidget::OnThemeChanged()
{
    ApplyTabRoleStyle();
    update();
}

void NyanTabWidget::ApplyTabRoleStyle()
{
    const auto normal   = Theme().Resolve(WidgetKind::TabWidget, 0, InteractionState::Normal);
    const auto selected = Theme().Resolve(WidgetKind::TabWidget, 0, InteractionState::Selected);
    const auto hovered  = Theme().Resolve(WidgetKind::TabWidget, 0, InteractionState::Hovered);

    // Build a stylesheet based on TabRole + theme colors.
    const QColor bgActive   = selected.background;
    const QColor bgInactive = normal.background;
    const QColor bgBar      = hovered.background;
    const QColor fgActive   = selected.foreground;
    const QColor fgInactive = normal.foreground;
    const QColor accent     = selected.border;
    const QColor border     = normal.border;

    const int radius = static_cast<int>(normal.radiusPx);
    QFont font = normal.font;

    QString tabPadding;
    switch (_tabRole) {
    case TabRole::Document:
        setTabsClosable(true);
        tabPadding = QStringLiteral("padding: 6px 12px;");
        break;
    case TabRole::Settings:
        setTabsClosable(false);
        tabPadding = QStringLiteral("padding: 4px 10px;");
        break;
    case TabRole::Panel:
        setTabsClosable(false);
        tabPadding = QStringLiteral("padding: 3px 8px;");
        break;
    default:
        tabPadding = QStringLiteral("padding: 6px 12px;");
        break;
    }

    const QString css = QStringLiteral(
        "QTabWidget::pane { border: 1px solid %1; background: %2; }"
        "QTabBar::tab { background: %3; color: %4; %5"
        " border-top-left-radius: %6px; border-top-right-radius: %6px;"
        " margin-right: 1px; }"
        "QTabBar::tab:selected { background: %7; color: %8;"
        " border-bottom: 2px solid %9; }"
        "QTabBar::tab:hover:!selected { background: %10; }"
    ).arg(
        border.name(),         // %1
        bgActive.name(),       // %2
        bgInactive.name(),     // %3
        fgInactive.name(),     // %4
        tabPadding,            // %5
        QString::number(radius), // %6
        bgActive.name(),       // %7
        fgActive.name(),       // %8
        accent.name()          // %9
    ).arg(
        bgBar.name()           // %10
    );

    setStyleSheet(css);
    setFont(font);
}

} // namespace matcha::gui
