#include <Matcha/Widgets/Controls/NyanNotification.h>

#include "../Core/InteractionEventFilter.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>

namespace matcha::gui {

// ============================================================================
// NyanNotification
// ============================================================================

NyanNotification::NyanNotification(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , ThemeAware(WidgetKind::Notification)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedWidth(kWidth);

    InitLayout();

    _dismissTimer = new QTimer(this);
    _dismissTimer->setSingleShot(true);
    connect(_dismissTimer, &QTimer::timeout, this, &NyanNotification::OnDismissTimeout);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanNotification::~NyanNotification() = default;

void NyanNotification::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(kPadding, kPadding, kPadding, kPadding);
    _layout->setSpacing(8);

    // Icon
    _iconLabel = new QLabel(this);
    _iconLabel->setFixedSize(kIconSize, kIconSize);
    _layout->addWidget(_iconLabel);

    // Message
    _messageLabel = new QLabel(this);
    _messageLabel->setWordWrap(true);
    _messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _layout->addWidget(_messageLabel, 1);

    // Action button (hidden by default)
    _actionButton = new QPushButton(this);
    _actionButton->setFlat(true);
    _actionButton->setCursor(Qt::PointingHandCursor);
    _actionButton->hide();
    connect(_actionButton, &QPushButton::clicked, this, [this]() {
        if (_actionCallback) {
            _actionCallback();
        }
        Q_EMIT ActionClicked();
    });
    _layout->addWidget(_actionButton);

    // Close button
    _closeButton = new QPushButton("×", this);
    _closeButton->setFixedSize(20, 20);
    _closeButton->setFlat(true);
    _closeButton->setCursor(Qt::PointingHandCursor);
    connect(_closeButton, &QPushButton::clicked, this, &NyanNotification::OnCloseClicked);
    _layout->addWidget(_closeButton);

    UpdateIcon();
}

// -- Configuration --

void NyanNotification::SetMessage(const QString& message)
{
    _message = message;
    _messageLabel->setText(message);
    adjustSize();
}

auto NyanNotification::Message() const -> QString
{
    return _message;
}

void NyanNotification::SetType(NotificationType type)
{
    if (_type != type) {
        _type = type;
        UpdateIcon();
        update();
    }
}

auto NyanNotification::Type() const -> NotificationType
{
    return _type;
}

void NyanNotification::SetDuration(std::chrono::milliseconds duration)
{
    _duration = duration;
}

auto NyanNotification::Duration() const -> std::chrono::milliseconds
{
    return _duration;
}

void NyanNotification::SetAction(const QString& text, std::function<void()> callback)
{
    _actionCallback = std::move(callback);
    _actionButton->setText(text);
    _actionButton->show();

    const auto& theme = Theme();
    QString style = QString(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  color: %1;"
        "  font-weight: bold;"
        "  padding: 4px 8px;"
        "}"
        "QPushButton:hover {"
        "  text-decoration: underline;"
        "}"
    ).arg(theme.Color(ColorToken::Primary).name());
    _actionButton->setStyleSheet(style);
}

void NyanNotification::ClearAction()
{
    _actionCallback = nullptr;
    _actionButton->hide();
}

// -- Display --

void NyanNotification::ShowAt(const QPoint& globalPos)
{
    // Position off-screen initially for slide-in
    _slideOffset = -width();
    move(globalPos.x() + _slideOffset, globalPos.y());

    show();
    StartShowAnimation();

    // Start dismiss timer
    if (_duration.count() > 0) {
        _dismissTimer->start(static_cast<int>(_duration.count()));
    }
}

void NyanNotification::Dismiss()
{
    _dismissTimer->stop();
    StartDismissAnimation();
}

// -- Animation Property --

auto NyanNotification::SlideOffset() const -> int
{
    return _slideOffset;
}

void NyanNotification::SetSlideOffset(int offset)
{
    _slideOffset = offset;
    // Move widget based on offset
    QPoint pos = this->pos();
    pos.setX(pos.x() - _slideOffset + offset);
    move(pos);
    _slideOffset = offset;
}

// -- Size hints --

auto NyanNotification::sizeHint() const -> QSize
{
    return {kWidth, qMax(kMinHeight, _layout->sizeHint().height())};
}

auto NyanNotification::minimumSizeHint() const -> QSize
{
    return {kWidth, kMinHeight};
}

// -- Paint --

void NyanNotification::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();

