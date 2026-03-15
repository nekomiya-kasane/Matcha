/**
 * @file NyanTableWidget.cpp
 * @brief Implementation of NyanTableWidget themed table widget.
 */

#include <Matcha/Widgets/Controls/NyanTableWidget.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QHeaderView>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanTableWidget::NyanTableWidget(QWidget* parent)
    : QTableWidget(parent)
    , ThemeAware(WidgetKind::TableWidget)
{
    setAlternatingRowColors(true);
    ApplyStyle();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanTableWidget::NyanTableWidget(int rows, int columns, QWidget* parent)
    : QTableWidget(rows, columns, parent)
    , ThemeAware(WidgetKind::TableWidget)
{
    setAlternatingRowColors(true);
    ApplyStyle();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanTableWidget::~NyanTableWidget() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanTableWidget::SetCheckableHeaders(bool enabled)
{
    if (_checkableHeaders == enabled) {
        return;
    }
    _checkableHeaders = enabled;
    SetupCheckableHeaders();
}

auto NyanTableWidget::HasCheckableHeaders() const -> bool
{
    return _checkableHeaders;
}

auto NyanTableWidget::CheckedRows() const -> QList<int>
{
    QList<int> result;
    if (!_checkableHeaders) {
        return result;
    }
    for (int row = 0; row < rowCount(); ++row) {
        auto* item = verticalHeaderItem(row);
        if (item != nullptr && item->checkState() == Qt::Checked) {
            result.append(row);
        }
    }
    return result;
}

void NyanTableWidget::SetRowChecked(int row, bool checked)
{
    if (!_checkableHeaders || row < 0 || row >= rowCount()) {
        return;
    }
    auto* item = verticalHeaderItem(row);
    if (item == nullptr) {
        item = new QTableWidgetItem();
        setVerticalHeaderItem(row, item);
    }
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

// ============================================================================
// Theme
// ============================================================================

void NyanTableWidget::OnThemeChanged()
{
    ApplyStyle();
}

void NyanTableWidget::ApplyStyle()
{
    const auto normal   = Theme().Resolve(WidgetKind::TableWidget, 0, InteractionState::Normal);
    const auto hovered  = Theme().Resolve(WidgetKind::TableWidget, 0, InteractionState::Hovered);
    const auto selected = Theme().Resolve(WidgetKind::TableWidget, 0, InteractionState::Selected);

    setFont(normal.font);

    const QString css = QStringLiteral(
        "QTableWidget { background: %1; border: 1px solid %7; border-radius: %6px; gridline-color: %7; }"
        "QTableWidget::item { color: %2; padding: 4px; }"
        "QTableWidget::item:alternate { background: %3; }"
        "QTableWidget::item:selected { background: %5; color: %2; }"
        "QHeaderView::section { background: %4; color: %2; padding: 4px; border: none; border-bottom: 1px solid %7; }"
    ).arg(normal.background.name(), normal.foreground.name(),
          hovered.background.name(), selected.background.name(),
          selected.foreground.name(),
          QString::number(static_cast<int>(normal.radiusPx)),
          normal.border.name());

    setStyleSheet(css);
}

void NyanTableWidget::SetupCheckableHeaders()
{
    if (_checkableHeaders) {
        for (int row = 0; row < rowCount(); ++row) {
            auto* item = verticalHeaderItem(row);
            if (item == nullptr) {
                item = new QTableWidgetItem(QString::number(row + 1));
                setVerticalHeaderItem(row, item);
            }
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
        }
        verticalHeader()->setSectionsClickable(true);
        connect(verticalHeader(), &QHeaderView::sectionClicked, this, [this](int row) {
            auto* item = verticalHeaderItem(row);
            if (item != nullptr) {
                const bool newState = item->checkState() != Qt::Checked;
                item->setCheckState(newState ? Qt::Checked : Qt::Unchecked);
                emit RowCheckedChanged(row, newState);
            }
        });
    } else {
        for (int row = 0; row < rowCount(); ++row) {
            auto* item = verticalHeaderItem(row);
            if (item != nullptr) {
                item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
            }
        }
    }
}

} // namespace matcha::gui