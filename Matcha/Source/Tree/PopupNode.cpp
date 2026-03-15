/**
 * @file PopupNode.cpp
 * @brief Implementation of PopupNode — base class for popup-type UiNodes.
 */

#include "Matcha/Tree/PopupNode.h"
#include "Matcha/Tree/WidgetNotification.h"

#include <QApplication>
#include <QKeyEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>

namespace {
/// @brief Returns true if a QApplication exists (needed for QWidget creation).
inline auto HasQApp() -> bool { return QApplication::instance() != nullptr; }
} // namespace

namespace matcha::fw {

// ============================================================================
// PopupEventFilter — handles Escape key and deactivation
// ============================================================================

class PopupNode::PopupEventFilter : public QObject {
public:
    explicit PopupEventFilter(PopupNode* owner, QObject* parent = nullptr)
        : QObject(parent), _owner(owner) {}

    auto eventFilter(QObject* watched, QEvent* event) -> bool override
    {
        if (watched != _owner->_popupWidget) {
            return false;
        }

        switch (event->type()) {
        case QEvent::KeyPress: {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape && _owner->_closeOnEscape) {
                _owner->Close();
                return true;
            }
            break;
        }
        case QEvent::Hide:
            // Qt::Popup auto-hides on click-away; detect and sync our state.
            if (_owner->_isOpen) {
                _owner->_isOpen = false;
                _owner->OnClosed();
                PopupClosed notif;
                _owner->SendNotification(_owner, notif);
            }
            break;
        default:
            break;
        }
        return false;
    }

private:
    PopupNode* _owner;
};

// ============================================================================
// Construction / Destruction
// ============================================================================

MATCHA_IMPLEMENT_CLASS(PopupNode, UiNode)

PopupNode::PopupNode(std::string id, NodeType type, PopupBehavior behavior)
    : UiNode(std::move(id), type)
    , _behavior(behavior)
{
    // Set defaults based on behavior (matches OverlayPolicy spec)
    switch (behavior) {
    case PopupBehavior::Dropdown:
        _closeOnEscape = true;
        _autoPosition  = true;
        break;
    case PopupBehavior::Tooltip:
        _closeOnEscape = false;
        _autoPosition  = true;
        break;
    case PopupBehavior::Floating:
        _closeOnEscape = false;
        _autoPosition  = false;
        break;
    }
}

PopupNode::~PopupNode()
{
    delete _popupWidget;
}

// ============================================================================
// Open / Close
// ============================================================================

void PopupNode::Open(UiNode* anchor, PopupPlacement placement)
{
    if (_isOpen) {
        Close();
    }

    _anchor = anchor;
    _placement = placement;

    EnsurePopupWidget();

    if (_autoPosition && anchor != nullptr) {
        ApplyPosition(anchor, placement);
    }

    if (_popupWidget != nullptr) {
        _popupWidget->show();
    }
    _isOpen = true;

    OnOpened();
    PopupOpened notif;
    SendNotification(this, notif);
}

void PopupNode::OpenAtPoint(Point screenPos)
{
    if (_isOpen) {
        Close();
    }

    _anchor = nullptr;

    EnsurePopupWidget();

    if (_popupWidget != nullptr) {
        const auto sz = PreferredSize();
        _popupWidget->resize(sz.w, sz.h);
        _popupWidget->move(screenPos.x, screenPos.y);
        _popupWidget->show();
    }
    _isOpen = true;

    OnOpened();
    PopupOpened notif;
    SendNotification(this, notif);
}

void PopupNode::Close()
{
    if (!_isOpen) {
        return;
    }

    if (_popupWidget != nullptr) {
        _popupWidget->hide();
    }
    // Note: the Hide event handler in PopupEventFilter will set _isOpen = false
    // and fire PopupClosed. But if hide() didn't trigger the event (e.g. already
    // hidden), ensure we clean up:
    if (_isOpen) {
        _isOpen = false;
        OnClosed();
        PopupClosed notif;
        SendNotification(this, notif);
    }
}

auto PopupNode::IsOpen() const -> bool
{
    return _isOpen;
}

auto PopupNode::Behavior() const -> PopupBehavior
{
    return _behavior;
}

auto PopupNode::Anchor() const -> UiNode*
{
    return _anchor;
}

// ============================================================================
// Configuration
// ============================================================================

void PopupNode::SetCloseOnEscape(bool v) { _closeOnEscape = v; }
void PopupNode::SetAutoPosition(bool v) { _autoPosition = v; }
void PopupNode::SetOffset(Point offset) { _offset = offset; }
void PopupNode::SetMinHeight(int px) { _minHeight = px; }

// ============================================================================
// Widget bridge
// ============================================================================

auto PopupNode::Widget() -> QWidget*
{
    EnsurePopupWidget();
    return _popupWidget;
}

auto PopupNode::PopupWidget() -> QWidget*
{
    return _popupWidget;
}

// ============================================================================
// Internal: lazy widget creation
// ============================================================================

void PopupNode::EnsurePopupWidget()
{
    if (_popupWidget != nullptr || !HasQApp()) {
        return;
    }

    // Determine Qt window flags from behavior
    Qt::WindowFlags flags = Qt::FramelessWindowHint;
    switch (_behavior) {
    case PopupBehavior::Dropdown:
        flags |= Qt::Popup;
        break;
    case PopupBehavior::Tooltip:
        flags |= Qt::ToolTip;
        break;
    case PopupBehavior::Floating:
        flags |= Qt::Tool;
        break;
    }

    _popupWidget = new QWidget(nullptr, flags);
    _popupWidget->setAttribute(Qt::WA_TranslucentBackground);

    if (_behavior == PopupBehavior::Tooltip) {
        _popupWidget->setAttribute(Qt::WA_ShowWithoutActivating);
    }

    // Layout for content
    auto* layout = new QVBoxLayout(_popupWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Let the subclass create its content
    auto* content = CreatePopupContent(_popupWidget);
    if (content != nullptr) {
        layout->addWidget(content);
    }

    // Install event filter for Escape / Hide tracking
    _eventFilter = new PopupEventFilter(this, _popupWidget);
    _popupWidget->installEventFilter(_eventFilter);
}

// ============================================================================
// Internal: positioning via PopupPositioner
// ============================================================================

void PopupNode::ApplyPosition(UiNode* anchor, PopupPlacement placement)
{
    if (anchor == nullptr || anchor->Widget() == nullptr || _popupWidget == nullptr) {
        return;
    }

    auto* anchorWidget = anchor->Widget();
    const QPoint anchorGlobal = anchorWidget->mapToGlobal(QPoint(0, 0));
    const QSize anchorSize = anchorWidget->size();

    // Determine viewport (screen geometry)
    Rect viewport{};
    QScreen* screen = QApplication::screenAt(anchorGlobal);
    if (screen == nullptr) {
        screen = QApplication::primaryScreen();
    }
    if (screen != nullptr) {
        const QRect avail = screen->availableGeometry();
        viewport = {avail.x(), avail.y(), avail.width(), avail.height()};
    }

    const auto preferred = PreferredSize();
    _popupWidget->resize(preferred.w, preferred.h);

    // Use the popup's actual size hint if content has a better idea
    const QSize hint = _popupWidget->sizeHint();
    const Size popupSize = {
        hint.isValid() ? hint.width() : preferred.w,
        hint.isValid() ? hint.height() : preferred.h
    };

    PopupRequest req;
    req.anchorRect = {anchorGlobal.x(), anchorGlobal.y(), anchorSize.width(), anchorSize.height()};
    req.popupSize  = popupSize;
    req.placement  = placement;
    req.offset     = _offset;
    req.viewport   = viewport;
    req.minHeight  = _minHeight;
    req.strategy   = OverflowStrategy::All;

    const auto result = PopupPositioner::Compute(req);

    _popupWidget->resize(result.size.w, result.size.h);
    _popupWidget->move(result.position.x, result.position.y);
}

} // namespace matcha::fw
