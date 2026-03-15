/**
 * @file NyanCadDevToolsWindow.cpp
 * @brief Unified DevTools window: Notification Log + UI Inspector in tabs.
 */

#include "NyanCadDevToolsWindow.h"
#include "LayoutBoundsOverlay.h"
#include "WidgetPickerFilter.h"

#include "Matcha/Tree/Controls/LabelNode.h"
#include "Matcha/Tree/Controls/PlainTextEditNode.h"
#include "Matcha/Tree/Controls/PushButtonNode.h"
#include "Matcha/Event/Notification.h"
#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/WidgetNode.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"
#include "Matcha/Widgets/Shell/NyanScrollBar.h"

#include <QAbstractScrollArea>
#include <QBrush>
#include <QEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <chrono>
#include <format>

namespace nyancad {

namespace {

/// DevTools window: close -> hide (don't destroy, don't affect app exit).
class DevToolsHideOnCloseFilter : public QObject {
public:
    using QObject::QObject;

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override
    {
        if (event->type() == QEvent::Close) {
            event->ignore();
            if (auto* w = qobject_cast<QWidget*>(obj)) {
                w->hide();
            }
            return true;
        }
        return QObject::eventFilter(obj, event);
    }
};

// ---------------------------------------------------------------------------
// Frameless window edge-resize filter (installed on the window itself).
//
// Requires that the window's top-level layout has a margin >= edgeWidth so
// there is a strip of the bare QWidget exposed around child content.
// This strip receives mouse events directly on the window.
// ---------------------------------------------------------------------------

class FramelessResizeFilter : public QObject {
public:
    explicit FramelessResizeFilter(QWidget* window, int edgeWidth = 5)
        : QObject(window), _win(window), _edge(edgeWidth) {}

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override
    {
        if (obj != _win) { return false; }
        switch (event->type()) {
        case QEvent::MouseMove:
            return handleMouseMove(static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonPress:
            return handlePress(static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonRelease:
            return handleRelease();
        default:
            break;
        }
        return false;
    }

private:
    enum Edge : unsigned { None = 0, Left = 1, Right = 2, Top = 4, Bottom = 8 };

    auto edgesAt(const QPoint& pos) const -> unsigned
    {
        unsigned e = None;
        if (pos.x() < _edge)                     { e |= Left; }
        if (pos.x() > _win->width()  - _edge)    { e |= Right; }
        if (pos.y() < _edge)                      { e |= Top; }
        if (pos.y() > _win->height() - _edge)     { e |= Bottom; }
        return e;
    }

    static auto cursorForEdges(unsigned e) -> Qt::CursorShape
    {
        if ((e & (Left | Top))     == (Left | Top))     { return Qt::SizeFDiagCursor; }
        if ((e & (Right | Bottom)) == (Right | Bottom)) { return Qt::SizeFDiagCursor; }
        if ((e & (Left | Bottom))  == (Left | Bottom))  { return Qt::SizeBDiagCursor; }
        if ((e & (Right | Top))    == (Right | Top))    { return Qt::SizeBDiagCursor; }
        if ((e & (Left | Right)) != 0)                  { return Qt::SizeHorCursor; }
        if ((e & (Top | Bottom)) != 0)                  { return Qt::SizeVerCursor; }
        return Qt::ArrowCursor;
    }

    auto handleMouseMove(QMouseEvent* me) -> bool
    {
        if (_resizing) {
            auto gp = me->globalPosition().toPoint();
            auto geo = _startGeo;
            if (_activeEdge & Left)   { geo.setLeft(gp.x()); }
            if (_activeEdge & Right)  { geo.setRight(gp.x()); }
            if (_activeEdge & Top)    { geo.setTop(gp.y()); }
            if (_activeEdge & Bottom) { geo.setBottom(gp.y()); }
            if (geo.width()  >= _win->minimumWidth() &&
                geo.height() >= _win->minimumHeight()) {
                _win->setGeometry(geo);
            }
            return true;
        }
        auto e = edgesAt(me->pos());
        _win->setCursor(e != None ? cursorForEdges(e) : Qt::ArrowCursor);
        return false;
    }

    auto handlePress(QMouseEvent* me) -> bool
    {
        if (me->button() != Qt::LeftButton) { return false; }
        auto e = edgesAt(me->pos());
        if (e == None) { return false; }
        _resizing = true;
        _activeEdge = e;
        _startGeo = _win->geometry();
        return true;
    }

    auto handleRelease() -> bool
    {
        if (!_resizing) { return false; }
        _resizing = false;
        _activeEdge = None;
        return true;
    }

    QWidget* _win;
    int      _edge;
    bool     _resizing   = false;
    unsigned _activeEdge = None;
    QRect    _startGeo;
};

// ---------------------------------------------------------------------------
// Color palette for Inspector keywords
// ---------------------------------------------------------------------------

const QColor kColorNodeType    {100, 180, 255};   // steel blue
const QColor kColorId          {180, 220, 130};   // green-yellow
const QColor kColorHidden      {160, 160, 160};   // gray
const QColor kColorPropKey     {200, 170, 255};   // lavender
const QColor kColorPropSection {255, 180,  80};   // orange
const QColor kColorBoolTrue    {100, 220, 130};   // green
const QColor kColorBoolFalse   {255, 110, 110};   // red
const QColor kColorNumeric     {130, 200, 255};   // light blue

auto NodeTypeString(matcha::fw::NodeType type) -> QString
{
    switch (type) {
    case matcha::fw::NodeType::Shell:            return QStringLiteral("Shell");
    case matcha::fw::NodeType::WindowNode:       return QStringLiteral("WindowNode");
    case matcha::fw::NodeType::TitleBar:         return QStringLiteral("TitleBar");
    case matcha::fw::NodeType::MenuBar:          return QStringLiteral("MenuBar");
    case matcha::fw::NodeType::Menu:             return QStringLiteral("Menu");
    case matcha::fw::NodeType::MenuItem:         return QStringLiteral("MenuItem");
    case matcha::fw::NodeType::ActionBar:        return QStringLiteral("ActionBar");
    case matcha::fw::NodeType::ActionTab:        return QStringLiteral("ActionTab");
    case matcha::fw::NodeType::ActionToolbar:    return QStringLiteral("ActionToolbar");
    case matcha::fw::NodeType::ActionButton:     return QStringLiteral("ActionButton");
    case matcha::fw::NodeType::StatusBar:        return QStringLiteral("StatusBar");
    case matcha::fw::NodeType::Dialog:           return QStringLiteral("Dialog");
    case matcha::fw::NodeType::Container:        return QStringLiteral("Container");
    case matcha::fw::NodeType::WorkspaceFrame:   return QStringLiteral("WorkspaceFrame");
    case matcha::fw::NodeType::ControlBar:       return QStringLiteral("ControlBar");
    case matcha::fw::NodeType::DocumentArea:     return QStringLiteral("DocumentArea");
    case matcha::fw::NodeType::DocumentBar:      return QStringLiteral("DocumentBar");
    case matcha::fw::NodeType::Viewport:         return QStringLiteral("Viewport");
    case matcha::fw::NodeType::ViewportGroup:    return QStringLiteral("ViewportGroup");
    default:                                     return QStringLiteral("Type(%1)").arg(static_cast<int>(type));
    }
}

} // anonymous namespace

// ============================================================================
// Construction
// ============================================================================

NyanCadDevToolsWindow::NyanCadDevToolsWindow()
    : FloatingWindowNode("devtools_window",
                         matcha::fw::WindowId::From(9998),
                         matcha::fw::WindowKind::Floating)
{
}

NyanCadDevToolsWindow::~NyanCadDevToolsWindow()
{
    if (_picker != nullptr) {
        _picker->Deactivate();
    }
}

// ============================================================================
// BuildContent
// ============================================================================

void NyanCadDevToolsWindow::BuildContent(QWidget* contentParent, QVBoxLayout* /*layout*/)
{
    // Window properties
    constexpr int kResizeEdge = 5;
    if (auto* win = Widget()) {
        win->setMinimumSize(700, 500);
        win->resize(820, 600);
        win->setWindowFlags(win->windowFlags() | Qt::WindowStaysOnTopHint);
        win->setMouseTracking(true);
        win->installEventFilter(new FramelessResizeFilter(win, kResizeEdge));
        win->installEventFilter(new DevToolsHideOnCloseFilter(win));

        // Add margin to the top-level layout so a strip of the bare QWidget
        // is exposed around the edges — this strip receives mouse events
        // directly, enabling the window-level FramelessResizeFilter to work.
        if (auto* topLayout = win->layout()) {
            topLayout->setContentsMargins(kResizeEdge, 0, kResizeEdge, kResizeEdge);
            // Top margin = 0 because the title bar already covers the top edge.
            // The title bar drag handles top-edge resize implicitly via drag.
        }
    }

    SetTitle("DevTools");

    // KEY FIX: add content to contentParent's layout (the _centralArea layout),
    // NOT the top-level window layout. This eliminates the blank space bug.
    auto* centralLayout = qobject_cast<QVBoxLayout*>(contentParent->layout());
    if (centralLayout == nullptr) {
        return;
    }

    // -- QTabWidget fills the entire central area --
    auto* tabs = new QTabWidget(contentParent);
    tabs->addTab(BuildNotificationTab(tabs), QStringLiteral("Notifications"));
    tabs->addTab(BuildInspectorTab(tabs), QStringLiteral("Inspector"));
    centralLayout->addWidget(tabs, /*stretch=*/1);

    // -- Create overlay and picker --
    _overlay = new LayoutBoundsOverlay();
    _picker = new WidgetPickerFilter(_overlay);

    QObject::connect(_picker, &WidgetPickerFilter::Picked,
                     _picker, [this](QWidget* w) { OnWidgetPicked(w); });
}

// ============================================================================
// Tab: Notifications
// ============================================================================

auto NyanCadDevToolsWindow::BuildNotificationTab(QWidget* tabParent) -> QWidget*
{
    auto* page = new QWidget(tabParent);
    auto* pageLay = new QVBoxLayout(page);
    pageLay->setContentsMargins(0, 0, 0, 0);
    pageLay->setSpacing(0);

    // Header row: Label + stretch + Clear button
    auto* header = new QWidget(page);
    auto* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(4, 4, 4, 4);
    headerLay->setSpacing(4);

    auto titleNode = std::make_unique<matcha::fw::LabelNode>("devtools-notif-title");
    titleNode->SetText("UiNode Notification Traffic");
    headerLay->addWidget(titleNode->Widget());
    AddNode(std::move(titleNode));

    headerLay->addStretch();

    auto clearNode = std::make_unique<matcha::fw::PushButtonNode>("devtools-notif-clear");
    clearNode->SetText("Clear");
    clearNode->SetFixedSize(60, 24);
    headerLay->addWidget(clearNode->Widget());

    Subscribe(clearNode.get(), "Activated",
        [this](matcha::EventNode& /*subscriber*/, matcha::Notification& /*notif*/) {
            if (_logNode) { _logNode->Clear(); }
            _notifCount = 0;
        });
    AddNode(std::move(clearNode));
    pageLay->addWidget(header);

    // Log area
    auto logNode = std::make_unique<matcha::fw::PlainTextEditNode>("devtools-notif-log");
    logNode->SetReadOnly(true);
    logNode->SetMaximumBlockCount(2000);
    logNode->SetFont("Consolas", 9);
    logNode->SetStyleSheet(
        "QPlainTextEdit {"
        "  background: #1e1e2e;"
        "  color: #cdd6f4;"
        "  border: 1px solid #45475a;"
        "  border-radius: 4px;"
        "}"
    );
    pageLay->addWidget(logNode->Widget(), /*stretch=*/1);
    _logNode = logNode.get();

    // Install NyanScrollBar (theme-aware, auto-hide, hover-expand) on log area
    if (auto* scrollArea = qobject_cast<QAbstractScrollArea*>(logNode->Widget())) {
        scrollArea->setVerticalScrollBar(
            new matcha::gui::NyanScrollBar(Qt::Vertical, scrollArea));
        scrollArea->setHorizontalScrollBar(
            new matcha::gui::NyanScrollBar(Qt::Horizontal, scrollArea));
    }

    AddNode(std::move(logNode));

    return page;
}

// ============================================================================
// Tab: Inspector
// ============================================================================

auto NyanCadDevToolsWindow::BuildInspectorTab(QWidget* tabParent) -> QWidget*
{
    auto* page = new QWidget(tabParent);
    auto* pageLay = new QVBoxLayout(page);
    pageLay->setContentsMargins(0, 0, 0, 0);
    pageLay->setSpacing(0);

    // Toolbar
    auto* toolbar = new QWidget(page);
    auto* toolbarLay = new QHBoxLayout(toolbar);
    toolbarLay->setContentsMargins(4, 4, 4, 4);
    toolbarLay->setSpacing(4);

    _pickerBtn = new QPushButton(QStringLiteral("Pick Widget"), toolbar);
    _pickerBtn->setCheckable(true);
    _pickerBtn->setFixedHeight(24);
    toolbarLay->addWidget(_pickerBtn);

    _boundsBtn = new QPushButton(QStringLiteral("Show Bounds"), toolbar);
    _boundsBtn->setCheckable(true);
    _boundsBtn->setFixedHeight(24);
    toolbarLay->addWidget(_boundsBtn);

    auto* refreshBtn = new QPushButton(QStringLiteral("Refresh"), toolbar);
    refreshBtn->setFixedHeight(24);
    toolbarLay->addWidget(refreshBtn);

    toolbarLay->addStretch();
    pageLay->addWidget(toolbar);

    // Splitter: Tree (left) + Properties (right)
    auto* splitter = new QSplitter(Qt::Horizontal, page);

    _treeWidget = new QTreeWidget(splitter);
    _treeWidget->setHeaderLabels({QStringLiteral("Node"), QStringLiteral("Type"),
                                   QStringLiteral("Id")});
    _treeWidget->setColumnWidth(0, 250);
    _treeWidget->setColumnWidth(1, 100);
    _treeWidget->setAlternatingRowColors(true);
    _treeWidget->setFont(QFont(QStringLiteral("Consolas"), 9));
    splitter->addWidget(_treeWidget);

    _propTable = new QTableWidget(0, 2, splitter);
    _propTable->setHorizontalHeaderLabels({QStringLiteral("Property"),
                                            QStringLiteral("Value")});
    _propTable->horizontalHeader()->setStretchLastSection(true);
    _propTable->setAlternatingRowColors(true);
    _propTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _propTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _propTable->setFont(QFont(QStringLiteral("Consolas"), 9));
    splitter->addWidget(_propTable);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    pageLay->addWidget(splitter, /*stretch=*/1);

    // Signal connections
    QObject::connect(refreshBtn, &QPushButton::clicked, refreshBtn, [this]() {
        RefreshTree();
    });
    QObject::connect(_boundsBtn, &QPushButton::toggled, _boundsBtn, [this](bool /*checked*/) {
        ToggleBounds();
    });
    QObject::connect(_pickerBtn, &QPushButton::toggled, _pickerBtn, [this](bool /*checked*/) {
        TogglePicker();
    });
    QObject::connect(_treeWidget, &QTreeWidget::currentItemChanged,
                     _treeWidget, [this]() { OnTreeItemSelected(); });

    return page;
}

// ============================================================================
// Bind
// ============================================================================

void NyanCadDevToolsWindow::Bind(matcha::fw::Application& app)
{
    _app = &app;

    // Notification interception: this node becomes Shell's parent
    app.GetShell().SetParent(this);
    LogNotification("system", "DevTools is Shell's parent in CommandNode tree");

    // Inspector overlay target
    if (_overlay != nullptr) {
        _overlay->SetTargetWindow(app.MainWindow().Widget());
    }

    RefreshTree();
}

// ============================================================================
// Notification Log
// ============================================================================

auto NyanCadDevToolsWindow::AnalyseNotification(matcha::CommandNode* sender,
                                                  matcha::Notification& notif) -> matcha::PropagationMode
{
    std::string_view senderId = sender != nullptr ? sender->Id() : "(null)";
    LogNotification(senderId, notif.ClassName());
    return matcha::PropagationMode::TransmitToParent;
}

void NyanCadDevToolsWindow::LogNotification(std::string_view senderId, std::string_view className)
{
    if (_logNode == nullptr) { return; }
    ++_notifCount;

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::floor<std::chrono::milliseconds>(now);
    auto dp = std::chrono::floor<std::chrono::days>(time);
    auto tod = std::chrono::hh_mm_ss{time - dp};

    auto line = std::format("[{:02}:{:02}:{:02}.{:03}] #{:4d}  {:<28s} <- {}",
        tod.hours().count(), tod.minutes().count(), tod.seconds().count(),
        tod.subseconds().count(),
        _notifCount, className, senderId);
    _logNode->AppendPlainText(line);
}

// ============================================================================
// Inspector: Tree
// ============================================================================

void NyanCadDevToolsWindow::RefreshTree()
{
    if (_app == nullptr || _treeWidget == nullptr) { return; }

    _treeWidget->clear();
    _itemToNode.clear();

    auto& shell = _app->GetShell();
    auto* shellItem = new QTreeWidgetItem(_treeWidget);
    shellItem->setText(0, QStringLiteral("Shell"));
    shellItem->setText(1, QStringLiteral("Shell"));
    shellItem->setText(2, QString::fromUtf8(shell.Id().data(),
                                             static_cast<int>(shell.Id().size())));

    for (const auto& child : shell.Children()) {
        auto* uiChild = dynamic_cast<matcha::fw::UiNode*>(child.get());
        if (uiChild != nullptr) {
            PopulateNode(shellItem, uiChild);
        }
    }

    _treeWidget->expandToDepth(2);
}

void NyanCadDevToolsWindow::PopulateNode(QTreeWidgetItem* parentItem,
                                          matcha::fw::UiNode* node)
{
    if (node == nullptr) { return; }

    auto* item = new QTreeWidgetItem(parentItem);

    const auto typeStr = NodeTypeString(node->Type());
    const auto idStr = QString::fromUtf8(node->Id().data(),
                                          static_cast<int>(node->Id().size()));
    const auto nameStr = QString::fromUtf8(node->Name().data(),
                                            static_cast<int>(node->Name().size()));

    QString displayName = typeStr;
    if (!nameStr.isEmpty()) {
        displayName += QStringLiteral(" \"%1\"").arg(nameStr);
    }

    auto* widget = node->Widget();
    if (widget != nullptr && !widget->isVisible()) {
        displayName += QStringLiteral(" [hidden]");
    }

    item->setText(0, displayName);
    item->setText(1, typeStr);
    item->setText(2, idStr);

    item->setForeground(1, QBrush(kColorNodeType));
    item->setForeground(2, QBrush(kColorId));
    if (widget != nullptr && !widget->isVisible()) {
        item->setForeground(0, QBrush(kColorHidden));
    }

    _itemToNode[item] = node;

    for (size_t i = 0; i < node->NodeCount(); ++i) {
        PopulateNode(item, node->NodeAt(i));
    }
}

// ============================================================================
// Inspector: Property Panel
// ============================================================================

void NyanCadDevToolsWindow::OnTreeItemSelected()
{
    auto* item = _treeWidget->currentItem();
    if (item == nullptr) { return; }

    auto it = _itemToNode.find(item);
    if (it != _itemToNode.end()) {
        ShowPropertiesForNode(it.value());
        auto* widget = it.value()->Widget();
        if (_overlay != nullptr) {
            _overlay->SetPickedWidget(widget);
        }
    }
}

void NyanCadDevToolsWindow::ShowPropertiesForNode(matcha::fw::UiNode* node)
{
    if (_propTable == nullptr || node == nullptr) { return; }

    _propTable->setRowCount(0);

    auto addRow = [this](const QString& key, const QString& value,
                         const QColor& keyColor = kColorPropKey,
                         const QColor& valColor = QColor()) {
        int row = _propTable->rowCount();
        _propTable->insertRow(row);
        auto* keyItem = new QTableWidgetItem(key);
        keyItem->setForeground(QBrush(keyColor));
        _propTable->setItem(row, 0, keyItem);
        auto* valItem = new QTableWidgetItem(value);
        if (valColor.isValid()) { valItem->setForeground(QBrush(valColor)); }
        _propTable->setItem(row, 1, valItem);
    };

    addRow(QStringLiteral("Id"), QString::fromUtf8(node->Id().data(),
                                                    static_cast<int>(node->Id().size())),
           kColorPropKey, kColorId);
    addRow(QStringLiteral("Name"), QString::fromUtf8(node->Name().data(),
                                                      static_cast<int>(node->Name().size())));
    addRow(QStringLiteral("NodeType"), NodeTypeString(node->Type()),
           kColorPropKey, kColorNodeType);
    addRow(QStringLiteral("ChildCount"), QString::number(node->NodeCount()),
           kColorPropKey, kColorNumeric);

    auto* widget = node->Widget();
    if (widget != nullptr) {
        ShowPropertiesForWidget(widget);
    } else {
        addRow(QStringLiteral("Widget"), QStringLiteral("(none)"),
               kColorPropKey, kColorHidden);
    }
}

void NyanCadDevToolsWindow::ShowPropertiesForWidget(QWidget* widget)
{
    if (_propTable == nullptr || widget == nullptr) { return; }

    auto addRow = [this](const QString& key, const QString& value,
                         const QColor& keyColor = kColorPropKey,
                         const QColor& valColor = QColor()) {
        int row = _propTable->rowCount();
        _propTable->insertRow(row);
        auto* keyItem = new QTableWidgetItem(key);
        keyItem->setForeground(QBrush(keyColor));
        _propTable->setItem(row, 0, keyItem);
        auto* valItem = new QTableWidgetItem(value);
        if (valColor.isValid()) { valItem->setForeground(QBrush(valColor)); }
        _propTable->setItem(row, 1, valItem);
    };

    auto boolColor = [](bool v) { return v ? kColorBoolTrue : kColorBoolFalse; };
    auto boolStr   = [](bool v) { return v ? QStringLiteral("true") : QStringLiteral("false"); };

    addRow(QStringLiteral("--- Widget ---"), QString(), kColorPropSection);

    addRow(QStringLiteral("ClassName"),
           QString::fromUtf8(widget->metaObject()->className()),
           kColorPropKey, kColorNodeType);
    addRow(QStringLiteral("ObjectName"), widget->objectName());
    addRow(QStringLiteral("Visible"), boolStr(widget->isVisible()),
           kColorPropKey, boolColor(widget->isVisible()));
    addRow(QStringLiteral("Enabled"), boolStr(widget->isEnabled()),
           kColorPropKey, boolColor(widget->isEnabled()));

    auto g = widget->geometry();
    addRow(QStringLiteral("Geometry"),
           QStringLiteral("(%1, %2) %3x%4")
               .arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()),
           kColorPropKey, kColorNumeric);

    auto globalPos = widget->mapToGlobal(QPoint(0, 0));
    addRow(QStringLiteral("GlobalPos"),
           QStringLiteral("(%1, %2)").arg(globalPos.x()).arg(globalPos.y()),
           kColorPropKey, kColorNumeric);

    addRow(QStringLiteral("MinimumSize"),
           QStringLiteral("%1x%2").arg(widget->minimumWidth()).arg(widget->minimumHeight()),
           kColorPropKey, kColorNumeric);
    addRow(QStringLiteral("MaximumSize"),
           QStringLiteral("%1x%2").arg(widget->maximumWidth()).arg(widget->maximumHeight()),
           kColorPropKey, kColorNumeric);
    addRow(QStringLiteral("SizeHint"),
           QStringLiteral("%1x%2").arg(widget->sizeHint().width()).arg(widget->sizeHint().height()),
           kColorPropKey, kColorNumeric);

    auto sp = widget->sizePolicy();
    addRow(QStringLiteral("SizePolicy"),
           QStringLiteral("H:%1 V:%2")
               .arg(static_cast<int>(sp.horizontalPolicy()))
               .arg(static_cast<int>(sp.verticalPolicy())),
           kColorPropKey, kColorNumeric);

    const auto dynProps = widget->dynamicPropertyNames();
    if (!dynProps.isEmpty()) {
        addRow(QStringLiteral("--- Dynamic Properties ---"), QString(), kColorPropSection);
    }
    for (const auto& name : dynProps) {
        const auto val = widget->property(name.constData());
        addRow(QString::fromUtf8(name), val.toString());
    }
}

// ============================================================================
// Inspector: Widget Picker
// ============================================================================

void NyanCadDevToolsWindow::OnWidgetPicked(QWidget* widget)
{
    if (widget == nullptr) { return; }

    auto* widgetNode = matcha::fw::WidgetNode::FromWidget(widget);

    if (widgetNode != nullptr) {
        for (auto it = _itemToNode.begin(); it != _itemToNode.end(); ++it) {
            if (it.value() == widgetNode) {
                _treeWidget->setCurrentItem(it.key());
                _treeWidget->scrollToItem(it.key());
                if (_pickerBtn != nullptr) { _pickerBtn->setChecked(false); }
                return;
            }
        }
    }

    // Fallback: show widget properties directly
    _propTable->setRowCount(0);
    ShowPropertiesForWidget(widget);
    if (_pickerBtn != nullptr) { _pickerBtn->setChecked(false); }
}

void NyanCadDevToolsWindow::ToggleBounds()
{
    if (_overlay == nullptr) { return; }
    _overlay->SetOverlayVisible(_boundsBtn->isChecked());
}

void NyanCadDevToolsWindow::TogglePicker()
{
    if (_picker == nullptr) { return; }
    if (_pickerBtn->isChecked()) {
        _picker->Activate();
        if (_overlay != nullptr && !_overlay->IsOverlayVisible()) {
            _overlay->SetOverlayVisible(true);
            _boundsBtn->setChecked(true);
        }
    } else {
        _picker->Deactivate();
    }
}

} // namespace nyancad
