#include "Matcha/UiNodes/Shell/WindowNode.h"

#include "Matcha/UiNodes/ActionBar/ActionBarNode.h"
#include "Matcha/UiNodes/Shell/ControlBar.h"
#include "Matcha/UiNodes/Document/DocumentArea.h"
#include "Matcha/UiNodes/Shell/DocumentToolBarNode.h"
#include "Matcha/UiNodes/Shell/LogoButtonNode.h"
#include "Matcha/UiNodes/Shell/MainTitleBarNode.h"
#include "Matcha/UiNodes/Shell/StatusBarNode.h"
#include "Matcha/UiNodes/Shell/TitleBarNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Shell/WorkspaceFrame.h"
#include "Matcha/Widgets/ActionBar/ActionBarFloatingFrame.h"
#include "Matcha/Widgets/ActionBar/NyanActionBar.h"
#include "Matcha/Widgets/Shell/NyanDocumentToolBar.h"
#include "Matcha/Widgets/Shell/NyanLogoButton.h"
#include "Matcha/Widgets/Shell/NyanMainTitleBar.h"
#include "Matcha/Widgets/Shell/NyanStatusBar.h"
#include "Matcha/Widgets/ActionBar/TrapezoidHandle.h"
#include "Matcha/Widgets/Core/UpdateGuard.h"

#include <algorithm>
#include <QAbstractButton>
#include <QApplication>
#include <QToolButton>
#include <QCloseEvent>
#include <QGridLayout>
#include <QMainWindow>
#include <QPointer>
#include <QMouseEvent>
#include <QPainter>
#include <QStackedWidget>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace matcha::fw {

// ---------------------------------------------------------------------------
// ActionBarOverlayFilter -- repositions ActionBar, TrapezoidHandle, MiniButton
// ---------------------------------------------------------------------------

namespace {

// ---------------------------------------------------------------------------
// DockPreviewOverlay -- translucent highlight shown during ActionBar drag
// ---------------------------------------------------------------------------

class DockPreviewOverlay : public QWidget {
public:
    explicit DockPreviewOverlay(QWidget* parent)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        hide();
    }

protected:
    void paintEvent(QPaintEvent* /*event*/) override
    {
        QPainter p(this);
        p.fillRect(rect(), QColor(80, 160, 255, 60));
        p.setPen(QPen(QColor(80, 160, 255, 180), 2));
        p.drawRect(rect().adjusted(1, 1, -1, -1));
    }
};

// ---------------------------------------------------------------------------
// DragGhostOverlay -- semi-transparent ghost that follows cursor during drag
// ---------------------------------------------------------------------------

class DragGhostOverlay : public QWidget {
public:
    explicit DragGhostOverlay(QWidget* parent)
        : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint
                        | Qt::WindowStaysOnTopHint)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setFocusPolicy(Qt::NoFocus);
        hide();
    }

    void SetSize(QSize sz)
    {
        resize(sz);
        update();
    }

protected:
    void paintEvent(QPaintEvent* /*event*/) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(80, 160, 255, 200), 2));
        p.setBrush(QColor(50, 55, 65, 140));
        p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 6, 6);

        // Grip indicator
        int gripW = 40;
        int gripX = (width() - gripW) / 2;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(120, 130, 150, 180));
        p.drawRoundedRect(gripX, 5, gripW, 3, 1.5, 1.5);
    }
};

// ---------------------------------------------------------------------------
// ActionBarOverlayFilter -- repositions, collapse, drag-to-dock/undock
// ---------------------------------------------------------------------------