    // Background with shadow effect
    QRect bgRect = rect().adjusted(2, 2, -2, -2);

    // Shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 30));
    p.drawRoundedRect(bgRect.adjusted(2, 2, 2, 2), kRadius, kRadius);

    // Background
    p.setBrush(theme.Color(ColorToken::SurfaceElevated));
    p.setPen(QPen(TypeColor(), 2));
    p.drawRoundedRect(bgRect, kRadius, kRadius);

    // Left accent bar
    QRect accentRect(bgRect.left(), bgRect.top() + kRadius, 3, bgRect.height() - (2 * kRadius));
    p.fillRect(accentRect, TypeColor());
}

void NyanNotification::OnThemeChanged()
{
    UpdateIcon();

    const auto& theme = Theme();

    // Update message label style
    _messageLabel->setStyleSheet(QString("color: %1;").arg(theme.Color(ColorToken::TextPrimary).name()));

    // Update close button style
    QString closeStyle = QString(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  color: %1;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  color: %2;"
        "}"
    ).arg(theme.Color(ColorToken::TextTertiary).name(),
          theme.Color(ColorToken::TextPrimary).name());
    _closeButton->setStyleSheet(closeStyle);

    update();
}

void NyanNotification::UpdateIcon()
{
    // Use colored square as placeholder (proper icons in Phase 4)
    QPixmap pixmap(kIconSize, kIconSize);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(TypeColor());
    p.setPen(Qt::NoPen);

    switch (_type) {
    case NotificationType::Success:
        // Checkmark circle
        p.drawEllipse(0, 0, kIconSize, kIconSize);
        p.setPen(QPen(Qt::white, 2));
        p.drawLine(5, 10, 8, 14);
        p.drawLine(8, 14, 15, 6);
        break;

    case NotificationType::Warning:
        // Triangle
        {
            QPolygonF triangle;
            triangle << QPointF(kIconSize / 2.0, 2)
                     << QPointF(kIconSize - 2, kIconSize - 2)
                     << QPointF(2, kIconSize - 2);
            p.drawPolygon(triangle);
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(kIconSize / 2, 7, kIconSize / 2, 12);
            p.drawPoint(kIconSize / 2, 15);
        }
        break;

    case NotificationType::Error:
        // X circle
        p.drawEllipse(0, 0, kIconSize, kIconSize);
        p.setPen(QPen(Qt::white, 2));
        p.drawLine(6, 6, 14, 14);
        p.drawLine(14, 6, 6, 14);
        break;

    case NotificationType::Info:
    default:
        // Info circle
        p.drawEllipse(0, 0, kIconSize, kIconSize);
        p.setPen(QPen(Qt::white, 2));
        p.drawLine(kIconSize / 2, 7, kIconSize / 2, 14);
        p.drawPoint(kIconSize / 2, 5);
        break;
    }

    _iconLabel->setPixmap(pixmap);
}

void NyanNotification::StartShowAnimation()
{
    if (_slideAnimation) {
        _slideAnimation->stop();
        delete _slideAnimation;
    }

    _slideAnimation = new QPropertyAnimation(this, "pos", this);
    _slideAnimation->setEasingCurve(QEasingCurve::OutCubic);
    _slideAnimation->setDuration(kAnimationDuration);

    QPoint startPos = pos();
    QPoint endPos = startPos;
    endPos.setX(startPos.x() - _slideOffset);

    _slideAnimation->setStartValue(startPos);
    _slideAnimation->setEndValue(endPos);
    _slideAnimation->start();

    _slideOffset = 0;
}

void NyanNotification::StartDismissAnimation()
{
    if (_slideAnimation) {
        _slideAnimation->stop();
        delete _slideAnimation;
    }

    _slideAnimation = new QPropertyAnimation(this, "pos", this);
    _slideAnimation->setEasingCurve(QEasingCurve::InCubic);
    _slideAnimation->setDuration(kAnimationDuration);

    QPoint startPos = pos();
    QPoint endPos = startPos;
    endPos.setX(startPos.x() + width() + 20);

    _slideAnimation->setStartValue(startPos);
    _slideAnimation->setEndValue(endPos);

    connect(_slideAnimation, &QPropertyAnimation::finished, this, [this]() {
        hide();
        Q_EMIT Dismissed();
    });

    _slideAnimation->start();
}

