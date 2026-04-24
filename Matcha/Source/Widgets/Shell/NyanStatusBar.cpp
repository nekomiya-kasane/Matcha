#include <Matcha/Widgets/Shell/NyanStatusBar.h>

#include <QHBoxLayout>
#include <QPainter>

#include <algorithm>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanStatusBar::NyanStatusBar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::StatusBar)
{
    setFixedHeight(kHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(kHPadding, 0, kHPadding, 0);
    _layout->setSpacing(kHPadding);

    // Central stretch separates left items from right items
    _stretchIndex = 0;
    _layout->addStretch(1);
}

NyanStatusBar::~NyanStatusBar() = default;

// ============================================================================
// Item management
// ============================================================================

auto NyanStatusBar::AddItem(const QString& id, QWidget* widget,
                            StatusBarSide side) -> QWidget*
{
    if (widget == nullptr) { return nullptr; }

    // Reject duplicate id
    auto it = std::ranges::find_if(_items, [&](const ItemEntry& e) {
        return e.id == id;
    });
    if (it != _items.end()) { return nullptr; }

    // Reparent and clamp height
    widget->setParent(this);
    widget->setFixedHeight(kHeight);

    _items.push_back({id, widget, side});
    RebuildLayout();

    return widget;
}

auto NyanStatusBar::RemoveItem(const QString& id) -> bool
{
    auto it = std::ranges::find_if(_items, [&](const ItemEntry& e) {
        return e.id == id;
    });
    if (it == _items.end()) { return false; }

    QWidget* w = it->widget;
    _items.erase(it);

    _layout->removeWidget(w);
    w->hide();
    w->deleteLater();

    RebuildLayout();
    return true;
}

auto NyanStatusBar::FindItem(const QString& id) const -> QWidget*
{
    auto it = std::ranges::find_if(_items, [&](const ItemEntry& e) {
        return e.id == id;
    });
    return (it != _items.end()) ? it->widget : nullptr;
}

auto NyanStatusBar::ItemCount() const -> int
{
    return static_cast<int>(_items.size());
}

// ============================================================================
// Size hints
// ============================================================================

auto NyanStatusBar::sizeHint() const -> QSize
{
    return {400, kHeight};
}

auto NyanStatusBar::minimumSizeHint() const -> QSize
{
    return {200, kHeight};
}

// ============================================================================
// Paint
// ============================================================================

void NyanStatusBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    const auto& theme = Theme();

    p.fillRect(rect(), theme.Color(ColorToken::colorPrimary));

    p.setPen(theme.Color(ColorToken::colorBorder));
    p.drawLine(0, 0, width(), 0);
}

void NyanStatusBar::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private
// ============================================================================

void NyanStatusBar::RebuildLayout()
{
    // Remove all widgets and spacers from layout (without deleting widgets)
    while (_layout->count() > 0) {
        auto* item = _layout->takeAt(0);
        // Don't delete widgets -- only delete spacer items
        if (item->widget() == nullptr) {
            delete item;
        }
    }

    // Left items
    for (const auto& entry : _items) {
        if (entry.side == StatusBarSide::Left) {
            _layout->addWidget(entry.widget);
        }
    }

    // Central stretch
    _stretchIndex = _layout->count();
    _layout->addStretch(1);

    // Right items
    for (const auto& entry : _items) {
        if (entry.side == StatusBarSide::Right) {
            _layout->addWidget(entry.widget);
        }
    }
}

} // namespace matcha::gui
