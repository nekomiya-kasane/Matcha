/**
 * @file NyanLabel.cpp
 * @brief Implementation of NyanLabel role-based themed label.
 */

#include <Matcha/Widgets/Controls/NyanLabel.h>

#include "../Core/InteractionEventFilter.h"

#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

NyanLabel::NyanLabel(QWidget* parent)
    : QLabel(parent)
    , ThemeAware(WidgetKind::Label)
{
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanLabel::NyanLabel(const QString& text, QWidget* parent)
    : QLabel(text, parent)
    , ThemeAware(WidgetKind::Label)
{
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanLabel::~NyanLabel() = default;

void NyanLabel::SetRole(LabelRole role)
{
    _role = role;
    update();
}

auto NyanLabel::Role() const -> LabelRole
{
    return _role;
}

void NyanLabel::SetElideMode(Qt::TextElideMode mode)
{
    _elideMode = mode;
    update();
}

auto NyanLabel::ElideMode() const -> Qt::TextElideMode
{
    return _elideMode;
}

void NyanLabel::paintEvent(QPaintEvent* /*event*/)
{
    if (text().isEmpty()) {
        return;
    }

    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto istate = !isEnabled() ? InteractionState::Disabled
                                     : InteractionState::Normal;
    const auto style = Theme().Resolve(WidgetKind::Label, 0, istate);

    // Override font with role-specific font (Label has multiple font roles)
    const FontRole fontRole = ToFontRole(_role);
    const auto& fontSpec = Theme().Font(fontRole);
    QFont f(fontSpec.family, fontSpec.sizeInPt, fontSpec.weight, fontSpec.italic);
    p.setFont(f);
    p.setOpacity(style.opacity);
    p.setPen(style.foreground);

    // Elide text if necessary
    const QFontMetrics fm(f);
    const QString elidedText = fm.elidedText(text(), _elideMode, rect().width());
    p.drawText(rect(), alignment(), elidedText);
}

void NyanLabel::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui