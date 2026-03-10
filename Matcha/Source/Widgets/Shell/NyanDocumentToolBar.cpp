/**
 * @file NyanDocumentToolBar.cpp
 * @brief Implementation of NyanDocumentToolBar -- module combo + tab bar + global buttons.
 */

#include <Matcha/Widgets/Shell/NyanDocumentToolBar.h>
#include <Matcha/Widgets/Shell/NyanTabBar.h>

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>

namespace matcha::gui {

NyanDocumentToolBar::NyanDocumentToolBar(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::DocumentToolBar)
{
    setFixedHeight(kHeight);
    InitLayout();
    UpdateStyles();
}

NyanDocumentToolBar::~NyanDocumentToolBar() = default;

// ============================================================================
// Layout
// ============================================================================

void NyanDocumentToolBar::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(8, 0, 8, 0);
    _layout->setSpacing(8);

    // Module combo
    _moduleCombo = new QComboBox(this);
    _moduleCombo->setMinimumWidth(120);
    connect(_moduleCombo, &QComboBox::currentTextChanged,
            this, &NyanDocumentToolBar::ModuleChanged);
    _layout->addWidget(_moduleCombo);

    // Separator line between ModuleCombo and TabBar
    _separator = new QFrame(this);
    _separator->setFrameShape(QFrame::VLine);
    _separator->setFixedWidth(1);
    _layout->addWidget(_separator);

    // TabBar will be inserted here by SetTabBar() with stretch=1,
    // which pushes GlobalButtonContainer to the far right.

    // Global button container (hidden until business layer populates it)
    _globalButtonContainer = new QWidget(this);
    _globalButtonContainer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _layout->addWidget(_globalButtonContainer);
    _globalButtonContainer->hide();
}

// ============================================================================
// Tab Bar
// ============================================================================

void NyanDocumentToolBar::SetTabBar(NyanTabBar* tabBar)
{
    if (_tabBar) {
        _layout->removeWidget(_tabBar);
    }

    _tabBar = tabBar;

    if (_tabBar) {
        // Insert right after the separator (ModuleCombo | Separator | TabBar | stretch | GlobalBtns)
        int insertIndex = _layout->indexOf(_separator) + 1;
        _layout->insertWidget(insertIndex, _tabBar, 1);
    }
}

auto NyanDocumentToolBar::GetTabBar() const -> NyanTabBar*
{
    return _tabBar;
}

// ============================================================================
// Module Combo
// ============================================================================

void NyanDocumentToolBar::SetModuleItems(const QStringList& items)
{
    _moduleCombo->clear();
    _moduleCombo->addItems(items);
}

auto NyanDocumentToolBar::CurrentModule() const -> QString
{
    return _moduleCombo->currentText();
}

void NyanDocumentToolBar::SetCurrentModule(const QString& name)
{
    int index = _moduleCombo->findText(name);
    if (index >= 0) {
        _moduleCombo->setCurrentIndex(index);
    }
}

// ============================================================================
// Containers
// ============================================================================

auto NyanDocumentToolBar::GlobalButtonContainer() -> QWidget*
{
    return _globalButtonContainer;
}

// ============================================================================
// Size hints
// ============================================================================

auto NyanDocumentToolBar::sizeHint() const -> QSize
{
    return {800, kHeight};
}

auto NyanDocumentToolBar::minimumSizeHint() const -> QSize
{
    return {200, kHeight};
}

// ============================================================================
// Paint
// ============================================================================

void NyanDocumentToolBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.fillRect(rect(), Theme().Color(ColorToken::PrimaryBgHover));
}

// ============================================================================
// Theme
// ============================================================================

void NyanDocumentToolBar::OnThemeChanged()
{
    UpdateStyles();
    update();
}

void NyanDocumentToolBar::UpdateStyles()
{
    const auto& theme = Theme();

    QString style = QString(
        "QWidget { background-color: %1; }"
        "QComboBox { background-color: %2; border: 1px solid %3; border-radius: 3px; padding: 4px 8px; color: %4; }"
        "QComboBox:hover { border-color: %5; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "QFrame { background-color: %6; }"
    ).arg(theme.Color(ColorToken::PrimaryBgHover).name(),
          theme.Color(ColorToken::SurfaceElevated).name(),
          theme.Color(ColorToken::BorderStrong).name(),
          theme.Color(ColorToken::TextPrimary).name(),
          theme.Color(ColorToken::Primary).name(),
          theme.Color(ColorToken::BorderStrong).name());
    setStyleSheet(style);
}

} // namespace matcha::gui
