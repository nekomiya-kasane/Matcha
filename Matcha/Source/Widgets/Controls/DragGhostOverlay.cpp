/**
 * @file DragGhostOverlay.cpp
 * @brief DragGhostOverlay renders a drag preview following the cursor.
 */

#include <Matcha/Widgets/Controls/DragGhostOverlay.h>

#include <QCursor>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QTimer>

namespace matcha::gui {

DragGhostOverlay::DragGhostOverlay(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , ThemeAware(WidgetKind::Notification)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setMouseTracking(false);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(8, 6, 8, 6);
    lay->setSpacing(6);

    _label = new QLabel(this);
    _label->setStyleSheet("color: white; font-size: 12px;");
    lay->addWidget(_label);

    _badge = new QLabel(this);
    _badge->setStyleSheet(
        "color: white; background: #e8a840; border-radius: 8px;"
        "font-size: 10px; font-weight: bold; padding: 2px 6px;");
    _badge->hide();
    lay->addWidget(_badge);

    _followTimer = new QTimer(this);
    _followTimer->setInterval(16); // ~60fps
    connect(_followTimer, &QTimer::timeout, this, &DragGhostOverlay::UpdatePosition);
}

DragGhostOverlay::~DragGhostOverlay()
{
    StopFollowing();
}

void DragGhostOverlay::SetConfig(const matcha::fw::DragPreviewConfig& config)
{
    _config = config;

    // Update label text
    if (!config.label.empty()) {
        _label->setText(QString::fromStdString(config.label));
        _label->show();
    } else {
        _label->setText("Drag item");
        _label->show();
    }

    // Update badge
    if (config.badgeCount > 0) {
        _badge->setText(QString::number(config.badgeCount));
        _badge->show();
    } else {
        _badge->hide();
    }

    // Apply opacity
    setWindowOpacity(config.opacity);

    // Apply size constraints
    setMaximumSize(config.maxWidth, config.maxHeight);
    adjustSize();
}

void DragGhostOverlay::StartFollowing()
{
    UpdatePosition();
    show();
    _followTimer->start();
}

void DragGhostOverlay::StopFollowing()
{
    _followTimer->stop();
    hide();
}

auto DragGhostOverlay::sizeHint() const -> QSize
{
    return {qMin(layout()->sizeHint().width(), _config.maxWidth),
            qMin(layout()->sizeHint().height(), _config.maxHeight)};
}

void DragGhostOverlay::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();

    // Background
    QColor bgColor;
    switch (_config.style) {
    case matcha::fw::DragPreviewStyle::Ghost:
        bgColor = theme.Color(ColorToken::SurfaceElevated);
        bgColor.setAlphaF(0.85);
        break;
    case matcha::fw::DragPreviewStyle::Icon:
        bgColor = theme.Color(ColorToken::Primary);
        bgColor.setAlphaF(0.9);
        break;
    case matcha::fw::DragPreviewStyle::Compact:
    case matcha::fw::DragPreviewStyle::Custom:
        bgColor = theme.Color(ColorToken::SurfaceElevated);
        break;
    }

    p.setPen(Qt::NoPen);
    p.setBrush(bgColor);
    p.drawRoundedRect(rect(), 6, 6);

    // Border
    p.setPen(QPen(theme.Color(ColorToken::BorderDefault), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 6, 6);
}

void DragGhostOverlay::OnThemeChanged()
{
    update();
}

void DragGhostOverlay::UpdatePosition()
{
    QPoint cursor = QCursor::pos();
    move(cursor.x() + _config.offsetX, cursor.y() + _config.offsetY);
}

} // namespace matcha::gui
