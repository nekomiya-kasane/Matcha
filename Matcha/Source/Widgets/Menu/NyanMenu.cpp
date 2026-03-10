#include <Matcha/Widgets/Menu/NyanMenu.h>
#include <Matcha/Widgets/Menu/NyanMenuBar.h>
#include <Matcha/Widgets/Menu/NyanMenuCheckItem.h>
#include <Matcha/Widgets/Menu/NyanMenuItem.h>
#include <Matcha/Widgets/Menu/NyanMenuSeparator.h>

#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScreen>
#include <QVBoxLayout>

namespace matcha::gui {

NyanMenu::NyanMenu(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , ThemeAware(WidgetKind::Menu)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    InitLayout();

    _submenuTimer = new QTimer(this);
    _submenuTimer->setSingleShot(true);
    connect(_submenuTimer, &QTimer::timeout, this, &NyanMenu::OnSubmenuHoverTimeout);
}

NyanMenu::~NyanMenu() = default;

void NyanMenu::InitLayout()
{
    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(kBorderWidth + 2, kBorderWidth + 2, kBorderWidth + 2, kBorderWidth + 2);
    _layout->setSpacing(0);
}

// -- Item Management --

auto NyanMenu::AddItem(const QString& text, const QIcon& icon) -> NyanMenuItem*
{
    auto* item = new NyanMenuItem(this);
    item->SetText(text);
    if (!icon.isNull()) {
        item->SetIcon(icon);
    }

    connect(item, &NyanMenuItem::Triggered, this, &NyanMenu::OnItemTriggered);

    _layout->addWidget(item);
    return item;
}

auto NyanMenu::AddSeparator() -> NyanMenuSeparator*
{
    auto* separator = new NyanMenuSeparator(this);
    _layout->addWidget(separator);
    return separator;
}

auto NyanMenu::AddCheckItem(const QString& text, bool checked) -> NyanMenuCheckItem*
{
    auto* item = new NyanMenuCheckItem(this);
    item->SetText(text);
    item->SetChecked(checked);

    connect(item, &NyanMenuItem::Triggered, this, &NyanMenu::OnItemTriggered);

    _layout->addWidget(item);
    return item;
}

auto NyanMenu::AddSubmenu(const QString& text, const QIcon& icon) -> NyanMenu*
{
    auto* submenu = new NyanMenu(this);
    submenu->_isSubmenu = true;

    auto* item = new NyanMenuItem(this);
    item->SetText(text);
    if (!icon.isNull()) {
        item->SetIcon(icon);
    }
    item->SetSubmenuIndicator(true);
    item->setProperty("submenu", QVariant::fromValue(submenu));

    _layout->addWidget(item);
    return submenu;
}

void NyanMenu::AddWidget(QWidget* widget)
{
    if (widget) {
        widget->setParent(this);
        _layout->addWidget(widget);
    }
}

void NyanMenu::Clear()
{
    HideSubmenu();

    while (_layout->count() > 0) {
        QLayoutItem* item = _layout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    _hoveredIndex = -1;
}

auto NyanMenu::ItemCount() const -> int
{
    return _layout->count();
}

// -- Popup Control --

void NyanMenu::Popup(const QPoint& globalPos)
{
    Q_EMIT AboutToShow();

    adjustSize();

    QPoint pos = globalPos;
    ClampToScreen(pos);

    move(pos);
    StartShowAnimation(globalPos);
    show();
    setFocus();
}

void NyanMenu::Close()
{
    _explicitClose = true;
    HideSubmenu();
    Q_EMIT AboutToHide();
    hide();
    _explicitClose = false;
}

auto NyanMenu::IsOpen() const -> bool
{
    return isVisible();
}

auto NyanMenu::IsSubmenu() const -> bool
{
    return _isSubmenu;
}

auto NyanMenu::ParentMenu() const -> NyanMenu*
{
    if (_isSubmenu) {
        return qobject_cast<NyanMenu*>(parentWidget());
    }
    return nullptr;
}

auto NyanMenu::ActiveSubmenu() const -> NyanMenu*
{
    return _activeSubmenu;
}

void NyanMenu::HandleExternalMouseMove(const QPoint& globalPos)
{
    // Convert global position to local coordinates
    QPoint localPos = mapFromGlobal(globalPos);

    // Check if position is within our geometry
    if (!rect().contains(localPos)) {
        return;
    }

    // Find which item is under the position and process as if mouse moved here
    for (int i = 0; i < ItemCount(); ++i) {
        QWidget* widget = ItemAt(i);
        if (widget && widget->geometry().contains(localPos)) {
            if (auto* item = qobject_cast<NyanMenuItem*>(widget)) {
                if (item->HasSubmenuIndicator()) {
                    auto* submenu = item->property("submenu").value<NyanMenu*>();

                    if (_activeSubmenu && _activeSubmenu == submenu) {
                        // Same trigger: keep submenu open
                        UpdateHoveredItem(i);
                        return;
                    }

                    // Different submenu trigger: close old, start new
                    if (_activeSubmenu) {
                        _submenuTimer->stop();
                        HideSubmenu();
                    }
                    _pendingSubmenu = item;
                    _submenuTimer->start(kSubmenuDelay);
                } else {
                    // Non-submenu item: close any open submenu
                    _submenuTimer->stop();
                    _pendingSubmenu = nullptr;
                    HideSubmenu();
                }
            }
            UpdateHoveredItem(i);
            return;
        }
    }
}

// -- Animation Property --

auto NyanMenu::AnimationOffset() const -> int
{
    return _animationOffset;
}

void NyanMenu::SetAnimationOffset(int offset)
{
    _animationOffset = offset;
    update();
}

// -- Paint --

void NyanMenu::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();

    // Apply animation offset
    if (_animationOffset != 0) {
        p.translate(0, _animationOffset);
    }

    // Background with border
    QRect bgRect = rect();
    if (_animationOffset != 0) {
        bgRect.translate(0, -_animationOffset);
    }

    p.setPen(QPen(theme.Color(ColorToken::BorderDefault), kBorderWidth));
    p.setBrush(theme.Color(ColorToken::SurfaceElevated));
    p.drawRoundedRect(bgRect.adjusted(1, 1, -1, -1), kRadius, kRadius);
}

void NyanMenu::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
}

