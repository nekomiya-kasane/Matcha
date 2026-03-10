#include <Matcha/Widgets/Menu/NyanDialogFootBar.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>

namespace matcha::gui {

NyanDialogFootBar::NyanDialogFootBar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Dialog)
{
    setFixedHeight(kHeight);
    InitLayout();
    UpdateButtonStyles();
}

NyanDialogFootBar::~NyanDialogFootBar() = default;

void NyanDialogFootBar::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(12, 8, 12, 8);
    _layout->setSpacing(8);

    // Spacer for custom widget area
    _layout->addStretch(1);

    // Confirm button
    _confirmButton = new QPushButton(tr("Confirm"), this);
    _confirmButton->setMinimumWidth(kButtonWidth);
    _confirmButton->setCursor(Qt::PointingHandCursor);
    connect(_confirmButton, &QPushButton::clicked, this, [this]() {
        Q_EMIT ConfirmClicked();
        Q_EMIT ButtonClicked(DialogButtonCode::Confirm);
    });
    _layout->addWidget(_confirmButton);

    // Apply button
    _applyButton = new QPushButton(tr("Apply"), this);
    _applyButton->setMinimumWidth(kButtonWidth);
    _applyButton->setCursor(Qt::PointingHandCursor);
    _applyButton->hide();
    connect(_applyButton, &QPushButton::clicked, this, [this]() {
        Q_EMIT ApplyClicked();
        Q_EMIT ButtonClicked(DialogButtonCode::Apply);
    });
    _layout->addWidget(_applyButton);

    // Cancel button
    _cancelButton = new QPushButton(tr("Cancel"), this);
    _cancelButton->setMinimumWidth(kButtonWidth);
    _cancelButton->setCursor(Qt::PointingHandCursor);
    connect(_cancelButton, &QPushButton::clicked, this, [this]() {
        Q_EMIT CancelClicked();
        Q_EMIT ButtonClicked(DialogButtonCode::Cancel);
    });
    _layout->addWidget(_cancelButton);
}

// -- Confirm button --

void NyanDialogFootBar::SetConfirmText(const QString& text)
{
    _confirmButton->setText(text);
}

auto NyanDialogFootBar::ConfirmText() const -> QString
{
    return _confirmButton->text();
}

void NyanDialogFootBar::SetConfirmVisible(bool visible)
{
    _confirmButton->setVisible(visible);
}

auto NyanDialogFootBar::IsConfirmVisible() const -> bool
{
    return _confirmButton->isVisible();
}

void NyanDialogFootBar::SetConfirmEnabled(bool enabled)
{
    _confirmButton->setEnabled(enabled);
}

auto NyanDialogFootBar::IsConfirmEnabled() const -> bool
{
    return _confirmButton->isEnabled();
}

// -- Apply button --

void NyanDialogFootBar::SetApplyText(const QString& text)
{
    _applyButton->setText(text);
}

auto NyanDialogFootBar::ApplyText() const -> QString
{
    return _applyButton->text();
}

void NyanDialogFootBar::SetApplyVisible(bool visible)
{
    _applyButton->setVisible(visible);
}

auto NyanDialogFootBar::IsApplyVisible() const -> bool
{
    return _applyButton->isVisible();
}

void NyanDialogFootBar::SetApplyEnabled(bool enabled)
{
    _applyButton->setEnabled(enabled);
}

auto NyanDialogFootBar::IsApplyEnabled() const -> bool
{
    return _applyButton->isEnabled();
}

// -- Cancel button --

void NyanDialogFootBar::SetCancelText(const QString& text)
{
    _cancelButton->setText(text);
}

auto NyanDialogFootBar::CancelText() const -> QString
{
    return _cancelButton->text();
}

void NyanDialogFootBar::SetCancelVisible(bool visible)
{
    _cancelButton->setVisible(visible);
}

auto NyanDialogFootBar::IsCancelVisible() const -> bool
{
    return _cancelButton->isVisible();
}

void NyanDialogFootBar::SetCancelEnabled(bool enabled)
{
    _cancelButton->setEnabled(enabled);
}

auto NyanDialogFootBar::IsCancelEnabled() const -> bool
{
    return _cancelButton->isEnabled();
}

// -- Custom widget --

void NyanDialogFootBar::SetCustomWidget(QWidget* widget)
{
    if (_customWidget) {
        _layout->removeWidget(_customWidget);
    }

    _customWidget = widget;

    if (_customWidget) {
        _layout->insertWidget(0, _customWidget);
    }
}

auto NyanDialogFootBar::CustomWidget() const -> QWidget*
{
    return _customWidget;
}

// -- Size hints --

auto NyanDialogFootBar::sizeHint() const -> QSize
{
    return {300, kHeight};
}

auto NyanDialogFootBar::minimumSizeHint() const -> QSize
{
    return {200, kHeight};
}

// -- Paint --

void NyanDialogFootBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();

    // Background
    p.fillRect(rect(), theme.Color(ColorToken::FillHover));

    // Top border
    p.setPen(theme.Color(ColorToken::BorderStrong));
    p.drawLine(0, 0, width(), 0);
}

void NyanDialogFootBar::OnThemeChanged()
{
    UpdateButtonStyles();
    update();
}

void NyanDialogFootBar::UpdateButtonStyles()
{
    const auto& theme = Theme();

    // Confirm button (primary style)
    QString confirmStyle = QString(
        "QPushButton {"
        "  background-color: %1;"
        "  border: none;"
        "  border-radius: 4px;"
        "  color: white;"
        "  padding: 6px 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: %2; }"
        "QPushButton:pressed { background-color: %3; }"
        "QPushButton:disabled { background-color: %4; color: %5; }"
    ).arg(theme.Color(ColorToken::Primary).name(),
          theme.Color(ColorToken::PrimaryHover).name(),
          theme.Color(ColorToken::PrimaryActive).name(),
          theme.Color(ColorToken::PrimaryBorderHover).name(),
          theme.Color(ColorToken::TextTertiary).name());
    _confirmButton->setStyleSheet(confirmStyle);

    // Apply button (secondary style)
    QString applyStyle = QString(
        "QPushButton {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 4px;"
        "  color: %3;"
        "  padding: 6px 16px;"
        "}"
        "QPushButton:hover { background-color: %4; }"
        "QPushButton:disabled { background-color: %5; color: %6; }"
    ).arg(theme.Color(ColorToken::SurfaceElevated).name(),
          theme.Color(ColorToken::BorderStrong).name(),
          theme.Color(ColorToken::TextPrimary).name(),
          theme.Color(ColorToken::FillActive).name(),
          theme.Color(ColorToken::FillHover).name(),
          theme.Color(ColorToken::TextTertiary).name());
    _applyButton->setStyleSheet(applyStyle);

    // Cancel button (secondary style)
    _cancelButton->setStyleSheet(applyStyle);
}

} // namespace matcha::gui
