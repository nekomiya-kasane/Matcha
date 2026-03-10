#pragma once

/**
 * @file ViewportGroup.h
 * @brief ViewportGroup UiNode -- binary split tree for viewport layout.
 *
 * Each DocumentPage contains one ViewportGroup. The ViewportGroup manages
 * a binary split tree of Viewport children, supporting dynamic splitting,
 * removal, ratio adjustment, drag-to-split/swap, and maximize/restore.
 *
 * The binary split tree (TreeNode) is a pure layout topology separate from
 * the UiNode child list. Viewports remain flat UiNode children for traversal.
 *
 * @see SplitTreeNode.h for the tree data structure.
 * @see 05_Greenfield_Plan.md "Multi-Viewport State Machine" section.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/Types.h>
#include <Matcha/UiNodes/Document/SplitTreeNode.h>
#include <Matcha/UiNodes/Core/UiNode.h>

#include <map>
#include <optional>
#include <vector>

class QWidget;
class QKeyEvent;

namespace matcha::gui {
class ViewportFrame;
} // namespace matcha::gui

namespace matcha::fw {

class Viewport;

/** @brief Interaction state of the ViewportGroup. */
enum class ViewportGroupState : uint8_t {
    Normal,    ///< Split tree displayed, all viewports visible.
    Dragging,  ///< User is dragging a viewport header.
    Maximized, ///< Single viewport fills area, others hidden.
    Resizing,  ///< User is dragging a splitter divider.
};

/**
 * @brief UiNode managing a binary split tree of Viewports.
 *
 * **Operations**:
 * - SplitViewport: split an existing viewport into two
 * - RemoveViewport: collapse a split, sibling takes full space
 * - SplitAndMove: detach source, split target, insert source as sibling
 * - SwapViewports: exchange two viewport positions in the tree
 * - MaximizeViewport: temporarily show only one viewport
 * - RestoreLayout: undo maximize, restore split tree
 *
 * **Ownership**: DocumentPage owns ViewportGroup.
 * ViewportGroup owns Viewport UiNode children (flat list).
 * The binary split tree owns layout topology (no QWidget ownership).
 */
class MATCHA_EXPORT ViewportGroup : public UiNode {
public:
    explicit ViewportGroup(std::string name);
    ~ViewportGroup() override;

    ViewportGroup(const ViewportGroup&)            = delete;
    ViewportGroup& operator=(const ViewportGroup&) = delete;
    ViewportGroup(ViewportGroup&&)                 = delete;
    ViewportGroup& operator=(ViewportGroup&&)      = delete;

    // -- State --

    /** @brief Get current interaction state. */
    [[nodiscard]] auto State() const -> ViewportGroupState { return _state; }

    // -- Split tree operations --

    /** @brief Split a viewport into two. Returns the new sibling viewport. */
    auto SplitViewport(ViewportId target, SplitDirection direction)
        -> Expected<observer_ptr<Viewport>>;

    /** @brief Remove a viewport, its sibling takes full space. */
    auto RemoveViewport(ViewportId target) -> Expected<void>;

    /** @brief Get the currently active (focused) viewport. */
    [[nodiscard]] auto ActiveViewport() -> observer_ptr<Viewport>;

    /** @brief Set the active viewport by ID. */
    void SetActiveViewport(ViewportId id);

    /** @brief Temporarily maximize a single viewport (hide others). */
    auto MaximizeViewport(ViewportId target) -> Expected<void>;

    /** @brief Restore the full split tree layout after maximize. */
    void RestoreLayout();

    // -- Viewport rearrangement (drag-and-drop) --

    /**
     * @brief Detach source viewport, split target, place source as sibling.
     *
     * This implements the full drag-to-edge-zone workflow:
     * 1. Detach source leaf from tree (collapse parent if needed)
     * 2. Replace target leaf with SplitNode(direction, 0.5)
     * 3. Arrange children based on drop zone
     */
    auto SplitAndMove(ViewportId source, ViewportId target, DropZone zone)
        -> Expected<void>;

    /** @brief Swap the positions of two viewports in the tree. */
    auto SwapViewports(ViewportId a, ViewportId b) -> Expected<void>;

    // -- Drag-and-drop framework API --

    /**
     * @brief Begin a viewport drag operation. Sets state to Dragging.
     * @param source The viewport being dragged.
     * @return false if drag cannot start (e.g. only 1 viewport, or maximized).
     */
    auto BeginDrag(ViewportId source) -> bool;

