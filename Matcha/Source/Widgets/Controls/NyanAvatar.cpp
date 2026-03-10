/**
 * @file NyanAvatar.cpp
 * @brief Implementation of NyanAvatar user avatar display.
 */

#include <Matcha/Widgets/Controls/NyanAvatar.h>

#include "../Core/InteractionEventFilter.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace matcha::gui {

NyanAvatar::NyanAvatar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Avatar)
{
    setFixedSize(static_cast<int>(_size), static_cast<int>(_size));
    setCursor(Qt::PointingHandCursor);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanAvatar::~NyanAvatar() = default;

void NyanAvatar::SetImage(const QPixmap& image)
{
    _image = image;
    update();
}

void NyanAvatar::SetInitials(std::string_view initials)
{
    _initials.assign(initials);
    update();
}

void NyanAvatar::SetSize(AvatarSize size)
{
    _size = size;
    int px = static_cast<int>(size);
    setFixedSize(px, px);
    update();
}

void NyanAvatar::SetOnlineStatus(OnlineStatus status)
{
    _status = status;
    update();
}

auto NyanAvatar::sizeHint() const -> QSize
{
    int px = static_cast<int>(_size);
    return {px, px};
}

auto NyanAvatar::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

void NyanAvatar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto style = Theme().Resolve(WidgetKind::Avatar, 0, InteractionState::Normal);

    int sz = static_cast<int>(_size);
    QRectF circleRect(0, 0, sz, sz);

    // Clip to circle
    QPainterPath clipPath;
    clipPath.addEllipse(circleRect);
    p.setClipPath(clipPath);

    if (!_image.isNull()) {
        // Draw scaled image
        p.drawPixmap(0, 0, sz, sz, _image);
    } else {
        // Draw initials fallback
        p.fillRect(rect(), style.background);
        p.setPen(style.foreground);
        QFont f = style.font;
        f.setPointSize(sz / 3);
        f.setBold(true);
        p.setFont(f);
        p.drawText(circleRect.toRect(), Qt::AlignCenter,
                   QString::fromStdString(_initials));
    }

    p.setClipping(false);

    // Online status indicator dot (semantic colors — not in variant matrix)
    if (_status != OnlineStatus::None) {
        int dotSize = sz / 4;
        int dotX = sz - dotSize;
        int dotY = sz - dotSize;

        QColor dotColor;
        switch (_status) {
        case OnlineStatus::Online:  dotColor = Theme().Color(ColorToken::Success); break;
        case OnlineStatus::Away:    dotColor = Theme().Color(ColorToken::Warning); break;
        case OnlineStatus::Busy:    dotColor = Theme().Color(ColorToken::Error);   break;
        case OnlineStatus::Offline: dotColor = Theme().Color(ColorToken::TextDisabled);  break;
        default: break;
        }

        // White ring around dot
        p.setPen(Qt::NoPen);
        p.setBrush(Theme().Color(ColorToken::Surface));
        p.drawEllipse(dotX - 1, dotY - 1, dotSize + 2, dotSize + 2);

        p.setBrush(dotColor);
        p.drawEllipse(dotX, dotY, dotSize, dotSize);
    }
}

void NyanAvatar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT Clicked();
    }
    QWidget::mousePressEvent(event);
}

void NyanAvatar::OnThemeChanged() { update(); }

} // namespace matcha::gui