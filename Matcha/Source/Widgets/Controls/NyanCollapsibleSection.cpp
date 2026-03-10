/**
 * @file NyanCollapsibleSection.cpp
 * @brief Implementation of NyanCollapsibleSection collapsible content group.
 */

#include <Matcha/Widgets/Controls/NyanCollapsibleSection.h>

#include "../Core/InteractionEventFilter.h"

#include <Matcha/UiNodes/Core/WidgetNode.h>
#include <Matcha/Widgets/Core/IAnimationService.h>

#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QTimer>

#include <array>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanCollapsibleSection::NyanCollapsibleSection(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::CollapsibleSection)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, kTitleHeight, 0, 0);
    layout->setSpacing(0);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanCollapsibleSection::~NyanCollapsibleSection() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanCollapsibleSection::SetTitle(const QString& title)
{
    _title = title;
    update();
}

auto NyanCollapsibleSection::Title() const -> QString
{
    return _title;
}

void NyanCollapsibleSection::SetExpanded(bool expanded)
{
    if (_expanded == expanded) {
        return;
    }
    _expanded = expanded;
    AnimateExpand(expanded);
    emit ExpandToggled(expanded);
}

auto NyanCollapsibleSection::IsExpanded() const -> bool
{
    return _expanded;
}

void NyanCollapsibleSection::SetContent(QWidget* content)
{
    if (_content != nullptr) {
        layout()->removeWidget(_content);
        delete _content;
    }
    _content = content;
    if (_content != nullptr) {
        _content->setParent(this);
        layout()->addWidget(_content);
        _contentHeight = _content->sizeHint().height();
        UpdateContentVisibility();
    }
    updateGeometry();
}

auto NyanCollapsibleSection::Content() const -> QWidget*
{
    return _content;
}

auto NyanCollapsibleSection::sizeHint() const -> QSize
{
    int h = kTitleHeight;
    if (_expanded && _content != nullptr) {
        h += _content->sizeHint().height();
    }
    int w = 200;
    if (_content != nullptr) {
        w = std::max(w, _content->sizeHint().width());
    }
    return {w, h};
}

auto NyanCollapsibleSection::minimumSizeHint() const -> QSize
{
    return {100, kTitleHeight};
}

// ============================================================================
// Events
// ============================================================================

void NyanCollapsibleSection::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const auto style = Theme().Resolve(WidgetKind::CollapsibleSection, 0, InteractionState::Normal);

    // Title background.
    const QRect titleRect(0, 0, width(), kTitleHeight);
    p.fillRect(titleRect, style.background);

    // Arrow.
    const int arrowX = kHPadding;
    const int arrowY = (kTitleHeight - kArrowSize) / 2;
    const QRectF arrowRect(arrowX, arrowY, kArrowSize, kArrowSize);

    p.save();
    p.translate(arrowRect.center());
    p.rotate(_arrowRotation);
    p.translate(-arrowRect.center());

    p.setPen(QPen(style.border, 1.5));
    p.setBrush(Qt::NoBrush);

    // Draw chevron pointing right (will be rotated).
    const std::array<QPointF, 3> chevron = {{
        {arrowRect.left() + 3, arrowRect.top() + 2},
        {arrowRect.right() - 3, arrowRect.center().y()},
        {arrowRect.left() + 3, arrowRect.bottom() - 2}
    }};
    p.drawPolyline(chevron.data(), static_cast<int>(chevron.size()));
    p.restore();

    // Title text.
    const int textX = kHPadding + kArrowSize + kHPadding;
    const QRect textRect(textX, 0, width() - textX - kHPadding, kTitleHeight);

    p.setFont(style.font);
    p.setPen(style.foreground);
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, _title);
}

void NyanCollapsibleSection::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && event->position().y() < kTitleHeight) {
        SetExpanded(!_expanded);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void NyanCollapsibleSection::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private
// ============================================================================

void NyanCollapsibleSection::AnimateExpand(bool expanded)
{
    const int durationMs = Theme().AnimationMs(AnimationToken::Normal);

    // Animate arrow rotation.
    auto* arrowAnim = new QPropertyAnimation(this, "arrowRotation", this);
    arrowAnim->setDuration(durationMs);
    arrowAnim->setStartValue(_arrowRotation);
    arrowAnim->setEndValue(expanded ? 90.0 : 0.0);
    connect(arrowAnim, &QPropertyAnimation::valueChanged, this, [this](const QVariant& v) {
        _arrowRotation = v.toReal();
        update();
    });
    arrowAnim->start(QAbstractAnimation::DeleteWhenStopped);

    // Animate height via AnimationService (centralized).
    if (_content != nullptr) {
        _contentHeight = _content->sizeHint().height();
        const int startH = height();
        const int endH = expanded ? (kTitleHeight + _contentHeight) : kTitleHeight;

        auto* node = fw::WidgetNode::FromWidget(this);
        if (node != nullptr) {
            node->AnimateProperty(fw::AnimationPropertyId::MaximumHeight,
                                  fw::AnimatableValue::FromInt(startH),
                                  fw::AnimatableValue::FromInt(endH),
                                  fw::AnimationToken::Normal);
            node->AnimateProperty(fw::AnimationPropertyId::MinimumHeight,
                                  fw::AnimatableValue::FromInt(startH),
                                  fw::AnimatableValue::FromInt(endH),
                                  fw::AnimationToken::Normal);
        } else {
            setMaximumHeight(endH);
            setMinimumHeight(endH);
        }

        // Defer content visibility update until animation settles.
        // Use a single-shot timer approximating animation duration.
        QTimer::singleShot(durationMs, this, [this]() {
            UpdateContentVisibility();
        });
    }
}

void NyanCollapsibleSection::UpdateContentVisibility()
{
    if (_content != nullptr) {
        _content->setVisible(_expanded);
    }
}

} // namespace matcha::gui