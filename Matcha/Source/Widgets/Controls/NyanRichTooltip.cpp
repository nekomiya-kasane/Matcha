/**
 * @file NyanRichTooltip.cpp
 * @brief Implementation of NyanRichTooltip two-tier delay tooltip.
 */

#include <Matcha/Widgets/Controls/NyanRichTooltip.h>

#include "../_Private/SimpleWidgetEventFilter.h"

#include <Matcha/Interaction/PopupPositioner.h>
#include <Matcha/Widgets/Controls/NyanLabel.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanRichTooltip::NyanRichTooltip(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint)
    , ThemeAware(WidgetKind::Tooltip)
    , _titleLabel(new NyanLabel( this))
    , _shortcutLabel(new NyanLabel( this))
    , _descriptionLabel(new NyanLabel( this))
    , _tier1Timer(new QTimer(this))
    , _tier2Timer(new QTimer(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setMaximumWidth(kMaxWidth);

    // Configure labels
    _titleLabel->SetRole(LabelRole::Name);
    _shortcutLabel->SetRole(LabelRole::Caption);
    _descriptionLabel->SetRole(LabelRole::Body);
    _descriptionLabel->setWordWrap(true);
    _descriptionLabel->setVisible(false);

    // Layout: title row + description + preview
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(kPadding + kShadowInset,
                                   kPadding + kShadowInset,
                                   kPadding + kShadowInset,
                                   kPadding + kShadowInset);
    mainLayout->setSpacing(6);

    // Title row: [title] ---stretch--- [shortcut]
    auto* titleRow = new QHBoxLayout;
    titleRow->setSpacing(8);
    titleRow->addWidget(_titleLabel);
    titleRow->addStretch(1);
    titleRow->addWidget(_shortcutLabel);
    mainLayout->addLayout(titleRow);

    // Description (Tier 2 only)
    mainLayout->addWidget(_descriptionLabel);

    // Timers
    _tier1Timer->setSingleShot(true);
    _tier2Timer->setSingleShot(true);

    connect(_tier1Timer, &QTimer::timeout, this, &NyanRichTooltip::ShowTier1);
    connect(_tier2Timer, &QTimer::timeout, this, &NyanRichTooltip::ExpandToTier2);
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanRichTooltip::~NyanRichTooltip() = default;

// ============================================================================
// Content Setters
// ============================================================================

void NyanRichTooltip::SetIcon(const QPixmap& icon)
{
    _icon = icon.scaled(kIconSize, kIconSize, Qt::KeepAspectRatio,
                        Qt::SmoothTransformation);
    update();
}

void NyanRichTooltip::SetTitle(const QString& title)
{
    _titleLabel->setText(title);
}

auto NyanRichTooltip::Title() const -> QString
{
    return _titleLabel->text();
}

void NyanRichTooltip::SetDescription(const QString& description)
{
    _descriptionLabel->setText(description);
}

void NyanRichTooltip::SetShortcut(const QString& shortcut)
{
    _shortcutLabel->setText(shortcut);
}

void NyanRichTooltip::SetPreviewImage(const QPixmap& image)
{
    _previewImage = image;
    _previewWidget = nullptr;
    update();
}

void NyanRichTooltip::SetPreviewWidget(QWidget* widget)
{
    if (_previewWidget != nullptr && _previewWidget != widget) {
        _previewWidget->setParent(nullptr);
        _previewWidget->deleteLater();
    }
    _previewWidget = widget;
    if (_previewWidget != nullptr) {
        _previewWidget->setParent(this);
        layout()->addWidget(_previewWidget);
        _previewWidget->setVisible(_tier == Tier::Detail);
    }
    _previewImage = {};
}

// ============================================================================
// Delay Configuration
// ============================================================================

void NyanRichTooltip::SetTier1Delay(int ms) { _tier1DelayMs = ms; }
auto NyanRichTooltip::Tier1Delay() const -> int { return _tier1DelayMs; }

void NyanRichTooltip::SetTier2Delay(int ms) { _tier2DelayMs = ms; }
auto NyanRichTooltip::Tier2Delay() const -> int { return _tier2DelayMs; }

// ============================================================================
// Show / Hide
// ============================================================================

void NyanRichTooltip::ShowForWidget(QWidget* trigger)
{
    _triggerWidget = trigger;

    // Cancel any pending timers
    _tier1Timer->stop();
    _tier2Timer->stop();

    if (_tier1DelayMs <= 0) {
        ShowTier1();
    } else {
        _tier1Timer->start(_tier1DelayMs);
    }
}

void NyanRichTooltip::Hide()
{
    _tier1Timer->stop();
    _tier2Timer->stop();
    _tier = Tier::Hidden;
    hide();
}

auto NyanRichTooltip::sizeHint() const -> QSize
{
    return QWidget::sizeHint();
}

// ============================================================================
// Painting
// ============================================================================

void NyanRichTooltip::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                     | QPainter::SmoothPixmapTransform);

    const auto style = Theme().Resolve(WidgetKind::Tooltip, 0, InteractionState::Normal);

    // -- Shadow --
    p.setPen(Qt::NoPen);
    for (int i = 0; i < kShadowInset; ++i) {
        const int alpha = 20 - (i * 4);
        p.setBrush(QColor(0, 0, 0, std::max(alpha, 2)));
        p.drawRoundedRect(rect().adjusted(i, i, -i, -i),
                          style.radiusPx + (kShadowInset - i),
                          style.radiusPx + (kShadowInset - i));
    }

    // -- Background --
    QRect bodyRect = rect().adjusted(kShadowInset, kShadowInset,
                                     -kShadowInset, -kShadowInset);
    p.setPen(QPen(style.border, style.borderWidthPx));
    p.setBrush(style.background);
    p.drawRoundedRect(bodyRect, style.radiusPx, style.radiusPx);

    // -- Icon (painted manually, left of title) --
    if (!_icon.isNull()) {
        const int iconX = bodyRect.left() + kPadding;
        const int iconY = bodyRect.top() + kPadding;
        p.drawPixmap(iconX, iconY, _icon);
    }

    // -- Preview image (Tier 2 only, below description) --
    if (_tier == Tier::Detail && !_previewImage.isNull() && _previewWidget == nullptr) {
        const int imgX = bodyRect.left() + kPadding;
        const int imgY = bodyRect.bottom() - kPadding - _previewImage.height();
        p.drawPixmap(imgX, imgY, _previewImage.scaledToWidth(
                         bodyRect.width() - (kPadding * 2), Qt::SmoothTransformation));
    }
}

