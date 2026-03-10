/**
 * @file NyanInputDialog.cpp
 * @brief Implementation of NyanInputDialog themed input prompt.
 */

#include <Matcha/Widgets/Menu/NyanInputDialog.h>

#include <Matcha/Widgets/Controls/NyanComboBox.h>
#include <Matcha/Widgets/Controls/NyanLabel.h>
#include <Matcha/Widgets/Controls/NyanLineEdit.h>
#include <Matcha/Widgets/Controls/NyanPushButton.h>

#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>

namespace matcha::gui {

namespace {
constexpr int kDialogPadding = 16;
constexpr int kButtonGap     = 8;
constexpr int kFieldGap      = 12;
constexpr int kMinWidth      = 300;
} // namespace

// ============================================================================
// Construction
// ============================================================================

NyanInputDialog::NyanInputDialog(QWidget* parent)
    : QDialog(parent)
    , ThemeAware(WidgetKind::Dialog)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumWidth(kMinWidth);
}

NyanInputDialog::~NyanInputDialog() = default;

// ============================================================================
// Static Methods
// ============================================================================

auto NyanInputDialog::GetText(
    QWidget* parent,
    const QString& title,
    const QString& label,
    const QString& defaultValue) -> std::optional<QString>
{
    NyanInputDialog dlg(parent);
    dlg.setWindowTitle(title);

    auto* mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setContentsMargins(kDialogPadding, kDialogPadding,
                                   kDialogPadding, kDialogPadding);
    mainLayout->setSpacing(kFieldGap);

    auto* lbl = new NyanLabel(&dlg);
    lbl->setText(label);
    lbl->SetRole(LabelRole::Name);
    mainLayout->addWidget(lbl);

    auto* edit = new NyanLineEdit(&dlg);
    edit->setText(defaultValue);
    mainLayout->addWidget(edit);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(kButtonGap);
    btnLayout->addStretch();

    auto* cancelBtn = new NyanPushButton(&dlg);
    cancelBtn->setText(QStringLiteral("Cancel"));
    cancelBtn->SetVariant(ButtonVariant::Secondary);
    cancelBtn->SetSize(ButtonSize::Small);
    btnLayout->addWidget(cancelBtn);

    auto* okBtn = new NyanPushButton(&dlg);
    okBtn->setText(QStringLiteral("OK"));
    okBtn->SetVariant(ButtonVariant::Primary);
    okBtn->SetSize(ButtonSize::Small);
    btnLayout->addWidget(okBtn);

    mainLayout->addLayout(btnLayout);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        return edit->text();
    }
    return std::nullopt;
}

auto NyanInputDialog::GetInt(
    QWidget* parent,
    const QString& title,
    const QString& label,
    int defaultValue,
    int minValue,
    int maxValue) -> std::optional<int>
{
    NyanInputDialog dlg(parent);
    dlg.setWindowTitle(title);

    auto* mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setContentsMargins(kDialogPadding, kDialogPadding,
                                   kDialogPadding, kDialogPadding);
    mainLayout->setSpacing(kFieldGap);

    auto* lbl = new NyanLabel(&dlg);
    lbl->setText(label);
    lbl->SetRole(LabelRole::Name);
    mainLayout->addWidget(lbl);

    auto* edit = new NyanLineEdit(&dlg);
    edit->SetInputMode(InputMode::Integer);
    edit->SetRange(minValue, maxValue);
    edit->setText(QString::number(defaultValue));
    mainLayout->addWidget(edit);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(kButtonGap);
    btnLayout->addStretch();

    auto* cancelBtn = new NyanPushButton(&dlg);
    cancelBtn->setText(QStringLiteral("Cancel"));
    cancelBtn->SetVariant(ButtonVariant::Secondary);
    cancelBtn->SetSize(ButtonSize::Small);
    btnLayout->addWidget(cancelBtn);

    auto* okBtn = new NyanPushButton(&dlg);
    okBtn->setText(QStringLiteral("OK"));
    okBtn->SetVariant(ButtonVariant::Primary);
    okBtn->SetSize(ButtonSize::Small);
    btnLayout->addWidget(okBtn);

    mainLayout->addLayout(btnLayout);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        auto val = edit->NumericValue();
        if (val.has_value()) {
            return static_cast<int>(val.value());
        }
        return defaultValue;
    }
    return std::nullopt;
}

auto NyanInputDialog::GetItem(
    QWidget* parent,
    const QString& title,
    const QString& label,
    const std::vector<QString>& items,
    int currentIndex) -> std::optional<QString>
{
    NyanInputDialog dlg(parent);
    dlg.setWindowTitle(title);

    auto* mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setContentsMargins(kDialogPadding, kDialogPadding,
                                   kDialogPadding, kDialogPadding);
    mainLayout->setSpacing(kFieldGap);

    auto* lbl = new NyanLabel(&dlg);
    lbl->setText(label);
    lbl->SetRole(LabelRole::Name);
    mainLayout->addWidget(lbl);

    auto* combo = new NyanComboBox(&dlg);
    for (const auto& item : items) {
        combo->addItem(item);
    }
    if (currentIndex >= 0 && currentIndex < static_cast<int>(items.size())) {
        combo->setCurrentIndex(currentIndex);
    }
    mainLayout->addWidget(combo);

    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(kButtonGap);
    btnLayout->addStretch();

    auto* cancelBtn = new NyanPushButton(&dlg);
    cancelBtn->setText(QStringLiteral("Cancel"));
    cancelBtn->SetVariant(ButtonVariant::Secondary);
    cancelBtn->SetSize(ButtonSize::Small);
    btnLayout->addWidget(cancelBtn);

    auto* okBtn = new NyanPushButton(&dlg);
    okBtn->setText(QStringLiteral("OK"));
    okBtn->SetVariant(ButtonVariant::Primary);
    okBtn->SetSize(ButtonSize::Small);
    btnLayout->addWidget(okBtn);

    mainLayout->addLayout(btnLayout);

    QObject::connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        return combo->currentText();
    }
    return std::nullopt;
}

// ============================================================================
// Painting
// ============================================================================

void NyanInputDialog::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    const auto style = Theme().Resolve(WidgetKind::Tooltip, 0, InteractionState::Normal);

    // Shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 40));
    p.drawRoundedRect(rect().adjusted(3, 3, 3, 3), style.radiusPx, style.radiusPx);

    // Background
    p.setPen(QPen(style.border, style.borderWidthPx));
    p.setBrush(style.background);
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), style.radiusPx, style.radiusPx);
}

void NyanInputDialog::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