class ActionBarOverlayFilter : public QObject {
public:
    ActionBarOverlayFilter(gui::NyanActionBar* actionBar,
                           gui::TrapezoidHandle* trapezoid,
                           QWidget* parent)
        : QObject(parent)
        , _actionBar(actionBar)
        , _trapezoid(trapezoid)
        , _preview(new DockPreviewOverlay(parent))
        , _floatingFrame(new gui::ActionBarFloatingFrame(parent->window()))
        , _ghost(new DragGhostOverlay(parent->window()))
    {
        _floatingFrame->SetDockTarget(parent);

        // Wire trapezoid click -> expand ActionBar
        QObject::connect(_trapezoid, &gui::TrapezoidHandle::Clicked,
            _actionBar, [this]() { _actionBar->SetCollapsed(false); });

        // Wire ActionBar collapsed state changes -> update trapezoid visibility
        QObject::connect(_actionBar, &gui::NyanActionBar::CollapsedChanged,
            this, [this](bool /*collapsed*/) {
                Reposition(qobject_cast<QWidget*>(QObject::parent()));
            });

        // Wire floating frame drag signals for edge-snap re-docking
        QObject::connect(_floatingFrame, &gui::ActionBarFloatingFrame::DragNearEdge,
            this, [this](QPoint globalPos) {
                auto* container = qobject_cast<QWidget*>(QObject::parent());
                if (container == nullptr) { return; }
                auto edge = DetectEdge(container, globalPos);
                if (edge.has_value()) {
                    ShowEdgePreview(container, edge.value());
                } else {
                    _preview->hide();
                }
            });

        QObject::connect(_floatingFrame, &gui::ActionBarFloatingFrame::DragFinished,
            this, [this](QPoint globalPos) {
                _preview->hide();
                auto* container = qobject_cast<QWidget*>(QObject::parent());
                if (container == nullptr) { return; }
                auto edge = DetectEdge(container, globalPos);
                if (edge.has_value()) {
                    // Re-dock: take ActionBar from floating frame back to container
                    _floatingFrame->TakeActionBar();
                    _floatingFrame->hide();
                    _actionBar->setParent(container);
                    _actionBar->SetDocked(true);
                    _actionBar->SetDockSide(edge.value());
                    if (_actionBar->IsCollapsed()) {
                        _actionBar->SetCollapsed(false);
                    }
                    Reposition(container);
                }
            });

        // Install ourselves on ActionBar to intercept mouse events for drag
        _actionBar->installEventFilter(this);
    }

protected:
    auto eventFilter(QObject* obj, QEvent* ev) -> bool override
    {
        // ActionBar mouse events -> drag handling
        if (obj == _actionBar) {
            return HandleActionBarMouse(ev);
        }

        // During active drag, mouse events go to the container (because
        // ActionBar is hidden and the container holds the mouse grab).
        // Route them through the drag state machine.
        if (_dragging && obj == QObject::parent()) {
            if (ev->type() == QEvent::MouseMove) {
                return HandleDragMove(ev);
            }
            if (ev->type() == QEvent::MouseButtonRelease) {
                return HandleDragRelease(ev);
            }
        }

        // Container layout events -> reposition (skip during drag to avoid flicker)
        if (!_dragPending && !_dragging) {
            if (ev->type() == QEvent::Resize || ev->type() == QEvent::Show
                || ev->type() == QEvent::LayoutRequest) {
                Reposition(qobject_cast<QWidget*>(obj));
            }
        }

        return QObject::eventFilter(obj, ev);
    }

private:
    // -----------------------------------------------------------------------
    // Drag state machine
    // -----------------------------------------------------------------------

    auto HandleActionBarMouse(QEvent* ev) -> bool
    {
        if (ev->type() == QEvent::MouseButtonPress) {
            return HandleDragPress(ev);
        }
        if (ev->type() == QEvent::MouseMove && _dragPending) {
            return HandleDragMove(ev);
        }
        if (ev->type() == QEvent::MouseButtonRelease && _dragPending) {
            return HandleDragRelease(ev);
        }
        return false;
    }

    auto HandleDragPress(QEvent* ev) -> bool
    {
        auto* me = dynamic_cast<QMouseEvent*>(ev);
        if (me != nullptr && me->button() == Qt::LeftButton && IsEmptyArea(me->pos())) {
            _dragPending = true;
            _dragStartPos = me->globalPosition().toPoint();
            _dragOffset = me->pos();
            return false; // let Qt process normally
        }
        return false;
    }

