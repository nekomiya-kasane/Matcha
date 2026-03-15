/**
 * @file MnemonicState.cpp
 * @brief MnemonicState implementation — parsing, visibility, and rendering.
 */

#include "Matcha/Interaction/Focus/MnemonicState.h"

#include <QFontMetrics>
#include <QPainter>
#include <QRect>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace matcha::gui {

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

MnemonicState::MnemonicState(QObject* parent)
    : QObject(parent)
{
}

MnemonicState::~MnemonicState() = default;

// ---------------------------------------------------------------------------
// Static parsing
// ---------------------------------------------------------------------------

auto MnemonicState::Parse(const QString& rawText) -> MnemonicParseResult
{
    MnemonicParseResult result;
    result.mnemonicChar = QChar::Null;
    result.underlineIndex = -1;

    if (rawText.isEmpty()) {
        return result;
    }

    QString display;
    display.reserve(rawText.size());

    bool mnemonicFound = false;

    for (int i = 0; i < rawText.size(); ++i) {
        if (rawText[i] == QLatin1Char('&')) {
            // Check next character
            if (i + 1 < rawText.size()) {
                if (rawText[i + 1] == QLatin1Char('&')) {
                    // Escaped '&&' -> literal '&'
                    display.append(QLatin1Char('&'));
                    ++i; // skip second '&'
                } else if (!mnemonicFound) {
                    // First unescaped '&' -> mnemonic
                    result.underlineIndex = display.size();
                    result.mnemonicChar = rawText[i + 1];
                    display.append(rawText[i + 1]);
                    mnemonicFound = true;
                    ++i; // skip mnemonic char (already appended)
                } else {
                    // Subsequent '&' after mnemonic already found -> treat as literal
                    display.append(rawText[i + 1]);
                    ++i;
                }
            }
            // Trailing '&' at end of string is silently dropped
        } else {
            display.append(rawText[i]);
        }
    }

    result.displayText = display;
    return result;
}

// ---------------------------------------------------------------------------
// Visibility state
// ---------------------------------------------------------------------------

auto MnemonicState::ShouldShowUnderline() const -> bool
{
    return _altHeld || _altActivated || _alwaysShow;
}

void MnemonicState::SetAltHeld(bool held)
{
    if (_altHeld == held) {
        return;
    }
    bool wasShouldShow = ShouldShowUnderline();
    _altHeld = held;
    bool nowShouldShow = ShouldShowUnderline();
    if (wasShouldShow != nowShouldShow) {
        Q_EMIT UnderlineVisibilityChanged(nowShouldShow);
    }
}

auto MnemonicState::IsAltHeld() const -> bool
{
    return _altHeld;
}

void MnemonicState::SetAlwaysShow(bool always)
{
    if (_alwaysShow == always) {
        return;
    }
    bool wasShouldShow = ShouldShowUnderline();
    _alwaysShow = always;
    bool nowShouldShow = ShouldShowUnderline();
    if (wasShouldShow != nowShouldShow) {
        Q_EMIT UnderlineVisibilityChanged(nowShouldShow);
    }
}

auto MnemonicState::IsAlwaysShow() const -> bool
{
    return _alwaysShow;
}

void MnemonicState::SetAltActivated(bool activated)
{
    if (_altActivated == activated) {
        return;
    }
    bool wasShouldShow = ShouldShowUnderline();
    _altActivated = activated;
    bool nowShouldShow = ShouldShowUnderline();
    if (wasShouldShow != nowShouldShow) {
        Q_EMIT UnderlineVisibilityChanged(nowShouldShow);
    }
}

auto MnemonicState::IsAltActivated() const -> bool
{
    return _altActivated;
}

void MnemonicState::Deactivate()
{
    SetAltActivated(false);
}

auto MnemonicState::QueryOsKeyboardCues() -> bool
{
#ifdef Q_OS_WIN
    BOOL cues = FALSE;
    if (SystemParametersInfoW(SPI_GETKEYBOARDCUES, 0, &cues, 0)) {
        return cues != FALSE;
    }
#endif
    return false;
}

// ---------------------------------------------------------------------------
// Rendering helper
// ---------------------------------------------------------------------------

void MnemonicState::DrawMnemonicText(QPainter& painter, const QRect& rect,
                                      int flags, const QString& rawText,
                                      bool showUnderline)
{
    auto parsed = Parse(rawText);

    // Draw the display text (without '&' markers)
    painter.drawText(rect, flags, parsed.displayText);

    // Draw underline if requested and a mnemonic exists
    if (!showUnderline || parsed.underlineIndex < 0) {
        return;
    }

    const QFontMetrics fm(painter.font());

    // Calculate the bounding rect of the drawn text to determine the baseline
    QRect boundingRect;
    painter.drawText(rect, flags, parsed.displayText, &boundingRect);

    // X offset of the mnemonic character within the text
    int xOffsetInText = fm.horizontalAdvance(parsed.displayText, parsed.underlineIndex);
    int charWidth = fm.horizontalAdvance(parsed.mnemonicChar);

    // Determine text start X based on alignment
    int textStartX = boundingRect.left();

    // Y position: baseline + 1px
    // QFontMetrics::ascent() gives distance from top of bounding rect to baseline
    int baselineY = boundingRect.top() + fm.ascent();
    int underlineY = baselineY + 1;

    // Draw the underline
    QPen savedPen = painter.pen();
    QPen underlinePen = savedPen;
    underlinePen.setWidth(1);
    painter.setPen(underlinePen);
    painter.drawLine(textStartX + xOffsetInText, underlineY,
                     textStartX + xOffsetInText + charWidth, underlineY);
    painter.setPen(savedPen);
}

} // namespace matcha::gui