    /**
     * @brief Complete a drag-and-drop. Applies SplitAndMove or SwapViewports.
     * @param source The dragged viewport.
     * @param target The drop target viewport.
     * @param zone   Where on the target the source was dropped.
     * @return Result of the underlying tree mutation.
     */
    auto HandleDrop(ViewportId source, ViewportId target, DropZone zone)
        -> Expected<void>;

    /** @brief Cancel a drag operation. Returns to Normal state. */
    void CancelDrag();

    // -- Widget synchronization --

    /**
     * @brief Rebuild the QSplitter widget tree from the binary split tree.
     * @internal Called by DocumentPage after tree mutations. Not part of public API.
     *
     * Detaches all viewport widgets, destroys intermediate splitters,
     * recursively walks the tree to create new NyanSplitter hierarchy,
     * sets sizes from ratios.
     *
     * @param parent  Parent QWidget to host the root splitter.
     *                If nullptr, uses the previously set container.
     */
    void RebuildWidgetTree(QWidget* parent = nullptr);

    /** @brief Get the root QWidget of the current splitter tree (for embedding). */
    [[nodiscard]] auto RootWidget() const -> QWidget*;

    // -- Keyboard shortcuts --

    /**
     * @brief Process a key event for viewport shortcuts.
     *
     * Supported shortcuts:
     * - Ctrl+\           Split active viewport horizontally
     * - Ctrl+Shift+\     Split active viewport vertically
     * - Ctrl+W            Close active viewport (if >1)
     * - Ctrl+Shift+Enter  Maximize / restore active viewport
     *
     * @return true if the key event was consumed.
     */
    auto HandleKeyEvent(QKeyEvent* event) -> bool;

    // -- Tree queries --

    /** @brief Get the number of viewport leaves in the tree. */
    [[nodiscard]] auto ViewportCount() const -> int;

    /** @brief Get all viewport IDs in depth-first order. */
    [[nodiscard]] auto AllViewportIds() const -> std::vector<ViewportId>;

    /** @brief Direct access to the tree root (for testing). */
    [[nodiscard]] auto TreeRoot() const -> const TreeNode* { return _treeRoot.get(); }

    /** @brief Find a Viewport UiNode by ID. */
    [[nodiscard]] auto FindViewport(ViewportId id) -> Viewport*;

    // -- ID generation --

    /** @brief Generate a unique ViewportId. */
    [[nodiscard]] auto NextViewportId() -> ViewportId;

private:
    void FireActiveViewportChanged(ViewportId vpId);
    void FireViewportMoved(ViewportId vpId);
    void FireViewportCreated(ViewportId newId, ViewportId splitFrom, SplitDirection dir);
    void FireViewportRemoved(ViewportId removedId);
    void FireViewportSwapped(ViewportId a, ViewportId b);
    void FireViewportMaximized(ViewportId vpId);
    void FireViewportRestored(ViewportId vpId);
    void FireStateChanged(ViewportGroupState oldState, ViewportGroupState newState);
    void FireSplitRatioChanged(ViewportId first, ViewportId second, double ratio);
    void FireLayoutRebuilt();

    /** @brief Helper to set state with notification. */
    void SetState(ViewportGroupState newState);

    /** @brief Detach a leaf from the tree, collapsing its parent SplitNode. */
    auto DetachLeaf(ViewportId id) -> std::unique_ptr<TreeNode>;

    /** @brief Recursively build QSplitter subtree from a TreeNode. */
    auto BuildWidgetSubtree(const TreeNode* node, QWidget* parent) -> QWidget*;

    /** @brief Destroy all intermediate splitters (not viewport widgets). */
    void DestroyIntermediateSplitters();

    /** @brief Connect splitterMoved signals to ratio writeback. */
    void ConnectSplitterSignals();

    /** @brief Write splitter sizes back to the tree node ratio. */
    void OnSplitterMoved(QWidget* splitter);

    ViewportGroupState                           _state = ViewportGroupState::Normal;
    std::unique_ptr<TreeNode>                    _treeRoot;
    std::optional<ViewportId>                    _activeViewportId;
    std::optional<ViewportId>                    _maximizedId;
    uint32_t                                     _nextVpId = 1;
    QWidget*                                     _container = nullptr;
    QWidget*                                     _rootWidget = nullptr;
    std::vector<QWidget*>                          _splitters;
    std::vector<gui::ViewportFrame*>             _frames;
    std::map<QWidget*, TreeNode*>                _splitterToNode;
    std::optional<ViewportId>                    _dragSourceId;
};

} // namespace matcha::fw
