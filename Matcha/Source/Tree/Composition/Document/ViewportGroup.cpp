/**
 * @file ViewportGroup.cpp
 * @brief Implementation of ViewportGroup UiNode -- binary split tree for viewports.
 */

#include <Matcha/Tree/Composition/Document/ViewportGroup.h>

#include <Matcha/Tree/Composition/Document/SplitTreeNode.h>
#include <Matcha/Tree/UiNodeNotification.h>
#include <Matcha/Tree/Composition/Document/Viewport.h>
#include <Matcha/Widgets/Shell/NyanSplitter.h>
#include <Matcha/Widgets/Shell/ViewportFrame.h>
#include <Matcha/Widgets/Shell/ViewportHeaderBar.h>
#include <Matcha/Widgets/Shell/ViewportWidget.h>

#include <QDebug>
#include <QKeyEvent>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <cassert>

namespace matcha::fw {

// ============================================================================
// DnD diagnostic logging
// ============================================================================

static void DumpTreeNode(const TreeNode* node, int depth, const std::vector<gui::ViewportFrame*>& frames)
{
    if (!node) return;
    QString indent(depth * 2, ' ');

    if (IsLeaf(*node)) {
        const auto& leaf = AsLeaf(*node);
        auto vpId = leaf.viewportId;
        auto* vp = leaf.viewport;

        // Find the matching frame for widget geometry
        QRect frameGeom;
        QRect globalGeom;
        bool found = false;
        for (auto* f : frames) {
            if (f->GetViewportId() == vpId) {
                frameGeom = f->geometry();
                globalGeom = QRect(f->mapToGlobal(QPoint(0,0)), f->size());
                found = true;
                break;
            }
        }

        QString name = vp ? QString::fromStdString(std::string(vp->Name())) : "<null>";
        if (found) {
            gui::ViewportFrame* matchedFrame = nullptr;
            for (auto* f : frames) {
                if (f->GetViewportId() == vpId) { matchedFrame = f; break; }
            }
            qDebug().noquote() << indent
                << QString("Leaf VP#%1 \"%2\"").arg(vpId.value).arg(name);
            qDebug().noquote() << indent
                << QString("  Frame  local=(%1,%2 %3x%4) global=(%5,%6 %7x%8)")
                       .arg(frameGeom.x()).arg(frameGeom.y())
                       .arg(frameGeom.width()).arg(frameGeom.height())
                       .arg(globalGeom.x()).arg(globalGeom.y())
                       .arg(globalGeom.width()).arg(globalGeom.height());
            if (matchedFrame && matchedFrame->HeaderBar()) {
                auto* hdr = matchedFrame->HeaderBar();
                auto hdrLocal = hdr->geometry();
                auto hdrGlobal = QRect(hdr->mapToGlobal(QPoint(0,0)), hdr->size());
                qDebug().noquote() << indent
                    << QString("  Header local=(%1,%2 %3x%4) global=(%5,%6 %7x%8) visible=%9")
                           .arg(hdrLocal.x()).arg(hdrLocal.y())
                           .arg(hdrLocal.width()).arg(hdrLocal.height())
                           .arg(hdrGlobal.x()).arg(hdrGlobal.y())
                           .arg(hdrGlobal.width()).arg(hdrGlobal.height())
                           .arg(hdr->isVisible() ? "Y" : "N");
            }
        } else {
            qDebug().noquote() << indent
                << QString("Leaf VP#%1 \"%2\" (no widget geometry)").arg(vpId.value).arg(name);
        }
    } else {
        const auto& sn = AsSplit(*node);
        qDebug().noquote() << indent
            << QString("Split %1 ratio=%2")
                   .arg(sn.direction == SplitDirection::Horizontal ? "H" : "V")
                   .arg(sn.ratio, 0, 'f', 3);
        DumpTreeNode(sn.first.get(), depth + 1, frames);
        DumpTreeNode(sn.second.get(), depth + 1, frames);
    }
}

static void DumpViewportLayout(const char* phase, const TreeNode* root,
                        const std::vector<gui::ViewportFrame*>& frames,
                        ViewportGroupState state,
                        std::optional<ViewportId> dragSrc = {})
{
    qDebug() << "";
    qDebug().noquote() << QString("===== [DnD Diag] %1 | state=%2%3 =====")
        .arg(phase)
        .arg([&]{
            switch(state) {
            case ViewportGroupState::Normal:   return "Normal";
            case ViewportGroupState::Dragging: return "Dragging";
            case ViewportGroupState::Maximized:return "Maximized";
            case ViewportGroupState::Resizing: return "Resizing";
            }
            return "?";
        }())
        .arg(dragSrc ? QString(" dragSrc=VP#%1").arg(dragSrc->value) : "");
    DumpTreeNode(root, 1, frames);
    qDebug().noquote() << "===== [/DnD Diag] =====";
    qDebug() << "";
}

// ============================================================================
// Construction
// ============================================================================

ViewportGroup::ViewportGroup(std::string name)
    : UiNode("viewport-group", NodeType::ViewportGroup, std::move(name))
{
    // Create the initial single-viewport tree
    auto initialId = NextViewportId();
    auto vpNode = std::make_unique<Viewport>("viewport-1", initialId);
    auto* vpPtr = static_cast<Viewport*>(AddNode(std::move(vpNode)));

    _treeRoot = MakeLeaf(initialId, vpPtr);
    _activeViewportId = initialId;
}

ViewportGroup::~ViewportGroup() = default;

// ============================================================================
// ID generation
// ============================================================================

auto ViewportGroup::NextViewportId() -> ViewportId
{
    return ViewportId::From(_nextVpId++);
}

// ============================================================================
// Viewport lookup
// ============================================================================

auto ViewportGroup::FindViewport(ViewportId id) -> Viewport*
{
    for (auto* child : DescendantsOfType(NodeType::Viewport)) {
        auto* vp = static_cast<Viewport*>(child);
        if (vp->GetViewportId() == id) {
            return vp;
        }
    }
    return nullptr;
}

// ============================================================================
// Split tree operations
// ============================================================================

auto ViewportGroup::SplitViewport(ViewportId target, SplitDirection direction)
    -> Expected<observer_ptr<Viewport>>
{
    if (_state == ViewportGroupState::Maximized) {
        return std::unexpected(ErrorCode::InvalidArgument);
    }

    auto* leafNode = FindLeaf(_treeRoot.get(), target);
    if (leafNode == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    // Create new viewport UiNode
    auto newId = NextViewportId();
    auto vpUiNode = std::make_unique<Viewport>("split-viewport", newId);
    auto* newVp = static_cast<Viewport*>(AddNode(std::move(vpUiNode)));

    // Get the existing leaf data before we replace it
    auto& existingLeaf = AsLeaf(*leafNode);
    auto existingId = existingLeaf.viewportId;
    auto* existingVp = existingLeaf.viewport;

    // Replace the leaf with a SplitNode containing [existing, new]
    auto existingLeafNode = MakeLeaf(existingId, existingVp);
    auto newLeafNode = MakeLeaf(newId, newVp);

    SplitNode sn;
    sn.direction = direction;
    sn.ratio = 0.5;
    sn.first = std::move(existingLeafNode);
    sn.second = std::move(newLeafNode);

    *leafNode = std::move(sn);

    FireViewportCreated(newId, target, direction);
    return observer_ptr<Viewport>(newVp);
}

auto ViewportGroup::RemoveViewport(ViewportId target) -> Expected<void>
{
    if (_state == ViewportGroupState::Maximized) {
        return std::unexpected(ErrorCode::InvalidArgument);
    }

    auto* leafNode = FindLeaf(_treeRoot.get(), target);
    if (leafNode == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    if (ViewportCount() <= 1) {
        return std::unexpected(ErrorCode::InvalidArgument);
    }

    // Detach the leaf (collapses parent SplitNode)
    auto detached = DetachLeaf(target);
    (void)detached; // leaf is discarded

    // Remove the Viewport UiNode
    auto* vpNode = FindViewport(target);
    if (vpNode != nullptr) {
        RemoveNode(vpNode);
    }

    FireViewportRemoved(target);

    // Update active viewport if needed
    if (_activeViewportId == target) {
        auto ids = AllViewportIds();
        _activeViewportId = ids.empty() ? std::nullopt : std::optional(ids.front());
        if (_activeViewportId) {
            FireActiveViewportChanged(*_activeViewportId);
        }
    }

    return {};
}

auto ViewportGroup::ActiveViewport() -> observer_ptr<Viewport>
{
    if (!_activeViewportId) {
        return observer_ptr<Viewport>(nullptr);
    }
    return observer_ptr<Viewport>(FindViewport(*_activeViewportId));
}

void ViewportGroup::SetActiveViewport(ViewportId id)
{
    if (FindViewport(id) != nullptr) {
        _activeViewportId = id;
        FireActiveViewportChanged(id);
    }
}

auto ViewportGroup::MaximizeViewport(ViewportId target) -> Expected<void>
{
    if (_state == ViewportGroupState::Maximized) {
        return std::unexpected(ErrorCode::InvalidArgument);
    }

    if (FindViewport(target) == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    _maximizedId = target;
    SetState(ViewportGroupState::Maximized);
    FireViewportMaximized(target);
    return {};
}

void ViewportGroup::RestoreLayout()
{
    if (_state != ViewportGroupState::Maximized) {
        return;
    }
    auto restoredId = _maximizedId;
    _maximizedId.reset();
    SetState(ViewportGroupState::Normal);
    if (restoredId) {
        FireViewportRestored(*restoredId);
    }
}

// ============================================================================
// Viewport rearrangement (drag-and-drop)
// ============================================================================

auto ViewportGroup::DetachLeaf(ViewportId id) -> std::unique_ptr<TreeNode>
{
    if (_treeRoot == nullptr) {
        return nullptr;
    }

    // Special case: root IS the target leaf -- cannot detach root
    if (IsLeaf(*_treeRoot) && AsLeaf(*_treeRoot).viewportId == id) {
        return nullptr;
    }

    // FindParentOf returns the SplitNode whose direct child is the target leaf.
    auto* parentNode = FindParentOf(_treeRoot.get(), id);
    if (parentNode == nullptr) {
        return nullptr;
    }

    auto& parent = AsSplit(*parentNode);

    // Determine which direct child is the target leaf
    std::unique_ptr<TreeNode> detached;
    std::unique_ptr<TreeNode> sibling;

    if (parent.first && IsLeaf(*parent.first) &&
        AsLeaf(*parent.first).viewportId == id) {
        detached = std::move(parent.first);
        sibling = std::move(parent.second);
    } else {
        assert(parent.second && IsLeaf(*parent.second) &&
               AsLeaf(*parent.second).viewportId == id);
        detached = std::move(parent.second);
        sibling = std::move(parent.first);
    }

    // Collapse: replace the parent SplitNode with the remaining sibling
    assert(sibling != nullptr);
    *parentNode = std::move(*sibling);

    return detached;
}

auto ViewportGroup::SplitAndMove(ViewportId source, ViewportId target, DropZone zone)
    -> Expected<void>
{
    if (source == target) {
        return std::unexpected(ErrorCode::InvalidArgument);
    }
    if (zone == DropZone::Center) {
        return SwapViewports(source, target);
    }
    if (FindLeaf(_treeRoot.get(), source) == nullptr ||
        FindLeaf(_treeRoot.get(), target) == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }

    // Step 1: Detach source from tree
    auto* srcVp = FindViewport(source);
    auto detached = DetachLeaf(source);
    if (detached == nullptr) {
        return std::unexpected(ErrorCode::InvalidArgument);
    }

    // Step 2: Find target leaf (may have moved due to detach collapse)
    auto* targetLeaf = FindLeaf(_treeRoot.get(), target);
    if (targetLeaf == nullptr) {
        // Put source back -- shouldn't happen
        return std::unexpected(ErrorCode::NotFound);
    }

    // Step 3: Replace target with SplitNode containing [source, target]
    auto& tgtLeafData = AsLeaf(*targetLeaf);
    auto tgtId = tgtLeafData.viewportId;
    auto* tgtVp = tgtLeafData.viewport;

    auto direction = DropZoneToDirection(zone);
    auto sourceFirst = IsSourceFirst(zone);

    auto srcLeaf = MakeLeaf(source, srcVp);
    auto tgtLeaf = MakeLeaf(tgtId, tgtVp);

    SplitNode sn;
    sn.direction = direction;
    sn.ratio = 0.5;
    sn.first = sourceFirst ? std::move(srcLeaf) : std::move(tgtLeaf);
    sn.second = sourceFirst ? std::move(tgtLeaf) : std::move(srcLeaf);

    *targetLeaf = std::move(sn);

    FireViewportMoved(source);
    return {};
}

auto ViewportGroup::SwapViewports(ViewportId a, ViewportId b) -> Expected<void>
{
    auto* leafA = FindLeaf(_treeRoot.get(), a);
    auto* leafB = FindLeaf(_treeRoot.get(), b);
    if (leafA == nullptr || leafB == nullptr) {
        return std::unexpected(ErrorCode::NotFound);
    }
    if (a == b) {
        return std::unexpected(ErrorCode::InvalidArgument);
    }

    // Swap the leaf contents (viewportId and viewport pointer)
    auto& la = AsLeaf(*leafA);
    auto& lb = AsLeaf(*leafB);
    std::swap(la.viewportId, lb.viewportId);
    std::swap(la.viewport, lb.viewport);

    FireViewportSwapped(a, b);
    FireViewportMoved(a);
    FireViewportMoved(b);
    return {};
}

// ============================================================================
// Tree queries
// ============================================================================

auto ViewportGroup::ViewportCount() const -> int
{
    return CountLeaves(_treeRoot.get());
}

auto ViewportGroup::AllViewportIds() const -> std::vector<ViewportId>
{
    std::vector<ViewportId> ids;
    CollectLeafIds(_treeRoot.get(), ids);
    return ids;
}

// ============================================================================
// Widget synchronization
// ============================================================================

void ViewportGroup::RebuildWidgetTree(QWidget* parent)
{
    if (parent != nullptr) {
        _container = parent;
    }
    if (_container == nullptr || _treeRoot == nullptr) {
        return;
    }

    // Step 1: Detach all ViewportFrame widgets (which contain ViewportWidgets)
    for (auto* frame : _frames) {
        frame->setParent(nullptr);
        frame->hide();
    }
    // Also detach any orphaned raw ViewportWidgets (from pre-frame builds)
    for (auto* child : DescendantsOfType(NodeType::Viewport)) {
        auto* vp = static_cast<Viewport*>(child);
        auto* widget = vp->GetWidget();
        if (widget != nullptr && widget->parentWidget() != nullptr) {
            widget->setParent(nullptr);
            widget->hide();
        }
    }

    // Step 2: Destroy all intermediate splitters and old frames
    DestroyIntermediateSplitters();

    // Step 3: Handle maximized state -- show only the maximized viewport
    if (_state == ViewportGroupState::Maximized && _maximizedId) {
        auto* maxVp = FindViewport(*_maximizedId);
        if (maxVp != nullptr) {
            if (maxVp->GetWidget() == nullptr) {
                maxVp->CreateWidget(_container);
            }
            auto* frame = new gui::ViewportFrame(*_maximizedId, maxVp->GetWidget(), _container);
            frame->SetLabel(QString::fromStdString(std::string(maxVp->Name())));
            frame->HeaderBar()->SetCloseButtonVisible(false);
            _frames.push_back(frame);
            _rootWidget = frame;

            if (_container->layout() == nullptr) {
                auto* lay = new QVBoxLayout(_container);
                lay->setContentsMargins(0, 0, 0, 0);
                lay->setSpacing(0);
            }
            auto* lay = _container->layout();
            while (lay->count() > 0) {
                auto* item = lay->takeAt(0);
                delete item;
            }
            lay->addWidget(_rootWidget);
            _rootWidget->show();
            return;
        }
    }

    // Step 4: Recursively build the new widget tree
    _rootWidget = BuildWidgetSubtree(_treeRoot.get(), _container);

    // Step 5: Ensure root widget fills the container
    if (_rootWidget != nullptr) {
        if (_container->layout() == nullptr) {
            auto* lay = new QVBoxLayout(_container);
            lay->setContentsMargins(0, 0, 0, 0);
            lay->setSpacing(0);
        }
        auto* lay = _container->layout();
        while (lay->count() > 0) {
            auto* item = lay->takeAt(0);
            delete item;
        }
        lay->addWidget(_rootWidget);
        _rootWidget->show();
    }

    // Step 6: Connect splitter signals for ratio writeback
    ConnectSplitterSignals();

    // Step 7: Force layout activation so all child geometries are valid
    _container->updateGeometry();
    if (_rootWidget != nullptr) {
        _rootWidget->updateGeometry();
    }
    for (auto* frame : _frames) {
        frame->updateGeometry();
        if (frame->layout() != nullptr) {
            frame->layout()->activate();
        }
    }

    DumpViewportLayout("AFTER REBUILD", _treeRoot.get(), _frames, _state);

    FireLayoutRebuilt();
}

auto ViewportGroup::BuildWidgetSubtree(const TreeNode* node, QWidget* parent) -> QWidget*
{
    if (node == nullptr) {
        return nullptr;
    }

    if (IsLeaf(*node)) {
        const auto& leaf = AsLeaf(*node);
        auto* vp = leaf.viewport;
        if (vp == nullptr) {
            return nullptr;
        }
        // Ensure the viewport has a widget
        if (vp->GetWidget() == nullptr) {
            vp->CreateWidget(parent);
        }
        // Reset stale geometry/size policy from previous frame's layout so
        // the new QVBoxLayout in ViewportFrame can position the header correctly.
        auto* vpw = vp->GetWidget();
        qDebug().noquote() << QString("[Rebuild] VP#%1 vpw BEFORE reset: geom=(%2,%3 %4x%5) sizeHint=(%6x%7) minSize=(%8x%9) policy=(%10,%11)")
            .arg(leaf.viewportId.value)
            .arg(vpw->geometry().x()).arg(vpw->geometry().y())
            .arg(vpw->geometry().width()).arg(vpw->geometry().height())
            .arg(vpw->sizeHint().width()).arg(vpw->sizeHint().height())
            .arg(vpw->minimumSize().width()).arg(vpw->minimumSize().height())
            .arg(vpw->sizePolicy().horizontalPolicy()).arg(vpw->sizePolicy().verticalPolicy());
        vpw->setGeometry(0, 0, 0, 0);
        vpw->setMinimumSize(0, 0);
        vpw->resize(0, 0);
        vpw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        qDebug().noquote() << QString("[Rebuild] VP#%1 vpw AFTER reset: geom=(%2,%3 %4x%5) sizeHint=(%6x%7)")
            .arg(leaf.viewportId.value)
            .arg(vpw->geometry().x()).arg(vpw->geometry().y())
            .arg(vpw->geometry().width()).arg(vpw->geometry().height())
            .arg(vpw->sizeHint().width()).arg(vpw->sizeHint().height());
        // Wrap in ViewportFrame (header bar + viewport widget + overlay)
        auto* frame = new gui::ViewportFrame(leaf.viewportId, vpw, parent);
        frame->SetLabel(QString::fromStdString(std::string(vp->Name())));
        _frames.push_back(frame);
        // Log frame layout state after construction
        auto* frameLayout = frame->layout();
        qDebug().noquote() << QString("[Rebuild] VP#%1 frame created: size=(%2x%3) layout=%4 layoutItems=%5")
            .arg(leaf.viewportId.value)
            .arg(frame->width()).arg(frame->height())
            .arg(frameLayout ? "yes" : "no")
            .arg(frameLayout ? frameLayout->count() : -1);

        // Wire frame signals to ViewportGroup API.
        // IMPORTANT: RebuildWidgetTree() destroys all frames, so we must
        // defer it via QTimer::singleShot(0) to avoid deleting the frame
        // that is currently emitting the signal (use-after-free crash).
        QObject::connect(frame, &gui::ViewportFrame::dragStarted,
                         frame, [this](ViewportId id) {
            BeginDrag(id);
        });
        QObject::connect(frame, &gui::ViewportFrame::dragEnded,
                         frame, [this](ViewportId /*id*/, bool accepted) {
            if (!accepted) {
                CancelDrag();
            }
        });
        QObject::connect(frame, &gui::ViewportFrame::closeRequested,
                         frame, [this](ViewportId id) {
            if (RemoveViewport(id).has_value()) {
                QTimer::singleShot(0, _container, [this]() { RebuildWidgetTree(); });
            }
        });
        QObject::connect(frame, &gui::ViewportFrame::maximizeToggled,
                         frame, [this](ViewportId id) {
            if (_state == ViewportGroupState::Maximized) {
                RestoreLayout();
            } else {
                (void)MaximizeViewport(id);
            }
            QTimer::singleShot(0, _container, [this]() { RebuildWidgetTree(); });
        });
        QObject::connect(frame, &gui::ViewportFrame::splitHRequested,
                         frame, [this](ViewportId id) {
            if (SplitViewport(id, SplitDirection::Horizontal).has_value()) {
                QTimer::singleShot(0, _container, [this]() { RebuildWidgetTree(); });
            }
        });
        QObject::connect(frame, &gui::ViewportFrame::splitVRequested,
                         frame, [this](ViewportId id) {
            if (SplitViewport(id, SplitDirection::Vertical).has_value()) {
                QTimer::singleShot(0, _container, [this]() { RebuildWidgetTree(); });
            }
        });
        QObject::connect(frame, &gui::ViewportFrame::viewportDropped,
                         frame, [this](ViewportId src, ViewportId tgt, int zone) {
            auto dz = static_cast<DropZone>(zone);
            if (HandleDrop(src, tgt, dz).has_value()) {
                QTimer::singleShot(0, _container, [this]() { RebuildWidgetTree(); });
            } else {
                CancelDrag();
            }
        });

        frame->show();
        return frame;
    }

    // SplitNode: create a NyanSplitter
    const auto& sn = AsSplit(*node);
    auto orientation = (sn.direction == SplitDirection::Horizontal)
                           ? Qt::Horizontal
                           : Qt::Vertical;

    qDebug().noquote() << QString("[Rebuild] SplitNode dir=%1 -> Qt::%2 ratio=%3")
        .arg(sn.direction == SplitDirection::Horizontal ? "H" : "V")
        .arg(orientation == Qt::Horizontal ? "Horizontal" : "Vertical")
        .arg(sn.ratio, 0, 'f', 3);

    auto* splitter = new gui::NyanSplitter(orientation, parent);
    _splitters.push_back(splitter);
    // Store mapping for ratio writeback (const_cast safe: we own the tree)
    _splitterToNode[splitter] = const_cast<TreeNode*>(node);

    auto* firstWidget = BuildWidgetSubtree(sn.first.get(), splitter);
    auto* secondWidget = BuildWidgetSubtree(sn.second.get(), splitter);

    if (firstWidget != nullptr) {
        splitter->addWidget(firstWidget);
    }
    if (secondWidget != nullptr) {
        splitter->addWidget(secondWidget);
    }

    // Set sizes based on ratio
    if (splitter->count() == 2) {
        int totalSize = (orientation == Qt::Horizontal)
                            ? splitter->width()
                            : splitter->height();
        if (totalSize <= 0) {
            totalSize = 1000; // Default for sizing before layout
        }
        auto firstSize = static_cast<int>(sn.ratio * totalSize);
        auto secondSize = totalSize - firstSize;
        splitter->setSizes({firstSize, secondSize});
    }

    return splitter;
}

void ViewportGroup::DestroyIntermediateSplitters()
{
    // Destroy frames first -- detach inner ViewportWidget to preserve it
    for (auto* frame : _frames) {
        auto* inner = frame->InnerWidget();
        if (inner != nullptr) {
            inner->setParent(nullptr);
            inner->hide();
        }
        delete frame;
    }
    _frames.clear();

    for (auto* w : _splitters) {
        auto* splitter = qobject_cast<QSplitter*>(w);
        if (splitter != nullptr) {
            // Reparent children out before deleting to avoid cascade deletion
            while (splitter->count() > 0) {
                auto* child = splitter->widget(0);
                child->setParent(nullptr);
            }
        }
        delete w;
    }
    _splitters.clear();
    _splitterToNode.clear();
    _rootWidget = nullptr;
}

auto ViewportGroup::RootWidget() const -> QWidget*
{
    return _rootWidget;
}

// ============================================================================
// Resize: splitterMoved -> ratio writeback
// ============================================================================

void ViewportGroup::ConnectSplitterSignals()
{
    for (auto* w : _splitters) {
        auto* splitter = qobject_cast<QSplitter*>(w);
        if (splitter == nullptr) {
            continue;
        }
        QObject::connect(splitter, &QSplitter::splitterMoved,
                         splitter, [this, w](int /*pos*/, int /*index*/) {
            OnSplitterMoved(w);
        });
    }
}

void ViewportGroup::OnSplitterMoved(QWidget* splitter)
{
    auto it = _splitterToNode.find(splitter);
    if (it == _splitterToNode.end() || it->second == nullptr) {
        return;
    }
    auto* node = it->second;
    if (!IsSplit(*node)) {
        return;
    }

    auto* qsp = qobject_cast<QSplitter*>(splitter);
    if (qsp == nullptr || qsp->count() != 2) {
        return;
    }

    auto sizes = qsp->sizes();
    int total = sizes[0] + sizes[1];
    if (total <= 0) {
        return;
    }

    auto& sn = AsSplit(*node);
    sn.ratio = std::clamp(static_cast<double>(sizes[0]) / total, 0.1, 0.9);

    // Fire ratio changed with the two child viewport IDs
    auto firstId = GetFirstLeafId(sn.first.get());
    auto secondId = GetFirstLeafId(sn.second.get());
    if (firstId && secondId) {
        FireSplitRatioChanged(*firstId, *secondId, sn.ratio);
    }
}

// ============================================================================
// Drag-and-drop framework API
// ============================================================================

auto ViewportGroup::BeginDrag(ViewportId source) -> bool
{
    if (_state != ViewportGroupState::Normal) {
        qDebug() << "[DnD] BeginDrag REJECTED: state is not Normal";
        return false;
    }
    if (ViewportCount() <= 1) {
        qDebug() << "[DnD] BeginDrag REJECTED: only 1 viewport";
        return false;
    }
    if (FindViewport(source) == nullptr) {
        qDebug() << "[DnD] BeginDrag REJECTED: source VP not found";
        return false;
    }

    DumpViewportLayout("BEFORE DRAG", _treeRoot.get(), _frames, _state);

    SetState(ViewportGroupState::Dragging);
    _dragSourceId = source;

    qDebug().noquote() << QString("[DnD] BeginDrag OK: source=VP#%1").arg(source.value);
    return true;
}

auto ViewportGroup::HandleDrop(ViewportId source, ViewportId target, DropZone zone)
    -> Expected<void>
{
    if (_state != ViewportGroupState::Dragging) {
        qDebug() << "[DnD] HandleDrop REJECTED: state is not Dragging";
        return std::unexpected(ErrorCode::InvalidArgument);
    }

    auto zoneName = [&]{
        switch(zone) {
        case DropZone::Center: return "Center";
        case DropZone::Top:    return "Top";
        case DropZone::Bottom: return "Bottom";
        case DropZone::Left:   return "Left";
        case DropZone::Right:  return "Right";
        }
        return "?";
    }();

    qDebug().noquote() << QString("[DnD] HandleDrop: src=VP#%1 tgt=VP#%2 zone=%3")
        .arg(source.value).arg(target.value).arg(zoneName);

    DumpViewportLayout("IN DRAG (before mutation)", _treeRoot.get(), _frames, _state, _dragSourceId);

    SetState(ViewportGroupState::Normal);
    _dragSourceId.reset();

    Expected<void> result;
    if (zone == DropZone::Center) {
        result = SwapViewports(source, target);
    } else {
        result = SplitAndMove(source, target, zone);
    }

    if (result.has_value()) {
        DumpViewportLayout("AFTER DROP (tree mutated)", _treeRoot.get(), _frames, _state);
    } else {
        qDebug() << "[DnD] HandleDrop FAILED: tree mutation error";
    }
    return result;
}

void ViewportGroup::CancelDrag()
{
    if (_state == ViewportGroupState::Dragging) {
        qDebug().noquote() << QString("[DnD] CancelDrag: was dragging VP#%1")
            .arg(_dragSourceId ? QString::number(_dragSourceId->value) : "?");
        DumpViewportLayout("DRAG CANCELLED", _treeRoot.get(), _frames, _state, _dragSourceId);
        SetState(ViewportGroupState::Normal);
        _dragSourceId.reset();
    }
}

// ============================================================================
// Keyboard shortcuts
// ============================================================================

auto ViewportGroup::HandleKeyEvent(QKeyEvent* event) -> bool
{
    if (event == nullptr || event->type() != QEvent::KeyPress) {
        return false;
    }

    auto mods = event->modifiers();
    auto key = event->key();

    // Ctrl+\ -- split active viewport horizontally
    if (key == Qt::Key_Backslash && mods == Qt::ControlModifier) {
        if (_activeViewportId) {
            auto result = SplitViewport(*_activeViewportId, SplitDirection::Horizontal);
            if (result.has_value()) {
                RebuildWidgetTree();
                return true;
            }
        }
        return false;
    }

    // Ctrl+Shift+\ -- split active viewport vertically
    if (key == Qt::Key_Backslash &&
        mods == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (_activeViewportId) {
            auto result = SplitViewport(*_activeViewportId, SplitDirection::Vertical);
            if (result.has_value()) {
                RebuildWidgetTree();
                return true;
            }
        }
        return false;
    }

    // Ctrl+W -- close active viewport
    if (key == Qt::Key_W && mods == Qt::ControlModifier) {
        if (_activeViewportId && ViewportCount() > 1) {
            auto result = RemoveViewport(*_activeViewportId);
            if (result.has_value()) {
                RebuildWidgetTree();
                return true;
            }
        }
        return false;
    }

    // Ctrl+Shift+Enter -- maximize / restore
    if (key == Qt::Key_Return &&
        mods == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (_state == ViewportGroupState::Maximized) {
            RestoreLayout();
            RebuildWidgetTree();
            return true;
        }
        if (_activeViewportId) {
            auto result = MaximizeViewport(*_activeViewportId);
            if (result.has_value()) {
                RebuildWidgetTree();
                return true;
            }
        }
        return false;
    }

    return false;
}

// ============================================================================
// Notification dispatch
// ============================================================================

void ViewportGroup::FireActiveViewportChanged(ViewportId vpId)
{
    ActiveVpChanged notif(vpId);
    SendNotification(this, notif);
}

void ViewportGroup::FireViewportMoved(ViewportId vpId)
{
    VpMoved notif(vpId);
    SendNotification(this, notif);
}

void ViewportGroup::FireViewportCreated(ViewportId newId, ViewportId splitFrom, SplitDirection dir)
{
    VpCreated notif(newId, splitFrom, static_cast<uint8_t>(dir));
    SendNotification(this, notif);
}

void ViewportGroup::FireViewportRemoved(ViewportId removedId)
{
    VpRemoved notif(removedId);
    SendNotification(this, notif);
}

void ViewportGroup::FireViewportSwapped(ViewportId a, ViewportId b)
{
    VpSwapped notif(a, b);
    SendNotification(this, notif);
}

void ViewportGroup::FireViewportMaximized(ViewportId vpId)
{
    VpMaximized notif(vpId);
    SendNotification(this, notif);
}

void ViewportGroup::FireViewportRestored(ViewportId vpId)
{
    VpRestored notif(vpId);
    SendNotification(this, notif);
}

void ViewportGroup::FireStateChanged(ViewportGroupState oldState, ViewportGroupState newState)
{
    VpStateChanged notif(static_cast<uint8_t>(oldState), static_cast<uint8_t>(newState));
    SendNotification(this, notif);
}

void ViewportGroup::FireSplitRatioChanged(ViewportId first, ViewportId second, double ratio)
{
    VpSplitRatioChanged notif(first, second, ratio);
    SendNotification(this, notif);
}

void ViewportGroup::FireLayoutRebuilt()
{
    VpLayoutRebuilt notif;
    SendNotification(this, notif);
}

void ViewportGroup::SetState(ViewportGroupState newState)
{
    if (_state != newState) {
        auto old = _state;
        _state = newState;
        FireStateChanged(old, newState);
    }
}

} // namespace matcha::fw
