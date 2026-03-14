/**
 * @file NyanListWidget.cpp
 * @brief Implementation of NyanListWidget themed list widget.
 */

#include <Matcha/Widgets/Controls/NyanListWidget.h>

#include "../Core/SimpleWidgetEventFilter.h"

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanListWidget::NyanListWidget(QWidget* parent)
    : QListWidget(parent)
    , ThemeAware(WidgetKind::ListWidget)
{
    setAlternatingRowColors(true);
    ApplyStyle();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanListWidget::~NyanListWidget() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanListWidget::SetDragReorderEnabled(bool enabled)
{
    _dragReorderEnabled = enabled;
    if (enabled) {
        setDragDropMode(QAbstractItemView::InternalMove);
        setDefaultDropAction(Qt::MoveAction);
        setDragEnabled(true);
    } else {
        setDragDropMode(QAbstractItemView::NoDragDrop);
        setDragEnabled(false);
    }
}

auto NyanListWidget::IsDragReorderEnabled() const -> bool
{
    return _dragReorderEnabled;
}

// ============================================================================
// Theme
// ============================================================================

void NyanListWidget::OnThemeChanged()
{
    ApplyStyle();
}

void NyanListWidget::ApplyStyle()
{
    const auto style    = Theme().Resolve(WidgetKind::ListWidget, 0, InteractionState::Normal);
    const auto hovered  = Theme().Resolve(WidgetKind::ListWidget, 0, InteractionState::Hovered);
    const auto selected = Theme().Resolve(WidgetKind::ListWidget, 0, InteractionState::Selected);

    setFont(style.font);

    const QString css = QStringLiteral(
        "QListWidget { background: %1; border: none; border-radius: %6px; }"
        "QListWidget::item { color: %2; padding: 4px 8px; }"
        "QListWidget::item:alternate { background: %3; }"
        "QListWidget::item:hover { background: %4; }"
        "QListWidget::item:selected { background: %5; color: %2; }"
    ).arg(style.background.name(), style.foreground.name(),
          hovered.background.name(), hovered.background.name(),
          selected.background.name(),
          QString::number(static_cast<int>(style.radiusPx)));

    setStyleSheet(css);
}

} // namespace matcha::gui