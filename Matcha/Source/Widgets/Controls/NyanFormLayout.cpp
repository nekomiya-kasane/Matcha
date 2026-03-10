/**
 * @file NyanFormLayout.cpp
 * @brief Implementation of NyanFormLayout label-input pair form layout.
 */

#include <Matcha/Widgets/Controls/NyanFormLayout.h>

#include "../Core/InteractionEventFilter.h"

#include <QFormLayout>
#include <QLabel>
#include <QPainter>

namespace matcha::gui {

NyanFormLayout::NyanFormLayout(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::FormLayout)
{
    SetupUi();
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanFormLayout::~NyanFormLayout() = default;

void NyanFormLayout::AddRow(std::string_view label, QWidget* widget)
{
    auto* lbl = new QLabel(QString::fromUtf8(label.data(), static_cast<int>(label.size())), this);
    lbl->setFixedWidth(_labelWidth);
    lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _labels.push_back(lbl);
    _formLayout->addRow(lbl, widget);
}

void NyanFormLayout::AddSection(std::string_view title)
{
    auto* lbl = new QLabel(QString::fromUtf8(title.data(), static_cast<int>(title.size())), this);
    auto f = lbl->font();
    f.setBold(true);
    lbl->setFont(f);
    _formLayout->addRow(lbl);
}

void NyanFormLayout::SetLabelWidth(int width)
{
    _labelWidth = width;
    for (auto* lbl : _labels) {
        lbl->setFixedWidth(width);
    }
}

auto NyanFormLayout::RowCount() const -> int
{
    return _formLayout->rowCount();
}

auto NyanFormLayout::sizeHint() const -> QSize { return {300, 200}; }
auto NyanFormLayout::minimumSizeHint() const -> QSize { return {200, 100}; }

void NyanFormLayout::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void NyanFormLayout::OnThemeChanged() { update(); }

void NyanFormLayout::SetupUi()
{
    _formLayout = new QFormLayout(this);
    _formLayout->setContentsMargins(8, 8, 8, 8);
    _formLayout->setSpacing(6);
}

} // namespace matcha::gui