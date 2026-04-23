/**
 * @file NyanMainTitleBar.cpp
 * @brief Implementation of NyanMainTitleBar -- single-row title bar (Row 1 only).
 */

#include <Matcha/Widgets/Shell/NyanMainTitleBar.h>
#include <Matcha/Widgets/Menu/NyanMenuBar.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>

namespace matcha::gui {

NyanMainTitleBar::NyanMainTitleBar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::MainTitleBar)
{
    setFixedHeight(kHeight);
    InitLayout();
    UpdateButtonStyles();
}

NyanMainTitleBar::~NyanMainTitleBar() = default;

// ============================================================================
// Layout
// ============================================================================

void NyanMainTitleBar::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(8, 0, 0, 0);
    _layout->setSpacing(4);

    // Menu bar (business layer populates menus via MenuBar()->AddMenu())
    _menuBar = new NyanMenuBar(this);
    _layout->addWidget(_menuBar);

    // Quick command container (hidden until business layer populates it)
    _quickCommandContainer = new QWidget(this);
    _quickCommandContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _layout->addWidget(_quickCommandContainer);
    _quickCommandContainer->hide();

    // Title logo (centered)
    _titleLogoLabel = new QLabel(this);
    _titleLogoLabel->setAlignment(Qt::AlignCenter);
    _layout->addWidget(_titleLogoLabel);

    // Spacer to push buttons to right
    _layout->addStretch(1);

    // Window control buttons
    _minimizeButton = new QPushButton(QStringLiteral("\u2500"), this);
    _minimizeButton->setFixedSize(46, kHeight);
    _minimizeButton->setFlat(true);
    _minimizeButton->setCursor(Qt::PointingHandCursor);
    connect(_minimizeButton, &QPushButton::clicked, this, &NyanMainTitleBar::MinimizeRequested);
    _layout->addWidget(_minimizeButton);

    _maximizeButton = new QPushButton(QStringLiteral("\u25A1"), this);
    _maximizeButton->setFixedSize(46, kHeight);
    _maximizeButton->setFlat(true);
    _maximizeButton->setCursor(Qt::PointingHandCursor);
    connect(_maximizeButton, &QPushButton::clicked, this, &NyanMainTitleBar::MaximizeRequested);
    _layout->addWidget(_maximizeButton);

    _closeButton = new QPushButton(QStringLiteral("\u2715"), this);
    _closeButton->setFixedSize(46, kHeight);
    _closeButton->setFlat(true);
    _closeButton->setCursor(Qt::PointingHandCursor);
    connect(_closeButton, &QPushButton::clicked, this, &NyanMainTitleBar::CloseRequested);
    _layout->addWidget(_closeButton);
}

// ============================================================================
// Public API
// ============================================================================

void NyanMainTitleBar::SetTitleLogo(const QPixmap& titleLogo)
{
    _titleLogo = titleLogo;
    _titleLogoLabel->setPixmap(_titleLogo);
}

auto NyanMainTitleBar::MenuBar() -> NyanMenuBar*
{
    return _menuBar;
}

auto NyanMainTitleBar::QuickCommandContainer() -> QWidget*
{
    return _quickCommandContainer;
}

void NyanMainTitleBar::SetTitle(const QString& title)
{
    _title = title;
    update();
}

auto NyanMainTitleBar::Title() const -> QString
{
    return _title;
}

// ============================================================================
// Size hints
// ============================================================================

auto NyanMainTitleBar::sizeHint() const -> QSize
{
    return {800, kHeight};
}

auto NyanMainTitleBar::minimumSizeHint() const -> QSize
{
    return {400, kHeight};
}

// ============================================================================
// Paint
// ============================================================================

void NyanMainTitleBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();
    p.fillRect(rect(), theme.Color(ColorToken::colorPrimaryBg));

    // Draw title text if no title logo
    if (_titleLogo.isNull() && !_title.isEmpty()) {
        p.setPen(theme.Color(ColorToken::colorText));
        p.setFont(QFont("Segoe UI", 9));
        QRect titleRect = rect();
        titleRect.setLeft(100);
        titleRect.setRight(width() - 150);
        p.drawText(titleRect, Qt::AlignCenter, _title);
    }
}

// ============================================================================
// Mouse events (window drag)
// ============================================================================

void NyanMainTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        auto* child = childAt(event->pos());
        if (child == nullptr || child == this) {
            _dragStartPos = event->globalPosition();
            _dragging = true;
        }
    }
    QWidget::mousePressEvent(event);
}

void NyanMainTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (_dragging && (event->buttons() & Qt::LeftButton)) {
        auto* win = window();
        if (win != nullptr && !win->isMaximized()) {
            auto delta = event->globalPosition() - _dragStartPos;
            win->move(win->pos() + QPoint(static_cast<int>(delta.x()),
                                          static_cast<int>(delta.y())));
            _dragStartPos = event->globalPosition();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void NyanMainTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    _dragging = false;
    if (event->button() == Qt::LeftButton) {
        Q_EMIT MaximizeRequested();
    }
    QWidget::mouseDoubleClickEvent(event);
}

// ============================================================================
// Theme
// ============================================================================

void NyanMainTitleBar::OnThemeChanged()
{
    UpdateButtonStyles();
    update();
}

void NyanMainTitleBar::UpdateButtonStyles()
{
    const auto& theme = Theme();

    QString style = QString(
        "QWidget { background-color: %1; }"
        "QPushButton { background: transparent; border: none; color: %2; font-size: 12px; }"
        "QPushButton:hover { background-color: %3; }"
    ).arg(theme.Color(ColorToken::colorPrimaryBg).name(),
          theme.Color(ColorToken::colorText).name(),
          theme.Color(ColorToken::colorFillTertiary).name());
    setStyleSheet(style);

    // Close button special style (red on hover)
    QString closeStyle = QString(
        "QPushButton { background: transparent; border: none; color: %1; font-size: 16px; font-weight: bold; }"
        "QPushButton:hover { background-color: %2; color: white; }"
    ).arg(theme.Color(ColorToken::colorText).name(),
          theme.Color(ColorToken::colorError).name());
    _closeButton->setStyleSheet(closeStyle);
}

} // namespace matcha::gui