    auto HandleDragMove(QEvent* ev) -> bool
    {
        auto* me = dynamic_cast<QMouseEvent*>(ev);
        if (me == nullptr) { return false; }
        const QPoint globalPos = me->globalPosition().toPoint();

        if (!_dragging) {
            if ((globalPos - _dragStartPos).manhattanLength()
                < QApplication::startDragDistance()) {
                return false;
            }
            BeginDrag();
        }

        // Move ghost to cursor position
        QPoint ghostPos(globalPos.x() - (_ghostSize.width() / 2),
                        globalPos.y() - 10);
        _ghost->move(ghostPos);

        // Update dock preview
        auto* container = qobject_cast<QWidget*>(QObject::parent());
        if (container != nullptr) {
            auto edge = DetectEdge(container, globalPos);
            if (edge.has_value()) {
                _ghost->hide();
                ShowEdgePreview(container, edge.value());
            } else {
                _preview->hide();
                _ghost->show();
            }
        }
        return true;
    }

    auto HandleDragRelease(QEvent* ev) -> bool
    {
        auto* me = dynamic_cast<QMouseEvent*>(ev);
        if (me == nullptr || me->button() != Qt::LeftButton) { return false; }

        const bool wasDragging = _dragging;
        _dragPending = false;
        _dragging = false;
        _ghost->hide();
        _preview->hide();

        // Release mouse grab acquired in BeginDrag
        auto* container = qobject_cast<QWidget*>(QObject::parent());
        if (container != nullptr) {
            container->releaseMouse();
        }

        if (wasDragging) {
            if (container != nullptr) {
                FinalizeDrop(container, me->globalPosition().toPoint());
            }
            return true;
        }
        return false;
    }

    void BeginDrag()
    {
        _dragging = true;

        // Save pre-drag state
        _wasDocked = _actionBar->IsDocked();
        _wasInFloatingFrame = _floatingFrame->isVisible();
        _preDragDockSide = _actionBar->GetDockSide();

        // Compute ghost size = horizontal ActionBar sizeHint (what floating will look like)
        // Temporarily switch to horizontal to get correct sizeHint
        auto origOrientation = _actionBar->Orientation();
        if (origOrientation == Qt::Vertical) {
            _actionBar->SetOrientation(Qt::Horizontal);
        }
        _ghostSize = _actionBar->sizeHint();
        _ghostSize.setWidth(std::max(_ghostSize.width(), 300));
        _ghostSize.setHeight(_ghostSize.height() + 12); // grip + border
        if (origOrientation == Qt::Vertical) {
            _actionBar->SetOrientation(origOrientation);
        }

        // Hide ActionBar (or floating frame) -- only the ghost is visible
        if (_wasInFloatingFrame) {
            _floatingFrame->hide();
        } else {
            _actionBar->hide();
        }

        _ghost->SetSize(_ghostSize);
        _ghost->move(_dragStartPos - QPoint(_ghostSize.width() / 2, 10));
        _ghost->show();
        _ghost->raise();

        // Grab mouse on container so we keep receiving MouseMove/Release
        // even though the ActionBar is now hidden.
        auto* container = qobject_cast<QWidget*>(QObject::parent());
        if (container != nullptr) {
            container->grabMouse();
        }
    }

    /// @brief Check if pos is on empty area (no interactive child widget).
    /// Drag is allowed on: tab bar background, tab content areas, toolbar
    /// backgrounds, stacked widget. Drag is NOT allowed on: buttons,
    /// combo boxes, line edits, or other interactive controls.
    auto IsEmptyArea(QPoint pos) const -> bool
    {
        QWidget* child = _actionBar->childAt(pos);
        if (child == nullptr) {
            return true;
        }
        const QString cls = QString::fromUtf8(child->metaObject()->className());

        // Reject: actual interactive controls (buttons, edits, combos)
        if (qobject_cast<QAbstractButton*>(child) != nullptr) { return false; }
        if (cls.contains(QLatin1String("LineEdit"))) { return false; }
        if (cls.contains(QLatin1String("ComboBox"))) { return false; }

        // Allow: everything else (tab bar, tab content, toolbar bg,
        // stacked widget, scroll areas, labels, etc.)
        return true;
    }

    // -----------------------------------------------------------------------
    // Edge detection + preview
    // -----------------------------------------------------------------------

    static constexpr int kEdgeThreshold = 48; // px from container edge = dock zone

    static auto DetectEdge(QWidget* container, QPoint globalPos)
        -> std::optional<gui::DockSide>
    {
        const QPoint local = container->mapFromGlobal(globalPos);
        const int w = container->width();
        const int h = container->height();

        if (local.y() >= h - kEdgeThreshold) { return gui::DockSide::Bottom; }
        if (local.y() <= kEdgeThreshold)      { return gui::DockSide::Top; }
        if (local.x() <= kEdgeThreshold)      { return gui::DockSide::Left; }
        if (local.x() >= w - kEdgeThreshold)  { return gui::DockSide::Right; }
        return std::nullopt; // center = undock
    }

    void ShowEdgePreview(QWidget* container, gui::DockSide side)
    {
        const int w = container->width();
        const int h = container->height();
        constexpr int band = 6;

        const int contentW = std::max(_ghostSize.width(), 300);
        const int contentH = std::max(_ghostSize.height(), 40);

        QRect previewRect;
        switch (side) {
        case gui::DockSide::Bottom:
            previewRect = QRect((w - contentW) / 2, h - contentH, contentW, contentH);
            break;
        case gui::DockSide::Top:
            previewRect = QRect((w - contentW) / 2, 0, contentW, contentH);
            break;
        case gui::DockSide::Left:
            previewRect = QRect(0, 0, band, h);
            break;
        case gui::DockSide::Right:
            previewRect = QRect(w - band, 0, band, h);
            break;
        }

        _preview->setGeometry(previewRect);
        _preview->show();
        _preview->raise();
    }

    void FinalizeDrop(QWidget* container, QPoint globalPos)
    {
        // ActionBar was hidden during drag. Take it back from floating frame if needed.
        if (_wasInFloatingFrame) {
            _floatingFrame->TakeActionBar();
            _floatingFrame->hide();
        }

        auto edge = DetectEdge(container, globalPos);
        if (edge.has_value()) {
            // Dock to edge
            _actionBar->setParent(container);
            _actionBar->SetDocked(true);
            _actionBar->SetDockSide(edge.value());
            if (_actionBar->IsCollapsed()) {
                _actionBar->SetCollapsed(false);
            }
            Reposition(container);
        } else {
            // Undock: reparent into floating frame
            _actionBar->SetDocked(false);
            _actionBar->SetOrientation(Qt::Horizontal);
            _actionBar->SetDockSide(gui::DockSide::Bottom);
            _floatingFrame->SetActionBar(_actionBar);
            _floatingFrame->resize(_ghostSize);
            _floatingFrame->move(
                globalPos - QPoint(_ghostSize.width() / 2, 10));
            _floatingFrame->show();
            _floatingFrame->raise();
            if (_actionBar->IsCollapsed()) {
                _actionBar->SetCollapsed(false);
            }
        }
    }

    // -----------------------------------------------------------------------
    // Positioning (unchanged logic)
    // -----------------------------------------------------------------------

    void Reposition(QWidget* container)
    {
        if (container == nullptr || _actionBar == nullptr) { return; }

        // Re-entrancy guard: hide/show can trigger layout events that
        // call Reposition() again. Skip nested calls.
        if (_repositioning) { return; }
        _repositioning = true;

        const bool collapsed = _actionBar->IsCollapsed();
        const bool docked    = _actionBar->IsDocked();
        const auto side      = _actionBar->GetDockSide();
        const int  margin    = 16;

        if (collapsed && docked) {
            // Docked + collapsed: ActionBar hidden, show trapezoid on edge
            _actionBar->hide();
            _trapezoid->SetDockSide(side);
            _trapezoid->show();
            _trapezoid->raise();
            PositionTrapezoid(container, side);
            if (_actionBar->MiniButton() != nullptr) {
                _actionBar->MiniButton()->hide();
            }
        } else if (collapsed && !docked) {
            // Undocked + collapsed: hide floating frame, show mini-button
            // at the collapse button's position, clamped to container bounds.
            _trapezoid->hide();

            // Capture collapse button's global position before hiding anything
            auto* collapseBtn = _actionBar->CollapseButton();
            QPoint miniGlobal;
            if (collapseBtn != nullptr && collapseBtn->isVisible()) {
                miniGlobal = collapseBtn->mapToGlobal(QPoint(0, 0));
            } else if (_floatingFrame->isVisible()) {
                miniGlobal = _floatingFrame->mapToGlobal(QPoint(4, 4));
            } else {
                // Already collapsed+hidden (re-entrant layout call) — skip
                _repositioning = false;
                return;
            }

            // Save floating frame geometry for restore on expand
            if (_floatingFrame->isVisible()) {
                _savedFloatingFramePos = _floatingFrame->pos();
                _savedFloatingFrameSize = _floatingFrame->size();
            }

            // Hide ActionBar and floating frame
            _actionBar->hide();
            _floatingFrame->hide();

            // Position mini-button within container, clamped to visible bounds
            auto* mini = _actionBar->MiniButton();
            if (mini != nullptr) {
                mini->setParent(container);
                QPoint localPos = container->mapFromGlobal(miniGlobal);
                // Clamp to container rect with margin
                int mx = std::clamp(localPos.x(), margin,
                                    container->width() - mini->width() - margin);
                int my = std::clamp(localPos.y(), margin,
                                    container->height() - mini->height() - margin);
                mini->move(mx, my);
                mini->show();
                mini->raise();
            }
        } else {
            // Expanded: show ActionBar, hide trapezoid and mini-button
            _trapezoid->hide();
            if (_actionBar->MiniButton() != nullptr) {
                _actionBar->MiniButton()->hide();
            }
            _actionBar->adjustSize();
            _actionBar->show();

            if (docked) {
                PositionDocked(container, side);
            } else {
                // Undocked expand: restore floating frame at saved position
                _actionBar->SetOrientation(Qt::Horizontal);
                _floatingFrame->SetActionBar(_actionBar);
                if (_savedFloatingFrameSize.isValid()) {
                    _floatingFrame->resize(_savedFloatingFrameSize);
                    _floatingFrame->move(_savedFloatingFramePos);
                } else {
                    // Fallback: center above mini-button
                    auto* mini = _actionBar->MiniButton();
                    QPoint pos = mini != nullptr
                        ? container->mapToGlobal(mini->pos())
                        : container->mapToGlobal(
                            QPoint(container->width() / 2, container->height() - margin));
                    QSize sz = _actionBar->sizeHint();
                    sz.setWidth(std::max(sz.width(), 300));
                    sz.setHeight(sz.height() + 12);
                    _floatingFrame->resize(sz);
                    _floatingFrame->move(pos.x() - (sz.width() / 2), pos.y() - sz.height());
                }
                _floatingFrame->show();
                _floatingFrame->raise();
            }
            _actionBar->raise();
        }

        _repositioning = false;
    }

    void PositionTrapezoid(QWidget* container, gui::DockSide side)
    {
        const int tw = _trapezoid->width();
        const int th = _trapezoid->height();

        switch (side) {
        case gui::DockSide::Bottom:
            _trapezoid->move((container->width() - tw) / 2,
                             container->height() - th);
            break;
        case gui::DockSide::Top:
            _trapezoid->move((container->width() - tw) / 2, 0);
            break;
        case gui::DockSide::Left:
            _trapezoid->move(0, (container->height() - th) / 2);
            break;
        case gui::DockSide::Right:
            _trapezoid->move(container->width() - tw,
                             (container->height() - th) / 2);
            break;
        }
    }

