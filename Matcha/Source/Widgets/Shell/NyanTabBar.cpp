#include <Matcha/Widgets/Shell/NyanTabBar.h>
#include <Matcha/Widgets/Shell/NyanTabItem.h>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

#include <algorithm>

namespace matcha::gui {

NyanTabBar::NyanTabBar(TabStyle style, QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::DocumentBar)
    , _style(style)
{
    setFixedHeight(ItemHeight());
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAcceptDrops(true);
    UpdateAutoHide();
}

NyanTabBar::~NyanTabBar() = default;

// -- Tab Management --

auto NyanTabBar::AddTab(fw::PageId pageId, const QString& title) -> NyanTabItem*
{
    auto* item = new NyanTabItem(_style, pageId, title, this);
    _items.push_back(item);
    item->show();

    int idx = static_cast<int>(_items.size()) - 1;
    if (_activeIndex < 0) {
        _activeIndex = idx;
        item->SetActive(true);
    }

    // Wire item signals -> bar signals
    QObject::connect(item, &NyanTabItem::Pressed, this, &NyanTabBar::OnItemPressed);
    QObject::connect(item, &NyanTabItem::CloseRequested, this, &NyanTabBar::TabCloseRequested);

    RecalcLayout();
    UpdateAutoHide();
    return item;
}

auto NyanTabBar::InsertTab(int index, fw::PageId pageId, const QString& title) -> NyanTabItem*
{
    auto* item = new NyanTabItem(_style, pageId, title, this);
    index = std::clamp(index, 0, static_cast<int>(_items.size()));
    _items.insert(_items.begin() + index, item);
    item->show();

    // Adjust active index if insertion is before it
    if (_activeIndex >= index) {
        ++_activeIndex;
    }
    if (_activeIndex < 0) {
        _activeIndex = index;
        item->SetActive(true);
    }

    QObject::connect(item, &NyanTabItem::Pressed, this, &NyanTabBar::OnItemPressed);
    QObject::connect(item, &NyanTabItem::CloseRequested, this, &NyanTabBar::TabCloseRequested);

    RecalcLayout();
    UpdateAutoHide();
    return item;
}

void NyanTabBar::RemoveTab(fw::PageId pageId)
{
    int idx = IndexOfPage(pageId);
    if (idx < 0) { return; }

    auto* item = _items[static_cast<size_t>(idx)];
    _items.erase(_items.begin() + idx);
    item->deleteLater();

    // Adjust active index
    if (_items.empty()) {
        _activeIndex = -1;
    } else if (idx == _activeIndex) {
        _activeIndex = std::min(idx, static_cast<int>(_items.size()) - 1);
        _items[static_cast<size_t>(_activeIndex)]->SetActive(true);
    } else if (idx < _activeIndex) {
        --_activeIndex;
    }

    RecalcLayout();
    UpdateAutoHide();
}

void NyanTabBar::SetActiveTab(fw::PageId pageId)
{
    int idx = IndexOfPage(pageId);
    if (idx < 0 || idx == _activeIndex) { return; }

    // Deactivate old
    if (_activeIndex >= 0 && _activeIndex < static_cast<int>(_items.size())) {
        _items[static_cast<size_t>(_activeIndex)]->SetActive(false);
    }

    _activeIndex = idx;
    _items[static_cast<size_t>(idx)]->SetActive(true);
}

void NyanTabBar::SetTabTitle(fw::PageId pageId, const QString& title)
{
    auto* item = FindItem(pageId);
    if (item != nullptr) {
        item->SetTitle(title);
    }
}

void NyanTabBar::MoveTab(int fromIndex, int toIndex)
{
    int count = static_cast<int>(_items.size());
    if (fromIndex < 0 || fromIndex >= count) { return; }
    if (toIndex < 0 || toIndex >= count) { return; }
    if (fromIndex == toIndex) { return; }

    auto* item = _items[static_cast<size_t>(fromIndex)];
    _items.erase(_items.begin() + fromIndex);
    _items.insert(_items.begin() + toIndex, item);

    // Update active index to follow the active tab
    if (_activeIndex == fromIndex) {
        _activeIndex = toIndex;
    } else if (fromIndex < _activeIndex && toIndex >= _activeIndex) {
        --_activeIndex;
    } else if (fromIndex > _activeIndex && toIndex <= _activeIndex) {
        ++_activeIndex;
    }

    RecalcLayout();
}

auto NyanTabBar::ItemAt(int index) const -> NyanTabItem*
{
    if (index >= 0 && index < static_cast<int>(_items.size())) {
        return _items[static_cast<size_t>(index)];
    }
    return nullptr;
}

auto NyanTabBar::FindItem(fw::PageId pageId) const -> NyanTabItem*
{
    for (auto* item : _items) {
        if (item->GetPageId() == pageId) {
            return item;
        }
    }
    return nullptr;
}

auto NyanTabBar::IndexOfPage(fw::PageId pageId) const -> int
{
    for (size_t i = 0; i < _items.size(); ++i) {
        if (_items[i]->GetPageId() == pageId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

auto NyanTabBar::ActivePageId() const -> fw::PageId
{
    auto* item = ItemAt(_activeIndex);
    return (item != nullptr) ? item->GetPageId() : fw::PageId::From(0);
}

auto NyanTabBar::TabCount() const -> int
{
    return static_cast<int>(_items.size());
}

// -- Reorder (called by NyanTabItem during horizontal drag) --

void NyanTabBar::RequestReorder(NyanTabItem* item, int globalX)
{
    int fromIdx = -1;
    for (size_t i = 0; i < _items.size(); ++i) {
        if (_items[i] == item) {
            fromIdx = static_cast<int>(i);
            break;
        }
    }
    if (fromIdx < 0) { return; }

    // Convert global X to local bar X
    int localX = mapFromGlobal(QPoint(globalX, 0)).x();

    // Check if cursor crossed into adjacent tab's territory
    int toIdx = fromIdx;
    if (fromIdx > 0) {
        int prevMid = _items[static_cast<size_t>(fromIdx - 1)]->geometry().center().x();
        if (localX < prevMid) {
            toIdx = fromIdx - 1;
        }
    }
    if (fromIdx < static_cast<int>(_items.size()) - 1) {
        int nextMid = _items[static_cast<size_t>(fromIdx + 1)]->geometry().center().x();
        if (localX > nextMid) {
            toIdx = fromIdx + 1;
        }
    }

    if (toIdx != fromIdx) {
        auto pageId = item->GetPageId();
        MoveTab(fromIdx, toIdx);
        Q_EMIT TabReordered(pageId, fromIdx, toIdx);
    }
}

// -- Drag result handling (called by NyanTabItem after QDrag::exec) --

void NyanTabBar::HandleDragResult(fw::PageId pageId, Qt::DropAction result)
{
    if (result == Qt::IgnoreAction) {
        // No target accepted the drop -> void drop -> create new window
        Q_EMIT TabDraggedToVoid(pageId, QCursor::pos());
    }
    // If result == Qt::MoveAction, the drop target already handled it
}

// -- Metrics --

auto NyanTabBar::ItemHeight() const -> int
{
    return (_style == TabStyle::TitleBar) ? kTitleBarTabHeight : kFloatingTabHeight;
}

auto NyanTabBar::ItemWidth() const -> int
{
    return (_style == TabStyle::TitleBar) ? kTitleBarTabWidth : kFloatingTabWidth;
}

// -- Layout --

void NyanTabBar::RecalcLayout()
{
    int x = 0;
    int h = ItemHeight();
    int w = ItemWidth();
    for (auto* item : _items) {
        item->setGeometry(x, 0, w, h);
        x += w + kTabGap;
    }
    updateGeometry();
    update();
}

void NyanTabBar::UpdateAutoHide()
{
    if (_style == TabStyle::Floating) {
        setVisible(static_cast<int>(_items.size()) >= 2);
    }
}

auto NyanTabBar::AddButtonRect() const -> QRect
{
    int xAfterItems = 0;
    if (!_items.empty()) {
        const auto* last = _items.back();
        xAfterItems = last->geometry().right() + kTabGap + 2;
    }
    int y = (height() - kAddBtnSize) / 2;
    return {xAfterItems, y, kAddBtnSize, kAddBtnSize};
}

auto NyanTabBar::InsertIndexAt(int x) const -> int
{
    for (size_t i = 0; i < _items.size(); ++i) {
        int mid = _items[i]->geometry().center().x();
        if (x < mid) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(_items.size());
}

// -- Size hints --

auto NyanTabBar::sizeHint() const -> QSize
{
    int w = (static_cast<int>(_items.size()) * (ItemWidth() + kTabGap)) + kAddBtnSize + 8;
    return {w, ItemHeight()};
}

auto NyanTabBar::minimumSizeHint() const -> QSize
{
    return {ItemWidth() + kAddBtnSize + 8, ItemHeight()};
}

// -- Paint --

void NyanTabBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (_style == TabStyle::TitleBar) {
        DrawAddButton(p);
    }

    if (_insertIndicatorIndex >= 0) {
        DrawInsertionIndicator(p);
    }
}

void NyanTabBar::DrawAddButton(QPainter& painter) const
{
    QRect btnRect = AddButtonRect();
    if (btnRect.right() > width()) { return; }

    if (_hoveredAddButton) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 50));
        painter.drawRoundedRect(btnRect, btnRect.width() / 2.0, btnRect.height() / 2.0);
    }

    painter.setPen(QPen(QColor(255, 255, 255, _hoveredAddButton ? 255 : 160), 1.5));
    int cx = btnRect.center().x();
    int cy = btnRect.center().y();
    int arm = 5;
    painter.drawLine(cx - arm, cy, cx + arm, cy);
    painter.drawLine(cx, cy - arm, cx, cy + arm);
}

void NyanTabBar::DrawInsertionIndicator(QPainter& painter) const
{
    int x = 0;
    if (_insertIndicatorIndex <= 0) {
        x = 0;
    } else if (_insertIndicatorIndex >= static_cast<int>(_items.size())) {
        if (!_items.empty()) {
            x = _items.back()->geometry().right() + (kTabGap / 2);
        }
    } else {
        x = _items[static_cast<size_t>(_insertIndicatorIndex)]->geometry().left() - (kTabGap / 2);
    }

    // Draw a 2px-wide blue vertical line
    bool dark = (_style == TabStyle::TitleBar);
    QColor color = dark ? QColor(100, 180, 255) : QColor(0, 100, 220);
    painter.setPen(QPen(color, 2));
    painter.drawLine(x, 2, x, height() - 2);
}

// -- Mouse Events (add button only; items handle their own) --

void NyanTabBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton
        && _style == TabStyle::TitleBar
        && AddButtonRect().contains(event->pos())) {
        Q_EMIT AddTabRequested();
        return;
    }
    QWidget::mousePressEvent(event);
}

