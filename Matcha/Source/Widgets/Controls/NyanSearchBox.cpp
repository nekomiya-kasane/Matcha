/**
 * @file NyanSearchBox.cpp
 * @brief Implementation of NyanSearchBox themed search input.
 */

#include <Matcha/Widgets/Controls/NyanSearchBox.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <Matcha/Widgets/Controls/NyanLineEdit.h>

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanSearchBox::NyanSearchBox(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::LineEdit) // reuse LineEdit style tokens
    , _lineEdit(new NyanLineEdit( this))
{
    setFixedHeight(kFixedHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
    _lineEdit->setFrame(false);

    // Layout: [icon gap] [line edit] [clear button gap]
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(kHPadding + kIconSize + kIconGap, 0, kHPadding, 0);
    layout->setSpacing(0);
    layout->addWidget(_lineEdit);

    // Connect signals
    connect(_lineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        update(); // refresh clear button visibility
        if (_searchMode == SearchMode::Instant) {
            emit SearchChanged(text);
        }
    });

    connect(_lineEdit, &QLineEdit::returnPressed, this, [this]() {
        emit SearchSubmitted(_lineEdit->text());
    });
}

NyanSearchBox::~NyanSearchBox() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanSearchBox::SetPlaceholder(const QString& text)
{
    _lineEdit->setPlaceholderText(text);
}

auto NyanSearchBox::Placeholder() const -> QString
{
    return _lineEdit->placeholderText();
}

auto NyanSearchBox::Text() const -> QString
{
    return _lineEdit->text();
}

void NyanSearchBox::SetText(const QString& text)
{
    _lineEdit->setText(text);
}

void NyanSearchBox::SetSearchMode(SearchMode mode)
{
    _searchMode = mode;
}

auto NyanSearchBox::GetSearchMode() const -> SearchMode
{
    return _searchMode;
}

void NyanSearchBox::SetHistory(const std::vector<QString>& items)
{
    _history = items;
}

auto NyanSearchBox::History() const -> const std::vector<QString>&
{
    return _history;
}

void NyanSearchBox::Clear()
{
    _lineEdit->clear();
}

auto NyanSearchBox::sizeHint() const -> QSize
{
    return {200, kFixedHeight};
}

// ============================================================================
// Clear button helper
// ============================================================================

auto NyanSearchBox::ClearButtonRect() const -> QRect
{
    const int x = width() - kHPadding - kIconSize;
    const int y = (height() - kIconSize) / 2;
    return {x, y, kIconSize, kIconSize};
}

// ============================================================================
// Painting
// ============================================================================

void NyanSearchBox::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = !isEnabled()          ? InteractionState::Disabled
                      : _lineEdit->hasFocus() ? InteractionState::Focused
                      : underMouse()          ? InteractionState::Hovered
                                              : InteractionState::Normal;

    const auto style = Theme().Resolve(WidgetKind::LineEdit, 0, istate);
    const QRect r = rect();

    // -- Background --
    p.setPen(QPen(style.border, style.borderWidthPx));
    p.setBrush(style.background);
    p.drawRoundedRect(r.adjusted(0, 0, -1, -1), style.radiusPx, style.radiusPx);

    // -- Search icon (magnifying glass) --
    const int iconX = kHPadding;
    const int iconY = (r.height() - kIconSize) / 2;

    p.setPen(QPen(style.foreground, 1.5));
    p.setBrush(Qt::NoBrush);
    // Draw circle part of magnifying glass
    const int circleR = 5;
    const QPoint circleCenter(iconX + circleR + 1, iconY + circleR + 1);
    p.drawEllipse(circleCenter, circleR, circleR);
    // Draw handle
    p.drawLine(circleCenter.x() + 4, circleCenter.y() + 4,
               circleCenter.x() + 7, circleCenter.y() + 7);

    // -- Clear button (x) when text is non-empty --
    if (!_lineEdit->text().isEmpty()) {
        const QRect cr = ClearButtonRect();
        const int cx = cr.center().x();
        const int cy = cr.center().y();
        const int arm = 4;
        p.setPen(QPen(style.foreground, 1.5));
        p.drawLine(cx - arm, cy - arm, cx + arm, cy + arm);
        p.drawLine(cx - arm, cy + arm, cx + arm, cy - arm);
    }
}

void NyanSearchBox::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