void NyanMenu::hideEvent(QHideEvent* event)
{
    HideSubmenu();
    _hoveredIndex = -1;

    // When Qt auto-dismisses this popup (outside click), _explicitClose is
    // false. Cascade the close to the parent menu so the entire chain closes.
    // Use hide() instead of Close() so the parent's hideEvent also cascades
    // (Close() would set _explicitClose, blocking further propagation).
    if (!_explicitClose && _isSubmenu) {
        if (auto* parentMenu = qobject_cast<NyanMenu*>(parentWidget())) {
            Q_EMIT parentMenu->AboutToHide();
            parentMenu->hide();
        }
    }

    QWidget::hideEvent(event);
}

void NyanMenu::StartShowAnimation(const QPoint& globalPos)
{
    if (_showAnimation) {
        _showAnimation->stop();
        delete _showAnimation;
    }

    _showAnimation = new QPropertyAnimation(this, "animationOffset", this);
    _showAnimation->setEasingCurve(QEasingCurve::OutCubic);
    _showAnimation->setDuration(kAnimationDuration);

    // Determine animation direction based on cursor position
    int targetOffset = height();
    if (targetOffset > 160) {
        targetOffset = (targetOffset < 320) ? 160 : targetOffset / 2;
    }

    // If menu appears above cursor, slide down; otherwise slide up
    if (pos().y() + 35 >= globalPos.y()) {
        _showAnimation->setStartValue(-targetOffset);
    } else {
        _showAnimation->setStartValue(targetOffset);
    }

    _showAnimation->setEndValue(0);
    _showAnimation->start();
}

void NyanMenu::ClampToScreen(QPoint& pos)
{
    QScreen* screen = QApplication::screenAt(pos);
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }

    QRect screenRect = screen->availableGeometry();
    QSize menuSize = sizeHint();

    // Clamp right edge
    if (pos.x() + menuSize.width() > screenRect.right()) {
        pos.setX(screenRect.right() - menuSize.width());
    }
    // Clamp left edge
    if (pos.x() < screenRect.left()) {
        pos.setX(screenRect.left());
    }
    // Clamp bottom edge
    if (pos.y() + menuSize.height() > screenRect.bottom()) {
        pos.setY(screenRect.bottom() - menuSize.height());
    }
    // Clamp top edge
    if (pos.y() < screenRect.top()) {
        pos.setY(screenRect.top());
    }
}

// -- Keyboard Navigation --

