/**
 * @file FlowLayout.cpp
 * @brief Flow layout implementation (adapted from Qt Flow Layout Example).
 */

#include "FlowLayout.h"

#include <QWidget>

namespace matcha::gui::detail {

FlowLayout::FlowLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent)
    , _hSpace(hSpacing)
    , _vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : _hSpace(hSpacing)
    , _vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    while (!_itemList.empty()) {
        delete _itemList.back();
        _itemList.pop_back();
    }
}

void FlowLayout::addItem(QLayoutItem* item)
{
    _itemList.push_back(item);
}

auto FlowLayout::horizontalSpacing() const -> int
{
    if (_hSpace >= 0) {
        return _hSpace;
    }
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

auto FlowLayout::verticalSpacing() const -> int
{
    if (_vSpace >= 0) {
        return _vSpace;
    }
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

auto FlowLayout::count() const -> int
{
    return static_cast<int>(_itemList.size());
}

auto FlowLayout::itemAt(int index) const -> QLayoutItem*
{
    if (index >= 0 && index < static_cast<int>(_itemList.size())) {
        return _itemList[static_cast<std::size_t>(index)];
    }
    return nullptr;
}

auto FlowLayout::takeAt(int index) -> QLayoutItem*
{
    if (index >= 0 && index < static_cast<int>(_itemList.size())) {
        auto it = _itemList.begin() + index;
        QLayoutItem* item = *it;
        _itemList.erase(it);
        return item;
    }
    return nullptr;
}

auto FlowLayout::expandingDirections() const -> Qt::Orientations
{
    return {};
}

auto FlowLayout::hasHeightForWidth() const -> bool
{
    return true;
}

auto FlowLayout::heightForWidth(int width) const -> int
{
    return doLayout(QRect(0, 0, width, 0), true);
}

void FlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

auto FlowLayout::sizeHint() const -> QSize
{
    return minimumSize();
}

auto FlowLayout::minimumSize() const -> QSize
{
    QSize size;
    for (const auto* item : _itemList) {
        size = size.expandedTo(item->minimumSize());
    }

    const auto margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

auto FlowLayout::doLayout(const QRect& rect, bool testOnly) const -> int
{
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    getContentsMargins(&left, &top, &right, &bottom);

    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (auto* item : _itemList) {
        const QWidget* wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1) {
            spaceX = wid != nullptr
                         ? wid->style()->layoutSpacing(
                               QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal)
                         : 0;
        }
        int spaceY = verticalSpacing();
        if (spaceY == -1) {
            spaceY = wid != nullptr
                         ? wid->style()->layoutSpacing(
                               QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical)
                         : 0;
        }

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly) {
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    return y + lineHeight - rect.y() + bottom;
}

auto FlowLayout::smartSpacing(QStyle::PixelMetric pm) const -> int
{
    QObject* parentObj = parent();
    if (parentObj == nullptr) {
        return -1;
    }
    if (parentObj->isWidgetType()) {
        auto* pw = static_cast<QWidget*>(parentObj);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout*>(parentObj)->spacing();
}

} // namespace matcha::gui::detail
