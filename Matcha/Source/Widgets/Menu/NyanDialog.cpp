#include <Matcha/Widgets/Menu/NyanDialog.h>
#include <Matcha/Widgets/Menu/NyanDialogFootBar.h>
#include <Matcha/Widgets/Menu/NyanDialogTitleBar.h>

#include <QCloseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace matcha::gui {

NyanDialog::NyanDialog(QWidget* parent)
    : QDialog(parent)
    , ThemeAware(WidgetKind::Dialog)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(kMinWidth, kMinHeight);

    InitLayout();
    ConnectSignals();
}

NyanDialog::~NyanDialog() = default;

void NyanDialog::InitLayout()
{
    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(0);

    // Title bar
    _titleBar = new NyanDialogTitleBar(this);
    _layout->addWidget(_titleBar);

    // Scroll area wrapping the content widget
    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setFrameShape(QFrame::NoFrame);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _layout->addWidget(_scrollArea, 1);

    // Footer bar
    _footBar = new NyanDialogFootBar(this);
    _layout->addWidget(_footBar);
}

void NyanDialog::ConnectSignals()
{
    // Title bar signals
    connect(_titleBar, &NyanDialogTitleBar::CloseClicked, this, &NyanDialog::reject);
    connect(_titleBar, &NyanDialogTitleBar::MinimizeClicked, this, &QDialog::showMinimized);
    connect(_titleBar, &NyanDialogTitleBar::MaximizeClicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(_titleBar, &NyanDialogTitleBar::DragStarted, this, &NyanDialog::OnTitleBarDragStarted);
    connect(_titleBar, &NyanDialogTitleBar::DragMoved, this, &NyanDialog::OnTitleBarDragMoved);

    // Footer bar signals
    connect(_footBar, &NyanDialogFootBar::ConfirmClicked, this, &NyanDialog::ConfirmClicked);
    connect(_footBar, &NyanDialogFootBar::ConfirmClicked, this, &NyanDialog::accept);
    connect(_footBar, &NyanDialogFootBar::ApplyClicked, this, &NyanDialog::ApplyClicked);
    connect(_footBar, &NyanDialogFootBar::CancelClicked, this, &NyanDialog::CancelClicked);
    connect(_footBar, &NyanDialogFootBar::CancelClicked, this, &NyanDialog::reject);
}

// -- Title Bar --

auto NyanDialog::TitleBar() -> NyanDialogTitleBar*
{
    return _titleBar;
}

void NyanDialog::SetTitle(const QString& title)
{
    _titleBar->SetTitle(title);
    setWindowTitle(title);
}

auto NyanDialog::Title() const -> QString
{
    return _titleBar->Title();
}

void NyanDialog::SetIcon(const QIcon& icon)
{
    _titleBar->SetIcon(icon);
    setWindowIcon(icon);
}

// -- Footer Bar --

auto NyanDialog::FootBar() -> NyanDialogFootBar*
{
    return _footBar;
}

void NyanDialog::SetFootBarVisible(bool visible)
{
    _footBar->setVisible(visible);
}

auto NyanDialog::IsFootBarVisible() const -> bool
{
    return _footBar->isVisible();
}

// -- Content --

void NyanDialog::SetContent(QWidget* content)
{
    _content = content;

    if (_content) {
        _scrollArea->setWidget(_content);
    } else {
        _scrollArea->takeWidget();
    }
}

auto NyanDialog::Content() const -> QWidget*
{
    return _content;
}

// -- Modality --

void NyanDialog::SetDialogModality(DialogModality modality)
{
    _modality = modality;

    switch (modality) {
    case DialogModality::Modal:
        setModal(true);
        setWindowModality(Qt::ApplicationModal);
        break;
    case DialogModality::SemiModal:
        setModal(true);
        setWindowModality(Qt::WindowModal);
        break;
    case DialogModality::Modeless:
        setModal(false);
        setWindowModality(Qt::NonModal);
        break;
    }
}

auto NyanDialog::Modality() const -> DialogModality
{
    return _modality;
}

// -- Show --

auto NyanDialog::ShowModal() -> DialogResult
{
    SetDialogModality(DialogModality::Modal);
    exec();
    return _result;
}

void NyanDialog::ShowModeless()
{
    SetDialogModality(DialogModality::Modeless);
    AdjustSizeToContent();
    show();
    if (_embedded) {
        raise();
    }
}

void NyanDialog::SetEmbedded(QWidget* container)
{
    _embedded = true;
    // Strip OS window flags -- become a plain child widget
    setWindowFlags(Qt::Widget);
    setParent(container);
    setAttribute(Qt::WA_TranslucentBackground, false);
}

auto NyanDialog::IsEmbedded() const -> bool
{
    return _embedded;
}

// -- Result --

auto NyanDialog::Result() const -> DialogResult
{
    return _result;
}

void NyanDialog::accept()
{
    _result = DialogResult::Accepted;
    Q_EMIT Closed(_result);
    QDialog::accept();
}

void NyanDialog::reject()
{
    _result = DialogResult::Rejected;
    Q_EMIT Closed(_result);
    QDialog::reject();
}

// -- Resize --

void NyanDialog::SetResizable(bool resizable)
{
    _resizable = resizable;
    if (resizable) {
        setMinimumSize(kMinWidth, kMinHeight);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    } else {
        setFixedSize(size());
    }
}

auto NyanDialog::IsResizable() const -> bool
{
    return _resizable;
}

// -- Size hints --

auto NyanDialog::sizeHint() const -> QSize
{
    // Chrome height: title bar + footer bar
    const int chromeH = (_titleBar ? _titleBar->sizeHint().height() : 0)
                      + (_footBar && _footBar->isVisible() ? _footBar->sizeHint().height() : 0);

    int contentW = 400;
    int contentH = 200;
    if (_content != nullptr) {
        const QSize cs = _content->sizeHint();
        contentW = std::max(contentW, cs.width());
        contentH = cs.height();
    }

    return {std::max(contentW, kMinWidth),
            std::max(contentH + chromeH, kMinHeight)};
}

auto NyanDialog::minimumSizeHint() const -> QSize
{
    return {kMinWidth, kMinHeight};
}

// -- Events --

void NyanDialog::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();

    // Dialog body background with rounded corners
    constexpr int kRadius = 6;
    p.setPen(theme.Color(ColorToken::colorBorder));
    p.setBrush(theme.Color(ColorToken::colorPrimaryBg));
    p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), kRadius, kRadius);
}