    void PositionDocked(QWidget* container, gui::DockSide side)
    {
        const int cw = container->width();
        const int ch = container->height();
        const QSize hint = _actionBar->sizeHint();

        switch (side) {
        case gui::DockSide::Bottom: {
            int w = std::max(hint.width(), 300);
            _actionBar->resize(w, hint.height());
            _actionBar->move((cw - w) / 2, ch - hint.height());
            break;
        }
        case gui::DockSide::Top: {
            int w = std::max(hint.width(), 300);
            _actionBar->resize(w, hint.height());
            _actionBar->move((cw - w) / 2, 0);
            break;
        }
        case gui::DockSide::Left:
            _actionBar->resize(hint.width(), ch);
            _actionBar->move(0, 0);
            break;
        case gui::DockSide::Right:
            _actionBar->resize(hint.width(), ch);
            _actionBar->move(cw - hint.width(), 0);
            break;
        }
    }

    gui::NyanActionBar*            _actionBar;
    gui::TrapezoidHandle*          _trapezoid;
    DockPreviewOverlay*            _preview;
    gui::ActionBarFloatingFrame*   _floatingFrame;
    DragGhostOverlay*              _ghost;

    // Drag state
    bool   _dragPending  = false;
    bool   _dragging     = false;
    QPoint _dragStartPos;
    QPoint _dragOffset;
    QSize  _ghostSize;

    // Pre-drag state (to restore on cancel or for transition)
    bool            _wasDocked = false;
    bool            _wasInFloatingFrame = false;
    gui::DockSide   _preDragDockSide = gui::DockSide::Bottom;

    // Saved floating frame geometry for undocked collapse/expand
    QPoint _savedFloatingFramePos;
    QSize  _savedFloatingFrameSize;

