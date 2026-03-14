#include <Matcha/Widgets/Controls/NyanStructureTree.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace matcha::gui {

// ============================================================================
// NyanTreeView
// ============================================================================

NyanTreeView::NyanTreeView(QWidget* parent)
    : QTreeView(parent)
    , ThemeAware(WidgetKind::StructureTree)
{
    setHeaderHidden(true);
    setAnimated(true);
    setIndentation(20);
    setExpandsOnDoubleClick(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    SetBackgroundTransparent(true);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanTreeView::~NyanTreeView() = default;

void NyanTreeView::SetBackgroundTransparent(bool transparent)
{
    _backgroundTransparent = transparent;

    if (transparent) {
        setStyleSheet(R"(
QTreeView {background-color: rgba(0, 0, 0, 0)}
QTreeView QScrollBar:vertical {background-color: rgba(0, 0, 0, 10);}
QTreeView QScrollBar:horizontal {background-color: rgba(0, 0, 0, 10);}
QTreeView QScrollBar::handle:vertical {background-color: rgba(0, 0, 0, 30); width: 2px;}
QTreeView QScrollBar::handle:horizontal {background-color: rgba(0, 0, 0, 30); height: 2px;}
QTreeView QScrollBar::handle:hover {background-color: rgba(0, 0, 0, 50);}
QTreeView QScrollBar::add-line,QTreeView QScrollBar::sub-line {background-color: transparent; border: none;}
QTreeView QScrollBar::add-page:vertical,QTreeView QScrollBar::sub-page:vertical,QTreeView QScrollBar::add-page:horizontal,QTreeView QScrollBar::sub-page:horizontal {background-color: transparent; border-radius: 4px;}
)");
    } else {
        const auto& theme = Theme();
        QString style = QString(
            "QTreeView { background-color: %1; }"
        ).arg(theme.Color(ColorToken::SurfaceElevated).name());
        setStyleSheet(style);
    }
}

void NyanTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    if (!index.isValid()) {
        return;
    }

    const auto& theme = Theme();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(theme.Color(ColorToken::BorderStrong), 0.5));

    const bool hasChildren = model()->hasChildren(index);
    const bool isExpanded = this->isExpanded(index);
    const QModelIndex parentIndex = index.parent();
    const bool hasParent = parentIndex.isValid();

    if (hasChildren) {
        // Draw expand/collapse icon area
        QRect iconRect(rect.right() - 20, rect.center().y() - 6, 12, 12);

        // Draw line from icon to item
        painter->drawLine(rect.right() - 8, rect.center().y(), rect.right(), rect.center().y());

        // Draw expand/collapse indicator (simple triangle)
        painter->setBrush(theme.Color(ColorToken::TextSecondary));
        painter->setPen(Qt::NoPen);

        QPolygonF triangle;
        if (isExpanded) {
            // Down arrow
            triangle << QPointF(iconRect.left() + 2, iconRect.top() + 4)
                     << QPointF(iconRect.right() - 2, iconRect.top() + 4)
                     << QPointF(iconRect.center().x(), iconRect.bottom() - 3);
        } else {
            // Right arrow
            triangle << QPointF(iconRect.left() + 4, iconRect.top() + 2)
                     << QPointF(iconRect.left() + 4, iconRect.bottom() - 2)
                     << QPointF(iconRect.right() - 3, iconRect.center().y());
        }
        painter->drawPolygon(triangle);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(theme.Color(ColorToken::BorderStrong), 0.5));

        if (hasParent) {
            // Draw top connecting line
            painter->drawLine(iconRect.center().x() + 1, iconRect.top(),
                              iconRect.center().x() + 1, rect.top());

            // Draw bottom connecting line if not last sibling
            int rowCount = model()->rowCount(parentIndex);
            if (index.row() < rowCount - 1) {
                painter->drawLine(iconRect.center().x() + 1, iconRect.bottom() + 1,
                                  iconRect.center().x() + 1, rect.bottom());
            }
        }
    } else {
        // No children - draw |- connector
        if (!hasParent) {
            painter->restore();
            return;
        }

        int indent = indentation();
        int centerX = rect.right() - (indent / 2);

        // Horizontal line to item
        painter->drawLine(centerX, rect.center().y(), rect.right(), rect.center().y());

        // Vertical line up
        painter->drawLine(centerX, rect.center().y(), centerX, rect.top());

        // Vertical line down if not last sibling
        int rowCount = model()->rowCount(parentIndex);
        if (index.row() < rowCount - 1) {
            painter->drawLine(centerX, rect.center().y(), centerX, rect.bottom() + 1);
        }
    }

    // Draw vertical lines for ancestor levels
    QModelIndex ancestor = parentIndex;
    int posX = rect.right() - (indentation() * 2) + 6;

    while (ancestor.isValid() && posX > indentation()) {
        QModelIndex grandparent = ancestor.parent();
        if (!grandparent.isValid()) {
            break;
        }

        int ancestorRowCount = model()->rowCount(grandparent);
        if (ancestor.row() < ancestorRowCount - 1) {
            // Ancestor is not last sibling - draw vertical line
            painter->drawLine(posX - 1, rect.top(), posX - 1, rect.bottom());
        }

        posX -= indentation();
        ancestor = grandparent;
    }

    painter->restore();
}

