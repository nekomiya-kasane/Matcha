#include <Matcha/Widgets/Menu/NyanMenuBar.h>
#include <Matcha/Widgets/Menu/NyanMenu.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>

namespace matcha::gui {

NyanMenuBar::NyanMenuBar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::MenuBar)
{
    setFixedHeight(kHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);

    InitLayout();

    // Install event filter on application for Alt-key detection
    qApp->installEventFilter(this);
}

NyanMenuBar::~NyanMenuBar()
{
    qApp->removeEventFilter(this);
}

void NyanMenuBar::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(kPaddingH, 0, kPaddingH, 0);
    _layout->setSpacing(kSpacing);
    _layout->addStretch();
}

// -- Menu Management --

auto NyanMenuBar::AddMenu(const QString& title) -> NyanMenu*
{
    MenuEntry entry;
    entry.title = title;

    auto [displayTitle, mnemonic] = ExtractMnemonic(title);
    entry.mnemonic = mnemonic;

    entry.menu = new NyanMenu(this);

    // When popup auto-dismisses (outside click), reset menu bar state.
    // _switchingMenu guard prevents reset during menu-to-menu switches.
    int menuIdx = static_cast<int>(_menus.size()); // capture before append
    connect(entry.menu, &NyanMenu::AboutToHide, this, [this, menuIdx]() {
        if (!_switchingMenu) {
            _dismissedIndex = menuIdx;
            _dismissTimer.start();
            _menuOpen = false;
            _activeIndex = -1;
            update();
        }
    });

    CreateMenuButton(entry);

    // Insert before the stretch
    int insertIndex = _layout->count() - 1;
    _layout->insertWidget(insertIndex, entry.button);

    _menus.append(entry);

    return entry.menu;
}

void NyanMenuBar::RemoveMenu(NyanMenu* menu)
{
    for (int i = 0; i < _menus.size(); ++i) {
        if (_menus[i].menu == menu) {
            _layout->removeWidget(_menus[i].button);
            delete _menus[i].button;
            delete _menus[i].menu;
            _menus.removeAt(i);
            break;
        }
    }
}

auto NyanMenuBar::MenuAt(int index) const -> NyanMenu*
{
    if (index >= 0 && index < _menus.size()) {
        return _menus[index].menu;
    }
    return nullptr;
}

auto NyanMenuBar::MenuCount() const -> int
{
    return static_cast<int>(_menus.size());
}

// -- Size hints --

auto NyanMenuBar::sizeHint() const -> QSize
{
    return {200, kHeight};
}

auto NyanMenuBar::minimumSizeHint() const -> QSize
{
    return {100, kHeight};
}

// -- Paint --

void NyanMenuBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const auto& theme = Theme();

    // Background
    p.fillRect(rect(), theme.Color(ColorToken::SurfaceElevated));
}

// -- Keyboard Navigation --

void NyanMenuBar::keyPressEvent(QKeyEvent* event)
{
    if (_menuOpen) {
        switch (event->key()) {
        case Qt::Key_Left:
            NavigateMenu(-1);
            return;
        case Qt::Key_Right:
            NavigateMenu(1);
            return;
        case Qt::Key_Escape:
            CloseActiveMenu();
            return;
        default:
            break;
        }
    }

    QWidget::keyPressEvent(event);
}

bool NyanMenuBar::eventFilter(QObject* watched, QEvent* event)
{
    // Handle hover-to-open on menu buttons (non-popup path)
    if (event->type() == QEvent::Enter) {
        QVariant idx = watched->property("menuIndex");
        if (idx.isValid()) {
            OnMenuButtonHovered(idx.toInt());
            return false;
        }
    }

    // While a popup menu is open, Qt::Popup grabs the mouse -- button Enter
    // events won't fire. Track mouse position globally to detect when the
    // cursor moves over a different menu button.
    if (_menuOpen && event->type() == QEvent::MouseMove) {
        auto* me = dynamic_cast<QMouseEvent*>(event);
        if (me != nullptr) {
            QPoint globalPos = me->globalPosition().toPoint();
            for (int i = 0; i < _menus.size(); ++i) {
                if (i == _activeIndex) { continue; }
                QWidget* btn = _menus[i].button;
                QRect btnGlobal(btn->mapToGlobal(QPoint(0, 0)), btn->size());
                if (btnGlobal.contains(globalPos)) {
                    OpenMenu(i);
                    return false;
                }
            }
        }
    }

    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);

        // Alt key pressed alone
        if (keyEvent->key() == Qt::Key_Alt) {
            _altPressed = true;
            return false;
        }

        // Alt + letter for mnemonic
        if (_altPressed && keyEvent->modifiers() == Qt::AltModifier) {
            QString key = keyEvent->text().toUpper();
            for (int i = 0; i < _menus.size(); ++i) {
                if (_menus[i].mnemonic.toUpper() == key) {
                    OpenMenu(i);
                    _altPressed = false;
                    return true;
                }
            }
        }
    } else if (event->type() == QEvent::KeyRelease) {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Alt) {
            _altPressed = false;
        }
    }

    return false;
}

