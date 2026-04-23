#include <Matcha/Widgets/Menu/NyanMenuSeparator.h>

#include <QPainter>

namespace matcha::gui {

NyanMenuSeparator::NyanMenuSeparator(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::MenuSeparator)
{
    setFixedHeight(kHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

NyanMenuSeparator::~NyanMenuSeparator() = default;

// -- Size hints --

auto NyanMenuSeparator::sizeHint() const -> QSize
{
    return {152, kHeight};
}

auto NyanMenuSeparator::minimumSizeHint() const -> QSize
{
    return {50, kHeight};
}

// -- Paint --

void NyanMenuSeparator::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);

    const auto& theme = Theme();

    // Background
    p.fillRect(rect(), theme.Color(ColorToken::colorPrimary));

    // Separator line at bottom
    p.setPen(QPen(theme.Color(ColorToken::colorBorder), 1));
    p.drawLine(rect().left(), rect().bottom(), rect().right(), rect().bottom());
}

void NyanMenuSeparator::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
