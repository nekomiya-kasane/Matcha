#pragma once

/**
 * @file NyanDataTable.h
 * @brief Theme-aware read/write tabular data grid.
 *
 * Replaces old `NyanToolTable` with semantic naming and enhanced features.
 * Custom-painted header with sort indicator. Column resize by drag.
 * Cell editing on double-click. Row icon support.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanToolTable.h`
 * - SetTableHeaders -> SetHeaders
 * - AddRow(QStringList, QIcon) -> AddRow(QStringList, QIcon)
 * - GetCell/SetCell -> Cell/SetCell
 * - RemoveRows/RemoveSelectRows/Clear -> same
 * - AddTableDataClicked -> EmptyClicked
 * - DataChanged -> DataChanged + CellChanged + RowAdded
 *
 * @par Visual specification
 * - Background: Background4, 3px rounded corners
 * - Header: Background3, Foreground1 text, sort indicator arrow
 * - Rows: alternating Background1/Background2, Foreground1 text
 * - Selection: PrimaryNormal background, Foreground0 text
 * - Row icon: 16x16 in first column
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/WidgetEnums.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <functional>
#include <span>
#include <vector>

namespace matcha::gui {

class SimpleWidgetEventFilter;
class NyanScrollBar;

/// @brief Sort comparator: returns true if a < b.
using SortComparator = std::function<bool(const QString& a, const QString& b)>;

/// @brief Row filter predicate: returns true to include the row.
using RowFilterPredicate = std::function<bool(int row, const QStringList& cells)>;

/// @brief Column definition for DataTable.
struct ColumnDef {
    QString title;                                           ///< Display header text
    int width    = 80;                                       ///< Column width in px
    int minWidth = 40;                                       ///< Minimum width during resize
    Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter; ///< Cell text alignment
    bool sortable  = true;                                   ///< Whether header click sorts
    bool editable  = true;                                   ///< Per-column editable override
    bool visible   = true;                                   ///< Column visibility toggle
    bool resizable = true;                                   ///< Whether user can drag-resize
    SortComparator sortComparator;                            ///< Custom comparator (null = lexicographic)
};

/**
 * @brief Theme-aware read/write tabular data grid.
 *
 * Replaces old NyanToolTable. A11y role: Table.
 */
