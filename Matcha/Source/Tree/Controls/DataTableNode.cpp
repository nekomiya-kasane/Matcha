#include "Matcha/Tree/Controls/DataTableNode.h"
#include "Matcha/Tree/UiNodeNotification.h"

#include "Matcha/Widgets/Controls/NyanDataTable.h"
#include "Matcha/Theming/IThemeService.h"

#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QStringList>

namespace matcha::fw {

namespace {

auto ToQtAlignment(gui::HAlign h) -> Qt::Alignment
{
    switch (h) {
    case gui::HAlign::Left:   return Qt::AlignLeft   | Qt::AlignVCenter;
    case gui::HAlign::Center: return Qt::AlignHCenter | Qt::AlignVCenter;
    case gui::HAlign::Right:  return Qt::AlignRight   | Qt::AlignVCenter;
    }
    return Qt::AlignLeft | Qt::AlignVCenter;
}

auto FromQtAlignment(Qt::Alignment a) -> gui::HAlign
{
    if (a & Qt::AlignHCenter) { return gui::HAlign::Center; }
    if (a & Qt::AlignRight)   { return gui::HAlign::Right; }
    return gui::HAlign::Left;
}

} // anonymous namespace

MATCHA_IMPLEMENT_CLASS(DataTableNode, WidgetNode)

DataTableNode::DataTableNode(std::string id)
    : WidgetNode(std::move(id), NodeType::DataTable)
{
}

DataTableNode::~DataTableNode() = default;

// ============================================================================
// Column API
// ============================================================================

void DataTableNode::SetColumns(std::span<const DataColumnDef> columns)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        std::vector<gui::ColumnDef> guiCols;
        guiCols.reserve(columns.size());
        for (const auto& c : columns) {
            gui::ColumnDef cd;
            cd.title     = QString::fromStdString(c.title);
            cd.width     = c.width;
            cd.minWidth  = c.minWidth;
            cd.alignment = ToQtAlignment(c.alignment);
            cd.sortable  = c.sortable;
            cd.editable  = c.editable;
            cd.visible   = c.visible;
            cd.resizable = c.resizable;
            guiCols.push_back(std::move(cd));
        }
        w->SetColumns(guiCols);
    }
}

auto DataTableNode::GetColumns() const -> std::vector<DataColumnDef>
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        const auto& guiCols = w->Columns();
        std::vector<DataColumnDef> result;
        result.reserve(guiCols.size());
        for (const auto& c : guiCols) {
            DataColumnDef cd;
            cd.title     = c.title.toStdString();
            cd.width     = c.width;
            cd.minWidth  = c.minWidth;
            cd.alignment = FromQtAlignment(c.alignment);
            cd.sortable  = c.sortable;
            cd.editable  = c.editable;
            cd.visible   = c.visible;
            cd.resizable = c.resizable;
            result.push_back(std::move(cd));
        }
        return result;
    }
    return {};
}

// ============================================================================
// Header API (convenience)
// ============================================================================

void DataTableNode::SetHeaders(std::span<const std::string> headers)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        QStringList qHeaders;
        qHeaders.reserve(static_cast<int>(headers.size()));
        for (const auto& h : headers) {
            qHeaders.append(QString::fromStdString(h));
        }
        w->SetHeaders(qHeaders);
    }
}

auto DataTableNode::Headers() const -> std::vector<std::string>
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        const auto qHeaders = w->Headers();
        std::vector<std::string> result;
        result.reserve(static_cast<size_t>(qHeaders.size()));
        for (const auto& h : qHeaders) {
            result.push_back(h.toStdString());
        }
        return result;
    }
    return {};
}

auto DataTableNode::ColumnCount() const -> int
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->ColumnCount();
    }
    return 0;
}

// ============================================================================
// Row API
// ============================================================================

void DataTableNode::SetRowCount(int count)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        const int current = w->RowCount();
        if (count > current) {
            const int cols = w->ColumnCount();
            QStringList empty;
            empty.reserve(cols);
            for (int c = 0; c < cols; ++c) {
                empty.append(QString());
            }
            for (int r = current; r < count; ++r) {
                w->AddRow(empty);
            }
        } else if (count < current) {
            w->RemoveRows(count, current - count);
        }
    }
}

auto DataTableNode::RowCount() const -> int
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->RowCount();
    }
    return 0;
}

void DataTableNode::AddRow(std::span<const std::string> cellValues)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        QStringList qValues;
        qValues.reserve(static_cast<int>(cellValues.size()));
        for (const auto& v : cellValues) {
            qValues.append(QString::fromStdString(v));
        }
        w->AddRow(qValues);
    }
}

void DataTableNode::RemoveRow(int row)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        if (row >= 0 && row < w->RowCount()) {
            w->RemoveRows(row, 1);
        }
    }
}

auto DataTableNode::RemoveSelectedRows() -> std::vector<int>
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        const auto qRemoved = w->RemoveSelectedRows();
        std::vector<int> result;
        result.reserve(static_cast<size_t>(qRemoved.size()));
        for (int r : qRemoved) {
            result.push_back(r);
        }
        return result;
    }
    return {};
}

auto DataTableNode::RowData(int row) const -> std::vector<std::string>
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        const auto qRow = w->Row(row);
        std::vector<std::string> result;
        result.reserve(static_cast<size_t>(qRow.size()));
        for (const auto& cell : qRow) {
            result.push_back(cell.toStdString());
        }
        return result;
    }
    return {};
}

void DataTableNode::SetCellText(int row, int col, std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SetCell(row, col, QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto DataTableNode::CellText(int row, int col) const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->Cell(row, col).toStdString();
    }
    return {};
}

void DataTableNode::Clear()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->Clear();
    }
}

// ============================================================================
// Selection API
// ============================================================================