void NyanRichTooltip::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private
// ============================================================================

void NyanRichTooltip::ShowTier1()
{
    _tier = Tier::Brief;
    UpdateTierVisibility();

    if (_triggerWidget != nullptr) {
        PositionNear(_triggerWidget);
    }

    adjustSize();
    show();
    raise();

    // Start Tier 2 timer
    if (_tier2DelayMs > 0) {
        _tier2Timer->start(_tier2DelayMs);
    }
}

void NyanRichTooltip::ExpandToTier2()
{
    _tier = Tier::Detail;
    UpdateTierVisibility();
    adjustSize();

    if (_triggerWidget != nullptr) {
        PositionNear(_triggerWidget);
    }

    update();
}

void NyanRichTooltip::PositionNear(QWidget* trigger)
{
    const QPoint triggerGlobal = trigger->mapToGlobal(QPoint(0, 0));
    const QSize mySize = sizeHint();
    resize(mySize);

    // Determine viewport
    fw::Rect viewport{};
    QScreen* screen = QApplication::screenAt(triggerGlobal);
    if (screen == nullptr) { screen = QApplication::primaryScreen(); }
    if (screen != nullptr) {
        const QRect avail = screen->availableGeometry();
        viewport = {avail.x(), avail.y(), avail.width(), avail.height()};
    }

    fw::PopupRequest req;
    req.anchorRect = {triggerGlobal.x(), triggerGlobal.y(),
                      trigger->width(), trigger->height()};
    req.popupSize  = {mySize.width(), mySize.height()};
    req.placement  = fw::PopupPlacement::BottomStart;
    req.offset     = {0, 4};
    req.viewport   = viewport;
    req.strategy   = fw::OverflowStrategy::All;

    const auto result = fw::PopupPositioner::Compute(req);
    move(result.position.x, result.position.y);
}

void NyanRichTooltip::UpdateTierVisibility()
{
    const bool detail = (_tier == Tier::Detail);

    _descriptionLabel->setVisible(detail && !_descriptionLabel->text().isEmpty());

    if (_previewWidget != nullptr) {
        _previewWidget->setVisible(detail);
    }
}

} // namespace matcha::gui