void NyanNotification::OnDismissTimeout()
{
    Dismiss();
}

void NyanNotification::OnCloseClicked()
{
    Dismiss();
}

auto NyanNotification::TypeColor() const -> QColor
{
    const auto& theme = Theme();

    switch (_type) {
    case NotificationType::Success:
        return theme.Color(ColorToken::Success);
    case NotificationType::Warning:
        return theme.Color(ColorToken::Warning);
    case NotificationType::Error:
        return theme.Color(ColorToken::Error);
    case NotificationType::Info:
    default:
        return theme.Color(ColorToken::Primary);
    }
}

auto NyanNotification::TypeIcon() const -> QString
{
    switch (_type) {
    case NotificationType::Success:
        return "✓";
    case NotificationType::Warning:
        return "⚠";
    case NotificationType::Error:
        return "✕";
    case NotificationType::Info:
    default:
        return "ℹ";
    }
}

// ============================================================================
// NyanNotificationManager
// ============================================================================

NyanNotificationManager::NyanNotificationManager(QWidget* parent)
    : QObject(parent)
    , _parent(parent)
{
}

NyanNotificationManager::~NyanNotificationManager()
{
    DismissAll();
}

auto NyanNotificationManager::Show(const QString& message,
                                    NotificationType type,
                                    std::chrono::milliseconds duration) -> NyanNotification*
{
    auto* notification = new NyanNotification(_parent);
    notification->SetMessage(message);
    notification->SetType(type);
    notification->SetDuration(duration);

    connect(notification, &NyanNotification::Dismissed, this, &NyanNotificationManager::OnNotificationDismissed);

    if (_visible.size() < NyanNotification::kMaxVisible) {
        _visible.append(notification);
        RepositionNotifications();

        // Calculate position for this notification
        QPoint pos = _parent->mapToGlobal(_parent->rect().topRight());
        pos.setX(pos.x() - notification->width() - 20);
        int yOffset = 20;
        for (int i = 0; i < _visible.size() - 1; ++i) {
            yOffset += _visible[i]->height() + 10;
        }
        pos.setY(pos.y() + yOffset);

        notification->ShowAt(pos);
    } else {
        _queued.append(notification);
    }

    return notification;
}

auto NyanNotificationManager::ShowWithAction(const QString& message,
                                              const QString& actionText,
                                              std::function<void()> actionCallback,
                                              NotificationType type,
                                              std::chrono::milliseconds duration) -> NyanNotification*
{
    auto* notification = Show(message, type, duration);
    notification->SetAction(actionText, std::move(actionCallback));
    return notification;
}

void NyanNotificationManager::DismissAll()
{
    for (auto* notification : _visible) {
        notification->Dismiss();
    }
    for (auto* notification : _queued) {
        delete notification;
    }
    _queued.clear();
}

auto NyanNotificationManager::VisibleCount() const -> int
{
    return static_cast<int>(_visible.size());
}

void NyanNotificationManager::OnNotificationDismissed()
{
    auto* notification = qobject_cast<NyanNotification*>(sender());
    if (notification) {
        _visible.removeOne(notification);
        notification->deleteLater();

        RepositionNotifications();
        ShowNextQueued();
    }
}

void NyanNotificationManager::RepositionNotifications()
{
    QPoint basePos = _parent->mapToGlobal(_parent->rect().topRight());
    basePos.setX(basePos.x() - 20);

    int yOffset = 20;
    for (auto* notification : _visible) {
        QPoint pos(basePos.x() - notification->width(), basePos.y() + yOffset);
        notification->move(pos);
        yOffset += notification->height() + 10;
    }
}

void NyanNotificationManager::ShowNextQueued()
{
    if (_queued.isEmpty() || _visible.size() >= NyanNotification::kMaxVisible) {
        return;
    }

    auto* notification = _queued.takeFirst();
    _visible.append(notification);

    QPoint pos = _parent->mapToGlobal(_parent->rect().topRight());
    pos.setX(pos.x() - notification->width() - 20);
    int yOffset = 20;
    for (int i = 0; i < _visible.size() - 1; ++i) {
        yOffset += _visible[i]->height() + 10;
    }
    pos.setY(pos.y() + yOffset);

    notification->ShowAt(pos);
}

} // namespace matcha::gui