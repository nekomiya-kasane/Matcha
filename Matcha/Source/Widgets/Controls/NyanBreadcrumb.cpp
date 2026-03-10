/**
 * @file NyanBreadcrumb.cpp
 * @brief Implementation of NyanBreadcrumb themed path navigation widget.
 */

#include <Matcha/Widgets/Controls/NyanBreadcrumb.h>

#include "../Core/InteractionEventFilter.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanBreadcrumb::NyanBreadcrumb(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Label)
{
    setFixedHeight(kFixedHeight);
    setMouseTracking(true);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanBreadcrumb::~NyanBreadcrumb() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanBreadcrumb::SetItems(const std::vector<QString>& items)
{
    _items = items;
    RebuildLayout();
    updateGeometry();
    update();
}

auto NyanBreadcrumb::Items() const -> const std::vector<QString>&
{
    return _items;
}

void NyanBreadcrumb::SetSeparator(const QString& separator)
{
    _separator = separator;
    RebuildLayout();
    updateGeometry();
    update();
}

auto NyanBreadcrumb::Separator() const -> QString
{
    return _separator;
}

auto NyanBreadcrumb::sizeHint() const -> QSize
{
    const auto& fontSpec = Theme().Font(StyleSheet().font);
    QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
    QFontMetrics fm(f);

    int totalW = kHPadding * 2;
    for (size_t i = 0; i < _items.size(); ++i) {
        totalW += fm.horizontalAdvance(_items[i]);
        if (i + 1 < _items.size()) {
            totalW += fm.horizontalAdvance(_separator);
        }
    }
    return {totalW, kFixedHeight};
}

// ============================================================================
// Layout
// ============================================================================

void NyanBreadcrumb::RebuildLayout()
{
    _layout.clear();
    if (_items.empty()) {
        return;
    }

    const auto& fontSpec = Theme().Font(StyleSheet().font);
    QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
    QFontMetrics fm(f);

    const int sepW = fm.horizontalAdvance(_separator);
    const int ellipsisW = fm.horizontalAdvance(QStringLiteral("..."));
    const int availW = width() - (kHPadding * 2);

    // Calculate total width
    int totalW = 0;
    for (size_t i = 0; i < _items.size(); ++i) {
        totalW += fm.horizontalAdvance(_items[i]);
        if (i + 1 < _items.size()) {
            totalW += sepW;
        }
    }

    // Determine first visible index (overflow from left)
    int firstVisible = 0;
    if (totalW > availW && _items.size() > 1) {
        int remaining = totalW;
        while (firstVisible < static_cast<int>(_items.size()) - 1 && remaining > availW - ellipsisW - sepW) {
            remaining -= fm.horizontalAdvance(_items[static_cast<size_t>(firstVisible)]);
            remaining -= sepW;
            ++firstVisible;
        }
    }

    // Build layout entries
    int x = kHPadding;

    // Ellipsis prefix if overflow
    if (firstVisible > 0) {
        x += ellipsisW + sepW;
    }

    for (int i = firstVisible; i < static_cast<int>(_items.size()); ++i) {
        const int segW = fm.horizontalAdvance(_items[static_cast<size_t>(i)]);
        _layout.push_back({x, segW, i});
        x += segW;
        if (i + 1 < static_cast<int>(_items.size())) {
            x += sepW;
        }
    }
}

// ============================================================================
// Painting
// ============================================================================

void NyanBreadcrumb::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto style = Theme().Resolve(WidgetKind::Label, 0, InteractionState::Normal);
    const auto hoverStyle = Theme().Resolve(WidgetKind::Label, 0, InteractionState::Hovered);
    p.setFont(style.font);

    const QFontMetrics fm(style.font);
    const int sepW = fm.horizontalAdvance(_separator);

    // Overflow ellipsis
    if (!_layout.empty() && _layout.front().index > 0) {
        p.setPen(style.border);
        p.drawText(kHPadding, 0, fm.horizontalAdvance(QStringLiteral("...")), height(),
                   Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("..."));
        const int afterEllipsis = kHPadding + fm.horizontalAdvance(QStringLiteral("..."));
        p.drawText(afterEllipsis, 0, sepW, height(),
                   Qt::AlignLeft | Qt::AlignVCenter, _separator);
    }

    // Segments + separators
    for (size_t i = 0; i < _layout.size(); ++i) {
        const auto& seg = _layout[i];

        // Segment text
        if (seg.index == _hoveredIndex) {
            p.setPen(hoverStyle.foreground);
        } else {
            p.setPen(style.foreground);
        }
        p.drawText(seg.x, 0, seg.width, height(),
                   Qt::AlignLeft | Qt::AlignVCenter, _items[static_cast<size_t>(seg.index)]);

        // Separator after (except last)
        if (i + 1 < _layout.size()) {
            p.setPen(style.border);
            p.drawText(seg.x + seg.width, 0, sepW, height(),
                       Qt::AlignLeft | Qt::AlignVCenter, _separator);
        }
    }
}

void NyanBreadcrumb::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const int mx = event->position().toPoint().x();
    for (const auto& seg : _layout) {
        if (mx >= seg.x && mx < seg.x + seg.width) {
            emit ItemClicked(seg.index);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void NyanBreadcrumb::OnThemeChanged()
{
    RebuildLayout();
    update();
}

} // namespace matcha::gui