class MATCHA_EXPORT NyanDataTable : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a data table.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanDataTable(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanDataTable() override;

    NyanDataTable(const NyanDataTable&)            = delete;
    NyanDataTable& operator=(const NyanDataTable&) = delete;
    NyanDataTable(NyanDataTable&&)                 = delete;
    NyanDataTable& operator=(NyanDataTable&&)      = delete;

    // ========================================================================
    // Column API
    // ========================================================================

    /// @brief Set columns from ColumnDef array. Clears data if column count changes.
    void SetColumns(std::span<const ColumnDef> columns);

    /// @brief Get current column definitions.
    [[nodiscard]] auto Columns() const -> const std::vector<ColumnDef>&;

    /// @brief Get mutable column definition (for in-place edits; call update() after).
    [[nodiscard]] auto Column(int index) -> ColumnDef&;

    /// @brief Get const column definition.
    [[nodiscard]] auto Column(int index) const -> const ColumnDef&;

    // ========================================================================
    // Header API (convenience — delegates to Column API)
    // ========================================================================

    /// @brief Set column headers. Clears existing data if column count changes.
    void SetHeaders(const QStringList& headers);

    /// @brief Get current column headers.
    [[nodiscard]] auto Headers() const -> QStringList;

    /// @brief Get total column count (including hidden).
    [[nodiscard]] auto ColumnCount() const -> int;

    /// @brief Get visible column count.
    [[nodiscard]] auto VisibleColumnCount() const -> int;

    // ========================================================================
    // Row API
    // ========================================================================

    /// @brief Get row count.
    [[nodiscard]] auto RowCount() const -> int;

    /// @brief Add a row with data and optional icon.
    void AddRow(const QStringList& data, const QIcon& icon = {});

    /// @brief Get all cell values for a row.
    [[nodiscard]] auto Row(int row) const -> QStringList;

    /// @brief Get cell value at (row, column).
    [[nodiscard]] auto Cell(int row, int column) const -> QString;

    /// @brief Set cell value at (row, column).
    void SetCell(int row, int column, const QString& value);

    /// @brief Remove rows starting at index.
    void RemoveRows(int startRow, int count = 1);

    /// @brief Remove currently selected rows. Returns removed row indices (descending).
    [[nodiscard]] auto RemoveSelectedRows() -> QList<int>;

    /// @brief Clear all data and headers.
    void Clear();

    // ========================================================================
    // Selection API
    // ========================================================================

    /// @brief Set selection mode.
    void SetSelectionMode(SelectionMode mode);

    /// @brief Get selection mode.
    [[nodiscard]] auto GetSelectionMode() const -> SelectionMode;

    /// @brief Get selected row indices.
    [[nodiscard]] auto SelectedRows() const -> QList<int>;

    /// @brief Select a row programmatically.
    void SelectRow(int row);

    /// @brief Clear selection.
    void ClearSelection();

    // ========================================================================
    // Appearance API
    // ========================================================================

    /// @brief Enable or disable alternating row background colors (striped rows).
    void SetAlternatingRowColors(bool enabled);

    /// @brief Check if alternating row colors are enabled.
    [[nodiscard]] auto IsAlternatingRowColors() const -> bool;

    /// @brief Enable or disable cell editing on double-click.
    void SetEditable(bool editable);

    /// @brief Check if the table allows cell editing.
    [[nodiscard]] auto IsEditable() const -> bool;

    // ========================================================================
    // Column Width API
    // ========================================================================

    /// @brief Set the width of a specific column in pixels.
    void SetColumnWidth(int column, int width);

    /// @brief Get the width of a specific column in pixels.
    [[nodiscard]] auto ColumnWidth(int column) const -> int;

    // ========================================================================
    // Row Icon API
    // ========================================================================

    /// @brief Set the icon for an existing row.
    void SetRowIcon(int row, const QIcon& icon);

    /// @brief Get the icon for a row.
    [[nodiscard]] auto RowIcon(int row) const -> QIcon;

    // ========================================================================
    // Sort API
    // ========================================================================

    /// @brief Sort by column with specified order.
    void SortByColumn(int column, SortOrder order);

    /// @brief Get current sort column (-1 if none).
    [[nodiscard]] auto SortColumn() const -> int;

    /// @brief Get current sort order.
    [[nodiscard]] auto GetSortOrder() const -> SortOrder;

    // ========================================================================
    // Filter API
    // ========================================================================

    /// @brief Set a text filter. Rows whose any cell contains the text are shown.
    void SetFilterText(const QString& text);

    /// @brief Get current filter text.
    [[nodiscard]] auto FilterText() const -> QString;

    /// @brief Set a custom filter predicate (overrides text filter).
    void SetFilterPredicate(RowFilterPredicate predicate);

    /// @brief Clear all filters.
    void ClearFilter();

    /// @brief Get display row count (after filtering).
    [[nodiscard]] auto DisplayRowCount() const -> int;

    /// @brief Map display row index to actual data row index.
    [[nodiscard]] auto DisplayToDataRow(int displayRow) const -> int;

    // ========================================================================
    // Frozen Column API
    // ========================================================================

    /// @brief Set the number of columns frozen on the left (0 = none).
    void SetFrozenColumnCount(int count);

    /// @brief Get the number of frozen columns.
    [[nodiscard]] auto FrozenColumnCount() const -> int;

    // ========================================================================
    // Scroll API
    // ========================================================================

    /// @brief Scroll to make the given row visible.
    void ScrollToRow(int row);

    /// @brief Ensure the given row is visible (scroll only if needed).
    void EnsureRowVisible(int row);

    /// @brief Get the index of the first visible row.
    [[nodiscard]] auto FirstVisibleRow() const -> int;

    /// @brief Get the number of visible rows (fits in viewport).
    [[nodiscard]] auto VisibleRowCount() const -> int;

    /// @brief Get the currently hovered row (-1 if none).
    [[nodiscard]] auto HoveredRow() const -> int;

    // ========================================================================
    // Geometry
    // ========================================================================

    /// @brief Size hint: 250x100 (matches old NyanToolTable).
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Minimum size hint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when a cell value changes.
    void CellChanged(int row, int column, const QString& newValue);

    /// @brief Emitted when a row is added.
    void RowAdded(int row);

    /// @brief Emitted when data changes (add/remove/edit).
    void DataChanged();

    /// @brief Emitted when empty table "add data" area is clicked.
    void EmptyClicked();

    /// @brief Emitted when selection changes.
    void SelectionChanged();

    /// @brief Emitted when sort column or order changes.
    void SortChanged(int column, matcha::gui::SortOrder order);

protected:
    /// @brief Custom paint: header + rows + selection.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle mouse press for selection and header click (sort).
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Handle mouse move for column resize.
    void mouseMoveEvent(QMouseEvent* event) override;

    /// @brief Handle mouse release for column resize.
    void mouseReleaseEvent(QMouseEvent* event) override;

    /// @brief Handle double-click for cell editing.
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    /// @brief Handle mouse wheel for vertical scrolling.
    void wheelEvent(QWheelEvent* event) override;

    /// @brief Handle keyboard navigation (arrows, Enter, Esc, Tab).
    void keyPressEvent(QKeyEvent* event) override;

    /// @brief Handle resize to update scroll bar geometry.
    void resizeEvent(QResizeEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    struct RowData {
        QIcon icon;
        QStringList cells;
    };

    void UpdateColumnWidths();
    void RebuildFilteredIndices();
    void UpdateScrollBars();
    void LayoutScrollBars();
    [[nodiscard]] auto HitTestHeader(const QPoint& pos) const -> int;
    [[nodiscard]] auto HitTestRow(const QPoint& pos) const -> int;
    [[nodiscard]] auto HitTestColumnEdge(const QPoint& pos) const -> int;
    [[nodiscard]] auto ContentHeight() const -> int;
    [[nodiscard]] auto ViewportHeight() const -> int;
    [[nodiscard]] auto TotalColumnWidth() const -> int;
    [[nodiscard]] auto ViewportWidth() const -> int;
    [[nodiscard]] auto FrozenColumnsWidth() const -> int;
    [[nodiscard]] auto DataToDisplayRow(int dataRow) const -> int;
    void BeginCellEdit(int row, int col);

    static constexpr int kHeaderHeight = 28;
    static constexpr int kRowHeight    = 26;
    static constexpr int kIconSize     = 16;
    static constexpr int kHPadding     = 8;
    static constexpr int kResizeZone   = 4;
    static constexpr int kScrollBarWidth = 10;

    std::vector<ColumnDef> _columns;
    std::vector<RowData> _rows;
    QSet<int> _selectedRows;

    SelectionMode _selectionMode = SelectionMode::SingleRow;
    bool _editable = true;
    bool _alternatingRowColors = true;
    int _sortColumn = -1;
    SortOrder _sortOrder = SortOrder::None;

    int _resizingColumn = -1;
    int _resizeStartX = 0;
    int _resizeStartWidth = 0;

    // Scroll state
    int _vScrollOffset = 0;
    int _hScrollOffset = 0;
    NyanScrollBar* _vScrollBar = nullptr;
    NyanScrollBar* _hScrollBar = nullptr;

    // Frozen columns
    int _frozenColumnCount = 0;

    // Hover state
    int _hoveredRow = -1;

    // Filter state
    QString _filterText;
    RowFilterPredicate _filterPredicate;
    std::vector<int> _filteredIndices;  ///< Maps display row -> data row
    bool _filterActive = false;

    // Focus cell for keyboard navigation
    int _focusRow = -1;
    int _focusCol = -1;

    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
