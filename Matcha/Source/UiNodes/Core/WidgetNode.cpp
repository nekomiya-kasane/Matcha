#include "Matcha/UiNodes/Core/WidgetNode.h"

#include "Matcha/UiNodes/Core/FocusChain.h"
#include "Matcha/UiNodes/Core/FocusManager.h"
#include "Matcha/Widgets/Core/IAnimationService.h"

#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QString>
#include <QWidget>

namespace matcha::fw {

// -------------------------------------------------------------------------- //
//  FocusTabEventFilter — intercepts Tab/Shift+Tab for UiNode FocusChain
// -------------------------------------------------------------------------- //

class WidgetNode::FocusTabEventFilter : public QObject {
public:
    explicit FocusTabEventFilter(WidgetNode* owner, QWidget* target)
        : QObject(target), _owner(owner)
    {
        target->installEventFilter(this);
    }

protected:
    auto eventFilter(QObject* watched, QEvent* event) -> bool override
    {
        (void)watched;
        if (event->type() != QEvent::KeyPress) {
            return false;
        }
        auto* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() != Qt::Key_Tab && ke->key() != Qt::Key_Backtab) {
            return false;
        }

        // Find the enclosing focus scope (or the UiNode tree root)
        UiNode* scope = _owner->FindEnclosingFocusScope();
        if (scope == nullptr) {
            // Walk to root
            scope = _owner;
            while (scope->ParentNode() != nullptr) {
                scope = scope->ParentNode();
            }
        }

        auto chain = FocusChain::Collect(scope);
        if (chain.empty()) {
            return true; // swallow Tab but do nothing
        }

        // Find the currently focused WidgetNode.
        // If Qt focus is on an external QWidget not tracked by UiNode,
        // FromWidget returns nullptr — fall back to _owner so traversal
        // starts from the event filter's owning node rather than chain.front().
        WidgetNode* current = nullptr;
        QWidget* focused = QApplication::focusWidget();
        if (focused != nullptr) {
            current = WidgetNode::FromWidget(focused);
        }
        if (current == nullptr) {
            current = _owner;
        }

        const bool forward = (ke->key() == Qt::Key_Tab);
        WidgetNode* next = forward
            ? FocusChain::Next(chain, current)
            : FocusChain::Previous(chain, current);

        if (next != nullptr) {
            next->SetFocus();
            // Notify the global FocusManager (if wired)
            if (auto* mgr = GetFocusManager()) {
                mgr->NotifyFocusGained(next);
            }
        }
        return true; // consumed
    }

private:
    WidgetNode* _owner;
};

// -------------------------------------------------------------------------- //
//  DndEventFilter — internal QObject that intercepts DnD QEvents
// -------------------------------------------------------------------------- //

static auto ToDragAction(Qt::DropAction a) -> DragAction
{
    switch (a) {
    case Qt::MoveAction: return DragAction::Move;
    case Qt::LinkAction: return DragAction::Link;
    case Qt::CopyAction: return DragAction::Copy;
    default:             return DragAction::Ignore;
    }
}

static auto ExtractMimeTypes(const QMimeData* md) -> std::vector<std::string>
{
    std::vector<std::string> types;
    if (md == nullptr) { return types; }
    for (const auto& f : md->formats()) {
        types.push_back(f.toStdString());
    }
    return types;
}

// -------------------------------------------------------------------------- //
//  ContextMenuEventFilter — intercepts ContextMenu QEvent → ContextMenuRequest
// -------------------------------------------------------------------------- //

class WidgetNode::ContextMenuEventFilter : public QObject {
public:
    explicit ContextMenuEventFilter(WidgetNode* owner, QWidget* target)
        : QObject(target), _owner(owner)
    {
        target->installEventFilter(this);
        target->setContextMenuPolicy(Qt::CustomContextMenu);
    }

protected:
    auto eventFilter(QObject* watched, QEvent* event) -> bool override
    {
        (void)watched;
        if (event->type() == QEvent::ContextMenu) {
            auto* e = static_cast<QContextMenuEvent*>(event);
            ContextMenuRequest notif(e->globalX(), e->globalY());
            _owner->SendNotification(_owner, notif);
            return true;
        }
        return false;
    }

private:
    WidgetNode* _owner;
};

