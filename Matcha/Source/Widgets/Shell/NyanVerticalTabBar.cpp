#include <Matcha/Widgets/Shell/NyanVerticalTabBar.h>

#include <QPainter>
#include <QStyleOptionTab>

namespace matcha::gui {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

NyanVerticalTabBar::NyanVerticalTabBar(QWidget* parent)
    : QTabBar(parent)
{
    setDrawBase(false);
}

NyanVerticalTabBar::~NyanVerticalTabBar() = default;

// ---------------------------------------------------------------------------
// Vertical mode
// ---------------------------------------------------------------------------

void NyanVerticalTabBar::SetVertical(bool vertical)
{
    if (_vertical == vertical) { return; }
    _vertical = vertical;
    updateGeometry();
    update();
}

auto NyanVerticalTabBar::IsVertical() const -> bool
{
    return _vertical;
}

void NyanVerticalTabBar::SetRotateClockwise(bool cw)
{
    if (_rotateClockwise == cw) { return; }
    _rotateClockwise = cw;
    if (_vertical) {
        update();
    }
}

auto NyanVerticalTabBar::IsRotateClockwise() const -> bool
{
    return _rotateClockwise;
}

// ---------------------------------------------------------------------------
// CJK detection
// ---------------------------------------------------------------------------

auto NyanVerticalTabBar::IsCjk(char32_t ch) -> bool
{
    // CJK Unified Ideographs + Extension A/B + Compatibility
    if (ch >= 0x4E00 && ch <= 0x9FFF) { return true; }   // CJK Unified
    if (ch >= 0x3400 && ch <= 0x4DBF) { return true; }   // Extension A
    if (ch >= 0x20000 && ch <= 0x2A6DF) { return true; } // Extension B
    if (ch >= 0xF900 && ch <= 0xFAFF) { return true; }   // Compatibility
    // Hiragana + Katakana
    if (ch >= 0x3040 && ch <= 0x30FF) { return true; }
    // CJK Symbols and Punctuation
    if (ch >= 0x3000 && ch <= 0x303F) { return true; }
    // Fullwidth Latin / Halfwidth Katakana
    if (ch >= 0xFF00 && ch <= 0xFFEF) { return true; }
    // Hangul Syllables
    if (ch >= 0xAC00 && ch <= 0xD7AF) { return true; }
    return false;
}

// ---------------------------------------------------------------------------
// Vertical text measurement
// ---------------------------------------------------------------------------

auto NyanVerticalTabBar::MeasureVerticalText(const QString& text,
                                              const QFontMetrics& fm) -> QSize
{
    // For vertical text, we compute the total height needed and the max width.
    // CJK chars: each char stacks vertically, width = char width, height = char height * count
    // Latin runs: rotated 90 deg, so the "height" contributed is the run's horizontal width,
    //             and the "width" contributed is the font height.

    int totalHeight = 0;
    int maxWidth    = 0;

    // Helper: read one code point, advance index, return (codepoint, charLen)
    auto nextCodePoint = [&](int pos) -> std::pair<char32_t, int> {
        char32_t cp = text.at(pos).unicode();
        if (QChar::isHighSurrogate(static_cast<char16_t>(cp)) && (pos + 1) < text.size()) {
            cp = QChar::surrogateToUcs4(static_cast<char16_t>(cp),
                                        text.at(pos + 1).unicode());
            return {cp, 2};
        }
        return {cp, 1};
    };

    int i = 0;
    while (i < text.size()) {
        auto [ch, charLen] = nextCodePoint(i);

        if (IsCjk(ch)) {
            QString single = text.mid(i, charLen);
            QRect br = fm.boundingRect(single);
            totalHeight += br.height();
            maxWidth = std::max(maxWidth, br.width());
            i += charLen;
        } else {
            int runStart = i;
            while (i < text.size()) {
                auto [c2, len2] = nextCodePoint(i);
                if (IsCjk(c2)) { break; }
                i += len2;
            }
            QString run = text.mid(runStart, i - runStart);
            totalHeight += fm.horizontalAdvance(run);
            maxWidth = std::max(maxWidth, fm.height());
        }
    }

    return {maxWidth, totalHeight};
}

// ---------------------------------------------------------------------------
// tabSizeHint
// ---------------------------------------------------------------------------

auto NyanVerticalTabBar::tabSizeHint(int index) const -> QSize
{
    if (!_vertical) {
        return QTabBar::tabSizeHint(index);
    }

    const QString text = tabText(index);
    const QFontMetrics fm(font());

    constexpr int kPadH = 8;  // horizontal padding (becomes top/bottom in vertical)
    constexpr int kPadV = 12; // vertical padding

    QSize textSize = MeasureVerticalText(text, fm);

    int w = textSize.width() + (2 * kPadH);
    int h = textSize.height() + (2 * kPadV);

    // Enforce minimums
    w = std::max(w, 28);
    h = std::max(h, 40);

    return {w, h};
}

// ---------------------------------------------------------------------------
// paintEvent
// ---------------------------------------------------------------------------

void NyanVerticalTabBar::paintEvent(QPaintEvent* event)
{
    if (!_vertical) {
        QTabBar::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QStyleOptionTab opt;

    for (int i = 0; i < count(); ++i) {
        initStyleOption(&opt, i);

        const QRect tabRect = opt.rect;
        const bool selected = (i == currentIndex());
        const bool hovered  = opt.state & QStyle::State_MouseOver;

        // Draw tab background
        QColor bgColor;
        if (selected) {
            bgColor = opt.palette.color(QPalette::Window);
        } else if (hovered) {
            bgColor = opt.palette.color(QPalette::Midlight);
        } else {
            bgColor = opt.palette.color(QPalette::Button);
        }

        painter.fillRect(tabRect, bgColor);

        // Draw text
        painter.setPen(selected ? opt.palette.color(QPalette::WindowText)
                                : opt.palette.color(QPalette::ButtonText));

        PaintVerticalText(painter, tabRect, tabText(i), font());
    }
}

// ---------------------------------------------------------------------------
// PaintCjkChar -- single CJK character, centered, stacked top-to-bottom
// ---------------------------------------------------------------------------

void NyanVerticalTabBar::PaintCjkChar(QPainter& painter, const QRect& content,
                                       const QString& ch, const QFontMetrics& fm,
                                       int& yOff)
{
    QRect br = fm.boundingRect(ch);
    int x = content.left() + ((content.width() - br.width()) / 2);
    int y = content.top() + yOff + fm.ascent();
    painter.drawText(x, y, ch);
    yOff += br.height();
}

// ---------------------------------------------------------------------------
// PaintLatinRun -- rotated Latin text block
// ---------------------------------------------------------------------------

void NyanVerticalTabBar::PaintLatinRun(QPainter& painter, const QRect& content,
                                        const QString& run, const QFontMetrics& fm,
                                        int& yOff) const
{
    int runWidth  = fm.horizontalAdvance(run);
    int runHeight = fm.height();

    painter.save();

    int cx = content.left() + (content.width() / 2);
    int cy = content.top() + yOff + (runWidth / 2);

    painter.translate(cx, cy);
    painter.rotate(_rotateClockwise ? 90.0 : -90.0);

    QRect textRect(-runWidth / 2, -runHeight / 2, runWidth, runHeight);
    painter.drawText(textRect, Qt::AlignCenter, run);

    painter.restore();

    yOff += runWidth;
}

// ---------------------------------------------------------------------------
// PaintVerticalText -- dispatches to CJK / Latin helpers
// ---------------------------------------------------------------------------

void NyanVerticalTabBar::PaintVerticalText(QPainter& painter, const QRect& rect,
                                            const QString& text, const QFont& font) const
{
    const QFontMetrics fm(font);
    painter.setFont(font);

    constexpr int kPadH = 8;
    constexpr int kPadV = 12;
    QRect content = rect.adjusted(kPadH, kPadV, -kPadH, -kPadV);

    auto nextCodePoint = [&](int pos) -> std::pair<char32_t, int> {
        char32_t cp = text.at(pos).unicode();
        if (QChar::isHighSurrogate(static_cast<char16_t>(cp)) && (pos + 1) < text.size()) {
            cp = QChar::surrogateToUcs4(static_cast<char16_t>(cp),
                                        text.at(pos + 1).unicode());
            return {cp, 2};
        }
        return {cp, 1};
    };

    int yOff = 0;
    int i = 0;
    while (i < text.size()) {
        auto [ch, charLen] = nextCodePoint(i);

        if (IsCjk(ch)) {
            PaintCjkChar(painter, content, text.mid(i, charLen), fm, yOff);
            i += charLen;
        } else {
            int runStart = i;
            while (i < text.size()) {
                auto [c2, len2] = nextCodePoint(i);
                if (IsCjk(c2)) { break; }
                i += len2;
            }
            PaintLatinRun(painter, content, text.mid(runStart, i - runStart), fm, yOff);
        }
    }
}

} // namespace matcha::gui