void NyanTreeView::OnThemeChanged()
{
    SetBackgroundTransparent(_backgroundTransparent);
    update();
}

auto NyanTreeView::BranchIconSize() const -> QSize
{
    return {12, 12};
}

// ============================================================================
// NyanStructureTree
// ============================================================================

NyanStructureTree::NyanStructureTree(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::StructureTree)
{
    InitLayout();
    InitTreeView();

    _hideTimer = new QTimer(this);
    _hideTimer->setSingleShot(true);
    connect(_hideTimer, &QTimer::timeout, this, &NyanStructureTree::OnTitleBarHideTimeout);

    UpdateTitleBarVisibility();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanStructureTree::~NyanStructureTree() = default;

void NyanStructureTree::InitLayout()
{
    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);

    // Title bar
    _titleBar = new QWidget(this);
    _titleBar->setFixedHeight(kTitleBarHeight);

    auto* titleLayout = new QHBoxLayout(_titleBar);
    titleLayout->setContentsMargins(8, 0, 4, 0);
    titleLayout->setSpacing(4);

    _titleLabel = new QLabel(_titleBar);
    _titleLabel->setText(tr("Navigator"));
    titleLayout->addWidget(_titleLabel, 1);

    _pinButton = new QPushButton("📌", _titleBar);
    _pinButton->setFixedSize(20, 20);
    _pinButton->setFlat(true);
    _pinButton->setCheckable(true);
    _pinButton->setToolTip(tr("Pin title bar"));
    connect(_pinButton, &QPushButton::clicked, this, &NyanStructureTree::OnPinButtonClicked);
    titleLayout->addWidget(_pinButton);

    _collapseButton = new QPushButton("▼", _titleBar);
    _collapseButton->setFixedSize(20, 20);
    _collapseButton->setFlat(true);
    _collapseButton->setToolTip(tr("Collapse"));
    connect(_collapseButton, &QPushButton::clicked, this, &NyanStructureTree::OnCollapseButtonClicked);
    titleLayout->addWidget(_collapseButton);

    _layout->addWidget(_titleBar);
}

void NyanStructureTree::InitTreeView()
{
    _treeView = new NyanTreeView(this);
    _layout->addWidget(_treeView, 1);

    // Forward signals
    connect(_treeView, &QTreeView::doubleClicked, this, &NyanStructureTree::ItemDoubleClicked);

    connect(_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &NyanStructureTree::SelectionChanged);

    _treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_treeView, &QTreeView::customContextMenuRequested, this, [this](const QPoint& pos) {
        QModelIndex index = _treeView->indexAt(pos);
        Q_EMIT ContextMenuRequested(_treeView->mapToGlobal(pos), index);
    });
}

// -- Model --

void NyanStructureTree::SetModel(QAbstractItemModel* model)
{
    _treeView->setModel(model);

    // Reconnect selection changed signal
    if (_treeView->selectionModel()) {
        connect(_treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &NyanStructureTree::SelectionChanged);
    }
}

auto NyanStructureTree::Model() const -> QAbstractItemModel*
{
    return _treeView->model();
}

