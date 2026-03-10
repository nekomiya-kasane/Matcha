/**
 * @file NyanLineEdit.cpp
 * @brief Implementation of NyanLineEdit themed text/numeric input.
 */

#include <Matcha/Widgets/Controls/NyanLineEdit.h>

#include "../Core/InteractionEventFilter.h"

#include <QDoubleValidator>
#include <QFontMetrics>
#include <QIntValidator>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanLineEdit::NyanLineEdit(QWidget* parent)
    : QLineEdit(parent)
    , ThemeAware(WidgetKind::LineEdit)
{
    setFixedHeight(kFixedHeight);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanLineEdit::~NyanLineEdit() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanLineEdit::SetInputMode(InputMode mode)
{
    if (_inputMode == mode) {
        return;
    }
    _inputMode = mode;
    RebuildValidator();
    update();
}

auto NyanLineEdit::GetInputMode() const -> InputMode
{
    return _inputMode;
}

void NyanLineEdit::SetRange(double lower, double upper)
{
    if (lower > upper) {
        return;
    }
    _lowerValue = lower;
    _upperValue = upper;
    RebuildValidator();
}

auto NyanLineEdit::LowerValue() const -> double
{
    return _lowerValue;
}

auto NyanLineEdit::UpperValue() const -> double
{
    return _upperValue;
}

void NyanLineEdit::SetPrecision(int decimals)
{
    _precision = std::clamp(decimals, 1, 15);
    RebuildValidator();
}

auto NyanLineEdit::Precision() const -> int
{
    return _precision;
}

void NyanLineEdit::SetUnitSuffix(const QString& suffix)
{
    _unitSuffix = suffix;
    // Reserve right margin for suffix text
    if (!_unitSuffix.isEmpty()) {
        const auto& fontSpec = Theme().Font(StyleSheet().font);
        QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
        QFontMetrics fm(f);
        const int suffixW = fm.horizontalAdvance(_unitSuffix) + kSuffixGap + kHPadding;
        setTextMargins(0, 0, suffixW, 0);
    } else {
        setTextMargins(0, 0, 0, 0);
    }
    updateGeometry();
    update();
}

auto NyanLineEdit::UnitSuffix() const -> QString
{
    return _unitSuffix;
}

auto NyanLineEdit::TextWithoutSuffix() const -> QString
{
    return text();
}

void NyanLineEdit::SetBorderStyle(LineEditBorder border)
{
    if (_borderStyle == border) {
        return;
    }
    _borderStyle = border;
    update();
}

auto NyanLineEdit::BorderStyle() const -> LineEditBorder { return _borderStyle; }

auto NyanLineEdit::NumericValue() const -> std::optional<double>
{
    if (_inputMode == InputMode::Text) {
        return std::nullopt;
    }
    bool ok = false;
    const double val = text().toDouble(&ok);
    if (!ok) {
        return std::nullopt;
    }
    return val;
}

auto NyanLineEdit::sizeHint() const -> QSize
{
    QSize s = QLineEdit::sizeHint();
    if (!_unitSuffix.isEmpty()) {
        const auto& fontSpec = Theme().Font(StyleSheet().font);
        QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
        QFontMetrics fm(f);
        s.setWidth(s.width() + fm.horizontalAdvance(_unitSuffix) + kSuffixGap + kHPadding);
    }
    s.setHeight(kFixedHeight);
    return s;
}

auto NyanLineEdit::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanLineEdit::paintEvent(QPaintEvent* event)
{
    // Let QLineEdit paint text content first
    QLineEdit::paintEvent(event);

    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                      : hasFocus()   ? InteractionState::Focused
                      : underMouse() ? InteractionState::Hovered
                                     : InteractionState::Normal;

    const auto style = Theme().Resolve(WidgetKind::LineEdit, 0, istate);

    const QRect r = rect().adjusted(0, 0, -1, -1);

    // -- Draw border overlay --
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(style.border, style.borderWidthPx));

    if (_borderStyle == LineEditBorder::Minimal) {
        // Minimal: only top/bottom border lines, no rounded rect
        p.drawLine(r.topLeft(), r.topRight());
        p.drawLine(r.bottomLeft(), r.bottomRight());
    } else {
        p.drawRoundedRect(r, style.radiusPx, style.radiusPx);
    }

    // -- Draw unit suffix --
    if (!_unitSuffix.isEmpty()) {
        p.setFont(style.font);
        p.setPen(style.foreground);

        const QFontMetrics fm(style.font);
        const int suffixW = fm.horizontalAdvance(_unitSuffix);
        const QRect suffixRect(
            r.right() - suffixW - kHPadding,
            r.y(),
            suffixW + kHPadding,
            r.height()
        );
        p.drawText(suffixRect, Qt::AlignRight | Qt::AlignVCenter, _unitSuffix);
    }
}

void NyanLineEdit::OnThemeChanged()
{
    // Re-apply suffix margins with updated font metrics
    if (!_unitSuffix.isEmpty()) {
        const auto& fontSpec = Theme().Font(StyleSheet().font);
        QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
        QFontMetrics fm(f);
        const int suffixW = fm.horizontalAdvance(_unitSuffix) + kSuffixGap + kHPadding;
        setTextMargins(0, 0, suffixW, 0);
    }
    update();
}

// ============================================================================
// Validator setup
// ============================================================================

void NyanLineEdit::RebuildValidator()
{
    switch (_inputMode) {
    case InputMode::Text:
        setValidator(nullptr);
        break;
    case InputMode::Integer: {
        auto* v = new QIntValidator(
            static_cast<int>(_lowerValue),
            static_cast<int>(_upperValue),
            this);
        setValidator(v);
        break;
    }
    case InputMode::Double: {
        auto* v = new QDoubleValidator(
            _lowerValue, _upperValue, _precision, this);
        v->setNotation(QDoubleValidator::StandardNotation);
        setValidator(v);
        break;
    }
    default:
        setValidator(nullptr);
        break;
    }
}

} // namespace matcha::gui
