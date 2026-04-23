/**
 * @file NyanGroupBox.cpp
 * @brief Implementation of NyanGroupBox themed group box.
 */

#include <Matcha/Widgets/Controls/NyanGroupBox.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <Matcha/Tree/WidgetNode.h>
#include <Matcha/Animation/IAnimationService.h>

#include <QMouseEvent>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanGroupBox::NyanGroupBox(QWidget* parent)
    : QGroupBox(parent)
    , ThemeAware(WidgetKind::GroupBox)
{
    ApplyStyle();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanGroupBox::NyanGroupBox(const QString& title, QWidget* parent)
    : QGroupBox(title, parent)
    , ThemeAware(WidgetKind::GroupBox)
{
    ApplyStyle();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanGroupBox::~NyanGroupBox() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanGroupBox::SetCollapsible(bool collapsible)
{
    _collapsible = collapsible;
    if (collapsible && _expandedHeight == 0) {
        _expandedHeight = sizeHint().height();
    }
}

auto NyanGroupBox::IsCollapsible() const -> bool
{
    return _collapsible;
}

void NyanGroupBox::SetCollapsed(bool collapsed)
{
    if (!_collapsible || _collapsed == collapsed) {
        return;
    }
    _collapsed = collapsed;
    AnimateCollapse(collapsed);
    emit CollapsedChanged(collapsed);
}

auto NyanGroupBox::IsCollapsed() const -> bool
{
    return _collapsed;
}

// ============================================================================
// Events
// ============================================================================

void NyanGroupBox::mousePressEvent(QMouseEvent* event)
{
    if (_collapsible && event->button() == Qt::LeftButton) {
        // Check if click is in title area (top 24px).
        if (event->position().y() < 24) {
            SetCollapsed(!_collapsed);
            event->accept();
            return;
        }
    }
    QGroupBox::mousePressEvent(event);
}

void NyanGroupBox::OnThemeChanged()
{
    ApplyStyle();
    update();
}

// ============================================================================
// Private
// ============================================================================

void NyanGroupBox::ApplyStyle()
{
    const auto style = Theme().Resolve(WidgetKind::GroupBox, 0, InteractionState::Normal);

    const QString css = QStringLiteral(
        "QGroupBox { background: %1; border: %2px solid %3;"
        " border-radius: %4px; margin-top: 12px; padding-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left;"
        " padding: 0 4px; color: %5; }"
    ).arg(style.background.name(),
          QString::number(style.borderWidthPx),
          style.border.name(),
          QString::number(style.radiusPx),
          style.foreground.name());

    setStyleSheet(css);
    setFont(style.font);
}

void NyanGroupBox::AnimateCollapse(bool collapsed)
{
    if (_expandedHeight == 0) {
        _expandedHeight = sizeHint().height();
    }

    const int startH = height();
    const int endH = collapsed ? 24 : _expandedHeight;  // 24px = title height

    auto* node = fw::WidgetNode::FromWidget(this);
    if (node != nullptr) {
        node->AnimateProperty(fw::AnimationPropertyId::MaximumHeight,
                              fw::AnimatableValue::FromInt(startH),
                              fw::AnimatableValue::FromInt(endH),
                              fw::AnimationsToken::motionDurationDefault);
        node->AnimateProperty(fw::AnimationPropertyId::MinimumHeight,
                              fw::AnimatableValue::FromInt(startH),
                              fw::AnimatableValue::FromInt(endH),
                              fw::AnimationsToken::motionDurationDefault);
    } else {
        setMaximumHeight(endH);
        setMinimumHeight(endH);
    }
}

} // namespace matcha::gui