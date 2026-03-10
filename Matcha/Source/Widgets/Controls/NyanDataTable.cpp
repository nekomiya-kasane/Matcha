/**
 * @file NyanDataTable.cpp
 * @brief Implementation of NyanDataTable themed tabular data grid.
 */

#include <Matcha/Widgets/Controls/NyanDataTable.h>
#include <Matcha/Widgets/Shell/NyanScrollBar.h>

#include "../Core/InteractionEventFilter.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>

#include <algorithm>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanDataTable::NyanDataTable(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::DataTable)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    _interactionFilter = new InteractionEventFilter(this, nullptr);

    _vScrollBar = new NyanScrollBar(Qt::Vertical, this);
    _hScrollBar = new NyanScrollBar(Qt::Horizontal, this);
    _vScrollBar->setVisible(false);
    _hScrollBar->setVisible(false);

    QObject::connect(_vScrollBar, &QScrollBar::valueChanged, this, [this](int val) {
        _vScrollOffset = val;
        update();
    });
    QObject::connect(_hScrollBar, &QScrollBar::valueChanged, this, [this](int val) {
        _hScrollOffset = val;
        update();
    });
}

NyanDataTable::~NyanDataTable() = default;

// ============================================================================
// Column API
// ============================================================================

void NyanDataTable::SetColumns(std::span<const ColumnDef> columns)
{
    if (!_rows.empty() && static_cast<int>(columns.size()) != ColumnCount()) {
        _rows.clear();
        _selectedRows.clear();
        _sortColumn = -1;
        _sortOrder = SortOrder::None;
    }
    _columns.assign(columns.begin(), columns.end());
    UpdateScrollBars();
    emit DataChanged();
    update();
}

auto NyanDataTable::Columns() const -> const std::vector<ColumnDef>&
{
    return _columns;
}

auto NyanDataTable::Column(int index) -> ColumnDef&
{
    return _columns.at(static_cast<size_t>(index));
}

auto NyanDataTable::Column(int index) const -> const ColumnDef&
{
    return _columns.at(static_cast<size_t>(index));
}

// ============================================================================
// Header API (convenience)
// ============================================================================

void NyanDataTable::SetHeaders(const QStringList& headers)
{
    if (!_rows.empty() && headers.size() != ColumnCount()) {
        _rows.clear();
        _selectedRows.clear();
        _sortColumn = -1;
        _sortOrder = SortOrder::None;
    }
    _columns.clear();
    _columns.reserve(static_cast<size_t>(headers.size()));
    const int nCols = static_cast<int>(headers.size());
    const int defaultWidth = nCols > 0 ? std::max(80, (width() - 20) / nCols) : 80;
    for (const auto& h : headers) {
        ColumnDef cd;
        cd.title = h;
        cd.width = defaultWidth;
        _columns.push_back(std::move(cd));
    }
    UpdateScrollBars();
    update();
}

auto NyanDataTable::Headers() const -> QStringList
{
    QStringList result;
    result.reserve(ColumnCount());
    for (const auto& col : _columns) {
        result.append(col.title);
    }
    return result;
}

auto NyanDataTable::ColumnCount() const -> int
{
    return static_cast<int>(_columns.size());
}

auto NyanDataTable::VisibleColumnCount() const -> int
{
    int count = 0;
    for (const auto& col : _columns) {
        if (col.visible) { ++count; }
    }
    return count;
}

// ============================================================================
// Row API
// ============================================================================

auto NyanDataTable::RowCount() const -> int
{
    return static_cast<int>(_rows.size());
}

void NyanDataTable::AddRow(const QStringList& data, const QIcon& icon)
{
    RowData row;
    row.icon = icon;
    row.cells = data;
    // Pad or truncate to match column count.
    const int colCount = ColumnCount();
    while (row.cells.size() < colCount) {
        row.cells.append(QString());
    }
    if (row.cells.size() > colCount) {
        row.cells = row.cells.mid(0, colCount);
    }
    _rows.push_back(std::move(row));
    RebuildFilteredIndices();
    UpdateScrollBars();
    emit RowAdded(static_cast<int>(_rows.size()) - 1);
    emit DataChanged();
    update();
}

auto NyanDataTable::Row(int row) const -> QStringList
{
    if (row < 0 || row >= RowCount()) {
        return {};
    }
    return _rows[static_cast<size_t>(row)].cells;
}

auto NyanDataTable::Cell(int row, int column) const -> QString
{
    if (row < 0 || row >= RowCount() || column < 0 || column >= ColumnCount()) {
        return {};
    }
    const auto& cells = _rows[static_cast<size_t>(row)].cells;
    if (column >= cells.size()) {
        return {};
    }
    return cells[column];
}

void NyanDataTable::SetCell(int row, int column, const QString& value)
{
    if (row < 0 || row >= RowCount() || column < 0 || column >= ColumnCount()) {
        return;
    }
    auto& cells = _rows[static_cast<size_t>(row)].cells;
    while (cells.size() <= column) {
        cells.append(QString());
    }
    cells[column] = value;
    RebuildFilteredIndices();
    emit CellChanged(row, column, value);
    emit DataChanged();
    update();
}

void NyanDataTable::RemoveRows(int startRow, int count)
{
    if (startRow < 0 || startRow >= RowCount() || count <= 0) {
        return;
    }
    const int endRow = std::min(startRow + count, RowCount());
    _rows.erase(_rows.begin() + startRow, _rows.begin() + endRow);
    _selectedRows.clear();
    RebuildFilteredIndices();
    UpdateScrollBars();
    emit DataChanged();
    emit SelectionChanged();
    update();
}

auto NyanDataTable::RemoveSelectedRows() -> QList<int>
{
    QList<int> removed(_selectedRows.begin(), _selectedRows.end());
    std::sort(removed.begin(), removed.end(), std::greater<int>());
    for (int row : removed) {
        if (row >= 0 && row < RowCount()) {
            _rows.erase(_rows.begin() + row);
        }
    }
    _selectedRows.clear();
    RebuildFilteredIndices();
    UpdateScrollBars();
    emit DataChanged();
    emit SelectionChanged();
    update();
    return removed;
}

void NyanDataTable::Clear()
{
    _columns.clear();
    _rows.clear();
    _selectedRows.clear();
    _filteredIndices.clear();
    _filterActive = false;
    _filterText.clear();
    _filterPredicate = nullptr;
    _sortColumn = -1;
    _sortOrder = SortOrder::None;
    UpdateScrollBars();
    emit DataChanged();
    emit SelectionChanged();
    update();
}

// ============================================================================
// Selection API
// ============================================================================

void NyanDataTable::SetSelectionMode(SelectionMode mode)
{
    _selectionMode = mode;
    if (mode == SelectionMode::SingleRow && _selectedRows.size() > 1) {
        auto it = _selectedRows.begin();
        int first = *it;
        _selectedRows.clear();
        _selectedRows.insert(first);
        emit SelectionChanged();
        update();
    }
}

auto NyanDataTable::GetSelectionMode() const -> SelectionMode
{
    return _selectionMode;
}

auto NyanDataTable::SelectedRows() const -> QList<int>
{
    return QList<int>(_selectedRows.begin(), _selectedRows.end());
}

void NyanDataTable::SelectRow(int row)
{
    if (row < 0 || row >= RowCount()) {
        return;
    }
    if (_selectionMode == SelectionMode::SingleRow) {
        _selectedRows.clear();
    }
    _selectedRows.insert(row);
    emit SelectionChanged();
    update();
}

void NyanDataTable::ClearSelection()
{
    _selectedRows.clear();
    emit SelectionChanged();
    update();
}

// ============================================================================
// Sort API
// ============================================================================

void NyanDataTable::SortByColumn(int column, SortOrder order)
{
    if (column < 0 || column >= ColumnCount()) {
        _sortColumn = -1;
        _sortOrder = SortOrder::None;
        return;
    }
    _sortColumn = column;
    _sortOrder = order;

    if (order == SortOrder::None) {
        RebuildFilteredIndices();
        emit SortChanged(column, order);
        emit DataChanged();
        update();
        return;
    }

    const auto& customCmp = _columns[static_cast<size_t>(column)].sortComparator;

    std::sort(_rows.begin(), _rows.end(),
        [column, order, &customCmp](const RowData& a, const RowData& b) {
            const QString& va = column < a.cells.size() ? a.cells[column] : QString();
            const QString& vb = column < b.cells.size() ? b.cells[column] : QString();
            const bool less = customCmp ? customCmp(va, vb) : (va < vb);
            return (order == SortOrder::Ascending) ? less : !less;
        });

    _selectedRows.clear();
    RebuildFilteredIndices();
    emit SortChanged(column, order);
    emit DataChanged();
    emit SelectionChanged();
    update();
}

auto NyanDataTable::SortColumn() const -> int
{
    return _sortColumn;
}

auto NyanDataTable::GetSortOrder() const -> SortOrder
{
    return _sortOrder;
}

// ============================================================================
// Filter API
// ============================================================================

void NyanDataTable::SetFilterText(const QString& text)
{
    _filterText = text;
    _filterPredicate = nullptr;
    _filterActive = !text.isEmpty();
    RebuildFilteredIndices();
    _selectedRows.clear();
    UpdateScrollBars();
    emit DataChanged();
    emit SelectionChanged();
    update();
}

