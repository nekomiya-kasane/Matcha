#include <Matcha/Widgets/ActionBar/NyanActionTab.h>
#include <Matcha/Widgets/ActionBar/NyanActionToolbar.h>

#include <QBoxLayout>
#include <QPainter>

namespace matcha::gui {

NyanActionTab::NyanActionTab(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::ActionTab)
{
    _layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    _layout->setContentsMargins(8, 4, 8, 4);
    _layout->setSpacing(kSpacing);
    _layout->addStretch(1);

    setFixedHeight(kHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

NyanActionTab::~NyanActionTab() = default;

// -- Toolbars --

void NyanActionTab::AddToolbar(NyanActionToolbar* toolbar)
{
    if (!toolbar) {
        return;
    }

    toolbar->setParent(this);
    _layout->insertWidget(static_cast<int>(_toolbars.size()), toolbar);
    _toolbars.push_back(toolbar);

    ConnectToolbar(toolbar);
}

void NyanActionTab::RemoveToolbar(NyanActionToolbar* toolbar)
{
    auto it = std::find(_toolbars.begin(), _toolbars.end(), toolbar);
    if (it != _toolbars.end()) {
        _layout->removeWidget(toolbar);
        _toolbars.erase(it);
        toolbar->setParent(nullptr);
    }
}

auto NyanActionTab::ToolbarCount() const -> int
{
    return static_cast<int>(_toolbars.size());
}

auto NyanActionTab::ToolbarAt(int index) -> NyanActionToolbar*
{
    if (index < 0 || index >= static_cast<int>(_toolbars.size())) {
        return nullptr;
    }
    return _toolbars[static_cast<size_t>(index)];
}

// -- Label --

void NyanActionTab::SetLabel(const QString& label)
{
    _label = label;
}

auto NyanActionTab::Label() const -> QString
{
    return _label;
}

// -- Persistence --

void NyanActionTab::SetPersistence(TabPersistence persistence)
{
    _persistence = persistence;
}

auto NyanActionTab::Persistence() const -> TabPersistence
{
    return _persistence;
}

// -- Orientation --

void NyanActionTab::SetOrientation(Qt::Orientation orientation)
{
    if (_orientation == orientation) { return; }
    _orientation = orientation;

    _layout->setDirection(_orientation == Qt::Horizontal
                              ? QBoxLayout::LeftToRight
                              : QBoxLayout::TopToBottom);

    if (_orientation == Qt::Horizontal) {
        setFixedHeight(kHeight);
        setMaximumWidth(QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    } else {
        setFixedWidth(kHeight); // same thickness
        setMaximumHeight(QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }

    // Propagate to child toolbars
    for (auto* toolbar : _toolbars) {
        toolbar->SetOrientation(orientation);
    }

    updateGeometry();
    update();
}

auto NyanActionTab::Orientation() const -> Qt::Orientation
{
    return _orientation;
}

// -- Size hints --

auto NyanActionTab::sizeHint() const -> QSize
{
    int extent = 16; // margins
    for (const auto* toolbar : _toolbars) {
        QSize ts = toolbar->sizeHint();
        extent += (_orientation == Qt::Horizontal ? ts.width() : ts.height()) + kSpacing;
    }
    if (_orientation == Qt::Horizontal) {
        return {extent, kHeight};
    }
    return {kHeight, extent};
}

auto NyanActionTab::minimumSizeHint() const -> QSize
{
    if (_orientation == Qt::Horizontal) {
        return {100, kHeight};
    }
    return {kHeight, 100};
}

// -- Paint --

void NyanActionTab::paintEvent(QPaintEvent* /*event*/)
{
    // Transparent -- inherits background from ActionBar's QTabWidget::pane stylesheet
}

void NyanActionTab::OnThemeChanged()
{
    update();
}

void NyanActionTab::ConnectToolbar(NyanActionToolbar* toolbar)
{
    connect(toolbar, &NyanActionToolbar::ButtonClicked, this,
        [this, toolbar](const QString& buttonId, bool checked) {
            Q_EMIT ButtonClicked(toolbar->objectName(), buttonId, checked);
        });
}

} // namespace matcha::gui
