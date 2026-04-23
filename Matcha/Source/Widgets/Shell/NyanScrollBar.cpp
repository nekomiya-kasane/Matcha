/**
 * @file NyanScrollBar.cpp
 * @brief Implementation of NyanScrollBar themed scrollbar with auto-hide.
 */

#include <Matcha/Widgets/Shell/NyanScrollBar.h>

#include <QPaintEvent>
#include <QPainter>
#include <QVariantAnimation>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanScrollBar::NyanScrollBar(QWidget* parent)
    : QScrollBar(Qt::Vertical, parent)
    , ThemeAware(WidgetKind::ScrollBar)
{
    InitCommon();
}

NyanScrollBar::NyanScrollBar(Qt::Orientation orientation, QWidget* parent)
    : QScrollBar(orientation, parent)
    , ThemeAware(WidgetKind::ScrollBar)
{
    InitCommon();
}

NyanScrollBar::~NyanScrollBar() = default;

void NyanScrollBar::InitCommon()
{
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setMouseTracking(true);

    _idleTimer.setSingleShot(true);
    connect(&_idleTimer, &QTimer::timeout, this, &NyanScrollBar::OnIdleTimeout);

    // Restart idle timer on value changes (user scrolling).
    connect(this, &QScrollBar::valueChanged, this, [this]() {
        _visible = true;
        RestartIdleTimer();
        update();
    });

    // Set initial size based on orientation.
    if (orientation() == Qt::Vertical) {
        setFixedWidth(_thinPx);
    } else {
        setFixedHeight(_thinPx);
    }

    RestartIdleTimer();
}

// ============================================================================
// Public API
// ============================================================================

void NyanScrollBar::SetAutoHideMs(int ms)
{
    _autoHideMs = ms;
    if (ms <= 0) {
        _idleTimer.stop();
        _visible = true;
        update();
    } else {
        RestartIdleTimer();
    }
}

auto NyanScrollBar::AutoHideMs() const -> int
{
    return _autoHideMs;
}

void NyanScrollBar::SetThinThickness(int px)
{
    _thinPx = px;
    if (!_hovered) {
        SetCurrentThickness(px);
    }
}

void NyanScrollBar::SetExpandedThickness(int px)
{
    _expandedPx = px;
    if (_hovered) {
        SetCurrentThickness(px);
    }
}

auto NyanScrollBar::sizeHint() const -> QSize
{
    if (orientation() == Qt::Vertical) {
        return {_currentPx, 100};
    }
    return {100, _currentPx};
}

// ============================================================================
// Events
// ============================================================================

void NyanScrollBar::enterEvent(QEnterEvent* event)
{
    QScrollBar::enterEvent(event);
    _hovered = true;
    _visible = true;
    _idleTimer.stop();

    // Animate expand.
    auto* anim = new QVariantAnimation(this);
    anim->setDuration(Theme().AnimationMs(AnimationsToken::motionDurationFast));
    anim->setStartValue(_currentPx);
    anim->setEndValue(_expandedPx);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        SetCurrentThickness(v.toInt());
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void NyanScrollBar::leaveEvent(QEvent* event)
{
    QScrollBar::leaveEvent(event);
    _hovered = false;

    // Animate collapse.
    auto* anim = new QVariantAnimation(this);
    anim->setDuration(Theme().AnimationMs(AnimationsToken::motionDurationFast));
    anim->setStartValue(_currentPx);
    anim->setEndValue(_thinPx);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        SetCurrentThickness(v.toInt());
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    RestartIdleTimer();
}

// ============================================================================
// Painting
// ============================================================================

void NyanScrollBar::paintEvent(QPaintEvent* /*event*/)
{
    if (!_visible) {
        return;
    }

    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    const auto istate = _hovered ? InteractionState::Hovered : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::ScrollBar, 0, istate);

    const bool horiz = (orientation() == Qt::Horizontal);
    const int range = maximum() - minimum();

    // Geometry.
    const int trackLen = horiz ? width() : height();
    const int thickness = _currentPx;
    const int radius = thickness / 2;

    // -- Track --
    p.setPen(Qt::NoPen);
    p.setBrush(style.background);
    if (horiz) {
        p.drawRoundedRect(QRect(0, (height() - thickness) / 2, trackLen, thickness), radius, radius);
    } else {
        p.drawRoundedRect(QRect((width() - thickness) / 2, 0, thickness, trackLen), radius, radius);
    }

    // -- Handle --
    if (range <= 0) {
        return; // No scrollable content.
    }

    const int pageSize = pageStep();
    const int totalRange = range + pageSize;
    const int handleLen = std::max(kMinHandleLength,
        static_cast<int>(static_cast<double>(pageSize) / totalRange * trackLen));
    const int availableLen = trackLen - handleLen;
    const int handlePos = (range > 0)
        ? static_cast<int>(static_cast<double>(value() - minimum()) / range * availableLen)
        : 0;

    p.setBrush(style.foreground);

    if (horiz) {
        const int y = (height() - thickness) / 2;
        p.drawRoundedRect(QRect(handlePos, y, handleLen, thickness), radius, radius);
    } else {
        const int x = (width() - thickness) / 2;
        p.drawRoundedRect(QRect(x, handlePos, thickness, handleLen), radius, radius);
    }
}

void NyanScrollBar::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private helpers
// ============================================================================

void NyanScrollBar::RestartIdleTimer()
{
    if (_autoHideMs > 0) {
        _idleTimer.start(_autoHideMs);
    }
}

void NyanScrollBar::OnIdleTimeout()
{
    if (!_hovered) {
        _visible = false;
        update();
    }
}

void NyanScrollBar::SetCurrentThickness(int px)
{
    _currentPx = px;
    if (orientation() == Qt::Vertical) {
        setFixedWidth(px);
    } else {
        setFixedHeight(px);
    }
    update();
}

} // namespace matcha::gui
