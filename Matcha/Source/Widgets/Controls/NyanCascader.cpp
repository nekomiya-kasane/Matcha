/**
 * @file NyanCascader.cpp
 * @brief Implementation of NyanCascader multi-level cascading selector.
 */

#include <Matcha/Widgets/Controls/NyanCascader.h>

#include "../Core/InteractionEventFilter.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QPainter>

namespace matcha::gui {

NyanCascader::NyanCascader(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Cascader)
{
    SetupUi();
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanCascader::~NyanCascader() = default;

void NyanCascader::SetData(std::vector<CascaderItem> items)
{
    _rootItems = std::move(items);
    _selectedPath.clear();
    RebuildColumns();
}

void NyanCascader::SetPlaceholder(std::string_view text)
{
    _placeholder.assign(text);
}

auto NyanCascader::Value() const -> std::vector<std::string>
{
    return _selectedPath;
}

void NyanCascader::ClearSelection()
{
    _selectedPath.clear();
    RebuildColumns();
}

auto NyanCascader::sizeHint() const -> QSize { return {400, 200}; }
auto NyanCascader::minimumSizeHint() const -> QSize { return {200, 100}; }

void NyanCascader::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    const auto style = Theme().Resolve(WidgetKind::Cascader, 0, InteractionState::Normal);
    QPainter p(this);
    p.fillRect(rect(), style.background);
}

void NyanCascader::OnThemeChanged() { update(); }

void NyanCascader::SetupUi()
{
    _columnsLayout = new QHBoxLayout(this);
    _columnsLayout->setContentsMargins(0, 0, 0, 0);
    _columnsLayout->setSpacing(1);
    RebuildColumns();
}

void NyanCascader::RebuildColumns()
{
    // Clear existing columns
    for (auto* col : _columns) {
        _columnsLayout->removeWidget(col);
        delete col;
    }
    _columns.clear();

    // Build first column from root items
    if (_rootItems.empty()) return;

    auto* list = new QListWidget(this);
    for (const auto& item : _rootItems) {
        list->addItem(QString::fromStdString(item.label));
    }
    _columnsLayout->addWidget(list);
    _columns.push_back(list);

    int colIdx = 0;
    connect(list, &QListWidget::currentRowChanged,
            this, [this, colIdx](int row) { OnColumnItemSelected(colIdx, row); });
}

void NyanCascader::OnColumnItemSelected(int columnIndex, int row)
{
    if (row < 0) return;

    // Trim selection path and columns beyond this level
    _selectedPath.resize(static_cast<size_t>(columnIndex));
    while (static_cast<int>(_columns.size()) > columnIndex + 1) {
        auto* col = _columns.back();
        _columnsLayout->removeWidget(col);
        delete col;
        _columns.pop_back();
    }

    // Navigate to the selected item's children
    const std::vector<CascaderItem>* items = &_rootItems;
    for (int i = 0; i < columnIndex; ++i) {
        auto idx = _columns[static_cast<size_t>(i)]->currentRow();
        if (idx < 0 || idx >= static_cast<int>(items->size())) return;
        items = &(*items)[static_cast<size_t>(idx)].children;
    }

    if (row >= static_cast<int>(items->size())) return;
    const auto& selected = (*items)[static_cast<size_t>(row)];
    _selectedPath.push_back(selected.label);

    if (!selected.children.empty()) {
        auto* list = new QListWidget(this);
        for (const auto& child : selected.children) {
            list->addItem(QString::fromStdString(child.label));
        }
        _columnsLayout->addWidget(list);
        _columns.push_back(list);

        int nextCol = columnIndex + 1;
        connect(list, &QListWidget::currentRowChanged,
                this, [this, nextCol](int r) { OnColumnItemSelected(nextCol, r); });
    }

    Q_EMIT SelectionChanged(_selectedPath);
}

} // namespace matcha::gui