// -------------------------------------------------------------------------- //
//  DndEventFilter — internal QObject that intercepts DnD QEvents
// -------------------------------------------------------------------------- //

class WidgetNode::DndEventFilter : public QObject {
public:
    explicit DndEventFilter(WidgetNode* owner, QWidget* target)
        : QObject(target), _owner(owner)
    {
        target->installEventFilter(this);
    }

protected:
    auto eventFilter(QObject* watched, QEvent* event) -> bool override
    {
        (void)watched;
        switch (event->type()) {
        case QEvent::DragEnter: {
            auto* e = static_cast<QDragEnterEvent*>(event);
            DragEntered notif(ExtractMimeTypes(e->mimeData()),
                              ToDragAction(e->proposedAction()));
            _owner->SendNotification(_owner, notif);
            if (notif.IsAccepted()) {
                e->acceptProposedAction();
            } else {
                e->ignore();
            }
            return true;
        }
        case QEvent::DragMove: {
            auto* e = static_cast<QDragMoveEvent*>(event);
            auto pos = e->position().toPoint();
            DragMoved notif(pos.x(), pos.y(),
                            ExtractMimeTypes(e->mimeData()),
                            ToDragAction(e->proposedAction()));
            _owner->SendNotification(_owner, notif);
            if (notif.IsAccepted()) {
                e->acceptProposedAction();
            } else {
                e->ignore();
            }
            return true;
        }
        case QEvent::DragLeave: {
            DragLeft notif;
            _owner->SendNotification(_owner, notif);
            return true;
        }
        case QEvent::Drop: {
            auto* e = static_cast<QDropEvent*>(event);
            auto pos = e->position().toPoint();
            auto* md = e->mimeData();
            // Send one Dropped notification per mime format
            // Typically the subscriber checks the mimeType it cares about
            std::string mimeType;
            std::vector<uint8_t> data;
            if (md != nullptr && !md->formats().isEmpty()) {
                mimeType = md->formats().first().toStdString();
                auto raw = md->data(md->formats().first());
                data.assign(raw.begin(), raw.end());
            }
            Dropped notif(pos.x(), pos.y(),
                          std::move(mimeType), std::move(data),
                          ToDragAction(e->proposedAction()));
            _owner->SendNotification(_owner, notif);
            if (notif.IsAccepted()) {
                e->acceptProposedAction();
            } else {
                e->ignore();
            }
            return true;
        }
        default:
            return false;
        }
    }

private:
    WidgetNode* _owner;
};

MATCHA_IMPLEMENT_CLASS(WidgetNode, UiNode)

WidgetNode::WidgetNode(std::string id, NodeType type)
    : UiNode(std::move(id), type)
{
}

WidgetNode::~WidgetNode()
{
    // Event filters are QObjects parented to _widget. Qt's parent-child chain
    // deletes them when _widget is destroyed. We must NOT delete them here to
    // avoid a double-free when the QWidget tree is torn down before the
    // UiNode tree (e.g. during Application::Shutdown).
    _focusFilter = nullptr;
    _dndFilter = nullptr;
    _ctxMenuFilter = nullptr;
}

void WidgetNode::SetEnabled(bool enabled)
{
    EnsureWidget();
    if (_widget) {
        _widget->setEnabled(enabled);
    }
}

auto WidgetNode::IsEnabled() const -> bool
{
    return _widget ? _widget->isEnabled() : true;
}

void WidgetNode::SetVisible(bool visible)
{
    EnsureWidget();
    if (_widget) {
        _widget->setVisible(visible);
    }
}

auto WidgetNode::IsVisible() const -> bool
{
    return _widget ? _widget->isVisible() : false;
}