auto NyanDataTable::FilterText() const -> QString
{
    return _filterText;
}

void NyanDataTable::SetFilterPredicate(RowFilterPredicate predicate)
{
    _filterPredicate = std::move(predicate);
    _filterText.clear();
    _filterActive = static_cast<bool>(_filterPredicate);
    RebuildFilteredIndices();
    _selectedRows.clear();
    UpdateScrollBars();
    emit DataChanged();
    emit SelectionChanged();
    update();
}

void NyanDataTable::ClearFilter()
{
    _filterText.clear();
    _filterPredicate = nullptr;
    _filterActive = false;
    _filteredIndices.clear();
    _selectedRows.clear();
    UpdateScrollBars();
    emit DataChanged();
    emit SelectionChanged();
    update();
}

auto NyanDataTable::DisplayRowCount() const -> int
{
    return _filterActive ? static_cast<int>(_filteredIndices.size()) : RowCount();
}

auto NyanDataTable::DisplayToDataRow(int displayRow) const -> int
{
    if (!_filterActive) {
        return displayRow;
    }
    if (displayRow < 0 || displayRow >= static_cast<int>(_filteredIndices.size())) {
        return -1;
    }
    return _filteredIndices[static_cast<size_t>(displayRow)];
}

// ============================================================================
// Frozen Column API
// ============================================================================

void NyanDataTable::SetFrozenColumnCount(int count)
{
    _frozenColumnCount = std::clamp(count, 0, ColumnCount());
    UpdateScrollBars();
    update();
}

auto NyanDataTable::FrozenColumnCount() const -> int
{
    return _frozenColumnCount;
}

// ============================================================================
// Appearance API
// ============================================================================

void NyanDataTable::SetAlternatingRowColors(bool enabled)
{
    if (_alternatingRowColors != enabled) {
        _alternatingRowColors = enabled;
        update();
    }
}

auto NyanDataTable::IsAlternatingRowColors() const -> bool
{
    return _alternatingRowColors;
}

void NyanDataTable::SetEditable(bool editable)
{
    _editable = editable;
}

auto NyanDataTable::IsEditable() const -> bool
{
    return _editable;
}

// ============================================================================
// Column Width API
// ============================================================================

void NyanDataTable::SetColumnWidth(int column, int widthPx)
{
    if (column < 0 || column >= ColumnCount()) {
        return;
    }
    auto& col = _columns[static_cast<size_t>(column)];
    col.width = std::max(col.minWidth, widthPx);
    UpdateScrollBars();
    update();
}

auto NyanDataTable::ColumnWidth(int column) const -> int
{
    if (column < 0 || column >= ColumnCount()) {
        return 80;
    }
    return _columns[static_cast<size_t>(column)].width;
}

// ============================================================================
// Row Icon API
// ============================================================================

void NyanDataTable::SetRowIcon(int row, const QIcon& icon)
{
    if (row < 0 || row >= RowCount()) {
        return;
    }
    _rows[static_cast<size_t>(row)].icon = icon;
    update();
}

auto NyanDataTable::RowIcon(int row) const -> QIcon
{
    if (row < 0 || row >= RowCount()) {
        return {};
    }
    return _rows[static_cast<size_t>(row)].icon;
}

// ============================================================================
// Geometry
// ============================================================================

auto NyanDataTable::sizeHint() const -> QSize
{
    return {250, 100};
}

