#include <Matcha/Widgets/Controls/NyanSelectionInput.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>

namespace matcha::gui {

NyanSelectionInput::NyanSelectionInput(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::SelectionInput)
{
    setFixedHeight(kHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);

    InitLayout();
    UpdateVisualState();
}

NyanSelectionInput::~NyanSelectionInput() = default;

void NyanSelectionInput::InitLayout()
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins(kPadding, 2, 4, 2);
    _layout->setSpacing(4);

    // Display edit (read-only)
    _displayEdit = new QLineEdit(this);
    _displayEdit->setReadOnly(true);
    _displayEdit->setFrame(false);
    _displayEdit->setCursor(Qt::PointingHandCursor);
    _displayEdit->setFocusPolicy(Qt::NoFocus);
    _layout->addWidget(_displayEdit, 1);

    // Pick button
    _pickButton = new QPushButton("⊕", this);
    _pickButton->setFixedSize(20, 20);
    _pickButton->setFlat(true);
    _pickButton->setCursor(Qt::PointingHandCursor);
    _pickButton->setToolTip(tr("Pick entity"));
    connect(_pickButton, &QPushButton::clicked, this, &NyanSelectionInput::OnPickButtonClicked);
    _layout->addWidget(_pickButton);

    // Clear button (hidden by default)
    _clearButton = new QPushButton("×", this);
    _clearButton->setFixedSize(20, 20);
    _clearButton->setFlat(true);
    _clearButton->setCursor(Qt::PointingHandCursor);
    _clearButton->setToolTip(tr("Clear selection"));
    _clearButton->hide();
    connect(_clearButton, &QPushButton::clicked, this, &NyanSelectionInput::OnClearButtonClicked);
    _layout->addWidget(_clearButton);
}

// -- Configuration --

void NyanSelectionInput::SetMode(PickMode mode)
{
    _mode = mode;
    _pickButton->setEnabled(mode != PickMode::Disabled);
    update();
}

auto NyanSelectionInput::Mode() const -> PickMode
{
    return _mode;
}

void NyanSelectionInput::SetPrompt(const QString& prompt)
{
    _displayEdit->setPlaceholderText(prompt);
}

auto NyanSelectionInput::Prompt() const -> QString
{
    return _displayEdit->placeholderText();
}

void NyanSelectionInput::SetSelection(const QString& selection)
{
    _selection = selection;
    _displayEdit->setText(selection);
    _clearButton->setVisible(!selection.isEmpty());

    // Auto-update state based on selection
    if (selection.isEmpty()) {
        if (_state == SelectionState::Selected || _state == SelectionState::SelectedIdle) {
            SetState(SelectionState::Idle);
        }
    } else {
        if (_state == SelectionState::Idle || _state == SelectionState::PickingEmpty || _state == SelectionState::PickingIdle) {
            SetState(SelectionState::Selected);
        }
    }

    Q_EMIT SelectionChanged(selection);
}

auto NyanSelectionInput::Selection() const -> QString
{
    return _selection;
}

void NyanSelectionInput::SetState(SelectionState state)
{
    if (_state != state) {
        _state = state;
        UpdateVisualState();
        update();
    }
}

auto NyanSelectionInput::State() const -> SelectionState
{
    return _state;
}

void NyanSelectionInput::ClearSelection()
{
    SetSelection(QString());
    Q_EMIT ClearRequested();
}

auto NyanSelectionInput::HasSelection() const -> bool
{
    return !_selection.isEmpty();
}

// -- Size hints --

auto NyanSelectionInput::sizeHint() const -> QSize
{
    return {200, kHeight};
}

auto NyanSelectionInput::minimumSizeHint() const -> QSize
{
    return {100, kHeight};
}

// -- Paint --

void NyanSelectionInput::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    QRect bgRect = rect().adjusted(1, 1, -1, -1);
    p.setPen(QPen(StateBorderColor(), 1));
    p.setBrush(StateBackgroundColor());
    p.drawRoundedRect(bgRect, kRadius, kRadius);
}

void NyanSelectionInput::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _mode != PickMode::Disabled) {
        OnPickButtonClicked();
    }
    QWidget::mousePressEvent(event);
}

void NyanSelectionInput::OnThemeChanged()
{
    UpdateVisualState();
    update();
}

void NyanSelectionInput::UpdateVisualState()
{
    const auto& theme = Theme();

    // Update display edit style
    QString editStyle = QString(
        "QLineEdit {"
        "  background: transparent;"
        "  border: none;"
        "  color: %1;"
        "}"
        "QLineEdit::placeholder {"
        "  color: %2;"
        "}"
    ).arg(theme.Color(ColorToken::TextPrimary).name(),
          theme.Color(ColorToken::TextTertiary).name());
    _displayEdit->setStyleSheet(editStyle);

    // Update button styles
    QString buttonStyle = QString(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  color: %1;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  color: %2;"
        "}"
    ).arg(theme.Color(ColorToken::TextSecondary).name(),
          theme.Color(ColorToken::Primary).name());
    _pickButton->setStyleSheet(buttonStyle);

    QString clearStyle = QString(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  color: %1;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  color: %2;"
        "}"
    ).arg(theme.Color(ColorToken::TextTertiary).name(),
          theme.Color(ColorToken::Error).name());
    _clearButton->setStyleSheet(clearStyle);
}

void NyanSelectionInput::OnPickButtonClicked()
{
    if (_mode == PickMode::Disabled) {
        return;
    }

    // Transition to picking state
    if (_state == SelectionState::Idle) {
        SetState(SelectionState::PickingEmpty);
    } else if (_state == SelectionState::SelectedIdle) {
        SetState(SelectionState::Selected);
    }

    Q_EMIT PickRequested();
}

void NyanSelectionInput::OnClearButtonClicked()
{
    ClearSelection();
}

auto NyanSelectionInput::StateBackgroundColor() const -> QColor
{
    const auto& theme = Theme();

    switch (_state) {
    case SelectionState::PickingEmpty:
        return theme.Color(ColorToken::WarningBorder);
    case SelectionState::PickingIdle:
        return theme.Color(ColorToken::WarningBgHover);
    case SelectionState::Selected:
        return theme.Color(ColorToken::SuccessBorder);
    case SelectionState::SelectedIdle:
        return theme.Color(ColorToken::SuccessBgHover);
    case SelectionState::Idle:
    default:
        return theme.Color(ColorToken::FillHover);
    }
}

auto NyanSelectionInput::StateBorderColor() const -> QColor
{
    const auto& theme = Theme();

    switch (_state) {
    case SelectionState::PickingEmpty:
        return theme.Color(ColorToken::Warning);
    case SelectionState::PickingIdle:
        return theme.Color(ColorToken::BorderDefault);
    case SelectionState::Selected:
        return theme.Color(ColorToken::Success);
    case SelectionState::SelectedIdle:
        return theme.Color(ColorToken::BorderDefault);
    case SelectionState::Idle:
    default:
        return theme.Color(ColorToken::BorderDefault);
    }
}

} // namespace matcha::gui