void NyanMenu::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        UpdateHoveredItem(_hoveredIndex > 0 ? _hoveredIndex - 1 : ItemCount() - 1);
        break;

    case Qt::Key_Down:
        UpdateHoveredItem(_hoveredIndex < ItemCount() - 1 ? _hoveredIndex + 1 : 0);
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (_hoveredIndex >= 0) {
            ActivateItem(_hoveredIndex);
        }
        break;

    case Qt::Key_Escape:
        if (_isSubmenu) {
            // Notify parent to clear submenu state and reclaim focus
            if (auto* parentMenu = qobject_cast<NyanMenu*>(parentWidget())) {
                parentMenu->HideSubmenu();
                parentMenu->setFocus();
                break;
            }
        }
        Close();
        break;

    case Qt::Key_Right: {
        // Open submenu if hovered item has one
        bool handled = false;
        if (_hoveredIndex >= 0) {
            QWidget* widget = ItemAt(_hoveredIndex);
            if (auto* item = qobject_cast<NyanMenuItem*>(widget)) {
                if (item->HasSubmenuIndicator()) {
                    auto* submenu = item->property("submenu").value<NyanMenu*>();
                    if (submenu) {
                        ShowSubmenu(submenu, item);
                        handled = true;
                    }
                }
            }
        }
        // Forward to parent menu bar for menu switching
        if (!handled && !_isSubmenu) {
            if (auto* bar = qobject_cast<NyanMenuBar*>(parentWidget())) {
                QApplication::sendEvent(bar, event);
            }
        }
        break;
    }

    case Qt::Key_Left:
        if (_isSubmenu) {
            // Notify parent to clear submenu state and reclaim focus
            if (auto* parentMenu = qobject_cast<NyanMenu*>(parentWidget())) {
                parentMenu->HideSubmenu();
                parentMenu->setFocus();
                break;
            }
            Close();
        } else {
            // Forward to parent menu bar for menu switching
            if (auto* bar = qobject_cast<NyanMenuBar*>(parentWidget())) {
                QApplication::sendEvent(bar, event);
            }
        }
        break;

    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void NyanMenu::UpdateHoveredItem(int index)
{
    if (index == _hoveredIndex) {
        return;
    }

    // Skip separators
    while (index >= 0 && index < ItemCount()) {
        QWidget* widget = ItemAt(index);
        if (qobject_cast<NyanMenuSeparator*>(widget) == nullptr) {
            break;
        }
        // Move to next/previous item
        if (index > _hoveredIndex) {
            index++;
            if (index >= ItemCount()) {
                index = 0;
            }
        } else {
            index--;
            if (index < 0) {
                index = ItemCount() - 1;
            }
        }
    }

    _hoveredIndex = index;

    // Update visual state of all items
    for (int i = 0; i < ItemCount(); ++i) {
        QWidget* widget = ItemAt(i);
        if (auto* item = qobject_cast<NyanMenuItem*>(widget)) {
            // Force repaint by simulating enter/leave
            if (i == _hoveredIndex) {
                QEnterEvent enterEvent(QPointF(0, 0), QPointF(0, 0), QPointF(0, 0));
                QApplication::sendEvent(item, &enterEvent);
            } else {
                QEvent leaveEvent(QEvent::Leave);
                QApplication::sendEvent(item, &leaveEvent);
            }
        }
    }
}

void NyanMenu::ActivateItem(int index)
{
    QWidget* widget = ItemAt(index);
    if (auto* item = qobject_cast<NyanMenuItem*>(widget)) {
        if (item->HasSubmenuIndicator()) {
            auto* submenu = item->property("submenu").value<NyanMenu*>();
            if (submenu) {
                ShowSubmenu(submenu, item);
            }
        } else {
            Q_EMIT item->Triggered();
        }
    }
}

// -- Submenu Handling --

