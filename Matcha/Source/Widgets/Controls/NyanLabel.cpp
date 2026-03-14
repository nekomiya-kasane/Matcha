/**
 * @file NyanLabel.cpp
 * @brief Implementation of NyanLabel role-based themed label.
 */

#include <Matcha/Widgets/Controls/NyanLabel.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <Matcha/UiNodes/Core/MnemonicManager.h>
#include <Matcha/Widgets/Core/MnemonicState.h>

#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>

namespace matcha::gui {

NyanLabel::NyanLabel(QWidget* parent)
    : QLabel(parent)
    , ThemeAware(WidgetKind::Label)
{
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanLabel::NyanLabel(const QString& text, QWidget* parent)
    : QLabel(text, parent)
    , ThemeAware(WidgetKind::Label)
{
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanLabel::~NyanLabel()
{
    UnregisterMnemonic();
}

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

void NyanLabel::SetBuddy(QWidget* buddy)
{
    _buddy = buddy;
    UpdateMnemonicRegistration();
}

auto NyanLabel::Buddy() const -> QWidget*
{
    return _buddy;
}

void NyanLabel::setText(const QString& text)
{
    QLabel::setText(text);
    UpdateMnemonicRegistration();
    update();
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

    // Check if mnemonic underline should be shown
    auto* ms = GetMnemonicState();
    bool showUnderline = (ms != nullptr) && ms->ShouldShowUnderline();

    // Elide text if necessary
    const QFontMetrics fm(f);
    const QString elidedText = fm.elidedText(text(), _elideMode, rect().width());

    // Use DrawMnemonicText if the text has a mnemonic marker
    if (text().contains(QLatin1Char('&'))) {
        MnemonicState::DrawMnemonicText(p, rect(), alignment(), text(), showUnderline);
    } else {
        p.drawText(rect(), alignment(), elidedText);
    }
}

void NyanLabel::OnThemeChanged()
{
    update();
}

void NyanLabel::UpdateMnemonicRegistration()
{
    UnregisterMnemonic();

    if (_buddy == nullptr) { return; }

    auto parsed = MnemonicState::Parse(text());
    if (parsed.mnemonicChar.isNull()) { return; }

    auto* mgr = fw::GetMnemonicManager();
    if (mgr == nullptr) { return; }

    char16_t ch = parsed.mnemonicChar.unicode();
    QWidget* buddy = _buddy;
    _mnemonicId = mgr->Register({
        fw::MnemonicScope::Global,
        ch,
        [buddy]() {
            if (buddy != nullptr) {
                buddy->setFocus(Qt::ShortcutFocusReason);
            }
        },
        {} // no aliveToken — label outlives its registration
    });
}

void NyanLabel::UnregisterMnemonic()
{
    if (_mnemonicId == 0) { return; }

    if (auto* mgr = fw::GetMnemonicManager()) {
        mgr->Unregister(_mnemonicId);
    }
    _mnemonicId = 0;
}

} // namespace matcha::gui