// -- Private Methods --

void NyanMenuBar::CreateMenuButton(MenuEntry& entry)
{
    auto [displayTitle, mnemonic] = ExtractMnemonic(entry.title);

    auto* button = new QPushButton(displayTitle, this);
    button->setFlat(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setCursor(Qt::PointingHandCursor);

    // Style the button
    const auto& theme = Theme();
    QString style = QString(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0 8px;"
        "  color: %1;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background: %2;"
        "}"
        "QPushButton:pressed {"
        "  background: %3;"
        "}"
    ).arg(theme.Color(ColorToken::TextPrimary).name(),
          theme.Color(ColorToken::FillActive).name(),
          theme.Color(ColorToken::PrimaryBgHover).name());

    button->setStyleSheet(style);

    int index = _menus.size();

    connect(button, &QPushButton::clicked, this, [this, index]() {
        OnMenuButtonClicked(index);
    });

    // Hover detection for auto-open
    button->installEventFilter(this);
    button->setProperty("menuIndex", index);

    entry.button = button;
}

void NyanMenuBar::OnMenuButtonClicked(int index)
{
    if (_menuOpen && _activeIndex == index) {
        CloseActiveMenu();
        return;
    }

    // Suppress re-open if the popup was just auto-dismissed for this
    // same button (Qt::Popup grabs mouse -- the dismiss consumes the
    // click, then the button receives a second click event).
    if (_dismissedIndex == index && _dismissTimer.isValid()
        && _dismissTimer.elapsed() < 300) {
        _dismissedIndex = -1;
        return;
    }

    OpenMenu(index);
}

void NyanMenuBar::OnMenuButtonHovered(int index)
{
    if (_menuOpen && _activeIndex != index) {
        OpenMenu(index);
    }
}

void NyanMenuBar::OpenMenu(int index)
{
    if (index < 0 || index >= _menus.size()) {
        return;
    }

    _switchingMenu = true;
    CloseActiveMenu();
    _switchingMenu = false;

    _activeIndex = index;
    _menuOpen = true;

    MenuEntry& entry = _menus[index];
    Q_EMIT MenuAboutToShow(entry.menu);

    // Position menu below the button
    QPoint pos = entry.button->mapToGlobal(QPoint(0, entry.button->height()));
    entry.menu->Popup(pos);

    update();
}

void NyanMenuBar::CloseActiveMenu()
{
    if (_activeIndex >= 0 && _activeIndex < _menus.size()) {
        _menus[_activeIndex].menu->Close();
    }
    _menuOpen = false;
    _activeIndex = -1;
    update();
}

void NyanMenuBar::NavigateMenu(int delta)
{
    if (_menus.isEmpty()) {
        return;
    }

    int newIndex = _activeIndex + delta;
    if (newIndex < 0) {
        newIndex = static_cast<int>(_menus.size()) - 1;
    } else if (newIndex >= _menus.size()) {
        newIndex = 0;
    }

    OpenMenu(newIndex);
}

auto NyanMenuBar::ExtractMnemonic(const QString& title) -> QPair<QString, QString>
{
    QString displayTitle = title;
    QString mnemonic;

    auto ampIndex = title.indexOf('&');
    if (ampIndex >= 0 && ampIndex < title.length() - 1) {
        mnemonic = title.mid(ampIndex + 1, 1);
        displayTitle = title.left(ampIndex) + title.mid(ampIndex + 1);
    }

    return {displayTitle, mnemonic};
}

void NyanMenuBar::OnThemeChanged()
{
    // Re-style all buttons
    for (auto& entry : _menus) {
        if (auto* button = qobject_cast<QPushButton*>(entry.button)) {
            const auto& theme = Theme();
            QString style = QString(
                "QPushButton {"
                "  background: transparent;"
                "  border: none;"
                "  padding: 0 8px;"
                "  color: %1;"
                "  font-size: 12px;"
                "}"
                "QPushButton:hover {"
                "  background: %2;"
                "}"
                "QPushButton:pressed {"
                "  background: %3;"
                "}"
            ).arg(theme.Color(ColorToken::TextPrimary).name(),
                  theme.Color(ColorToken::FillActive).name(),
                  theme.Color(ColorToken::PrimaryBgHover).name());

            button->setStyleSheet(style);
        }
    }
    update();
}

} // namespace matcha::gui