void NyanMenu::mouseMoveEvent(QMouseEvent* event)
{
    _lastMousePos = event->globalPosition().toPoint();

    QPoint localPos = event->pos();

    // If mouse is outside this menu's rect, signal for UiNode-layer routing.
    // Qt popup grab delivers mouseMoveEvent even when cursor is outside.
    if (!rect().contains(localPos)) {
        Q_EMIT MouseExitedToward(_lastMousePos);
        QWidget::mouseMoveEvent(event);
        return;
    }

    // Find which item is under the mouse
    for (int i = 0; i < ItemCount(); ++i) {
        QWidget* widget = ItemAt(i);
        if (widget && widget->geometry().contains(localPos)) {
            if (auto* item = qobject_cast<NyanMenuItem*>(widget)) {
                if (item->HasSubmenuIndicator()) {
                    auto* submenu = item->property("submenu").value<NyanMenu*>();

                    if (_activeSubmenu && _activeSubmenu == submenu) {
                        // Hovering back to the same trigger item that owns
                        // the currently open submenu -- keep it open.
                        UpdateHoveredItem(i);
                        QWidget::mouseMoveEvent(event);
                        return;
                    }

                    // Different submenu trigger: close the old one first
                    if (_activeSubmenu) {
                        _submenuTimer->stop();
                        HideSubmenu();
                    }

                    // Start timer for the new submenu
                    _pendingSubmenu = item;
                    _submenuTimer->start(kSubmenuDelay);
                } else {
                    // Not a submenu item, hide any open submenu
                    _submenuTimer->stop();
                    _pendingSubmenu = nullptr;
                    HideSubmenu();
                }
            }
            UpdateHoveredItem(i);
            break;
        }
    }

    QWidget::mouseMoveEvent(event);
}

void NyanMenu::OnSubmenuHoverTimeout()
{
    if (_pendingSubmenu) {
        auto* submenu = _pendingSubmenu->property("submenu").value<NyanMenu*>();
        if (submenu) {
            ShowSubmenu(submenu, _pendingSubmenu);
        }
    }
}

void NyanMenu::ShowSubmenu(NyanMenu* submenu, NyanMenuItem* parentItem)
{
    if (_activeSubmenu == submenu) {
        return;
    }

    HideSubmenu();

    _activeSubmenu = submenu;
    _submenuAnchor = parentItem->mapToGlobal(parentItem->rect().topRight());

    // Position submenu to the right of parent item
    QPoint submenuPos = _submenuAnchor;
    submenu->Popup(submenuPos);
}

void NyanMenu::HideSubmenu()
{
    if (_activeSubmenu) {
        _dismissingSubmenu = true;
        _activeSubmenu->Close();
        _activeSubmenu = nullptr;
        _dismissingSubmenu = false;
    }
    _pendingSubmenu = nullptr;
}

void NyanMenu::OnItemTriggered()
{
    auto* item = qobject_cast<NyanMenuItem*>(sender());
    if (item) {
        Q_EMIT ItemTriggered(item);
    }

    // Close the entire menu chain upward
    Close();

    // Walk up the parent chain closing all ancestor menus
    NyanMenu* ancestor = _isSubmenu ? qobject_cast<NyanMenu*>(parentWidget()) : nullptr;
    while (ancestor) {
        bool isSub = ancestor->_isSubmenu;
        auto* next = isSub ? qobject_cast<NyanMenu*>(ancestor->parentWidget()) : nullptr;
        ancestor->Close();
        ancestor = next;
    }
}

void NyanMenu::focusOutEvent(QFocusEvent* event)
{
    // Suppress if we are programmatically closing our own submenu
    if (_dismissingSubmenu) {
        QWidget::focusOutEvent(event);
        return;
    }

    // Don't close if focus went to a submenu
    if (_activeSubmenu && _activeSubmenu->hasFocus()) {
        QWidget::focusOutEvent(event);
        return;
    }

    // Don't close if focus went to a child widget
    QWidget* focusWidget = QApplication::focusWidget();
    if (focusWidget && isAncestorOf(focusWidget)) {
        QWidget::focusOutEvent(event);
        return;
    }

    // Defer close to next event loop iteration so that setFocus() calls
    // from submenu Escape/Left handlers can take effect first.
    QTimer::singleShot(0, this, [this]() {
        if (!hasFocus() && isVisible()) {
            Close();
        }
    });
    QWidget::focusOutEvent(event);
}

auto NyanMenu::ItemAt(int index) const -> QWidget*
{
    if (index < 0 || index >= _layout->count()) {
        return nullptr;
    }
    QLayoutItem* item = _layout->itemAt(index);
    return item ? item->widget() : nullptr;
}

auto NyanMenu::IndexOfItem(QWidget* item) const -> int
{
    for (int i = 0; i < _layout->count(); ++i) {
        if (_layout->itemAt(i)->widget() == item) {
            return i;
        }
    }
    return -1;
}

void NyanMenu::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
