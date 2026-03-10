#include <Matcha/Widgets/ActionBar/NyanActionToolbar.h>
#include <Matcha/Widgets/Controls/NyanToolButton.h>

#include <QBoxLayout>

namespace matcha::gui {

NyanActionToolbar::NyanActionToolbar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::ActionToolbar)
{
    _layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(kSpacing);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

NyanActionToolbar::~NyanActionToolbar() = default;

// -- Buttons --

auto NyanActionToolbar::AddButton(const ActionButtonInfo& info) -> int
{
    auto* button = new NyanToolButton(this);
    button->setText(info.text);
    button->setIcon(info.icon);
    button->setToolTip(info.tooltip);
    button->setCheckable(info.checkable);

    if (info.icon.isNull() && !info.text.isEmpty()) {
        // Text-only button: auto-size, don't clamp to icon-only square
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setFixedHeight(kButtonSize);
    } else {
        button->setFixedSize(kButtonSize, kButtonSize);
    }

    connect(button, &NyanToolButton::clicked, this, [this, id = info.id](bool checked) {
        Q_EMIT ButtonClicked(id, checked);
    });

    _layout->addWidget(button);

    Item item;
    item.isSeparator = false;
    item.id = info.id;
    item.button = button;
    _items.push_back(item);

    return static_cast<int>(_items.size()) - 1;
}

auto NyanActionToolbar::AddSeparator() -> int
{
    auto* separator = new QWidget(this);
    if (_orientation == Qt::Horizontal) {
        separator->setFixedSize(kSeparatorSize, kButtonSize);
    } else {
        separator->setFixedSize(kButtonSize, kSeparatorSize);
    }
    separator->setStyleSheet(QString("background-color: %1;")
        .arg(Theme().Color(ColorToken::BorderStrong).name()));

    _layout->addWidget(separator);

    Item item;
    item.isSeparator = true;
    item.separator = separator;
    _items.push_back(item);

    return static_cast<int>(_items.size()) - 1;
}

void NyanActionToolbar::RemoveItem(int index)
{
    if (index < 0 || index >= static_cast<int>(_items.size())) {
        return;
    }

    auto& item = _items[static_cast<size_t>(index)];
    if (item.isSeparator) {
        _layout->removeWidget(item.separator);
        delete item.separator;
    } else {
        _layout->removeWidget(item.button);
        delete item.button;
    }

    _items.erase(_items.begin() + index);
}

auto NyanActionToolbar::ButtonCount() const -> int
{
    int count = 0;
    for (const auto& item : _items) {
        if (!item.isSeparator) {
            ++count;
        }
    }
    return count;
}

auto NyanActionToolbar::ItemCount() const -> int
{
    return static_cast<int>(_items.size());
}

auto NyanActionToolbar::Button(const QString& id) -> NyanToolButton*
{
    for (auto& item : _items) {
        if (!item.isSeparator && item.id == id) {
            return item.button;
        }
    }
    return nullptr;
}

auto NyanActionToolbar::ButtonAt(int index) -> NyanToolButton*
{
    if (index < 0 || index >= static_cast<int>(_items.size())) {
        return nullptr;
    }
    auto& item = _items[static_cast<size_t>(index)];
    return item.isSeparator ? nullptr : item.button;
}

auto NyanActionToolbar::IsSeparator(int index) const -> bool
{
    if (index < 0 || index >= static_cast<int>(_items.size())) {
        return false;
    }
    return _items[static_cast<size_t>(index)].isSeparator;
}

// -- State --

void NyanActionToolbar::SetButtonChecked(const QString& id, bool checked)
{
    if (auto* btn = Button(id)) {
        btn->setChecked(checked);
    }
}

auto NyanActionToolbar::IsButtonChecked(const QString& id) const -> bool
{
    for (const auto& item : _items) {
        if (!item.isSeparator && item.id == id) {
            return item.button->isChecked();
        }
    }
    return false;
}

void NyanActionToolbar::SetButtonEnabled(const QString& id, bool enabled)
{
    if (auto* btn = Button(id)) {
        btn->setEnabled(enabled);
    }
}

auto NyanActionToolbar::IsButtonEnabled(const QString& id) const -> bool
{
    for (const auto& item : _items) {
        if (!item.isSeparator && item.id == id) {
            return item.button->isEnabled();
        }
    }
    return false;
}

// -- Orientation --

void NyanActionToolbar::SetOrientation(Qt::Orientation orientation)
{
    if (_orientation == orientation) { return; }
    _orientation = orientation;

    if (_orientation == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    } else {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }

    RebuildLayout();
}

auto NyanActionToolbar::Orientation() const -> Qt::Orientation
{
    return _orientation;
}

void NyanActionToolbar::RebuildLayout()
{
    _layout->setDirection(_orientation == Qt::Horizontal
                              ? QBoxLayout::LeftToRight
                              : QBoxLayout::TopToBottom);

    // Update separator sizes
    for (auto& item : _items) {
        if (item.isSeparator && item.separator != nullptr) {
            if (_orientation == Qt::Horizontal) {
                item.separator->setFixedSize(kSeparatorSize, kButtonSize);
            } else {
                item.separator->setFixedSize(kButtonSize, kSeparatorSize);
            }
        }
    }

    updateGeometry();
    update();
}

// -- Size hints --

auto NyanActionToolbar::sizeHint() const -> QSize
{
    int extent = 0;
    for (const auto& item : _items) {
        extent += item.isSeparator ? kSeparatorSize : kButtonSize;
    }
    int spacing = _items.size() > 0
                      ? static_cast<int>((_items.size() - 1)) * kSpacing
                      : 0;
    extent += spacing;

    if (_orientation == Qt::Horizontal) {
        return {extent, kButtonSize};
    }
    return {kButtonSize, extent};
}

auto NyanActionToolbar::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

void NyanActionToolbar::OnThemeChanged()
{
    UpdateButtonStyles();
    update();
}

void NyanActionToolbar::UpdateButtonStyles()
{
    const auto& theme = Theme();
    QString separatorColor = theme.Color(ColorToken::BorderStrong).name();

    for (auto& item : _items) {
        if (item.isSeparator && item.separator) {
            item.separator->setStyleSheet(QString("background-color: %1;").arg(separatorColor));
        }
    }
}

} // namespace matcha::gui