void DataTableNode::SetSelectionMode(gui::SelectionMode mode)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SetSelectionMode(mode);
    }
}

auto DataTableNode::GetSelectionMode() const -> gui::SelectionMode
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->GetSelectionMode();
    }
    return gui::SelectionMode::SingleRow;
}

void DataTableNode::SelectRow(int row)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SelectRow(row);
    }
}

void DataTableNode::ClearSelection()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->ClearSelection();
    }
}

auto DataTableNode::SelectedRow() const -> int
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        const auto rows = w->SelectedRows();
        if (!rows.isEmpty()) {
            return rows.first();
        }
    }
    return -1;
}

auto DataTableNode::SelectedRows() const -> std::vector<int>
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        const auto qRows = w->SelectedRows();
        return {qRows.begin(), qRows.end()};
    }
    return {};
}

// ============================================================================
// Sort API
// ============================================================================

void DataTableNode::SetSortingEnabled(bool enabled)
{
    _sortingEnabled = enabled;
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        if (enabled) {
            w->SortByColumn(0, gui::SortOrder::Ascending);
        } else {
            w->SortByColumn(-1, gui::SortOrder::None);
        }
    }
}

auto DataTableNode::IsSortingEnabled() const -> bool
{
    return _sortingEnabled;
}

void DataTableNode::SortByColumn(int column, gui::SortOrder order)
{
    _sortingEnabled = (order != gui::SortOrder::None);
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SortByColumn(column, order);
    }
}

auto DataTableNode::SortColumn() const -> int
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->SortColumn();
    }
    return -1;
}

auto DataTableNode::GetSortOrder() const -> gui::SortOrder
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->GetSortOrder();
    }
    return gui::SortOrder::None;
}

// ============================================================================
// Appearance API
// ============================================================================

void DataTableNode::SetAlternatingRowColors(bool enabled)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SetAlternatingRowColors(enabled);
    }
}

auto DataTableNode::IsAlternatingRowColors() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->IsAlternatingRowColors();
    }
    return true;
}

void DataTableNode::SetEditable(bool editable)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SetEditable(editable);
    }
}

auto DataTableNode::IsEditable() const -> bool
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->IsEditable();
    }
    return true;
}

// ============================================================================
// Column Width API
// ============================================================================

void DataTableNode::SetColumnWidth(int column, int width)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SetColumnWidth(column, width);
    }
}

auto DataTableNode::ColumnWidth(int column) const -> int
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->ColumnWidth(column);
    }
    return 80;
}

// ============================================================================
// Row Icon API
// ============================================================================

void DataTableNode::SetRowIcon(int row, std::string_view iconId)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        if (iconId.empty()) {
            w->SetRowIcon(row, QIcon());
            return;
        }
        const QColor fg = gui::GetThemeService().Color(gui::ColorToken::colorText);
        const QPixmap pm = gui::GetThemeService().ResolveIcon(
            std::string(iconId), fw::IconToken::iconSizeSM, fg);
        if (!pm.isNull()) {
            w->SetRowIcon(row, QIcon(pm));
        }
    }
}

// ============================================================================
// Filter API
// ============================================================================

void DataTableNode::SetFilterText(std::string_view text)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SetFilterText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    }
}

auto DataTableNode::FilterText() const -> std::string
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->FilterText().toStdString();
    }
    return {};
}

void DataTableNode::ClearFilter()
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->ClearFilter();
    }
}

auto DataTableNode::DisplayRowCount() const -> int
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->DisplayRowCount();
    }
    return 0;
}

// ============================================================================
// Frozen Column API
// ============================================================================

void DataTableNode::SetFrozenColumnCount(int count)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->SetFrozenColumnCount(count);
    }
}

auto DataTableNode::FrozenColumnCount() const -> int
{
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        return w->FrozenColumnCount();
    }
    return 0;
}

// ============================================================================
// Scroll API
// ============================================================================

void DataTableNode::ScrollToRow(int row)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->ScrollToRow(row);
    }
}

void DataTableNode::EnsureRowVisible(int row)
{
    EnsureWidget();
    if (auto* w = qobject_cast<gui::NyanDataTable*>(_widget)) {
        w->EnsureRowVisible(row);
    }
}

// ============================================================================
// Widget Creation + Signal Wiring
// ============================================================================

auto DataTableNode::CreateWidget(QWidget* parent) -> QWidget*
{
    auto* w = new gui::NyanDataTable(parent);

    QObject::connect(w, &gui::NyanDataTable::SelectionChanged, w, [this, w]() {
        const auto selected = w->SelectedRows();
        if (!selected.isEmpty()) {
            CellSelected cellNotif(selected.first(), 0);
            SendNotification(this, cellNotif);
        }
        SelectionChanged selNotif;
        SendNotification(this, selNotif);
    });

    QObject::connect(w, &gui::NyanDataTable::CellChanged, w,
                     [this](int row, int column, const QString& newValue) {
                         CellChanged notif(row, column, newValue.toStdString());
                         SendNotification(this, notif);
                     });

    QObject::connect(w, &gui::NyanDataTable::DataChanged, w, [this]() {
        DataChanged notif;
        SendNotification(this, notif);
    });

    QObject::connect(w, &gui::NyanDataTable::RowAdded, w, [this](int row) {
        RowAdded notif(row);
        SendNotification(this, notif);
    });

    QObject::connect(w, &gui::NyanDataTable::SortChanged, w,
                     [this](int column, gui::SortOrder order) {
                         SortChanged notif(column, static_cast<int>(order));
                         SendNotification(this, notif);
                     });

    QObject::connect(w, &gui::NyanDataTable::EmptyClicked, w, [this]() {
        EmptyClicked notif;
        SendNotification(this, notif);
    });

    return w;
}

} // namespace matcha::fw
