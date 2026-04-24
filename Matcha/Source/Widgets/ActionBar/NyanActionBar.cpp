#include <Matcha/Widgets/ActionBar/NyanActionBar.h>
#include <Matcha/Widgets/ActionBar/NyanActionToolbar.h>
#include <Matcha/Widgets/Shell/NyanVerticalTabBar.h>

#include <QLabel>
#include <QTabBar>
#include <QToolButton>

namespace matcha::gui {

NyanActionBar::NyanActionBar(QWidget* parent)
    : QTabWidget(parent)
    , ThemeAware(WidgetKind::ActionBar)
{
    // Install custom tab bar that supports vertical text rendering
    _vertTabBar = new NyanVerticalTabBar(this);
    setTabBar(_vertTabBar);

    InitCollapseButton();
    InitMiniButton();
    InitStyle();

    connect(this, &QTabWidget::currentChanged, this, [this](int index) {
        if (index >= 0) {
            QString tabId = tabBar()->tabData(index).toString();
            Q_EMIT TabSwitched(tabId);
        }
    });
}

NyanActionBar::~NyanActionBar() = default;

void NyanActionBar::InitStyle()
{
    RebuildStyleSheet();
}

void NyanActionBar::RebuildStyleSheet()
{
    const auto& theme = Theme();

    const auto paneBg     = theme.Color(ColorToken::colorFillTertiary).name();
    const auto tabBg      = theme.Color(ColorToken::colorFillTertiary).name();
    const auto tabFg      = theme.Color(ColorToken::colorTextSecondary).name();
    const auto& tabSelBg  = paneBg;
    const auto tabSelFg   = theme.Color(ColorToken::colorText).name();
    const auto tabHoverBg = theme.Color(ColorToken::colorFillTertiary).name();
    const auto border     = theme.Color(ColorToken::colorBorder).name();

    // Pane border-radius and hidden border edge depend on dock side
    // Tab border-radius: rounded corners face away from content
    QString paneBorder;
    QString paneRadius;
    QString tabRadius;
    QString tabMargin;

    switch (_dockSide) {
    case DockSide::Bottom: // tab bar on top, pane below
        paneBorder = "border-top: none;";
        paneRadius = "border-bottom-left-radius: 6px;"
                     "border-bottom-right-radius: 6px;";
        tabRadius  = "border-top-left-radius: 6px;"
                     "border-top-right-radius: 6px;";
        tabMargin  = "margin-right: 2px;";
        break;
    case DockSide::Top: // tab bar on bottom, pane above
        paneBorder = "border-bottom: none;";
        paneRadius = "border-top-left-radius: 6px;"
                     "border-top-right-radius: 6px;";
        tabRadius  = "border-bottom-left-radius: 6px;"
                     "border-bottom-right-radius: 6px;";
        tabMargin  = "margin-right: 2px;";
        break;
    case DockSide::Left: // tab bar on right, pane on left
        paneBorder = "border-right: none;";
        paneRadius = "border-top-left-radius: 6px;"
                     "border-bottom-left-radius: 6px;";
        tabRadius  = "border-top-right-radius: 6px;"
                     "border-bottom-right-radius: 6px;";
        tabMargin  = "margin-bottom: 2px;";
        break;
    case DockSide::Right: // tab bar on left, pane on right
        paneBorder = "border-left: none;";
        paneRadius = "border-top-right-radius: 6px;"
                     "border-bottom-right-radius: 6px;";
        tabRadius  = "border-top-left-radius: 6px;"
                     "border-bottom-left-radius: 6px;";
        tabMargin  = "margin-bottom: 2px;";
        break;
    }

    QString style = QString(
        "QTabWidget { background: transparent; }"
        "QTabWidget::pane {"
        "  background-color: %1;"
        "  border: 1px solid %7;"
        "  %8"
        "  %9"
        "}"
        "QTabBar {"
        "  background-color: %1;"
        "  qproperty-drawBase: 0;"
        "}"
        "QTabBar::tab {"
        "  background-color: %2;"
        "  color: %3;"
        "  padding: 4px 12px;"
        "  %10"
        "  %11"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: %4;"
        "  color: %5;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background-color: %6;"
        "}"
    ).arg(paneBg, tabBg, tabFg, tabSelBg, tabSelFg, tabHoverBg, border,
          paneBorder, paneRadius, tabMargin, tabRadius);

    setStyleSheet(style);
}

// -- Tabs --

auto NyanActionBar::AddTab(const QString& name, TabPersistence persistence) -> QString
{
    QString tabId = GenerateTabId();

    auto* tab = new NyanActionTab(this);
    tab->SetPersistence(persistence);
    tab->setObjectName(tabId);

    int index = QTabWidget::addTab(tab, name);
    tabBar()->setTabData(index, tabId);

    _tabs[tabId] = tab;

    ConnectTab(tabId, tab);

    return tabId;
}

auto NyanActionBar::AddTab(NyanActionTab* tab) -> QString
{
    if (tab == nullptr) {
        return {};
    }

    QString tabId = tab->objectName();
    if (tabId.isEmpty()) {
        tabId = GenerateTabId();
        tab->setObjectName(tabId);
    }

    QString label = tab->Label();
    if (label.isEmpty()) {
        label = tabId;
    }

    int index = QTabWidget::addTab(tab, label);
    tabBar()->setTabData(index, tabId);

    _tabs[tabId] = tab;
    ConnectTab(tabId, tab);

    return tabId;
}

void NyanActionBar::RemoveTab(NyanActionTab* tab)
{
    if (tab == nullptr) {
        return;
    }

    QString tabId;
    for (auto it = _tabs.begin(); it != _tabs.end(); ++it) {
        if (it->second == tab) {
            tabId = it->first;
            _tabs.erase(it);
            break;
        }
    }

    if (tabId.isEmpty()) {
        return;
    }

    for (int i = 0; i < count(); ++i) {
        if (tabBar()->tabData(i).toString() == tabId) {
            QTabWidget::removeTab(i);
            break;
        }
    }
    // NOTE: does NOT delete the widget -- caller (node layer) owns it
}

void NyanActionBar::RemoveTab(const QString& tabId)
{
    auto it = _tabs.find(tabId);
    if (it == _tabs.end()) {
        return;
    }

    // Find tab index
    for (int i = 0; i < count(); ++i) {
        if (tabBar()->tabData(i).toString() == tabId) {
            QTabWidget::removeTab(i);
            break;
        }
    }

    delete it->second;
    _tabs.erase(it);
}

void NyanActionBar::RemoveTabsByPersistence(TabPersistence persistence)
{
    std::vector<QString> toRemove;
    for (const auto& [id, tab] : _tabs) {
        if (tab->Persistence() == persistence) {
            toRemove.push_back(id);
        }
    }

    for (const auto& id : toRemove) {
        RemoveTab(id);
    }
}

auto NyanActionBar::Tab(const QString& tabId) -> NyanActionTab*
{
    auto it = _tabs.find(tabId);
    return it != _tabs.end() ? it->second : nullptr;
}

auto NyanActionBar::CurrentTabId() const -> QString
{
    int index = currentIndex();
    if (index < 0) {
        return {};
    }
    return tabBar()->tabData(index).toString();
}

void NyanActionBar::SwitchTab(const QString& tabId)
{
    for (int i = 0; i < count(); ++i) {
        if (tabBar()->tabData(i).toString() == tabId) {
            setCurrentIndex(i);
            break;
        }
    }
}

auto NyanActionBar::TabCount() const -> int
{
    return static_cast<int>(_tabs.size());
}

// -- Display --

void NyanActionBar::SetDisplayMode(ActionBarDisplayMode mode)
{
    _displayMode = mode;
    // Display mode affects button layout in toolbars
    // This would be propagated to child toolbars
    update();
}

auto NyanActionBar::DisplayMode() const -> ActionBarDisplayMode
{
    return _displayMode;
}

void NyanActionBar::SetDockSide(DockSide side)
{
    _dockSide = side;

    switch (side) {
    case DockSide::Bottom:
        setTabPosition(QTabWidget::North);
        _vertTabBar->setShape(QTabBar::RoundedNorth);
        SetOrientation(Qt::Horizontal);
        break;
    case DockSide::Top:
        setTabPosition(QTabWidget::South);
        _vertTabBar->setShape(QTabBar::RoundedSouth);
        SetOrientation(Qt::Horizontal);
        break;
    case DockSide::Left:
        setTabPosition(QTabWidget::East);
        _vertTabBar->setShape(QTabBar::RoundedEast);
        SetOrientation(Qt::Vertical);
        _vertTabBar->SetRotateClockwise(true); // read top-to-bottom
        break;
    case DockSide::Right:
        setTabPosition(QTabWidget::West);
        _vertTabBar->setShape(QTabBar::RoundedWest);
        SetOrientation(Qt::Vertical);
        _vertTabBar->SetRotateClockwise(false); // read top-to-bottom
        break;
    }

    RebuildStyleSheet();
}

auto NyanActionBar::GetDockSide() const -> DockSide
{
    return _dockSide;
}

// -- Dock state --

void NyanActionBar::SetDocked(bool docked)
{
    if (_docked == docked) { return; }
    _docked = docked;
    UpdateCollapseVisuals();
}

auto NyanActionBar::IsDocked() const -> bool
{
    return _docked;
}

// -- Collapse --

void NyanActionBar::SetCollapsed(bool collapsed)
{
    if (_collapsed == collapsed) {
        return;
    }

    _collapsed = collapsed;
    // Emit FIRST so ActionBarOverlayFilter::Reposition() can capture
    // widget positions while ActionBar is still visible, then adjust size.
    Q_EMIT CollapsedChanged(collapsed);
    UpdateCollapseVisuals();
}

auto NyanActionBar::IsCollapsed() const -> bool
{
    return _collapsed;
}

auto NyanActionBar::CollapseButton() const -> QToolButton*
{
    return _collapseBtn;
}

auto NyanActionBar::MiniButton() const -> QToolButton*
{
    return _miniButton;
}

void NyanActionBar::InitCollapseButton()
{
    _collapseBtn = new QToolButton(this);
    _collapseBtn->setObjectName(QStringLiteral("actionbar-collapse-btn"));
    _collapseBtn->setToolTip(tr("Collapse ActionBar"));
    _collapseBtn->setAutoRaise(true);
    _collapseBtn->setFixedSize(20, 20);
    _collapseBtn->setText(QStringLiteral("\u25B2")); // up-pointing triangle

    // Place as left corner widget of the tab widget
    setCornerWidget(_collapseBtn, Qt::TopLeftCorner);

    connect(_collapseBtn, &QToolButton::clicked, this, [this]() {
        SetCollapsed(true);
    });
}

void NyanActionBar::InitMiniButton()
{
    // Mini-button: a small QToolButton shown when undocked + collapsed.
    // Clicking it expands the ActionBar back in-place.
    _miniButton = new QToolButton(parentWidget());
    _miniButton->setObjectName(QStringLiteral("actionbar-mini-btn"));
    _miniButton->setFixedSize(kMiniButtonWidth, kMiniButtonHeight);
    _miniButton->setCursor(Qt::PointingHandCursor);
    _miniButton->setAutoRaise(true);
    _miniButton->setText(QStringLiteral("\u25BC")); // down-pointing triangle
    _miniButton->setToolTip(tr("Expand ActionBar"));
    _miniButton->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  background: rgba(60, 60, 60, 200);"
        "  color: #cccccc;"
        "  border: 1px solid rgba(100, 100, 100, 150);"
        "  border-radius: 4px;"
        "  font-size: 10px;"
        "}"
        "QToolButton:hover {"
        "  background: rgba(80, 80, 80, 220);"
        "  color: #ffffff;"
        "}"));
    _miniButton->hide();

    connect(_miniButton, &QToolButton::clicked, this, [this]() {
        SetCollapsed(false);
    });
}

