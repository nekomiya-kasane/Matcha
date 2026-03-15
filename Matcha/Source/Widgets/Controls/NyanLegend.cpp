/**
 * @file NyanLegend.cpp
 * @brief Implementation of NyanLegend themed color-label-value legend widget.
 */

#include <Matcha/Widgets/Controls/NyanLegend.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanLegend::NyanLegend(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Legend)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanLegend::~NyanLegend() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanLegend::SetItems(const std::vector<LegendItem>& items)
{
    _items = items;
    updateGeometry();
    update();
}

auto NyanLegend::Items() const -> const std::vector<LegendItem>& { return _items; }

auto NyanLegend::AddItem(const LegendItem& item) -> int
{
    _items.push_back(item);
    updateGeometry();
    update();
    return static_cast<int>(_items.size()) - 1;
}

auto NyanLegend::InsertItem(int index, const LegendItem& item) -> int
{
    const int clamped = std::clamp(index, 0, static_cast<int>(_items.size()));
    _items.insert(_items.begin() + clamped, item);
    updateGeometry();
    update();
    return clamped;
}

void NyanLegend::SetItem(int index, const LegendItem& item)
{
    if (index >= 0 && index < static_cast<int>(_items.size())) {
        _items[static_cast<size_t>(index)] = item;
        update();
    }
}

void NyanLegend::RemoveItem(int index)
{
    if (index >= 0 && index < static_cast<int>(_items.size())) {
        _items.erase(_items.begin() + index);
        updateGeometry();
        update();
    }
}

void NyanLegend::ClearItems()
{
    _items.clear();
    updateGeometry();
    update();
}

auto NyanLegend::ItemCount() const -> int
{
    return static_cast<int>(_items.size());
}

void NyanLegend::SetUseDefault(bool use)
{
    if (_useDefault == use) {
        return;
    }
    _useDefault = use;
    updateGeometry();
    update();
}

auto NyanLegend::UseDefault() const -> bool { return _useDefault; }

void NyanLegend::SetSelectable(bool selectable)
{
    _selectable = selectable;
}

auto NyanLegend::IsSelectable() const -> bool { return _selectable; }

auto NyanLegend::sizeHint() const -> QSize
{
    int rows = static_cast<int>(_items.size());
    if (_useDefault) {
        ++rows;
    }
    return {160, std::max(rows * kItemHeight, kItemHeight)};
}

auto NyanLegend::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Hit testing
// ============================================================================

auto NyanLegend::ItemAtY(int y) const -> int
{
    if (y < 0) {
        return -2; // invalid
    }
    const int idx = y / kItemHeight;
    if (idx < static_cast<int>(_items.size())) {
        return idx;
    }
    // Default row (if enabled) is after all items, returns -1
    if (_useDefault && idx == static_cast<int>(_items.size())) {
        return -1;
    }
    return -2; // out of range
}

// ============================================================================
// Painting
// ============================================================================

void NyanLegend::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto style = Theme().Resolve(WidgetKind::Legend, 0, InteractionState::Normal);
    p.setFont(style.font);

    for (int i = 0; i < static_cast<int>(_items.size()); ++i) {
        const auto& item = _items[static_cast<size_t>(i)];
        const int y = i * kItemHeight;

        // -- Color swatch --
        const QRect swatchRect(kHPadding, y + ((kItemHeight - kSwatchSize) / 2),
                               kSwatchSize, kSwatchSize);
        p.setPen(QPen(style.border, 1));
        p.setBrush(item.color.isValid() ? QBrush(item.color) : QBrush(style.background));
        p.drawRect(swatchRect);

        // -- Text: name flag value --
        const int textX = kHPadding + kSwatchSize + kGap;
        const QRect textRect(textX, y, width() - textX - kHPadding, kItemHeight);
        p.setPen(style.foreground);
        const QString text = item.name + QStringLiteral(" ") + item.flag + item.value;
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }

    // -- Default legend row --
    if (_useDefault) {
        const auto disabledStyle = Theme().Resolve(WidgetKind::Legend, 0, InteractionState::Disabled);
        const int y = static_cast<int>(_items.size()) * kItemHeight;
        const QRect swatchRect(kHPadding, y + ((kItemHeight - kSwatchSize) / 2),
                               kSwatchSize, kSwatchSize);
        p.setPen(QPen(style.border, 1));
        p.setBrush(style.background);
        p.drawRect(swatchRect);

        const int textX = kHPadding + kSwatchSize + kGap;
        const QRect textRect(textX, y, width() - textX - kHPadding, kItemHeight);
        p.setPen(disabledStyle.foreground);
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("Default"));
    }
}

void NyanLegend::mouseReleaseEvent(QMouseEvent* event)
{
    if (!_selectable || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    const int idx = ItemAtY(event->position().toPoint().y());
    if (idx >= -1) { // -1 = default row, 0..N-1 = items, -2 = invalid
        emit ItemClicked(idx);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void NyanLegend::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui