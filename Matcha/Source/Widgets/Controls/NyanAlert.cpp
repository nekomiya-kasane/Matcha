/**
 * @file NyanAlert.cpp
 * @brief Implementation of NyanAlert blocking alert dialog.
 */

#include <Matcha/Widgets/Controls/NyanAlert.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

namespace matcha::gui {

NyanAlert::NyanAlert(QWidget* parent)
    : QDialog(parent)
    , ThemeAware(WidgetKind::Alert)
{
    SetupUi();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanAlert::~NyanAlert() = default;

auto NyanAlert::Show(AlertType type,
                     std::string_view title, std::string_view message,
                     QWidget* parent) -> AlertButton
{
    NyanAlert dlg(parent);
    dlg.SetAlertType(type);
    dlg.SetTitle(title);
    dlg.SetMessage(message);

    switch (type) {
    case AlertType::Info:
    case AlertType::Warning:
    case AlertType::Error:
        dlg._okBtn->setVisible(true);
        dlg._cancelBtn->setVisible(false);
        dlg._yesBtn->setVisible(false);
        dlg._noBtn->setVisible(false);
        break;
    case AlertType::Confirm:
        dlg._okBtn->setVisible(false);
        dlg._cancelBtn->setVisible(false);
        dlg._yesBtn->setVisible(true);
        dlg._noBtn->setVisible(true);
        break;
    }

    AlertButton result = AlertButton::Cancel;
    connect(dlg._okBtn, &QPushButton::clicked, &dlg, [&] { result = AlertButton::Ok; dlg.accept(); });
    connect(dlg._yesBtn, &QPushButton::clicked, &dlg, [&] { result = AlertButton::Yes; dlg.accept(); });
    connect(dlg._noBtn, &QPushButton::clicked, &dlg, [&] { result = AlertButton::No; dlg.reject(); });
    connect(dlg._cancelBtn, &QPushButton::clicked, &dlg, [&] { result = AlertButton::Cancel; dlg.reject(); });

    dlg.exec();
    return result;
}

auto NyanAlert::Confirm(
                        std::string_view title, std::string_view message,
                        QWidget* parent) -> bool
{
    return Show(AlertType::Confirm, title, message, parent) == AlertButton::Yes;
}

void NyanAlert::SetAlertType(AlertType type)
{
    _alertType = type;
    UpdateStyle();
}

void NyanAlert::SetTitle(std::string_view title)
{
    _titleLabel->setText(QString::fromUtf8(title.data(), static_cast<int>(title.size())));
}

void NyanAlert::SetMessage(std::string_view message)
{
    _messageLabel->setText(QString::fromUtf8(message.data(), static_cast<int>(message.size())));
}

void NyanAlert::paintEvent(QPaintEvent* event)
{
    QDialog::paintEvent(event);
    const auto style = Theme().Resolve(WidgetKind::Alert, 0, InteractionState::Normal);
    QPainter p(this);
    p.fillRect(rect(), style.background);
}

void NyanAlert::OnThemeChanged()
{
    UpdateStyle();
    update();
}

void NyanAlert::SetupUi()
{
    setMinimumWidth(300);
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    auto* headerLayout = new QHBoxLayout;
    _iconLabel = new QLabel(this);
    _iconLabel->setFixedSize(24, 24);
    headerLayout->addWidget(_iconLabel);

    _titleLabel = new QLabel(this);
    auto f = _titleLabel->font();
    f.setBold(true);
    f.setPointSize(11);
    _titleLabel->setFont(f);
    headerLayout->addWidget(_titleLabel, 1);
    mainLayout->addLayout(headerLayout);

    _messageLabel = new QLabel(this);
    _messageLabel->setWordWrap(true);
    mainLayout->addWidget(_messageLabel);

    mainLayout->addStretch();

    auto* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    _okBtn = new QPushButton(tr("OK"), this);
    _cancelBtn = new QPushButton(tr("Cancel"), this);
    _yesBtn = new QPushButton(tr("Yes"), this);
    _noBtn = new QPushButton(tr("No"), this);
    btnLayout->addWidget(_okBtn);
    btnLayout->addWidget(_yesBtn);
    btnLayout->addWidget(_noBtn);
    btnLayout->addWidget(_cancelBtn);
    mainLayout->addLayout(btnLayout);

    _okBtn->setVisible(false);
    _cancelBtn->setVisible(false);
    _yesBtn->setVisible(false);
    _noBtn->setVisible(false);
}

void NyanAlert::UpdateStyle()
{
    update();
}

} // namespace matcha::gui