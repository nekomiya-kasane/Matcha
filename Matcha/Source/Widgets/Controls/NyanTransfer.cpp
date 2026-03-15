/**
 * @file NyanTransfer.cpp
 * @brief Implementation of NyanTransfer dual-list transfer selector.
 */

#include <Matcha/Widgets/Controls/NyanTransfer.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

namespace matcha::gui {

NyanTransfer::NyanTransfer(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Transfer)
{
    SetupUi();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanTransfer::~NyanTransfer() = default;

void NyanTransfer::SetSourceItems(const std::vector<std::string>& items)
{
    _sourceList->clear();
    for (const auto& item : items) {
        _sourceList->addItem(QString::fromStdString(item));
    }
}

void NyanTransfer::SetTargetItems(const std::vector<std::string>& items)
{
    _targetList->clear();
    for (const auto& item : items) {
        _targetList->addItem(QString::fromStdString(item));
    }
}

auto NyanTransfer::SourceItems() const -> std::vector<std::string>
{
    std::vector<std::string> result;
    for (int i = 0; i < _sourceList->count(); ++i) {
        result.push_back(_sourceList->item(i)->text().toStdString());
    }
    return result;
}

auto NyanTransfer::TargetItems() const -> std::vector<std::string>
{
    std::vector<std::string> result;
    for (int i = 0; i < _targetList->count(); ++i) {
        result.push_back(_targetList->item(i)->text().toStdString());
    }
    return result;
}

void NyanTransfer::MoveSelectedToTarget()
{
    auto selected = _sourceList->selectedItems();
    for (auto* item : selected) {
        _targetList->addItem(_sourceList->takeItem(_sourceList->row(item)));
    }
    if (!selected.isEmpty()) Q_EMIT TransferChanged();
}

void NyanTransfer::MoveSelectedToSource()
{
    auto selected = _targetList->selectedItems();
    for (auto* item : selected) {
        _sourceList->addItem(_targetList->takeItem(_targetList->row(item)));
    }
    if (!selected.isEmpty()) Q_EMIT TransferChanged();
}

void NyanTransfer::MoveAllToTarget()
{
    while (_sourceList->count() > 0) {
        _targetList->addItem(_sourceList->takeItem(0));
    }
    Q_EMIT TransferChanged();
}

void NyanTransfer::MoveAllToSource()
{
    while (_targetList->count() > 0) {
        _sourceList->addItem(_targetList->takeItem(0));
    }
    Q_EMIT TransferChanged();
}

auto NyanTransfer::sizeHint() const -> QSize { return {500, 300}; }
auto NyanTransfer::minimumSizeHint() const -> QSize { return {300, 150}; }

void NyanTransfer::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    const auto style = Theme().Resolve(WidgetKind::Transfer, 0, InteractionState::Normal);
    QPainter p(this);
    p.fillRect(rect(), style.background);
}

void NyanTransfer::OnThemeChanged() { update(); }

void NyanTransfer::SetupUi()
{
    _sourceList = new QListWidget(this);
    _targetList = new QListWidget(this);
    _sourceList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _targetList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    _moveRightBtn = new QPushButton(QStringLiteral(">"), this);
    _moveLeftBtn = new QPushButton(QStringLiteral("<"), this);
    _moveAllRightBtn = new QPushButton(QStringLiteral(">>"), this);
    _moveAllLeftBtn = new QPushButton(QStringLiteral("<<"), this);

    auto* btnLayout = new QVBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(_moveAllRightBtn);
    btnLayout->addWidget(_moveRightBtn);
    btnLayout->addWidget(_moveLeftBtn);
    btnLayout->addWidget(_moveAllLeftBtn);
    btnLayout->addStretch();

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(_sourceList);
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(_targetList);

    connect(_moveRightBtn, &QPushButton::clicked,
            this, &NyanTransfer::MoveSelectedToTarget);
    connect(_moveLeftBtn, &QPushButton::clicked,
            this, &NyanTransfer::MoveSelectedToSource);
    connect(_moveAllRightBtn, &QPushButton::clicked,
            this, &NyanTransfer::MoveAllToTarget);
    connect(_moveAllLeftBtn, &QPushButton::clicked,
            this, &NyanTransfer::MoveAllToSource);
}

} // namespace matcha::gui