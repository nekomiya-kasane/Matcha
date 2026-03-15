#pragma once

/**
 * @file DataTableNode.h
 * @brief Typed WidgetNode wrapping NyanDataTable for UiNode tree.
 *
 * Features:
 * - Column headers, row add/remove, cell read/write
 * - Single/multi-row selection with programmatic control
 * - Column sort by column index + order (Ascending/Descending/None)
 * - Alternating row colors (striped), editable toggle
 * - Per-column width control, per-row icon
 * - Full Notification catalog: CellSelected, CellChanged, DataChanged,
 *   RowAdded, RowRemoved, SelectionChanged, SortChanged, EmptyClicked
 */

#include "Matcha/Tree/FSM/WidgetEnums.h"
#include "Matcha/Tree/WidgetNode.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

/// @brief Column definition for DataTableNode (Qt-free).
struct MATCHA_EXPORT DataColumnDef {
    std::string title;                        ///< Display header text
    int width    = 80;                        ///< Column width in px
    int minWidth = 40;                        ///< Minimum width during resize
    gui::HAlign alignment = gui::HAlign::Left; ///< Cell horizontal alignment
    bool sortable  = true;                    ///< Whether header click sorts
    bool editable  = true;                    ///< Per-column editable override
    bool visible   = true;                    ///< Column visibility toggle
    bool resizable = true;                    ///< Whether user can drag-resize
};

/**
 * @brief UiNode wrapper for NyanDataTable (Scheme D typed node).
 *
 * Dispatches: CellSelected, CellChanged, DataChanged, RowAdded,
 *             RowRemoved, SelectionChanged, SortChanged, EmptyClicked.
 */
class MATCHA_EXPORT DataTableNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using CellSelected     = matcha::fw::CellSelected;
        using CellChanged      = matcha::fw::CellChanged;
        using DataChanged      = matcha::fw::DataChanged;
        using RowAdded         = matcha::fw::RowAdded;
        using RowRemoved       = matcha::fw::RowRemoved;
        using SelectionChanged = matcha::fw::SelectionChanged;
        using SortChanged      = matcha::fw::SortChanged;
        using EmptyClicked     = matcha::fw::EmptyClicked;
    };

    explicit DataTableNode(std::string id);
    ~DataTableNode() override;

    DataTableNode(const DataTableNode&) = delete;
    auto operator=(const DataTableNode&) -> DataTableNode& = delete;
    DataTableNode(DataTableNode&&) = delete;
    auto operator=(DataTableNode&&) -> DataTableNode& = delete;

    // -- Column API --

    void SetColumns(std::span<const DataColumnDef> columns);
    [[nodiscard]] auto GetColumns() const -> std::vector<DataColumnDef>;

    // -- Header API (convenience) --

    void SetHeaders(std::span<const std::string> headers);
    [[nodiscard]] auto Headers() const -> std::vector<std::string>;
    [[nodiscard]] auto ColumnCount() const -> int;

    // -- Row API --

    void SetRowCount(int count);
    [[nodiscard]] auto RowCount() const -> int;

    void AddRow(std::span<const std::string> cellValues);
    void RemoveRow(int row);
    [[nodiscard]] auto RemoveSelectedRows() -> std::vector<int>;

    [[nodiscard]] auto RowData(int row) const -> std::vector<std::string>;

    void SetCellText(int row, int col, std::string_view text);
    [[nodiscard]] auto CellText(int row, int col) const -> std::string;

    void Clear();

    // -- Selection API --

    void SetSelectionMode(gui::SelectionMode mode);
    [[nodiscard]] auto GetSelectionMode() const -> gui::SelectionMode;

    void SelectRow(int row);
    void ClearSelection();
    [[nodiscard]] auto SelectedRow() const -> int;
    [[nodiscard]] auto SelectedRows() const -> std::vector<int>;

    // -- Sort API --

    void SetSortingEnabled(bool enabled);
    [[nodiscard]] auto IsSortingEnabled() const -> bool;

    void SortByColumn(int column, gui::SortOrder order);
    [[nodiscard]] auto SortColumn() const -> int;
    [[nodiscard]] auto GetSortOrder() const -> gui::SortOrder;

    // -- Appearance API --

    void SetAlternatingRowColors(bool enabled);
    [[nodiscard]] auto IsAlternatingRowColors() const -> bool;

    void SetEditable(bool editable);
    [[nodiscard]] auto IsEditable() const -> bool;

    // -- Column Width API --

    void SetColumnWidth(int column, int width);
    [[nodiscard]] auto ColumnWidth(int column) const -> int;

    // -- Row Icon API --

    void SetRowIcon(int row, std::string_view iconId);

    // -- Filter API --

    void SetFilterText(std::string_view text);
    [[nodiscard]] auto FilterText() const -> std::string;
    void ClearFilter();
    [[nodiscard]] auto DisplayRowCount() const -> int;

    // -- Frozen Column API --

    void SetFrozenColumnCount(int count);
    [[nodiscard]] auto FrozenColumnCount() const -> int;

    // -- Scroll API --

    void ScrollToRow(int row);
    void EnsureRowVisible(int row);

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;

private:
    bool _sortingEnabled = false;
};

} // namespace matcha::fw
