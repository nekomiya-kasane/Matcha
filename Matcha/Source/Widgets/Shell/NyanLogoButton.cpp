/**
 * @file NyanLogoButton.cpp
 * @brief Implementation of NyanLogoButton -- clickable logo spanning two shell rows.
 */

#include <Matcha/Widgets/Shell/NyanLogoButton.h>

#include <QMouseEvent>
#include <QPainter>

namespace matcha::gui {

NyanLogoButton::NyanLogoButton(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::LogoButton)
{
    setFixedSize(kLogoMargin, kLogoSize);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);

    const auto style = Theme().Resolve(WidgetKind::MainTitleBar, 0, InteractionState::Normal);
    _topColor = style.background;
    _bottomColor = Theme().Color(ColorToken::colorPrimaryBgHover);
}

NyanLogoButton::~NyanLogoButton() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanLogoButton::SetLogo(const QPixmap& logo)
{
    _logo = logo.scaled(kLogoSize, kLogoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    update();
}

auto NyanLogoButton::Logo() const -> const QPixmap&
{
    return _logo;
}

void NyanLogoButton::SetSplitY(int row1Height)
{
    _splitY = row1Height;
    update();
}

void NyanLogoButton::SetTopColor(const QColor& color)
{
    _topColor = color;
    update();
}

void NyanLogoButton::SetBottomColor(const QColor& color)
{
    _bottomColor = color;
    update();
}

auto NyanLogoButton::sizeHint() const -> QSize
{
    return {kLogoMargin, kLogoSize};
}

auto NyanLogoButton::minimumSizeHint() const -> QSize
{
    return {kLogoMargin, kLogoSize};
}

// ============================================================================
// Paint
// ============================================================================

void NyanLogoButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Two-color background: top follows TitleBar, bottom follows DocumentToolBar.
    p.fillRect(0, 0, width(), _splitY, _topColor);
    p.fillRect(0, _splitY, width(), height() - _splitY, _bottomColor);

    // Hover highlight.
    if (_hovered) {
        p.fillRect(rect(), QColor(255, 255, 255, 20));
    }

    // Draw logo centered.
    if (!_logo.isNull()) {
        const int x = (width() - _logo.width()) / 2;
        const int y = (height() - _logo.height()) / 2;
        p.drawPixmap(x, y, _logo);
    }
}

// ============================================================================
// Mouse events
// ============================================================================

void NyanLogoButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT Clicked();
    }
    QWidget::mousePressEvent(event);
}

void NyanLogoButton::enterEvent(QEnterEvent* event)
{
    _hovered = true;
    update();
    QWidget::enterEvent(event);
}

void NyanLogoButton::leaveEvent(QEvent* event)
{
    _hovered = false;
    update();
    QWidget::leaveEvent(event);
}

// ============================================================================
// Theme
// ============================================================================

void NyanLogoButton::OnThemeChanged()
{
    const auto style = Theme().Resolve(WidgetKind::MainTitleBar, 0, InteractionState::Normal);
    _topColor = style.background;
    _bottomColor = Theme().Color(ColorToken::colorPrimaryBgHover);
    update();
}

} // namespace matcha::gui
