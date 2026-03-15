#pragma once

/**
 * @file NyanStructureTree.h
 * @brief Generic hierarchical tree browser (Navigator).
 *
 * NyanStructureTree provides:
 * - QTreeView-based hierarchical display
 * - Custom branch drawing with expand/collapse icons
 * - Title bar with auto-hide behavior
 * - Transparent background support
 * - Drag-and-drop support
 *
 * @par Visual specification
 * - Transparent background by default
 * - Custom branch lines (Line3 color, 0.5px)
 * - Expand/collapse icons (12x12)
 * - Title bar auto-hides on mouse leave
 *
 * @note This is a generic tree widget. Business layer supplies the model.
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QTreeView>
#include <QWidget>

class QAbstractItemModel;
class QLabel;
class QPushButton;
class QTimer;
class QVBoxLayout;

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Generic hierarchical tree browser (Navigator).
 *
 * A11y role: Tree.
 */
class MATCHA_EXPORT NyanStructureTree : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a structure tree.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanStructureTree(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanStructureTree() override;

    NyanStructureTree(const NyanStructureTree&)            = delete;
    NyanStructureTree& operator=(const NyanStructureTree&) = delete;
    NyanStructureTree(NyanStructureTree&&)                 = delete;
    NyanStructureTree& operator=(NyanStructureTree&&)      = delete;

    // -- Model --

    /// @brief Set the tree model.
    void SetModel(QAbstractItemModel* model);

    /// @brief Get the tree model.
    [[nodiscard]] auto Model() const -> QAbstractItemModel*;

    /// @brief Get the underlying tree view.
    [[nodiscard]] auto TreeView() -> QTreeView*;

    // -- Configuration --

    /// @brief Set drag enabled.
    void SetDragEnabled(bool enabled);

    /// @brief Check if drag is enabled.
    [[nodiscard]] auto IsDragEnabled() const -> bool;

    /// @brief Set the context menu policy.
    void SetTreeContextMenuPolicy(Qt::ContextMenuPolicy policy);

    /// @brief Set transparent background.
    void SetBackgroundTransparent(bool transparent);

    /// @brief Check if background is transparent.
    [[nodiscard]] auto IsBackgroundTransparent() const -> bool;

    // -- Title Bar --

    /// @brief Set the title text.
    void SetTitle(const QString& title);

    /// @brief Get the title text.
    [[nodiscard]] auto Title() const -> QString;

    /// @brief Set title bar auto-hide behavior.
    void SetTitleBarAutoHide(bool autoHide);

    /// @brief Check if title bar auto-hides.
    [[nodiscard]] auto IsTitleBarAutoHide() const -> bool;

    /// @brief Set title bar pinned (always visible).
    void SetTitleBarPinned(bool pinned);

    /// @brief Check if title bar is pinned.
    [[nodiscard]] auto IsTitleBarPinned() const -> bool;

    // -- Collapse --

    /// @brief Set collapsed state.
    void SetCollapsed(bool collapsed);

    /// @brief Check if collapsed.
    [[nodiscard]] auto IsCollapsed() const -> bool;

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when collapse state changes.
    void CollapsedChanged(bool collapsed);

    /// @brief Emitted when an item is double-clicked.
    void ItemDoubleClicked(const QModelIndex& index);

    /// @brief Emitted when selection changes.
    void SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    /// @brief Emitted when context menu is requested.
    void ContextMenuRequested(const QPoint& globalPos, const QModelIndex& index);

protected:
    /// @brief Handle enter event for title bar show.
    void enterEvent(QEnterEvent* event) override;

    /// @brief Handle leave event for title bar hide.
    void leaveEvent(QEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void InitTreeView();
    void UpdateTitleBarVisibility();
    void OnCollapseButtonClicked();
    void OnPinButtonClicked();
    void OnTitleBarHideTimeout();

    static constexpr int kTitleBarHeight = 24;
    static constexpr int kHideDelay      = 500;

    QVBoxLayout*  _layout           = nullptr;
    QWidget*      _titleBar         = nullptr;
    QLabel*       _titleLabel       = nullptr;
    QPushButton*  _collapseButton   = nullptr;
    QPushButton*  _pinButton        = nullptr;
    QTreeView*    _treeView         = nullptr;
    QTimer*       _hideTimer        = nullptr;

    bool _titleBarAutoHide   = true;
    bool _titleBarPinned     = false;
    bool _collapsed          = false;
    bool _backgroundTransparent = true;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

/**
 * @brief Custom tree view with themed branch drawing.
 */
class MATCHA_EXPORT NyanTreeView : public QTreeView, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanTreeView(QWidget* parent = nullptr);
    ~NyanTreeView() override;

    NyanTreeView(const NyanTreeView&)            = delete;
    NyanTreeView& operator=(const NyanTreeView&) = delete;
    NyanTreeView(NyanTreeView&&)                 = delete;
    NyanTreeView& operator=(NyanTreeView&&)      = delete;

    void SetBackgroundTransparent(bool transparent);

protected:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
    void OnThemeChanged() override;

private:
    [[nodiscard]] auto BranchIconSize() const -> QSize;

    bool _backgroundTransparent = true;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