void NyanTabBar::mouseMoveEvent(QMouseEvent* event)
{
    if (_style == TabStyle::TitleBar) {
        bool addHover = AddButtonRect().contains(event->pos());
        if (addHover != _hoveredAddButton) {
            _hoveredAddButton = addHover;
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void NyanTabBar::leaveEvent(QEvent* event)
{
    _hoveredAddButton = false;
    update();
    QWidget::leaveEvent(event);
}

// -- Drag & Drop Events (bar as drop target) --

void NyanTabBar::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(kMimeType)) {
        event->acceptProposedAction();
        _insertIndicatorIndex = InsertIndexAt(event->position().toPoint().x());
        update();
    }
}

void NyanTabBar::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat(kMimeType)) {
        event->acceptProposedAction();
        int newIdx = InsertIndexAt(event->position().toPoint().x());
        if (newIdx != _insertIndicatorIndex) {
            _insertIndicatorIndex = newIdx;
            update();
        }
    }
}

void NyanTabBar::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    _insertIndicatorIndex = -1;
    update();
}

void NyanTabBar::dropEvent(QDropEvent* event)
{
    _insertIndicatorIndex = -1;
    update();

    if (!event->mimeData()->hasFormat(kMimeType)) { return; }

    QByteArray data = event->mimeData()->data(kMimeType);
    bool ok = false;
    auto pageIdVal = data.toULongLong(&ok);
    if (!ok) { return; }

    auto pageId = fw::PageId::From(pageIdVal);
    int insertIdx = InsertIndexAt(event->position().toPoint().x());

    event->acceptProposedAction();
    Q_EMIT TabDropReceived(pageId, insertIdx);
}

// -- Internal slot: item pressed -> update active + emit --

void NyanTabBar::OnItemPressed(fw::PageId pageId)
{
    int idx = IndexOfPage(pageId);
    if (idx < 0 || idx == _activeIndex) { return; }

    // Deactivate old
    if (_activeIndex >= 0 && _activeIndex < static_cast<int>(_items.size())) {
        _items[static_cast<size_t>(_activeIndex)]->SetActive(false);
    }

    _activeIndex = idx;
    _items[static_cast<size_t>(idx)]->SetActive(true);

    Q_EMIT TabPressed(pageId);
}

void NyanTabBar::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
