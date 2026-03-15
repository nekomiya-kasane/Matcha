#pragma once

/**
 * @file FlowLayout.h
 * @brief Flow layout that wraps items to the next row.
 *
 * INTERNAL to ContainerNode. Not a public header.
 * Adapted from the Qt Flow Layout Example.
 */

#include <QLayout>
#include <QStyle>

#include <vector>

namespace matcha::gui::detail {

class FlowLayout : public QLayout {
public:
    explicit FlowLayout(QWidget* parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout() override;

    void addItem(QLayoutItem* item) override;
    [[nodiscard]] auto horizontalSpacing() const -> int;
    [[nodiscard]] auto verticalSpacing() const -> int;
    [[nodiscard]] auto expandingDirections() const -> Qt::Orientations override;
    [[nodiscard]] auto hasHeightForWidth() const -> bool override;
    [[nodiscard]] auto heightForWidth(int) const -> int override;
    [[nodiscard]] auto count() const -> int override;
    [[nodiscard]] auto itemAt(int index) const -> QLayoutItem* override;
    [[nodiscard]] auto minimumSize() const -> QSize override;
    void setGeometry(const QRect& rect) override;
    [[nodiscard]] auto sizeHint() const -> QSize override;
    auto takeAt(int index) -> QLayoutItem* override;

private:
    auto doLayout(const QRect& rect, bool testOnly) const -> int;
    auto smartSpacing(QStyle::PixelMetric pm) const -> int;

    std::vector<QLayoutItem*> _itemList;
    int _hSpace;
    int _vSpace;
};

} // namespace matcha::gui::detail