void WidgetNode::SetToolTip(std::string_view tip)
{
    EnsureWidget();
    if (_widget) {
        _widget->setToolTip(QString::fromUtf8(tip.data(), static_cast<int>(tip.size())));
    }
}

void WidgetNode::SetMinimumSize(int w, int h)
{
    EnsureWidget();
    if (_widget) {
        _widget->setMinimumSize(w, h);
    }
}

void WidgetNode::SetMaximumSize(int w, int h)
{
    EnsureWidget();
    if (_widget) {
        _widget->setMaximumSize(w, h);
    }
}

void WidgetNode::SetFixedSize(int w, int h)
{
    EnsureWidget();
    if (_widget) {
        _widget->setFixedSize(w, h);
    }
}

void WidgetNode::SetFocus()
{
    EnsureWidget();
    if (_widget) {
        _widget->setFocus();
    }
}

auto WidgetNode::HasFocus() const -> bool
{
    return _widget ? _widget->hasFocus() : false;
}

void WidgetNode::SetFocusable(bool focusable)
{
    _focusable = focusable;
    if (_widget) {
        _widget->setFocusPolicy(focusable ? Qt::StrongFocus : Qt::NoFocus);
        if (focusable) {
            InstallFocusFilter();
        }
    }
}

auto WidgetNode::IsFocusable() const -> bool
{
    return _focusable;
}

void WidgetNode::SetTabIndex(int index)
{
    _tabIndex = index;
}

auto WidgetNode::TabIndex() const -> int
{
    return _tabIndex;
}

void WidgetNode::SetTooltip(TooltipSpec spec)
{
    _tooltip = std::move(spec);
}

auto WidgetNode::Tooltip() const -> const TooltipSpec&
{
    return _tooltip;
}

auto WidgetNode::HasTooltip() const -> bool
{
    return !_tooltip.IsEmpty();
}

void WidgetNode::SetStatusHint(std::string hint)
{
    _statusHint = std::move(hint);
    if (_widget) {
        _widget->setStatusTip(QString::fromStdString(_statusHint));
    }
}

auto WidgetNode::StatusHint() const -> const std::string&
{
    return _statusHint;
}

void WidgetNode::SetWhatsThis(std::string text)
{
    _whatsThis = std::move(text);
    if (_widget) {
        _widget->setWhatsThis(QString::fromStdString(_whatsThis));
    }
}

auto WidgetNode::WhatsThis() const -> const std::string&
{
    return _whatsThis;
}

void WidgetNode::SetHelpId(std::string id)
{
    _helpId = std::move(id);
}

auto WidgetNode::HelpId() const -> const std::string&
{
    return _helpId;
}

void WidgetNode::SetA11yRole(A11yRole role)
{
    _a11yRole = role;
}

auto WidgetNode::GetA11yRole() const -> A11yRole
{
    return _a11yRole;
}

void WidgetNode::SetAccessibleName(std::string name)
{
    _accessibleName = std::move(name);
    if (_widget) {
        _widget->setAccessibleName(QString::fromStdString(_accessibleName));
    }
}

auto WidgetNode::AccessibleName() const -> const std::string&
{
    return _accessibleName;
}

auto WidgetNode::Widget() -> QWidget*
{
    EnsureWidget();
    return _widget;
}

void WidgetNode::SetAcceptDrops(bool enabled)
{
    EnsureWidget();
    if (_widget == nullptr) { return; }

    _widget->setAcceptDrops(enabled);
    if (enabled && _dndFilter == nullptr) {
        _dndFilter = new DndEventFilter(this, _widget);
    }
}

void WidgetNode::SetContextMenuEnabled(bool enabled)
{
    EnsureWidget();
    if (_widget == nullptr) { return; }

    if (enabled && _ctxMenuFilter == nullptr) {
        _ctxMenuFilter = new ContextMenuEventFilter(this, _widget);
    }
}

void WidgetNode::SetIcon(std::string_view iconId)
{
    _iconId = iconId;
    if (_widget) {
        OnIconChanged();
    }
}

auto WidgetNode::Icon() const -> const IconId&
{
    return _iconId;
}

void WidgetNode::SetIconSize(fw::IconSize size)
{
    _iconSize = size;
    if (_widget) {
        OnIconChanged();
    }
}

auto WidgetNode::GetIconSize() const -> fw::IconSize
{
    return _iconSize;
}

void WidgetNode::OnIconChanged()
{
    // Default: no-op. Subclasses override to resolve token and apply to widget.
}

void WidgetNode::SetOpacity(double opacity)
{
    EnsureWidget();
    if (!_widget) { return; }
    auto* eff = qobject_cast<QGraphicsOpacityEffect*>(_widget->graphicsEffect());
    if (eff == nullptr) {
        eff = new QGraphicsOpacityEffect(_widget);
        _widget->setGraphicsEffect(eff);
    }
    eff->setOpacity(opacity);
}

auto WidgetNode::Opacity() const -> double
{
    if (!_widget) { return 1.0; }
    auto* eff = qobject_cast<QGraphicsOpacityEffect*>(_widget->graphicsEffect());
    if (eff == nullptr) { return 1.0; }
    return eff->opacity();
}

auto WidgetNode::FromWidget(QWidget* widget) -> WidgetNode*
{
    if (widget == nullptr) {
        return nullptr;
    }
    QVariant v = widget->property("matcha_widgetnode");
    if (!v.isValid()) {
        return nullptr;
    }
    return static_cast<WidgetNode*>(v.value<void*>());
}

void WidgetNode::EnsureWidget()
{
    if (_widget) {
        return;
    }
    auto* parentWidget = ParentNode() ? ParentNode()->Widget() : nullptr;
    _widget = CreateWidget(parentWidget);
    if (_widget) {
        _widget->setProperty("matcha_widgetnode",
                             QVariant::fromValue(static_cast<void*>(this)));

        // Apply deferred accessibility properties
        if (!_accessibleName.empty()) {
            _widget->setAccessibleName(QString::fromStdString(_accessibleName));
        }
        if (!_whatsThis.empty()) {
            _widget->setWhatsThis(QString::fromStdString(_whatsThis));
        }
        if (!_statusHint.empty()) {
            _widget->setStatusTip(QString::fromStdString(_statusHint));
        }
        if (_focusable) {
            _widget->setFocusPolicy(Qt::StrongFocus);
            InstallFocusFilter();
        }
    }
}

void WidgetNode::InstallFocusFilter()
{
    if (_focusFilter == nullptr && _widget != nullptr) {
        _focusFilter = new FocusTabEventFilter(this, _widget);
    }
}

// -------------------------------------------------------------------------- //
//  Animation API (RFC-08) — stubs, wired to AnimationService in Phase B
// -------------------------------------------------------------------------- //

auto WidgetNode::AnimateProperty(AnimationPropertyId property,
                                 AnimatableValue from,
                                 AnimatableValue to,
                                 AnimationToken duration,
                                 EasingToken easing) -> TransitionHandle
{
    auto* svc = gui::GetAnimationService();
    if (svc == nullptr) {
        return TransitionHandle::Invalid;
    }
    return svc->Animate(this, property, from, to, duration, easing);
}

auto WidgetNode::AnimateSpring(AnimationPropertyId property,
                               AnimatableValue from,
                               AnimatableValue to,
                               SpringSpec spring) -> TransitionHandle
{
    auto* svc = gui::GetAnimationService();
    if (svc == nullptr) {
        return TransitionHandle::Invalid;
    }
    return svc->AnimateSpring(this, property, from, to, spring);
}

void WidgetNode::CancelAnimation(TransitionHandle handle)
{
    auto* svc = gui::GetAnimationService();
    if (svc == nullptr) {
        return;
    }
    svc->Cancel(handle);
}

auto WidgetNode::IsAnimating(AnimationPropertyId property) -> bool
{
    auto* svc = gui::GetAnimationService();
    if (svc == nullptr) {
        return false;
    }
    return svc->IsAnimatingProperty(this, property);
}

} // namespace matcha::fw
