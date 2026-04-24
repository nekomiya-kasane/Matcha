#include <Matcha/Widgets/Menu/NyanDialogTitleBar.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>

namespace matcha::gui {

NyanDialogTitleBar::NyanDialogTitleBar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Dialog)
{
    setFixedHeight(kHeight);
    setCursor(Qt::ArrowCursor);
    InitLayout();
    UpdateButtonStyles();
}

NyanDialogTitleBar::~NyanDialogTitleBar() = default;

void NyanDialogTitleBar::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(8, 0, 4, 0);
    _layout->setSpacing(4);

    // Icon
    _iconLabel = new QLabel(this);
    _iconLabel->setFixedSize(kIconSize, kIconSize);
    _iconLabel->setScaledContents(true);
    _layout->addWidget(_iconLabel);

    // Title
    _titleLabel = new QLabel(this);
    _titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _layout->addWidget(_titleLabel, 1);

    // Minimize button
    _minimizeButton = new QPushButton("-", this);
    _minimizeButton->setFixedSize(kButtonSize, kButtonSize);
    _minimizeButton->setFlat(true);
    _minimizeButton->setCursor(Qt::PointingHandCursor);
    _minimizeButton->hide();
    connect(_minimizeButton, &QPushButton::clicked, this, &NyanDialogTitleBar::MinimizeClicked);
    _layout->addWidget(_minimizeButton);

    // Maximize button
    _maximizeButton = new QPushButton("□", this);
    _maximizeButton->setFixedSize(kButtonSize, kButtonSize);
    _maximizeButton->setFlat(true);
    _maximizeButton->setCursor(Qt::PointingHandCursor);
    _maximizeButton->hide();
    connect(_maximizeButton, &QPushButton::clicked, this, &NyanDialogTitleBar::MaximizeClicked);
    _layout->addWidget(_maximizeButton);

    // Close button
    _closeButton = new QPushButton("×", this);
    _closeButton->setFixedSize(kButtonSize, kButtonSize);
    _closeButton->setFlat(true);
    _closeButton->setCursor(Qt::PointingHandCursor);
    connect(_closeButton, &QPushButton::clicked, this, &NyanDialogTitleBar::CloseClicked);
    _layout->addWidget(_closeButton);
}

// -- Title --

void NyanDialogTitleBar::SetTitle(const QString& title)
{
    _titleLabel->setText(title);
}

auto NyanDialogTitleBar::Title() const -> QString
{
    return _titleLabel->text();
}

void NyanDialogTitleBar::SetIcon(const QIcon& icon)
{
    _icon = icon;
    _iconLabel->setPixmap(icon.pixmap(kIconSize, kIconSize));
    _iconLabel->setVisible(!icon.isNull());
}

auto NyanDialogTitleBar::Icon() const -> QIcon
{
    return _icon;
}

// -- Buttons --

void NyanDialogTitleBar::SetVisibleButtons(TitleBarButton buttons)
{
    _visibleButtons = buttons;
    _minimizeButton->setVisible((buttons & TitleBarButton::Minimize) != TitleBarButton::None);
    _maximizeButton->setVisible((buttons & TitleBarButton::Maximize) != TitleBarButton::None);
    _closeButton->setVisible((buttons & TitleBarButton::Close) != TitleBarButton::None);
}

auto NyanDialogTitleBar::VisibleButtons() const -> TitleBarButton
{
    return _visibleButtons;
}

void NyanDialogTitleBar::SetMinimizeVisible(bool visible)
{
    if (visible) {
        _visibleButtons = _visibleButtons | TitleBarButton::Minimize;
    } else {
        _visibleButtons = _visibleButtons & ~TitleBarButton::Minimize;
    }
    _minimizeButton->setVisible(visible);
}

auto NyanDialogTitleBar::IsMinimizeVisible() const -> bool
{
    return (_visibleButtons & TitleBarButton::Minimize) != TitleBarButton::None;
}

void NyanDialogTitleBar::SetMaximizeVisible(bool visible)
{
    if (visible) {
        _visibleButtons = _visibleButtons | TitleBarButton::Maximize;
    } else {
        _visibleButtons = _visibleButtons & ~TitleBarButton::Maximize;
    }
    _maximizeButton->setVisible(visible);
}

auto NyanDialogTitleBar::IsMaximizeVisible() const -> bool
{
    return (_visibleButtons & TitleBarButton::Maximize) != TitleBarButton::None;
}

void NyanDialogTitleBar::SetCloseVisible(bool visible)
{
    if (visible) {
        _visibleButtons = _visibleButtons | TitleBarButton::Close;
    } else {
        _visibleButtons = _visibleButtons & ~TitleBarButton::Close;
    }
    _closeButton->setVisible(visible);
}

auto NyanDialogTitleBar::IsCloseVisible() const -> bool
{
    return (_visibleButtons & TitleBarButton::Close) != TitleBarButton::None;
}

// -- Size hints --

auto NyanDialogTitleBar::sizeHint() const -> QSize
{
    return {200, kHeight};
}

auto NyanDialogTitleBar::minimumSizeHint() const -> QSize
{
    return {100, kHeight};
}

// -- Paint --

void NyanDialogTitleBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();

    // Background
    p.fillRect(rect(), theme.Color(ColorToken::colorFillHover));

    // Bottom border
    p.setPen(theme.Color(ColorToken::colorBorder));
    p.drawLine(0, height() - 1, width(), height() - 1);
}

void NyanDialogTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = true;
        _dragStartPos = event->globalPosition().toPoint();
        Q_EMIT DragStarted(_dragStartPos);
    }
    QWidget::mousePressEvent(event);
}

void NyanDialogTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (_dragging) {
        Q_EMIT DragMoved(event->globalPosition().toPoint());
    }
    QWidget::mouseMoveEvent(event);
}

void NyanDialogTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void NyanDialogTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && IsMaximizeVisible()) {
        Q_EMIT MaximizeClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void NyanDialogTitleBar::OnThemeChanged()
{
    UpdateButtonStyles();
    update();
}

void NyanDialogTitleBar::UpdateButtonStyles()
{
    const auto& theme = Theme();

    // Title style
    QString titleStyle = QString(
        "QLabel { color: %1; font-weight: bold; }"
    ).arg(theme.Color(ColorToken::colorText).name());
    _titleLabel->setStyleSheet(titleStyle);

    // Button style
    QString buttonStyle = QString(
        "QPushButton { background: transparent; border: none; color: %1; font-size: 14px; }"
        "QPushButton:hover { background-color: %2; }"
    ).arg(theme.Color(ColorToken::colorTextSecondary).name(),
          theme.Color(ColorToken::colorFillTertiary).name());
    _minimizeButton->setStyleSheet(buttonStyle);
    _maximizeButton->setStyleSheet(buttonStyle);

    // Close button special style
    QString closeStyle = QString(
        "QPushButton { background: transparent; border: none; color: %1; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: %2; color: white; }"
    ).arg(theme.Color(ColorToken::colorTextSecondary).name(),
          theme.Color(ColorToken::colorError).name());
    _closeButton->setStyleSheet(closeStyle);
}

} // namespace matcha::gui
