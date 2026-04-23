/**
 * @file NyanPopConfirm.cpp
 * @brief Implementation of NyanPopConfirm themed confirmation popup / dialog.
 */

#include <Matcha/Widgets/Menu/NyanPopConfirm.h>

#include <Matcha/Widgets/Controls/NyanLabel.h>
#include <Matcha/Widgets/Controls/NyanPushButton.h>

#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanPopConfirm::NyanPopConfirm(QWidget* parent)
    : QDialog(parent)
    , ThemeAware(WidgetKind::PopConfirm)
{
    InitLayout();
    _titleLabel->setText(tr("Confirm"));
}

NyanPopConfirm::NyanPopConfirm(const QString& title,
                               PopConfirmState state,
                               QWidget* parent)
    : QDialog(parent)
    , ThemeAware(WidgetKind::PopConfirm)
    , _state(state)
{
    InitLayout();
    _titleLabel->setText(title);
    ApplyStateIcon();
}

NyanPopConfirm::~NyanPopConfirm() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanPopConfirm::SetTitle(const QString& title)
{
    _titleLabel->setText(title);
}

auto NyanPopConfirm::Title() const -> QString
{
    return _titleLabel->text();
}

void NyanPopConfirm::SetState(PopConfirmState state)
{
    _state = state;
    ApplyStateIcon();
}

auto NyanPopConfirm::State() const -> PopConfirmState { return _state; }

void NyanPopConfirm::SetMessage(const QString& message)
{
    _messageLabel->setText(message);
    updateGeometry();
    update();
}

void NyanPopConfirm::SetMessage(const QStringList& messages)
{
    QString joined;
    for (int i = 0; i < messages.count(); ++i) {
        joined += messages[i];
        if (i < messages.count() - 1) {
            joined += QChar('\n');
        }
    }
    SetMessage(joined);
}

auto NyanPopConfirm::Message() const -> QString
{
    return _messageLabel->text();
}

void NyanPopConfirm::SetButtonText(const QString& text, PopConfirmButton button)
{
    switch (button) {
        case PopConfirmButton::Confirm: _confirmBtn->setText(text); break;
        case PopConfirmButton::Deny:    _denyBtn->setText(text);    break;
        case PopConfirmButton::Cancel:  _cancelBtn->setText(text);  break;
        default: break;
    }
}

void NyanPopConfirm::SetButtonVisible(bool visible, PopConfirmButton button)
{
    switch (button) {
        case PopConfirmButton::Confirm: _confirmBtn->setVisible(visible); break;
        case PopConfirmButton::Deny:    _denyBtn->setVisible(visible);    break;
        case PopConfirmButton::Cancel:  _cancelBtn->setVisible(visible);  break;
        default: break;
    }
}

void NyanPopConfirm::SetArrowPosition(ArrowPosition pos)
{
    _arrowPos = pos;
    update();
}

auto NyanPopConfirm::GetArrowPosition() const -> ArrowPosition { return _arrowPos; }

void NyanPopConfirm::ShowAt(QWidget* anchor)
{
    if (anchor != nullptr) {
        PositionRelativeTo(anchor);
    }
    show();
}

auto NyanPopConfirm::PopConfirm() -> PopConfirmCode
{
    exec();
    return _resultCode;
}

auto NyanPopConfirm::sizeHint() const -> QSize
{
    return QDialog::sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanPopConfirm::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                     | QPainter::SmoothPixmapTransform);

    const auto style = Theme().Resolve(WidgetKind::Tooltip, 0, InteractionState::Normal);
    const int shadowInset = 6;

    QRect drawRect = rect();

    // -- Shadow (matching old NyanDrawTool::DrawEffectShadow style) --
    p.setPen(Qt::NoPen);
    for (int i = 0; i < shadowInset; ++i) {
        const int alpha = 15 - (i * 2);
        p.setBrush(QColor(0, 0, 0, std::max(alpha, 2)));
        p.drawRect(drawRect.adjusted(i, i, -i, -i));
    }

    // -- Background --
    QRect bodyRect = drawRect.adjusted(shadowInset, shadowInset,
                                       -shadowInset, -shadowInset);
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    p.drawRect(bodyRect);
}

void NyanPopConfirm::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private
// ============================================================================

