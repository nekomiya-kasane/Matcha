/**
 * @file NyanStackedWidget.cpp
 * @brief Implementation of NyanStackedWidget themed stacked container.
 */

#include <Matcha/Widgets/Shell/NyanStackedWidget.h>

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanStackedWidget::NyanStackedWidget(QWidget* parent)
    : QStackedWidget(parent)
    , ThemeAware(WidgetKind::StackedWidget)
{
}

NyanStackedWidget::~NyanStackedWidget() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanStackedWidget::SetCrossFadeEnabled(bool enabled)
{
    _crossFade = enabled;
}

auto NyanStackedWidget::CrossFadeEnabled() const -> bool
{
    return _crossFade;
}

void NyanStackedWidget::SetCurrentIndex(int index)
{
    if (index == currentIndex() || index < 0 || index >= count()) {
        return;
    }

    if (!_crossFade) {
        setCurrentIndex(index);
        return;
    }

    // Cross-fade: fade out current, switch, fade in new.
    QWidget* outgoing = currentWidget();
    setCurrentIndex(index);
    QWidget* incoming = currentWidget();

    if (outgoing == nullptr || incoming == nullptr) {
        return;
    }

    const int durationMs = Theme().AnimationMs(AnimationToken::Normal);
    if (durationMs <= 0) {
        return;
    }

    // Fade-in the incoming widget.
    auto* effect = new QGraphicsOpacityEffect(incoming);
    incoming->setGraphicsEffect(effect);

    auto* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(durationMs);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    connect(anim, &QPropertyAnimation::finished, incoming, [incoming]() {
        incoming->setGraphicsEffect(nullptr);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ============================================================================
// Theme
// ============================================================================

void NyanStackedWidget::OnThemeChanged()
{
    update();
}

} // namespace matcha::gui
