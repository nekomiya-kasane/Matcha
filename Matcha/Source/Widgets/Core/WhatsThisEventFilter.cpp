/**
 * @file WhatsThisEventFilter.cpp
 * @brief Shift+F1 "What's This" mode event filter implementation.
 */

#include "WhatsThisEventFilter.h"

#include <Matcha/UiNodes/Core/WidgetNode.h>
#include <Matcha/Widgets/Core/IThemeService.h>

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>
#include <QWidget>

namespace matcha::gui {

namespace {
constexpr int kBubbleMaxWidth   = 300;
constexpr int kBubblePadding    = 12;
constexpr int kBubbleRadius     = 6;
constexpr int kBubbleGap        = 8;
constexpr int kBubbleAutoHideMs = 8000;
} // namespace

WhatsThisEventFilter::WhatsThisEventFilter(QObject* parent)
    : QObject(parent)
{
}

WhatsThisEventFilter::~WhatsThisEventFilter()
{
    Deactivate();
}

void WhatsThisEventFilter::Activate()
{
    if (_active) {
        return;
    }
    _active = true;
    QApplication::setOverrideCursor(Qt::WhatsThisCursor);
}

void WhatsThisEventFilter::Deactivate()
{
    if (!_active) {
        return;
    }
    _active = false;
    QApplication::restoreOverrideCursor();
    HideBubble();
}

auto WhatsThisEventFilter::eventFilter(QObject* watched, QEvent* event) -> bool
{
    // Shift+F1 toggles the mode regardless of current state
    if (event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_F1
            && (ke->modifiers() & Qt::ShiftModifier)) {
            if (_active) {
                Deactivate();
            } else {
                Activate();
            }
            return true;
        }
        // Escape exits What's This mode
        if (_active && ke->key() == Qt::Key_Escape) {
            Deactivate();
            return true;
        }
    }

    if (!_active) {
        return false;
    }

    // In What's This mode, intercept mouse clicks
    if (event->type() == QEvent::MouseButtonPress) {
        auto* widget = qobject_cast<QWidget*>(watched);
        if (widget == nullptr) {
            Deactivate();
            return true;
        }

        // Walk up the widget tree to find one with a WidgetNode that has WhatsThis text
        QWidget* target = widget;
        fw::WidgetNode* node = nullptr;
        while (target != nullptr) {
            node = fw::WidgetNode::FromWidget(target);
            if (node != nullptr && !node->WhatsThis().empty()) {
                break;
            }
            node = nullptr;
            target = target->parentWidget();
        }

        if (node != nullptr && target != nullptr) {
            ShowBubble(target, QString::fromStdString(node->WhatsThis()));
        }

        Deactivate();
        return true; // consume the click
    }

    return false;
}

void WhatsThisEventFilter::ShowBubble(QWidget* anchor, const QString& text)
{
    HideBubble();

    _bubble = new QLabel(nullptr, Qt::ToolTip | Qt::FramelessWindowHint);
    _bubble->setAttribute(Qt::WA_DeleteOnClose);
    _bubble->setWordWrap(true);
    _bubble->setMaximumWidth(kBubbleMaxWidth);
    _bubble->setText(text);
    _bubble->setContentsMargins(kBubblePadding, kBubblePadding,
                                kBubblePadding, kBubblePadding);

    // Theme-aware styling
    if (HasThemeService()) {
        const auto& theme = GetThemeService();
        const QColor bg = theme.Color(ColorToken::SurfaceElevated);
        const QColor fg = theme.Color(ColorToken::TextPrimary);
        const QColor border = theme.Color(ColorToken::BorderDefault);
        _bubble->setStyleSheet(
            QStringLiteral("QLabel {"
                           "  background: %1;"
                           "  color: %2;"
                           "  border: 1px solid %3;"
                           "  border-radius: %4px;"
                           "  padding: %5px;"
                           "}")
                .arg(bg.name(QColor::HexArgb),
                     fg.name(QColor::HexArgb),
                     border.name(QColor::HexArgb),
                     QString::number(kBubbleRadius),
                     QString::number(kBubblePadding)));
    }

    _bubble->adjustSize();

    // Position below the anchor widget with screen-edge clamping
    const QPoint anchorGlobal = anchor->mapToGlobal(QPoint(0, anchor->height()));
    int x = anchorGlobal.x();
    int y = anchorGlobal.y() + kBubbleGap;

    QScreen* screen = QApplication::screenAt(anchorGlobal);
    if (screen == nullptr) {
        screen = QApplication::primaryScreen();
    }
    if (screen != nullptr) {
        const QRect avail = screen->availableGeometry();
        const QSize sz = _bubble->sizeHint();

        if (y + sz.height() > avail.bottom()) {
            y = anchor->mapToGlobal(QPoint(0, 0)).y() - sz.height() - kBubbleGap;
        }
        if (y < avail.top()) {
            y = avail.top();
        }
        if (x + sz.width() > avail.right()) {
            x = avail.right() - sz.width();
        }
        if (x < avail.left()) {
            x = avail.left();
        }
    }

    _bubble->move(x, y);
    _bubble->show();

    // Auto-hide after timeout
    QTimer::singleShot(kBubbleAutoHideMs, _bubble, [this]() {
        HideBubble();
    });
}

void WhatsThisEventFilter::HideBubble()
{
    if (_bubble != nullptr) {
        _bubble->close();
        _bubble = nullptr;
    }
}

} // namespace matcha::gui