void NyanDialog::closeEvent(QCloseEvent* event)
{
    if (_result == DialogResult::Cancelled) {
        // Dialog was closed without explicit accept/reject
        Q_EMIT Closed(_result);
    }
    QDialog::closeEvent(event);
}

void NyanDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
        // Focus trapping: cycle within dialog children only.
        const bool forward = (event->key() == Qt::Key_Tab);
        const bool moved = forward ? focusNextChild() : focusPreviousChild();
        if (moved) {
            event->accept();
            return;
        }
        // If no focusable child found, swallow the event anyway to prevent escape.
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        reject();
        event->accept();
        return;
    }

    QDialog::keyPressEvent(event);
}

void NyanDialog::AdjustSizeToContent()
{
    const QSize hint = sizeHint();

    // Determine max available height from container or screen
    int maxH = 800; // fallback
    if (_embedded && parentWidget() != nullptr) {
        maxH = static_cast<int>(std::floor(
            parentWidget()->height() * kMaxHeightFraction));
    } else if (auto* scr = screen()) {
        maxH = static_cast<int>(std::floor(
            scr->availableGeometry().height() * kMaxHeightFraction));
    }

    const int finalW = hint.width();
    const int finalH = std::clamp(hint.height(), kMinHeight, maxH);

    resize(finalW, finalH);
}

void NyanDialog::OnThemeChanged()
{
    update();
}

void NyanDialog::OnTitleBarDragStarted(const QPoint& globalPos)
{
    // For embedded dialogs, mapToGlobal gives the correct global top-left;
    // for top-level dialogs, frameGeometry().topLeft() is already global.
    if (_embedded) {
        _dragOffset = globalPos - mapToGlobal(QPoint(0, 0));
    } else {
        _dragOffset = globalPos - frameGeometry().topLeft();
    }
}

void NyanDialog::OnTitleBarDragMoved(const QPoint& globalPos)
{
    if (_embedded && parentWidget() != nullptr) {
        // Clamp movement within parent widget bounds
        auto target = parentWidget()->mapFromGlobal(globalPos - _dragOffset);
        const auto parentRect = parentWidget()->rect();
        const int clampedX = std::clamp(target.x(), 0, parentRect.width() - width());
        const int clampedY = std::clamp(target.y(), 0, parentRect.height() - height());
        move(clampedX, clampedY);
    } else {
        move(globalPos - _dragOffset);
    }
}

} // namespace matcha::gui