    // Re-entrancy guard for Reposition()
    bool _repositioning = false;
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// CloseEventFilter -- intercepts QCloseEvent on the window
// ---------------------------------------------------------------------------

namespace {

class WindowCloseEventFilter : public QObject {
public:
    explicit WindowCloseEventFilter(WindowNode& node, QObject* parent = nullptr)
        : QObject(parent), _node(node) {}

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override
    {
        if (event->type() == QEvent::Close) {
            _node.MarkCloseRequested();
        }
        return QObject::eventFilter(obj, event);
    }

private:
    WindowNode& _node;
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// WindowNode
// ---------------------------------------------------------------------------

MATCHA_IMPLEMENT_CLASS(WindowNode, UiNode)

WindowNode::WindowNode(std::string id, WindowId windowId, WindowKind kind)
    : UiNode(std::move(id), NodeType::WindowNode)
    , _windowId(windowId)
    , _kind(kind)
{
}

WindowNode::~WindowNode()
{
    // Invariant: UiNode children must be destroyed BEFORE the Qt widget tree.
    // delete _mainWindow triggers Qt's parent-child cascade, which destroys
    // all child widgets. DestroyChildren() ensures every child UiNode's
    // destructor runs while its associated QWidget is still alive.
    DestroyChildren();

    // Guard: if _mainWindow was already destroyed by Qt's parent-child cascade
    // (e.g. a floating/debug window parented to the main window), QPointer
    // becomes null and we skip the delete — preventing a double-free.
    QPointer<QWidget> guard(_mainWindow);
    if (guard) {
        delete _mainWindow;
    }
    _mainWindow = nullptr;
    _centralArea = nullptr;
}

void WindowNode::SetTitle(std::string_view title)
{
    _title.assign(title);
    if (_mainWindow != nullptr) {
        _mainWindow->setWindowTitle(QString::fromUtf8(_title.data(), static_cast<int>(_title.size())));
    }
}

auto WindowNode::Title() const -> std::string
{
    return _title;
}

void WindowNode::Show()
{
    if (_mainWindow != nullptr) {
        _mainWindow->show();
    }
}

void WindowNode::Hide()
{
    if (_mainWindow != nullptr) {
        _mainWindow->hide();
    }
}

auto WindowNode::IsVisible() const -> bool
{
    return _mainWindow != nullptr && _mainWindow->isVisible();
}

void WindowNode::Close()
{
    if (_mainWindow != nullptr) {
        _mainWindow->close();
    }
}

void WindowNode::Minimize()
{
    if (_mainWindow != nullptr) {
        _mainWindow->showMinimized();
    }
}

void WindowNode::Maximize()
{
    if (_mainWindow != nullptr) {
        if (_mainWindow->isMaximized()) {
            _mainWindow->showNormal();
        } else {
            _mainWindow->showMaximized();
        }
    }
}

void WindowNode::SetMinimumSize(int w, int h)
{
    if (_mainWindow != nullptr) {
        _mainWindow->setMinimumSize(w, h);
    }
}

void WindowNode::Resize(int w, int h)
{
    if (_mainWindow != nullptr) {
        _mainWindow->resize(w, h);
    }
}

void WindowNode::Move(int x, int y)
{
    if (_mainWindow != nullptr) {
        _mainWindow->move(x, y);
    }
}

auto WindowNode::GetTitleBar() -> observer_ptr<TitleBarNode>
{
    for (auto* node : ChildrenOfType(NodeType::TitleBar)) {
        if (auto* tb = dynamic_cast<TitleBarNode*>(node)) {
            return make_observer(tb);
        }
    }
    return observer_ptr<TitleBarNode>{};
}

auto WindowNode::GetWorkspaceFrame() -> observer_ptr<WorkspaceFrame>
{
    for (auto* node : ChildrenOfType(NodeType::WorkspaceFrame)) {
        if (auto* ws = dynamic_cast<WorkspaceFrame*>(node)) {
            return make_observer(ws);
        }
    }
    return observer_ptr<WorkspaceFrame>{};
}

auto WindowNode::GetDocumentToolBar() -> observer_ptr<DocumentToolBarNode>
{
    for (auto* node : ChildrenOfType(NodeType::DocumentToolBar)) {
        if (auto* dt = dynamic_cast<DocumentToolBarNode*>(node)) {
            return make_observer(dt);
        }
    }
    return observer_ptr<DocumentToolBarNode>{};
}

auto WindowNode::GetLogoButton() -> observer_ptr<LogoButtonNode>
{
    for (auto* node : ChildrenOfType(NodeType::LogoButton)) {
        if (auto* lb = dynamic_cast<LogoButtonNode*>(node)) {
            return make_observer(lb);
        }
    }
    return observer_ptr<LogoButtonNode>{};
}

auto WindowNode::GetStatusBarNode() -> observer_ptr<StatusBarNode>
{
    for (auto* node : ChildrenOfType(NodeType::StatusBar)) {
        if (auto* sb = dynamic_cast<StatusBarNode*>(node)) {
            return make_observer(sb);
        }
    }
    return observer_ptr<StatusBarNode>{};
}

auto WindowNode::ContentArea() -> QWidget*
{
    return _centralArea;
}

auto WindowNode::Widget() -> QWidget*
{
    return _mainWindow;
}

auto WindowNode::FreezeUpdates() -> gui::UpdateGuard
{
    return gui::UpdateGuard::Create(_mainWindow);
}

void WindowNode::BuildWindow(QWidget* parent)
{
    if (_mainWindow != nullptr) {
        return;
    }

    auto* window = new QMainWindow(parent);

    if (_kind == WindowKind::Main) {
        window->setMinimumSize(kMinWidth, kMinHeight);
        window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    } else if (_kind == WindowKind::Floating) {
        window->setMinimumSize(800, 500);
        window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        if (parent != nullptr) {
            window->setParent(parent);
            window->setWindowFlags(window->windowFlags() | Qt::Window);
        }
    }

    window->installEventFilter(new WindowCloseEventFilter(*this, window));

    // Grid layout: Logo spans rows 0-1 col 0; TitleBar row 0 col 1;
    //              DocToolBar row 1 col 1; Central row 2 col 0-1; StatusBar row 3 col 0-1
    auto* container = new QWidget(window);
    auto* grid = new QGridLayout(container);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    static constexpr int kLogoColumnWidth = 64;
    static constexpr int kTitleBarHeight  = 28;
    static constexpr int kDocToolBarHeight = 36;

    // -- LogoButtonNode (row 0-1, col 0) --
    auto logoNode = std::make_unique<LogoButtonNode>("logo-button");
    auto* logoWidget = logoNode->LogoButton();
    grid->addWidget(logoWidget, 0, 0, 2, 1);
    grid->setColumnMinimumWidth(0, kLogoColumnWidth);
    AddNode(std::move(logoNode));

    // -- MainTitleBarNode (row 0, col 1) --
    auto titleBarNode = std::make_unique<MainTitleBarNode>("main-titlebar");
    auto* mainTitleBar = titleBarNode->MainTitleBar();
    grid->addWidget(mainTitleBar, 0, 1);
    grid->setRowMinimumHeight(0, kTitleBarHeight);

    // Connect TitleBar window control signals
    QObject::connect(mainTitleBar, &gui::NyanMainTitleBar::MinimizeRequested, window, [window]() {
        window->showMinimized();
    });
    QObject::connect(mainTitleBar, &gui::NyanMainTitleBar::MaximizeRequested, window, [window]() {
        if (window->isMaximized()) {
            window->showNormal();
        } else {
            window->showMaximized();
        }
    });
    QObject::connect(mainTitleBar, &gui::NyanMainTitleBar::CloseRequested, window, [window]() {
        window->close();
    });
    AddNode(std::move(titleBarNode));

    // -- DocumentToolBarNode (row 1, col 1) --
    auto docToolBarNode = std::make_unique<DocumentToolBarNode>("doc-toolbar");
    auto* docToolBarWidget = docToolBarNode->DocumentToolBar();
    grid->addWidget(docToolBarWidget, 1, 1);
    grid->setRowMinimumHeight(1, kDocToolBarHeight);
    AddNode(std::move(docToolBarNode));

    // -- Central area (row 2, col 0-1, stretch) --
    _centralArea = new QWidget(container);
    _centralArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid->addWidget(_centralArea, 2, 0, 1, 2);
    grid->setRowStretch(2, 1);

    // -- Create UiNode children that own the widgets --

    // WorkspaceFrame: ActionBarNode + DocumentArea + ControlBar
    auto wsFrame = std::make_unique<WorkspaceFrame>("main-workspace");
    wsFrame->SetContainerWidget(_centralArea);

    // ActionBar floats above central area (not in VBox layout)
    auto actionBarNode = std::make_unique<ActionBarNode>();
    actionBarNode->SetDocked(true);
    auto* actionBarWidget = actionBarNode->ActionBar();
    actionBarWidget->setParent(_centralArea);
    actionBarWidget->raise();

    // TrapezoidHandle: sibling of ActionBar on _centralArea, initially hidden
    auto* trapezoid = new gui::TrapezoidHandle(_centralArea);
    trapezoid->hide();

    // Overlay filter manages positioning of ActionBar, TrapezoidHandle, MiniButton
    auto* overlayFilter = new ActionBarOverlayFilter(
        actionBarWidget, trapezoid, _centralArea);
    _centralArea->installEventFilter(overlayFilter);

    wsFrame->AddNode(std::move(actionBarNode));

    auto docArea = std::make_unique<DocumentArea>("main-document-area");
    wsFrame->AddNode(std::move(docArea));

    auto controlBar = std::make_unique<ControlBar>("main-control-bar");
    wsFrame->AddNode(std::move(controlBar));

    AddNode(std::move(wsFrame));

    // -- StatusBarNode (row 3, col 0-1) --
    auto statusBarNode = std::make_unique<StatusBarNode>();
    grid->addWidget(statusBarNode->StatusBar(), 3, 0, 1, 2);
    AddNode(std::move(statusBarNode));

    window->setCentralWidget(container);

    _mainWindow = window;
}

auto WindowNode::IsBuilt() const -> bool
{
    return _mainWindow != nullptr;
}

void WindowNode::MarkCloseRequested()
{
    CloseRequested notif;
    SendNotification(this, notif);
    if (notif.IsCancelled()) {
        return;
    }
    _closeRequested = true;
}

auto WindowNode::IsCloseRequested() const -> bool
{
    return _closeRequested;
}

} // namespace matcha::fw
