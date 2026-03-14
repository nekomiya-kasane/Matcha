/**
 * @file NyanComboBox.cpp
 * @brief Implementation of NyanComboBox custom-painted combo box.
 */

#include <Matcha/Widgets/Controls/NyanComboBox.h>

#include "../Core/ComboBoxEventFilter.h"

#include <QCompleter>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QVariantAnimation>

namespace matcha::gui {

NyanComboBox::NyanComboBox(QWidget* parent)
    : QComboBox(parent)
    , ThemeAware(WidgetKind::ComboBox)
{
    setFixedHeight(kFixedHeight);
    _cbFilter = new ComboBoxEventFilter(this, nullptr);
}

NyanComboBox::~NyanComboBox() = default;

void NyanComboBox::SetPlaceholder(const QString& text)
{
    _placeholder = text;
    setPlaceholderText(text);
    update();
}

auto NyanComboBox::Placeholder() const -> QString
{
    return _placeholder;
}

void NyanComboBox::SetSearchEnabled(bool enabled)
{
    _searchEnabled = enabled;
    if (enabled) {
        setEditable(true);
        setInsertPolicy(QComboBox::NoInsert);
        if (auto* c = completer()) {
            c->setCompletionMode(QCompleter::PopupCompletion);
            c->setFilterMode(Qt::MatchContains);
        }
    } else if (!_editableMode) {
        setEditable(false);
    }
    update();
}

auto NyanComboBox::IsSearchEnabled() const -> bool { return _searchEnabled; }

void NyanComboBox::SetEditableMode(bool editable)
{
    _editableMode = editable;
    if (editable) {
        setEditable(true);
        setInsertPolicy(QComboBox::InsertAtBottom);
    } else if (!_searchEnabled) {
        setEditable(false);
    }
    update();
}

auto NyanComboBox::IsEditableMode() const -> bool { return _editableMode; }

void NyanComboBox::showPopup()
{
    QComboBox::showPopup();
    _popupVisible = true;
    _cbFilter->NotifyPopupOpened();

    // Animate arrow rotation 0 -> -180 degrees
    auto* anim = new QVariantAnimation(this);
    const int durationMs = Theme().AnimationMs(StyleSheet().transition.duration);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    anim->setStartValue(0.0);
    anim->setEndValue(-180.0);
    connect(anim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v) {
                _arrowAngle = v.toDouble();
                update();
            });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void NyanComboBox::hidePopup()
{
    if (!_popupVisible) {
        QComboBox::hidePopup();
        return;
    }
    _popupVisible = false;
    _cbFilter->NotifyPopupClosed();

    // Animate arrow rotation -180 -> 0 degrees
    auto* anim = new QVariantAnimation(this);
    const int durationMs = Theme().AnimationMs(StyleSheet().transition.duration);
    anim->setDuration(durationMs);
    anim->setEasingCurve(QEasingCurve::InOutSine);
    anim->setStartValue(-180.0);
    anim->setEndValue(0.0);
    connect(anim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& v) {
                _arrowAngle = v.toDouble();
                update();
            });
    connect(anim, &QVariantAnimation::finished, this,
            [this]() { QComboBox::hidePopup(); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

auto NyanComboBox::sizeHint() const -> QSize
{
    QSize s = QComboBox::sizeHint();
    if (s.width() < kMinWidth) {
        s.setWidth(kMinWidth);
    }
    if (s.height() < kFixedHeight) {
        s.setHeight(kFixedHeight);
    }
    return s;
}

auto NyanComboBox::minimumSizeHint() const -> QSize
{
    return sizeHint();
}

void NyanComboBox::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = _cbFilter->Controller().GetInteractionState();

    const auto style = Theme().Resolve(WidgetKind::ComboBox, 0, istate);
    const QRect r = rect().adjusted(1, 1, -1, -1);

    // -- Draw combo frame --
    p.setPen(QPen(style.border, style.borderWidthPx));
    p.setBrush(style.background);
    p.drawRoundedRect(r, style.radiusPx, style.radiusPx);

    // -- Draw text --
    const QRect textRect(
        style.paddingHPx, 0,
        r.width() - style.paddingHPx - kArrowMargin,
        r.height()
    );

    p.setPen(style.foreground);
    p.setFont(style.font);

    const QString displayText = currentIndex() >= 0
        ? currentText()
        : _placeholder;
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, displayText);

    // -- Draw arrow indicator --
    const QRect arrowRect(
        r.right() - kArrowMargin,
        (r.height() - kArrowSize) / 2,
        kArrowSize, kArrowSize
    );

    p.save();
    p.translate(arrowRect.center());
    p.rotate(_arrowAngle);

    // Draw a simple chevron-down arrow
    p.setPen(QPen(style.foreground, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    const qreal hs = kArrowSize * 0.2; // half-span
    QPainterPath arrow;
    arrow.moveTo(-hs, -hs * 0.5);
    arrow.lineTo(0, hs * 0.5);
    arrow.lineTo(hs, -hs * 0.5);
    p.drawPath(arrow);
    p.restore();
}

void NyanComboBox::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