auto NyanDataTable::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanDataTable::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto style    = Theme().Resolve(WidgetKind::DataTable, 0, InteractionState::Normal);
    const auto hovered  = Theme().Resolve(WidgetKind::DataTable, 0, InteractionState::Hovered);
    const auto selected = Theme().Resolve(WidgetKind::DataTable, 0, InteractionState::Selected);
    const auto disabled = Theme().Resolve(WidgetKind::DataTable, 0, InteractionState::Disabled);

    // Background.
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    p.drawRoundedRect(rect(), style.radiusPx, style.radiusPx);

    if (_columns.empty()) {
        p.setFont(style.font);
        p.setPen(disabled.foreground);
        p.drawText(rect(), Qt::AlignCenter, tr("Click to add data"));
        return;
    }

    const int vpH = ViewportHeight();
    const int vpW = ViewportWidth();
    const int frozenW = FrozenColumnsWidth();
    const int frozenCount = _frozenColumnCount;

    // Empty filter result feedback.
    if (_filterActive && DisplayRowCount() == 0) {
        p.save();
        p.setClipRect(QRect(0, 0, vpW, height()));

        // Still draw header background.
        const QRect hdrRect(0, 0, vpW, kHeaderHeight);
        p.setPen(Qt::NoPen);
        p.setBrush(hovered.background);
        const int r = static_cast<int>(style.radiusPx);
        p.drawRoundedRect(hdrRect.adjusted(0, 0, 0, r), r, r);
        p.fillRect(hdrRect.adjusted(0, r, 0, 0), hovered.background);

        // Draw header titles (simplified — all columns, scrollable).
        p.setFont(style.font);
        p.setPen(style.foreground);
        int hx = -_hScrollOffset;
        for (int c = 0; c < ColumnCount(); ++c) {
            const int cw = ColumnWidth(c);
            if (hx + cw > 0 && hx < vpW) {
                const QRect cr(hx + kHPadding, 0, cw - (kHPadding * 2), kHeaderHeight);
                p.drawText(cr, Qt::AlignLeft | Qt::AlignVCenter,
                           _columns[static_cast<size_t>(c)].title);
            }
            hx += cw;
        }

        // "No matching rows" message.
        p.setPen(disabled.foreground);
        const QRect msgRect(0, kHeaderHeight, vpW, vpH);
        p.drawText(msgRect, Qt::AlignCenter, tr("No matching rows"));
        p.restore();
        return;
    }

    // --- Helper: draw sort arrow at (arrowX, arrowY) ---
    auto drawSortArrow = [&](int arrowX, int arrowY) {
        p.save();
        p.setPen(QPen(disabled.foreground, 1.5));
        if (_sortOrder == SortOrder::Ascending) {
            p.drawLine(arrowX, arrowY + 2, arrowX + 4, arrowY - 2);
            p.drawLine(arrowX + 4, arrowY - 2, arrowX + 8, arrowY + 2);
        } else {
            p.drawLine(arrowX, arrowY - 2, arrowX + 4, arrowY + 2);
            p.drawLine(arrowX + 4, arrowY + 2, arrowX + 8, arrowY - 2);
        }
        p.restore();
    };

    // --- Helper: draw one cell (icon + text) ---
    auto drawCell = [&](const RowData& rowData, int col, int cellX, int cellY, int colW) {
        const auto align = _columns[static_cast<size_t>(col)].alignment;
        if (col == 0 && !rowData.icon.isNull()) {
            const int iconX = cellX + kHPadding;
            const int iconY = cellY + ((kRowHeight - kIconSize) / 2);
            rowData.icon.paint(&p, iconX, iconY, kIconSize, kIconSize);
            const QRect textRect(cellX + kHPadding + kIconSize + 4, cellY,
                                 colW - (kHPadding * 2) - kIconSize - 4, kRowHeight);
            if (col < rowData.cells.size()) {
                p.drawText(textRect, align, rowData.cells[col]);
            }
        } else {
            const QRect cellRect(cellX + kHPadding, cellY, colW - (kHPadding * 2), kRowHeight);
            if (col < rowData.cells.size()) {
                p.drawText(cellRect, align, rowData.cells[col]);
            }
        }
    };

    // ================================================================
    // Header background
    // ================================================================
    p.save();
    p.setClipRect(QRect(0, 0, vpW, height()));

    const QRect headerRect(0, 0, vpW, kHeaderHeight);
    p.setPen(Qt::NoPen);
    p.setBrush(hovered.background);
    const int rad = static_cast<int>(style.radiusPx);
    p.drawRoundedRect(headerRect.adjusted(0, 0, 0, rad), rad, rad);
    p.fillRect(headerRect.adjusted(0, rad, 0, 0), hovered.background);

    p.setFont(style.font);
    p.setPen(style.foreground);

    // ================================================================
    // Header: scrollable columns (clipped after frozen area)
    // ================================================================
    p.setClipRect(QRect(frozenW, 0, vpW - frozenW, kHeaderHeight));

    int x = frozenW - _hScrollOffset;
    for (int col = frozenCount; col < ColumnCount(); ++col) {
        const int colW = ColumnWidth(col);
        if (x + colW > frozenW && x < vpW) {
            const QRect cr(x + kHPadding, 0, colW - (kHPadding * 2), kHeaderHeight);
            p.drawText(cr, Qt::AlignLeft | Qt::AlignVCenter,
                       _columns[static_cast<size_t>(col)].title);
            if (col == _sortColumn && _sortOrder != SortOrder::None) {
                drawSortArrow(x + colW - kHPadding - 8,
                              kHeaderHeight / 2);  // NOLINT(bugprone-integer-division)
            }
        }
        x += colW;
    }

    // ================================================================
    // Header: frozen columns (painted on top, no h-scroll)
    // ================================================================
    if (frozenCount > 0) {
        p.setClipRect(QRect(0, 0, frozenW, kHeaderHeight));
        p.setPen(Qt::NoPen);
        p.setBrush(hovered.background);
        p.drawRect(QRect(0, 0, frozenW, kHeaderHeight));

        p.setPen(style.foreground);
        x = 0;
        for (int col = 0; col < frozenCount; ++col) {
            const int colW = ColumnWidth(col);
            const QRect cr(x + kHPadding, 0, colW - (kHPadding * 2), kHeaderHeight);
            p.drawText(cr, Qt::AlignLeft | Qt::AlignVCenter,
                       _columns[static_cast<size_t>(col)].title);
            if (col == _sortColumn && _sortOrder != SortOrder::None) {
                drawSortArrow(x + colW - kHPadding - 8,
                              kHeaderHeight / 2);  // NOLINT(bugprone-integer-division)
            }
            x += colW;
        }
    }

    // ================================================================
    // Rows: visible range computation
    // ================================================================
    const int dispCount = DisplayRowCount();
    const int firstDisp = _vScrollOffset / kRowHeight;
    const int lastDisp  = std::min(dispCount, firstDisp + (vpH / kRowHeight) + 2);
    const int yBase     = kHeaderHeight - (_vScrollOffset % kRowHeight);

    // ================================================================
    // Rows: scrollable columns (clipped after frozen area)
    // ================================================================
    p.setClipRect(QRect(frozenW, kHeaderHeight, vpW - frozenW, vpH));

    for (int di = firstDisp; di < lastDisp; ++di) {
        const int ri = DisplayToDataRow(di);
        if (ri < 0 || ri >= RowCount()) { continue; }
        const auto& rowData = _rows[static_cast<size_t>(ri)];
        const bool isSel = _selectedRows.contains(ri);
        const bool isHov = (di == _hoveredRow && !isSel);
        const bool odd   = _alternatingRowColors && ((di % 2) == 1);
        const int  y     = yBase + ((di - firstDisp) * kRowHeight);

        const QRect rowRect(frozenW, y, vpW - frozenW, kRowHeight);
        if (isSel) {
            p.fillRect(rowRect, selected.background);
        } else if (isHov) {
            p.fillRect(rowRect, hovered.background);
        } else {
            p.fillRect(rowRect, odd ? hovered.background : style.background);
        }
        p.setPen(style.foreground);

        x = frozenW - _hScrollOffset;
        for (int col = frozenCount; col < ColumnCount(); ++col) {
            const int colW = ColumnWidth(col);
            if (x + colW > frozenW && x < vpW) {
                drawCell(rowData, col, x, y, colW);
            }
            x += colW;
        }
    }

    // ================================================================
    // Rows: frozen columns (painted on top, no h-scroll)
    // ================================================================
    if (frozenCount > 0) {
        p.setClipRect(QRect(0, kHeaderHeight, frozenW, vpH));

        for (int di = firstDisp; di < lastDisp; ++di) {
            const int ri = DisplayToDataRow(di);
            if (ri < 0 || ri >= RowCount()) { continue; }
            const auto& rowData = _rows[static_cast<size_t>(ri)];
            const bool isSel = _selectedRows.contains(ri);
            const bool isHov = (di == _hoveredRow && !isSel);
            const bool odd   = _alternatingRowColors && ((di % 2) == 1);
            const int  y     = yBase + ((di - firstDisp) * kRowHeight);

            const QRect rowRect(0, y, frozenW, kRowHeight);
            if (isSel) {
                p.fillRect(rowRect, selected.background);
            } else if (isHov) {
                p.fillRect(rowRect, hovered.background);
            } else {
                p.fillRect(rowRect, odd ? hovered.background : style.background);
            }
            p.setPen(style.foreground);

            x = 0;
            for (int col = 0; col < frozenCount; ++col) {
                const int colW = ColumnWidth(col);
                drawCell(rowData, col, x, y, colW);
                x += colW;
            }
        }
    }

    p.restore();
}

// ============================================================================
// Mouse Events
// ============================================================================

void NyanDataTable::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint pos = event->pos();

    // Empty state click.
    if (_columns.empty()) {
        emit EmptyClicked();
        return;
    }

    // Header area interactions.
    if (pos.y() < kHeaderHeight) {
        // Check for column resize.
        const int resizeCol = HitTestColumnEdge(pos);
        if (resizeCol >= 0) {
            const auto& colDef = _columns[static_cast<size_t>(resizeCol)];
            if (colDef.resizable) {
                _resizingColumn = resizeCol;
                _resizeStartX = pos.x();
                _resizeStartWidth = colDef.width;
                setCursor(Qt::SplitHCursor);
                return;
            }
        }

        // Header click -> sort.
        const int col = HitTestHeader(pos);
        if (col >= 0 && _columns[static_cast<size_t>(col)].sortable) {
            SortOrder newOrder = SortOrder::Ascending;
            if (_sortColumn == col) {
                if (_sortOrder == SortOrder::Ascending) {
                    newOrder = SortOrder::Descending;
                } else if (_sortOrder == SortOrder::Descending) {
                    newOrder = SortOrder::None;
                }
            }
            SortByColumn(col, newOrder);
        }
        return;
    }

    // Row click -> selection.
    const int row = HitTestRow(pos);
    if (row >= 0) {
        const bool ctrl = event->modifiers() & Qt::ControlModifier;
        const bool shift = event->modifiers() & Qt::ShiftModifier;

        if (_selectionMode == SelectionMode::SingleRow || (!ctrl && !shift)) {
            _selectedRows.clear();
            _selectedRows.insert(row);
        } else if (ctrl) {
            if (_selectedRows.contains(row)) {
                _selectedRows.remove(row);
            } else {
                _selectedRows.insert(row);
            }
        } else if (shift && !_selectedRows.isEmpty()) {
            int anchor = *_selectedRows.begin();
            _selectedRows.clear();
            const int minR = std::min(anchor, row);
            const int maxR = std::max(anchor, row);
            for (int r = minR; r <= maxR; ++r) {
                _selectedRows.insert(r);
            }
        }
        emit SelectionChanged();
        update();
    }
}