void NyanActionBar::UpdateCollapseVisuals()
{
    // NOTE: This method ONLY manages ActionBar's own size constraints.
    // All hide/show/positioning of ActionBar, FloatingFrame, MiniButton,
    // and TrapezoidHandle is handled by ActionBarOverlayFilter::Reposition()
    // which is triggered via the CollapsedChanged signal.
    if (_collapsed) {
        hide();
    } else {
        if (_orientation == Qt::Horizontal) {
            setMaximumWidth(QWIDGETSIZE_MAX);
            setFixedHeight(kTabBarHeight + kContentHeight);
        } else {
            setMaximumHeight(QWIDGETSIZE_MAX);
            setFixedWidth(kTabBarHeight + kContentHeight);
        }
        show();
    }
}

// -- Orientation --

void NyanActionBar::SetOrientation(Qt::Orientation orientation)
{
    if (_orientation == orientation) { return; }
    _orientation = orientation;

    // Update vertical tab bar mode
    _vertTabBar->SetVertical(_orientation == Qt::Vertical);

    // Propagate to all child tabs
    for (auto& [id, tab] : _tabs) {
        tab->SetOrientation(orientation);
    }

    // Update size constraints
    if (_orientation == Qt::Horizontal) {
        setMaximumWidth(QWIDGETSIZE_MAX);
        setFixedHeight(kTabBarHeight + kContentHeight);
    } else {
        setMaximumHeight(QWIDGETSIZE_MAX);
        setFixedWidth(kTabBarHeight + kContentHeight);
    }

    updateGeometry();
    update();
}

auto NyanActionBar::Orientation() const -> Qt::Orientation
{
    return _orientation;
}

auto NyanActionBar::VerticalTabBar() const -> NyanVerticalTabBar*
{
    return _vertTabBar;
}

// -- Size hints --

auto NyanActionBar::sizeHint() const -> QSize
{
    if (_orientation == Qt::Horizontal) {
        int maxTabWidth = 0;
        for (const auto& [id, tab] : _tabs) {
            int tw = tab->sizeHint().width();
            if (tw > maxTabWidth) { maxTabWidth = tw; }
        }
        int tabBarWidth = tabBar()->sizeHint().width();
        int contentWidth = std::max(maxTabWidth, tabBarWidth) + 16;
        contentWidth = std::max(contentWidth, 300);
        return {contentWidth, kTabBarHeight + kContentHeight};
    }

    // Vertical
    int maxTabHeight = 0;
    for (const auto& [id, tab] : _tabs) {
        int th = tab->sizeHint().height();
        if (th > maxTabHeight) { maxTabHeight = th; }
    }
    int tabBarHeight = tabBar()->sizeHint().height();
    int contentHeight = std::max(maxTabHeight, tabBarHeight) + 16;
    contentHeight = std::max(contentHeight, 300);
    return {kTabBarHeight + kContentHeight, contentHeight};
}

auto NyanActionBar::minimumSizeHint() const -> QSize
{
    if (_orientation == Qt::Horizontal) {
        return {200, kTabBarHeight + kContentHeight};
    }
    return {kTabBarHeight + kContentHeight, 200};
}

void NyanActionBar::OnThemeChanged()
{
    InitStyle();
    update();
}

void NyanActionBar::ConnectTab(const QString& tabId, NyanActionTab* tab)
{
    connect(tab, &NyanActionTab::ButtonClicked, this,
        [this, tabId](const QString& toolbarId, const QString& buttonId, bool checked) {
            Q_EMIT ButtonClicked(tabId, toolbarId, buttonId, checked);
        });
}

auto NyanActionBar::GenerateTabId() -> QString
{
    return QString("tab_%1").arg(_nextTabId++);
}

} // namespace matcha::gui