auto NyanStructureTree::TreeView() -> QTreeView*
{
    return _treeView;
}

// -- Configuration --

void NyanStructureTree::SetDragEnabled(bool enabled)
{
    _treeView->setDragEnabled(enabled);
    _treeView->setAcceptDrops(enabled);
}

auto NyanStructureTree::IsDragEnabled() const -> bool
{
    return _treeView->dragEnabled();
}

void NyanStructureTree::SetTreeContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
    _treeView->setContextMenuPolicy(policy);
}

void NyanStructureTree::SetBackgroundTransparent(bool transparent)
{
    _backgroundTransparent = transparent;
    if (auto* treeView = dynamic_cast<NyanTreeView*>(_treeView)) {
        treeView->SetBackgroundTransparent(transparent);
    }
}

auto NyanStructureTree::IsBackgroundTransparent() const -> bool
{
    return _backgroundTransparent;
}

// -- Title Bar --

void NyanStructureTree::SetTitle(const QString& title)
{
    _titleLabel->setText(title);
}

auto NyanStructureTree::Title() const -> QString
{
    return _titleLabel->text();
}

void NyanStructureTree::SetTitleBarAutoHide(bool autoHide)
{
    _titleBarAutoHide = autoHide;
    UpdateTitleBarVisibility();
}

auto NyanStructureTree::IsTitleBarAutoHide() const -> bool
{
    return _titleBarAutoHide;
}

void NyanStructureTree::SetTitleBarPinned(bool pinned)
{
    _titleBarPinned = pinned;
    _pinButton->setChecked(pinned);
    UpdateTitleBarVisibility();
}

auto NyanStructureTree::IsTitleBarPinned() const -> bool
{
    return _titleBarPinned;
}

// -- Collapse --

void NyanStructureTree::SetCollapsed(bool collapsed)
{
    if (_collapsed != collapsed) {
        _collapsed = collapsed;
        _treeView->setVisible(!collapsed);
        _collapseButton->setText(collapsed ? "▶" : "▼");
        Q_EMIT CollapsedChanged(collapsed);
    }
}

auto NyanStructureTree::IsCollapsed() const -> bool
{
    return _collapsed;
}

// -- Size hints --

auto NyanStructureTree::sizeHint() const -> QSize
{
    return {200, 300};
}

auto NyanStructureTree::minimumSizeHint() const -> QSize
{
    return {100, kTitleBarHeight};
}

// -- Events --

void NyanStructureTree::enterEvent(QEnterEvent* /*event*/)
{
    _hideTimer->stop();
    if (_titleBarAutoHide && !_titleBarPinned) {
        _titleBar->setVisible(true);
    }
}

void NyanStructureTree::leaveEvent(QEvent* /*event*/)
{
    if (_titleBarAutoHide && !_titleBarPinned) {
        _hideTimer->start(kHideDelay);
    }
}

void NyanStructureTree::OnThemeChanged()
{
    const auto& theme = Theme();

    // Update title bar style
    QString titleBarStyle = QString(
        "QWidget { background-color: %1; }"
        "QLabel { color: %2; font-weight: bold; }"
        "QPushButton { background: transparent; border: none; color: %3; }"
        "QPushButton:hover { color: %4; }"
    ).arg(theme.Color(ColorToken::FillHover).name(),
          theme.Color(ColorToken::TextPrimary).name(),
          theme.Color(ColorToken::TextSecondary).name(),
          theme.Color(ColorToken::Primary).name());
    _titleBar->setStyleSheet(titleBarStyle);

    update();
}

void NyanStructureTree::UpdateTitleBarVisibility()
{
    if (_titleBarPinned || !_titleBarAutoHide) {
        _titleBar->setVisible(true);
    } else {
        _titleBar->setVisible(false);
    }
}

void NyanStructureTree::OnCollapseButtonClicked()
{
    SetCollapsed(!_collapsed);
}

void NyanStructureTree::OnPinButtonClicked()
{
    SetTitleBarPinned(_pinButton->isChecked());
}

void NyanStructureTree::OnTitleBarHideTimeout()
{
    if (_titleBarAutoHide && !_titleBarPinned && !underMouse()) {
        _titleBar->setVisible(false);
    }
}

} // namespace matcha::gui