void NyanDataTable::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->pos();

    if (_resizingColumn >= 0) {
        const int delta = pos.x() - _resizeStartX;
        auto& colDef = _columns[static_cast<size_t>(_resizingColumn)];
        colDef.width = std::max(colDef.minWidth, _resizeStartWidth + delta);
        UpdateScrollBars();
        update();
        return;
    }

    // Update cursor for resize zone.
    if (pos.y() < kHeaderHeight && HitTestColumnEdge(pos) >= 0) {
        setCursor(Qt::SplitHCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }

    // Track hovered row.
    const int newHover = HitTestRow(pos);
    if (newHover != _hoveredRow) {
        _hoveredRow = newHover;
        update();
    }
}

void NyanDataTable::mouseReleaseEvent(QMouseEvent* event)
{
    if (_resizingColumn >= 0) {
        _resizingColumn = -1;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void NyanDataTable::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (!_editable) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    const QPoint pos = event->pos();
    const int row = HitTestRow(pos);
    const int col = HitTestHeader(QPoint(pos.x(), 0));
    if (row < 0 || col < 0 || col >= ColumnCount()) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    // Per-column editable check.
    if (!_columns[static_cast<size_t>(col)].editable) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    BeginCellEdit(row, col);
}

void NyanDataTable::wheelEvent(QWheelEvent* event)
{
    const int delta = event->angleDelta().y();
    if (delta == 0) {
        QWidget::wheelEvent(event);
        return;
    }

    const int maxV = std::max(0, ContentHeight() - ViewportHeight());
    if (maxV <= 0) {
        QWidget::wheelEvent(event);
        return;
    }

    // 3 rows per wheel notch (120 delta units).
    const int scroll = -(delta * kRowHeight * 3) / 120;
    _vScrollOffset = std::clamp(_vScrollOffset + scroll, 0, maxV);
    _vScrollBar->setValue(_vScrollOffset);
    update();
    event->accept();
}

void NyanDataTable::keyPressEvent(QKeyEvent* event)
{
    const int dispCount = DisplayRowCount();
    if (dispCount == 0) {
        QWidget::keyPressEvent(event);
        return;
    }

    auto selectDisplayRow = [&](int dispRow) {
        const int dataRow = DisplayToDataRow(dispRow);
        if (dataRow < 0) { return; }
        _focusRow = dataRow;
        _selectedRows.clear();
        _selectedRows.insert(dataRow);
        EnsureRowVisible(dataRow);
        emit SelectionChanged();
        update();
    };

    switch (event->key()) {
    case Qt::Key_Up: {
        int di = DataToDisplayRow(_focusRow);
        if (di > 0) {
            --di;
        } else if (di < 0) {
            di = 0;
        }
        selectDisplayRow(di);
        break;
    }

    case Qt::Key_Down: {
        int di = DataToDisplayRow(_focusRow);
        if (di >= 0 && di < dispCount - 1) {
            ++di;
        } else if (di < 0) {
            di = 0;
        }
        selectDisplayRow(di);
        break;
    }

    case Qt::Key_Home:
        selectDisplayRow(0);
        break;

    case Qt::Key_End:
        selectDisplayRow(dispCount - 1);
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (_editable && _focusRow >= 0) {
            const int editCol = (_focusCol >= 0) ? _focusCol : 0;
            if (editCol < ColumnCount() && _columns[static_cast<size_t>(editCol)].editable) {
                BeginCellEdit(_focusRow, editCol);
            }
        }
        break;

    case Qt::Key_Tab: {
        if (_focusRow >= 0) {
            if (_focusCol < ColumnCount() - 1) {
                ++_focusCol;
            } else {
                _focusCol = 0;
                int di = DataToDisplayRow(_focusRow);
                if (di >= 0 && di < dispCount - 1) {
                    selectDisplayRow(di + 1);
                }
            }
            update();
        }
        break;
    }

    default:
        QWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}

void NyanDataTable::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    UpdateScrollBars();
    LayoutScrollBars();
}

void NyanDataTable::OnThemeChanged()
{
    update();
}

// ============================================================================
// Scroll API
// ============================================================================

void NyanDataTable::ScrollToRow(int row)
{
    if (row < 0 || row >= RowCount()) {
        return;
    }
    const int dispRow = DataToDisplayRow(row);
    if (dispRow < 0) { return; }
    _vScrollOffset = dispRow * kRowHeight;
    const int maxV = std::max(0, ContentHeight() - ViewportHeight());
    _vScrollOffset = std::clamp(_vScrollOffset, 0, maxV);
    _vScrollBar->setValue(_vScrollOffset);
    update();
}

void NyanDataTable::EnsureRowVisible(int row)
{
    if (row < 0 || row >= RowCount()) {
        return;
    }
    const int dispRow = DataToDisplayRow(row);
    if (dispRow < 0) { return; }
    const int vpH = ViewportHeight();
    const int rowTop = dispRow * kRowHeight;
    const int rowBot = rowTop + kRowHeight;

    if (rowTop < _vScrollOffset) {
        _vScrollOffset = rowTop;
    } else if (rowBot > _vScrollOffset + vpH) {
        _vScrollOffset = rowBot - vpH;
    } else {
        return;
    }

    const int maxV = std::max(0, ContentHeight() - vpH);
    _vScrollOffset = std::clamp(_vScrollOffset, 0, maxV);
    _vScrollBar->setValue(_vScrollOffset);
    update();
}

auto NyanDataTable::FirstVisibleRow() const -> int
{
    return _vScrollOffset / kRowHeight;
}

auto NyanDataTable::VisibleRowCount() const -> int
{
    return ViewportHeight() / kRowHeight;
}

auto NyanDataTable::HoveredRow() const -> int
{
    return _hoveredRow;
}

// ============================================================================
// Private Helpers
// ============================================================================

void NyanDataTable::UpdateColumnWidths()
{
    if (_columns.empty()) {
        return;
    }
    const int defaultWidth = std::max(80, (width() - 20) / ColumnCount());
    for (auto& col : _columns) {
        col.width = defaultWidth;
    }
    UpdateScrollBars();
}

void NyanDataTable::RebuildFilteredIndices()
{
    _filteredIndices.clear();
    if (!_filterActive) {
        return;
    }

    _filteredIndices.reserve(static_cast<size_t>(RowCount()));
    for (int i = 0; i < RowCount(); ++i) {
        const auto& cells = _rows[static_cast<size_t>(i)].cells;
        bool match = false;
        if (_filterPredicate) {
            match = _filterPredicate(i, cells);
        } else if (!_filterText.isEmpty()) {
            for (const auto& cell : cells) {
                if (cell.contains(_filterText, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        if (match) {
            _filteredIndices.push_back(i);
        }
    }
}

void NyanDataTable::UpdateScrollBars()
{
    const int cH = ContentHeight();
    const int vpH = ViewportHeight();
    const bool needV = cH > vpH;

    _vScrollBar->setVisible(needV);
    if (needV) {
        _vScrollBar->setRange(0, cH - vpH);
        _vScrollBar->setPageStep(vpH);
        _vScrollBar->setSingleStep(kRowHeight);
    } else {
        _vScrollOffset = 0;
        _vScrollBar->setValue(0);
    }

    const int scrollableW = TotalColumnWidth() - FrozenColumnsWidth();
    const int scrollableVpW = ViewportWidth() - FrozenColumnsWidth();
    const bool needH = scrollableW > scrollableVpW;

    _hScrollBar->setVisible(needH);
    if (needH) {
        _hScrollBar->setRange(0, scrollableW - scrollableVpW);
        _hScrollBar->setPageStep(scrollableVpW);
        _hScrollBar->setSingleStep(40);
    } else {
        _hScrollOffset = 0;
        _hScrollBar->setValue(0);
    }

    LayoutScrollBars();
}

void NyanDataTable::LayoutScrollBars()
{
    const bool hasV = _vScrollBar->isVisible();
    const bool hasH = _hScrollBar->isVisible();
    const int w = width();
    const int h = height();

    if (hasV) {
        _vScrollBar->setGeometry(w - kScrollBarWidth, 0, kScrollBarWidth,
                                 hasH ? h - kScrollBarWidth : h);
    }
    if (hasH) {
        _hScrollBar->setGeometry(0, h - kScrollBarWidth,
                                 hasV ? w - kScrollBarWidth : w, kScrollBarWidth);
    }
}

auto NyanDataTable::ContentHeight() const -> int
{
    return DisplayRowCount() * kRowHeight;
}

auto NyanDataTable::ViewportHeight() const -> int
{
    const bool hasH = TotalColumnWidth() > width();
    return height() - kHeaderHeight - (hasH ? kScrollBarWidth : 0);
}

auto NyanDataTable::TotalColumnWidth() const -> int
{
    int total = 0;
    for (int col = 0; col < ColumnCount(); ++col) {
        total += ColumnWidth(col);
    }
    return total;
}

auto NyanDataTable::ViewportWidth() const -> int
{
    const bool hasV = ContentHeight() > (height() - kHeaderHeight);
    return width() - (hasV ? kScrollBarWidth : 0);
}

auto NyanDataTable::FrozenColumnsWidth() const -> int
{
    int total = 0;
    for (int col = 0; col < _frozenColumnCount && col < ColumnCount(); ++col) {
        total += ColumnWidth(col);
    }
    return total;
}

auto NyanDataTable::DataToDisplayRow(int dataRow) const -> int
{
    if (!_filterActive) { return dataRow; }
    for (int d = 0; d < static_cast<int>(_filteredIndices.size()); ++d) {
        if (_filteredIndices[static_cast<size_t>(d)] == dataRow) { return d; }
    }
    return -1;
}

void NyanDataTable::BeginCellEdit(int row, int col)
{
    auto* editor = new QLineEdit(this);
    int cellX = 0;
    if (col < _frozenColumnCount) {
        for (int c = 0; c < col; ++c) {
            cellX += ColumnWidth(c);
        }
    } else {
        cellX = FrozenColumnsWidth() - _hScrollOffset;
        for (int c = _frozenColumnCount; c < col; ++c) {
            cellX += ColumnWidth(c);
        }
    }
    const int colW = ColumnWidth(col);
    const int dispRow = DataToDisplayRow(row);
    if (dispRow < 0) { delete editor; return; }
    const int cellY = kHeaderHeight + (dispRow * kRowHeight) - _vScrollOffset;
    editor->setGeometry(cellX, cellY, colW, kRowHeight);
    editor->setText(Cell(row, col));
    editor->selectAll();
    editor->setFocus();
    editor->show();

    QObject::connect(editor, &QLineEdit::editingFinished, this,
        [this, editor, row, col]() {
            SetCell(row, col, editor->text());
            editor->deleteLater();
        });
}

auto NyanDataTable::HitTestHeader(const QPoint& pos) const -> int
{
    if (pos.y() >= kHeaderHeight) {
        return -1;
    }
    const int px = pos.x();
    const int frozenW = FrozenColumnsWidth();

    // Check frozen columns first (no h-scroll offset).
    int x = 0;
    for (int col = 0; col < _frozenColumnCount && col < ColumnCount(); ++col) {
        const int colW = ColumnWidth(col);
        if (px >= x && px < x + colW) {
            return col;
        }
        x += colW;
    }

    // Check scrollable columns (apply h-scroll offset).
    x = frozenW - _hScrollOffset;
    for (int col = _frozenColumnCount; col < ColumnCount(); ++col) {
        const int colW = ColumnWidth(col);
        if (px >= x && px < x + colW && px >= frozenW) {
            return col;
        }
        x += colW;
    }
    return -1;
}

auto NyanDataTable::HitTestRow(const QPoint& pos) const -> int
{
    if (pos.y() < kHeaderHeight) {
        return -1;
    }
    const int displayRow = (pos.y() - kHeaderHeight + _vScrollOffset) / kRowHeight;
    if (displayRow >= 0 && displayRow < DisplayRowCount()) {
        return DisplayToDataRow(displayRow);
    }
    return -1;
}

auto NyanDataTable::HitTestColumnEdge(const QPoint& pos) const -> int
{
    const int px = pos.x();
    const int frozenW = FrozenColumnsWidth();

    // Check frozen column edges (no h-scroll offset).
    int x = 0;
    for (int col = 0; col < _frozenColumnCount && col < ColumnCount(); ++col) {
        x += ColumnWidth(col);
        if (std::abs(px - x) <= kResizeZone) {
            return col;
        }
    }

    // Check scrollable column edges.
    x = frozenW - _hScrollOffset;
    for (int col = _frozenColumnCount; col < ColumnCount(); ++col) {
        x += ColumnWidth(col);
        if (x > frozenW && std::abs(px - x) <= kResizeZone) {
            return col;
        }
    }
    return -1;
}

} // namespace matcha::gui