void NyanPopConfirm::InitLayout()
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedWidth(kFixedWidth);

    // -- Create widgets --
    _iconLabel = new NyanLabel( this);
    _iconLabel->setFixedSize(kIconSize, kIconSize);
    _iconLabel->setScaledContents(true);

    _titleLabel = new NyanLabel( this);
    _titleLabel->SetRole(LabelRole::Name);

    _messageLabel = new NyanLabel( this);
    _messageLabel->SetRole(LabelRole::Body);
    _messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _messageLabel->setWordWrap(true);

    _confirmBtn = new NyanPushButton( this);
    _confirmBtn->setText(tr("OK"));
    _confirmBtn->SetVariant(ButtonVariant::Primary);
    _confirmBtn->SetSize(ButtonSize::Small);

    _denyBtn = new NyanPushButton( this);
    _denyBtn->setText(tr("No"));
    _denyBtn->SetVariant(ButtonVariant::Secondary);
    _denyBtn->SetSize(ButtonSize::Small);
    _denyBtn->setVisible(false);

    _cancelBtn = new NyanPushButton( this);
    _cancelBtn->setText(tr("Cancel"));
    _cancelBtn->SetVariant(ButtonVariant::Secondary);
    _cancelBtn->SetSize(ButtonSize::Small);

    _closeBtn = new NyanPushButton( this);
    _closeBtn->SetVariant(ButtonVariant::Ghost);
    _closeBtn->SetSize(ButtonSize::Small);
    _closeBtn->setText(QStringLiteral("X"));

    // -- Layouts --
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(kButtonGap);
    mainLayout->setContentsMargins(kPadding, kPadding, kPadding, kPadding);

    // Title row: [icon] [title] ---stretch--- [close]
    auto* titleLayout = new QHBoxLayout;
    titleLayout->setSpacing(kButtonGap);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->addWidget(_iconLabel);
    titleLayout->addWidget(_titleLabel);
    titleLayout->addStretch(1);
    titleLayout->addWidget(_closeBtn);
    mainLayout->addLayout(titleLayout);

    // Message body
    mainLayout->addWidget(_messageLabel, 1);

    // Button row: ---stretch--- [confirm] [deny] [cancel]
    auto* btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(kButtonGap);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch(1);
    btnLayout->addWidget(_confirmBtn);
    btnLayout->addWidget(_denyBtn);
    btnLayout->addWidget(_cancelBtn);
    mainLayout->addLayout(btnLayout);

    // -- Connections --
    connect(_confirmBtn, &QPushButton::clicked, this, [this]() {
        _resultCode = PopConfirmCode::ConfirmCode;
        emit Confirmed();
        done(1);
    });

    connect(_denyBtn, &QPushButton::clicked, this, [this]() {
        _resultCode = PopConfirmCode::DenyCode;
        emit Denied();
        done(1);
    });

    connect(_cancelBtn, &QPushButton::clicked, this, [this]() {
        _resultCode = PopConfirmCode::CancelCode;
        emit Cancelled();
        done(0);
    });

    connect(_closeBtn, &QPushButton::clicked, this, [this]() {
        _resultCode = PopConfirmCode::CancelCode;
        emit Cancelled();
        done(0);
    });

    ApplyStateIcon();
}

void NyanPopConfirm::ApplyStateIcon()
{
    // State icon: use themed color tokens to paint a simple indicator.
    // In production, these would be SVG icons loaded from the resource system.
    // For now, set a colored square as a placeholder until the icon resource
    // system (Phase 4) is available.
    QPixmap px(kIconSize, kIconSize);
    px.fill(Qt::transparent);

    QPainter ip(&px);
    ip.setRenderHint(QPainter::Antialiasing);
    ip.setPen(Qt::NoPen);

    switch (_state) {
        case PopConfirmState::Warn:
            ip.setBrush(Theme().Color(ColorToken::colorWarning));
            break;
        case PopConfirmState::Error:
            ip.setBrush(Theme().Color(ColorToken::colorError));
            break;
        case PopConfirmState::Info:
        case PopConfirmState::Question:
        default:
            ip.setBrush(Theme().Color(ColorToken::colorPrimary));
            break;
    }

    ip.drawEllipse(0, 0, kIconSize, kIconSize);
    ip.end();

    _iconLabel->setPixmap(px);
}

void NyanPopConfirm::PositionRelativeTo(QWidget* anchor)
{
    const QPoint anchorGlobal = anchor->mapToGlobal(QPoint(0, 0));
    const QSize mySize = sizeHint();
    resize(mySize);

    const int x = anchorGlobal.x() + ((anchor->width() - mySize.width()) / 2);
    const int y = anchorGlobal.y() + anchor->height() + 4;

    move(x, y);
}

} // namespace matcha::gui
