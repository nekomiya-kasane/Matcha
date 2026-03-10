/**
 * @file NyanFloatingTitleBar.cpp
 * @brief Implementation of the lightweight floating window title bar.
 */

#include <Matcha/Widgets/Shell/NyanFloatingTitleBar.h>

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>

namespace matcha::gui {

NyanFloatingTitleBar::NyanFloatingTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(kHeight);
    setMouseTracking(true);
    InitLayout();
}

NyanFloatingTitleBar::~NyanFloatingTitleBar() = default;

void NyanFloatingTitleBar::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(6, 0, 0, 0);
    _layout->setSpacing(4);

    // Icon (16x16, optional -- hidden by default)
    _iconLabel = new QLabel(this);
    _iconLabel->setFixedSize(kIconSize, kIconSize);
    _iconLabel->hide();
    _layout->addWidget(_iconLabel);

    // Title label
    _titleLabel = new QLabel(this);
    _titleLabel->setStyleSheet("color: #333; font-size: 12px;");
    _layout->addWidget(_titleLabel);

    _layout->addStretch();

    // Window buttons
    auto makeButton = [this](const QString& text, const QString& objectName) {
        auto* btn = new QPushButton(text, this);
        btn->setObjectName(objectName);
        btn->setFixedSize(kButtonW, kButtonH);
        btn->setFlat(true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setStyleSheet(
            "QPushButton { border: none; font-size: 11px; color: #555; }"
            "QPushButton:hover { background: #E0E0E0; }");
        return btn;
    };

    _minimizeButton = makeButton(QStringLiteral("\u2500"), "minimize");
    _maximizeButton = makeButton(QStringLiteral("\u25A1"), "maximize");
    _closeButton    = makeButton(QStringLiteral("\u2715"), "close");

    _closeButton->setStyleSheet(
        "QPushButton { border: none; font-size: 11px; color: #555; }"
        "QPushButton:hover { background: #E81123; color: white; }");

    _layout->addWidget(_minimizeButton);
    _layout->addWidget(_maximizeButton);
    _layout->addWidget(_closeButton);

    connect(_minimizeButton, &QPushButton::clicked, this, &NyanFloatingTitleBar::MinimizeRequested);
    connect(_maximizeButton, &QPushButton::clicked, this, &NyanFloatingTitleBar::MaximizeRequested);
    connect(_closeButton,    &QPushButton::clicked, this, &NyanFloatingTitleBar::CloseRequested);
}

void NyanFloatingTitleBar::SetTitle(const QString& title)
{
    _titleLabel->setText(title);
}

auto NyanFloatingTitleBar::Title() const -> QString
{
    return _titleLabel->text();
}

void NyanFloatingTitleBar::SetIcon(const QIcon& icon)
{
    if (icon.isNull()) {
        _iconLabel->hide();
    } else {
        _iconLabel->setPixmap(icon.pixmap(kIconSize, kIconSize));
        _iconLabel->show();
    }
}

auto NyanFloatingTitleBar::sizeHint() const -> QSize
{
    return {400, kHeight};
}

auto NyanFloatingTitleBar::minimumSizeHint() const -> QSize
{
    return {200, kHeight};
}

void NyanFloatingTitleBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(0xF0, 0xF0, 0xF0));
    // Bottom border line
    p.setPen(QColor(0xD0, 0xD0, 0xD0));
    p.drawLine(0, height() - 1, width() - 1, height() - 1);
}

void NyanFloatingTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit MaximizeRequested();
    }
}

void NyanFloatingTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = true;
        _dragStartPos = event->globalPosition();
    }
}

void NyanFloatingTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (_dragging && (event->buttons() & Qt::LeftButton)) {
        auto* win = window();
        if (win != nullptr) {
            auto delta = event->globalPosition() - _dragStartPos;
            win->move(win->pos() + delta.toPoint());
            _dragStartPos = event->globalPosition();
        }
    }
}

void NyanFloatingTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = false;
    }
}

} // namespace matcha::gui
