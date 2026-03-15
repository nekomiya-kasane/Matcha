#pragma once

/**
 * @file NyanTabBar.h
 * @brief Unified self-drawn tab bar for document tabs.
 *
 * NyanTabBar is a pure QWidget with custom painting (no QTabBar inheritance).
 * Used in both Main window (TitleBar style) and Floating windows (Floating style).
 *
 * Tab identification uses PageId exclusively at the widget layer.
 *
 * @par Visual specification
 * - TabStyle::TitleBar: 24px height, accent background, white text, rounded corners
 * - TabStyle::Floating: 28px height, standard tab appearance, document-mode look
 *
 * @see TabBarNode for the UiNode wrapper.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/StrongId.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

#include <vector>

namespace matcha::gui { class NyanTabItem; }

namespace matcha::gui {

/// @brief Visual style for the tab bar.
enum class TabStyle : uint8_t {
    TitleBar,   ///< Embedded in main title bar row 2 (accent bg, white text, 24px)
    Floating,   ///< Standalone in floating window (28px, standard look)
};

/**
 * @brief Unified self-drawn tab bar for document tabs.
 *
 * A11y role: TabList.
 */
class MATCHA_EXPORT NyanTabBar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanTabBar(TabStyle style, QWidget* parent = nullptr);
    ~NyanTabBar() override;

    NyanTabBar(const NyanTabBar&)            = delete;
    NyanTabBar& operator=(const NyanTabBar&) = delete;
    NyanTabBar(NyanTabBar&&)                 = delete;
    NyanTabBar& operator=(NyanTabBar&&)      = delete;

    // -- Tab Management --

    /// @brief Add a tab at the end. Returns the new NyanTabItem widget.
    auto AddTab(fw::PageId pageId, const QString& title) -> NyanTabItem*;

    /// @brief Remove a tab by PageId. The NyanTabItem is deleted.
    void RemoveTab(fw::PageId pageId);

    /// @brief Set the active tab by PageId.
    void SetActiveTab(fw::PageId pageId);

    /// @brief Set tab title by PageId.
    void SetTabTitle(fw::PageId pageId, const QString& title);

    /// @brief Move a tab from one index to another (reorder).
    void MoveTab(int fromIndex, int toIndex);

    /// @brief Insert a tab at a specific index. Returns the NyanTabItem.
    auto InsertTab(int index, fw::PageId pageId, const QString& title) -> NyanTabItem*;

    /// @brief MIME type for tab drag & drop.
    static constexpr const char* kMimeType = "application/x-matcha-tab";

    /// @brief Get the NyanTabItem at a given index. Returns nullptr if invalid.
    [[nodiscard]] auto ItemAt(int index) const -> NyanTabItem*;

    /// @brief Find the NyanTabItem for a PageId. Returns nullptr if not found.
    [[nodiscard]] auto FindItem(fw::PageId pageId) const -> NyanTabItem*;

    /// @brief Get the index for a PageId. Returns -1 if not found.
    [[nodiscard]] auto IndexOfPage(fw::PageId pageId) const -> int;

    /// @brief Get the currently active PageId. Returns PageId{0} if none.
    [[nodiscard]] auto ActivePageId() const -> fw::PageId;

    /// @brief Get the active tab index. Returns -1 if none.
    [[nodiscard]] auto ActiveIndex() const -> int { return _activeIndex; }

    /// @brief Number of tabs.
    [[nodiscard]] auto TabCount() const -> int;

    /// @brief Get the visual style.
    [[nodiscard]] auto Style() const -> TabStyle { return _style; }

    /// @brief Called by NyanTabItem during reorder drag. Swaps if threshold crossed.
    void RequestReorder(NyanTabItem* item, int globalX);

    /// @brief Called by NyanTabItem when detach-mode QDrag completes.
    void HandleDragResult(fw::PageId pageId, Qt::DropAction result);

Q_SIGNALS:
    /// @brief Emitted when a tab is pressed (clicked).
    void TabPressed(matcha::fw::PageId pageId);

    /// @brief Emitted when a tab's close button is clicked.
    void TabCloseRequested(matcha::fw::PageId pageId);

    /// @brief Emitted when bar-internal reorder completes.
    void TabReordered(matcha::fw::PageId pageId, int oldIndex, int newIndex);

    /// @brief Emitted when an external tab is dropped onto this bar.
    void TabDropReceived(matcha::fw::PageId pageId, int insertIndex);

    /// @brief Emitted when a tab drag ends with no drop target.
    void TabDraggedToVoid(matcha::fw::PageId pageId, QPoint globalPos);

    /// @brief Emitted when the quick-add (+) button is clicked.
    void AddTabRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;
    void OnThemeChanged() override;

private:
    void RecalcLayout();
    void UpdateAutoHide();
    [[nodiscard]] auto AddButtonRect() const -> QRect;
    [[nodiscard]] auto InsertIndexAt(int x) const -> int;
    void DrawAddButton(QPainter& painter) const;
    void DrawInsertionIndicator(QPainter& painter) const;
    void OnItemPressed(fw::PageId pageId);

    // Style-dependent metrics
    [[nodiscard]] auto ItemWidth() const -> int;
    [[nodiscard]] auto ItemHeight() const -> int;

    TabStyle _style;
    std::vector<NyanTabItem*> _items;   ///< Owned as QWidget children
    int _activeIndex = -1;
    bool _hoveredAddButton = false;
    int  _insertIndicatorIndex = -1;    ///< Drop insertion indicator position (-1 = hidden)

    static constexpr int kTitleBarTabWidth   = 105;
    static constexpr int kTitleBarTabHeight  = 24;
    static constexpr int kFloatingTabWidth   = 120;
    static constexpr int kFloatingTabHeight  = 28;
    static constexpr int kAddBtnSize         = 20;
    static constexpr int kTabGap             = 2;
};

} // namespace matcha::gui
