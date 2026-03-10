/**
 * @file NyanColorPicker.cpp
 * @brief Implementation of NyanColorPicker themed color selection panel.
 */

#include <Matcha/Widgets/Controls/NyanColorPicker.h>

#include "../Core/InteractionEventFilter.h"

#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QSlider>
#include <QVBoxLayout>

#include <cmath>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanColorPicker::NyanColorPicker(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::ColorPicker)
{
    // Default preset colors.
    _presetColors = {{
        QColor(255, 0, 0),     // Red
        QColor(255, 165, 0),   // Orange
        QColor(255, 255, 0),   // Yellow
        QColor(0, 255, 0),     // Green
        QColor(0, 255, 255),   // Cyan
        QColor(0, 0, 255),     // Blue
        QColor(128, 0, 128),   // Purple
        QColor(255, 255, 255)  // White
    }};

    SetupUi();
    setMouseTracking(true);
    _interactionFilter = new InteractionEventFilter(this, nullptr);
}

NyanColorPicker::~NyanColorPicker() = default;

// ============================================================================
// Public API
// ============================================================================

void NyanColorPicker::SetColor(const QColor& color)
{
    _hue = color.hsvHue();
    if (_hue < 0) {
        _hue = 0;
    }
    _saturation = color.hsvSaturation();
    _value = color.value();
    _alpha = color.alpha();

    UpdateUiFromColor();
    update();
}

auto NyanColorPicker::Color() const -> QColor
{
    QColor c = QColor::fromHsv(_hue, _saturation, _value);
    c.setAlpha(_alpha);
    return c;
}

void NyanColorPicker::SetAlphaEnabled(bool enabled)
{
    _alphaEnabled = enabled;
    if (_alphaSlider != nullptr) {
        _alphaSlider->setVisible(enabled);
    }
    if (_alphaLabel != nullptr) {
        _alphaLabel->setVisible(enabled);
    }
    updateGeometry();
}

auto NyanColorPicker::IsAlphaEnabled() const -> bool
{
    return _alphaEnabled;
}

auto NyanColorPicker::sizeHint() const -> QSize
{
    return {280, 320};
}

auto NyanColorPicker::minimumSizeHint() const -> QSize
{
    return {250, 280};
}

// ============================================================================
// Painting
// ============================================================================

void NyanColorPicker::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const auto style = Theme().Resolve(WidgetKind::ColorPicker, 0, InteractionState::Normal);

    // Background.
    p.fillRect(rect(), style.background);

    PaintHsvWheel(p);
    PaintValueSlider(p);
    PaintPreview(p);
}

void NyanColorPicker::PaintHsvWheel(QPainter& p)
{
    const QRect wheelRect = HsvWheelRect();
    const int cx = wheelRect.center().x();
    const int cy = wheelRect.center().y();
    const int radius = wheelRect.width() / 2;

    // Draw HSV wheel (hue around circle, saturation from center).
    QImage wheelImage(wheelRect.size(), QImage::Format_ARGB32);
    wheelImage.fill(Qt::transparent);

    for (int y = 0; y < wheelRect.height(); ++y) {
        for (int x = 0; x < wheelRect.width(); ++x) {
            const int dx = x - radius;
            const int dy = y - radius;
            const double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));

            if (dist <= radius) {
                const double angle = std::atan2(static_cast<double>(dy), static_cast<double>(dx));
                const int hue = static_cast<int>((angle + M_PI) * 180.0 / M_PI) % 360;
                const int sat = static_cast<int>((dist / radius) * 255.0);
                const QColor c = QColor::fromHsv(hue, sat, _value);
                wheelImage.setPixelColor(x, y, c);
            }
        }
    }

    p.drawImage(wheelRect.topLeft(), wheelImage);

    // Draw selection indicator.
    const double selAngle = (_hue * M_PI / 180.0) - M_PI;
    const double selDist = (_saturation / 255.0) * radius;
    const int selX = cx + static_cast<int>(selDist * std::cos(selAngle));
    const int selY = cy + static_cast<int>(selDist * std::sin(selAngle));

    p.setPen(QPen(Qt::white, 2));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPoint(selX, selY), 6, 6);
    p.setPen(QPen(Qt::black, 1));
    p.drawEllipse(QPoint(selX, selY), 7, 7);
}

void NyanColorPicker::PaintValueSlider(QPainter& p)
{
    const QRect sliderRect = ValueSliderRect();

    // Draw value gradient.
    QLinearGradient grad(sliderRect.topLeft(), sliderRect.bottomLeft());
    grad.setColorAt(0.0, QColor::fromHsv(_hue, _saturation, 255));
    grad.setColorAt(1.0, QColor::fromHsv(_hue, _saturation, 0));

    p.fillRect(sliderRect, grad);

    // Draw selection indicator.
    const int selY = sliderRect.top() + static_cast<int>((1.0 - _value / 255.0) * sliderRect.height());
    p.setPen(QPen(Qt::white, 2));
    p.drawLine(sliderRect.left(), selY, sliderRect.right(), selY);
    p.setPen(QPen(Qt::black, 1));
    p.drawLine(sliderRect.left() - 1, selY, sliderRect.right() + 1, selY);
}

void NyanColorPicker::PaintPreview(QPainter& p)
{
    const QRect previewRect = PreviewRect();
    const QColor current = Color();

    // Checkerboard for alpha.
    if (_alphaEnabled && current.alpha() < 255) {
        const int checkSize = 8;
        for (int y = previewRect.top(); y < previewRect.bottom(); y += checkSize) {
            for (int x = previewRect.left(); x < previewRect.right(); x += checkSize) {
                const bool light = ((x / checkSize) + (y / checkSize)) % 2 == 0;
                p.fillRect(QRect(x, y, checkSize, checkSize), light ? Qt::white : Qt::lightGray);
            }
        }
    }

    p.fillRect(previewRect, current);

    const auto previewStyle = Theme().Resolve(WidgetKind::ColorPicker, 0, InteractionState::Normal);
    p.setPen(previewStyle.border);
    p.setBrush(Qt::NoBrush);
    p.drawRect(previewRect);
}

// ============================================================================
// Mouse Events
// ============================================================================

void NyanColorPicker::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint pos = event->pos();

    if (HitTestHsvWheel(pos)) {
        _draggingWheel = true;
        // Update hue/saturation from position.
        const QRect wheelRect = HsvWheelRect();
        const int cx = wheelRect.center().x();
        const int cy = wheelRect.center().y();
        const int radius = wheelRect.width() / 2;

        const int dx = pos.x() - cx;
        const int dy = pos.y() - cy;
        const double dist = std::min(std::sqrt(static_cast<double>(dx * dx + dy * dy)), static_cast<double>(radius));
        const double angle = std::atan2(static_cast<double>(dy), static_cast<double>(dx));

        _hue = (static_cast<int>((angle + M_PI) * 180.0 / M_PI)) % 360;
        _saturation = static_cast<int>((dist / radius) * 255.0);

        UpdateUiFromColor();
        emit ColorChanged(Color());
        update();
        return;
    }

    if (HitTestValueSlider(pos)) {
        _draggingValue = true;
        const QRect sliderRect = ValueSliderRect();
        const double ratio = 1.0 - static_cast<double>(pos.y() - sliderRect.top()) / sliderRect.height();
        _value = std::clamp(static_cast<int>(ratio * 255.0), 0, 255);

        UpdateUiFromColor();
        emit ColorChanged(Color());
        update();
        return;
    }

    // Check preset swatches.
    const int swatchY = HsvWheelRect().bottom() + 20;
    const int swatchStartX = 10;
    for (int i = 0; i < kSwatchCount; ++i) {
        const QRect swatchRect(swatchStartX + i * (kSwatchSize + 4), swatchY, kSwatchSize, kSwatchSize);
        if (swatchRect.contains(pos)) {
            SetColor(_presetColors[static_cast<size_t>(i)]);
            emit ColorChanged(Color());
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void NyanColorPicker::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->pos();

    if (_draggingWheel) {
        const QRect wheelRect = HsvWheelRect();
        const int cx = wheelRect.center().x();
        const int cy = wheelRect.center().y();
        const int radius = wheelRect.width() / 2;

        const int dx = pos.x() - cx;
        const int dy = pos.y() - cy;
        const double dist = std::min(std::sqrt(static_cast<double>(dx * dx + dy * dy)), static_cast<double>(radius));
        const double angle = std::atan2(static_cast<double>(dy), static_cast<double>(dx));

        _hue = (static_cast<int>((angle + M_PI) * 180.0 / M_PI)) % 360;
        _saturation = static_cast<int>((dist / radius) * 255.0);

        UpdateUiFromColor();
        emit ColorChanged(Color());
        update();
        return;
    }

    if (_draggingValue) {
        const QRect sliderRect = ValueSliderRect();
        const double ratio = 1.0 - static_cast<double>(pos.y() - sliderRect.top()) / sliderRect.height();
        _value = std::clamp(static_cast<int>(ratio * 255.0), 0, 255);

        UpdateUiFromColor();
        emit ColorChanged(Color());
        update();
        return;
    }

    QWidget::mouseMoveEvent(event);
}

void NyanColorPicker::mouseReleaseEvent(QMouseEvent* event)
{
    _draggingWheel = false;
    _draggingValue = false;
    QWidget::mouseReleaseEvent(event);
}

void NyanColorPicker::OnThemeChanged()
{
    update();
}

// ============================================================================
// Private Helpers
// ============================================================================

void NyanColorPicker::SetupUi()
{
    const auto setupStyle = Theme().Resolve(WidgetKind::ColorPicker, 0, InteractionState::Normal);

    const QString inputStyle = QStringLiteral(
        "background: %1; color: %2; border: 1px solid %3; border-radius: %4px; padding: 2px;"
    ).arg(setupStyle.background.name(), setupStyle.foreground.name(),
          setupStyle.border.name(), QString::number(static_cast<int>(setupStyle.radiusPx)));

    // Layout: wheel area at top, inputs below.
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    // Spacer for wheel area (painted manually).
    mainLayout->addSpacing(kWheelSize + 10);

    // Preset swatches (painted manually, but add spacer).
    mainLayout->addSpacing(kSwatchSize + 10);

    // RGB inputs.
    auto* rgbLayout = new QHBoxLayout();
    rgbLayout->setSpacing(4);

    auto* rLabel = new QLabel(QStringLiteral("R:"));
    _rEdit = new QLineEdit();
    _rEdit->setFixedWidth(40);
    _rEdit->setStyleSheet(inputStyle);
    connect(_rEdit, &QLineEdit::textChanged, this, &NyanColorPicker::UpdateFromRgb);

    auto* gLabel = new QLabel(QStringLiteral("G:"));
    _gEdit = new QLineEdit();
    _gEdit->setFixedWidth(40);
    _gEdit->setStyleSheet(inputStyle);
    connect(_gEdit, &QLineEdit::textChanged, this, &NyanColorPicker::UpdateFromRgb);

    auto* bLabel = new QLabel(QStringLiteral("B:"));
    _bEdit = new QLineEdit();
    _bEdit->setFixedWidth(40);
    _bEdit->setStyleSheet(inputStyle);
    connect(_bEdit, &QLineEdit::textChanged, this, &NyanColorPicker::UpdateFromRgb);

    rgbLayout->addWidget(rLabel);
    rgbLayout->addWidget(_rEdit);
    rgbLayout->addWidget(gLabel);
    rgbLayout->addWidget(_gEdit);
    rgbLayout->addWidget(bLabel);
    rgbLayout->addWidget(_bEdit);
    rgbLayout->addStretch();

    mainLayout->addLayout(rgbLayout);

    // Hex input.
    auto* hexLayout = new QHBoxLayout();
    hexLayout->setSpacing(4);

    auto* hexLabel = new QLabel(QStringLiteral("Hex:"));
    _hexEdit = new QLineEdit();
    _hexEdit->setFixedWidth(80);
    _hexEdit->setStyleSheet(inputStyle);
    connect(_hexEdit, &QLineEdit::textChanged, this, &NyanColorPicker::UpdateFromHex);

    hexLayout->addWidget(hexLabel);
    hexLayout->addWidget(_hexEdit);
    hexLayout->addStretch();

    mainLayout->addLayout(hexLayout);

    // Alpha slider.
    auto* alphaLayout = new QHBoxLayout();
    alphaLayout->setSpacing(4);

    _alphaLabel = new QLabel(QStringLiteral("Alpha:"));
    _alphaSlider = new QSlider(Qt::Horizontal);
    _alphaSlider->setRange(0, 255);
    _alphaSlider->setValue(255);
    connect(_alphaSlider, &QSlider::valueChanged, this, [this](int value) {
        _alpha = value;
        emit ColorChanged(Color());
    });

    alphaLayout->addWidget(_alphaLabel);
    alphaLayout->addWidget(_alphaSlider);

    _alphaLabel->setVisible(_alphaEnabled);
    _alphaSlider->setVisible(_alphaEnabled);

    mainLayout->addLayout(alphaLayout);

    // Buttons.
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    _eyedropperBtn = new QPushButton(QStringLiteral("Pick"));
    _eyedropperBtn->setFixedWidth(50);
    connect(_eyedropperBtn, &QPushButton::clicked, this, &NyanColorPicker::StartEyedropper);

    _okBtn = new QPushButton(QStringLiteral("OK"));
    _okBtn->setFixedWidth(50);
    connect(_okBtn, &QPushButton::clicked, this, [this]() {
        emit ColorConfirmed(Color());
    });

    btnLayout->addWidget(_eyedropperBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(_okBtn);

    mainLayout->addLayout(btnLayout);

    UpdateUiFromColor();
}

void NyanColorPicker::UpdateFromHsv()
{
    // HSV is the source of truth, update RGB/Hex.
    UpdateUiFromColor();
    emit ColorChanged(Color());
    update();
}

void NyanColorPicker::UpdateFromRgb()
{
    bool rOk = false;
    bool gOk = false;
    bool bOk = false;
    const int r = _rEdit->text().toInt(&rOk);
    const int g = _gEdit->text().toInt(&gOk);
    const int b = _bEdit->text().toInt(&bOk);

    if (!rOk || !gOk || !bOk) {
        return;
    }

    const QColor c = QColor(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255));
    _hue = c.hsvHue();
    if (_hue < 0) {
        _hue = 0;
    }
    _saturation = c.hsvSaturation();
    _value = c.value();

    // Update hex without triggering signal loop.
    _hexEdit->blockSignals(true);
    _hexEdit->setText(c.name().toUpper());
    _hexEdit->blockSignals(false);

    emit ColorChanged(Color());
    update();
}

void NyanColorPicker::UpdateFromHex()
{
    QString hex = _hexEdit->text();
    if (!hex.startsWith('#')) {
        hex = '#' + hex;
    }

    const QColor c(hex);
    if (!c.isValid()) {
        return;
    }

    _hue = c.hsvHue();
    if (_hue < 0) {
        _hue = 0;
    }
    _saturation = c.hsvSaturation();
    _value = c.value();

    // Update RGB without triggering signal loop.
    _rEdit->blockSignals(true);
    _gEdit->blockSignals(true);
    _bEdit->blockSignals(true);
    _rEdit->setText(QString::number(c.red()));
    _gEdit->setText(QString::number(c.green()));
    _bEdit->setText(QString::number(c.blue()));
    _rEdit->blockSignals(false);
    _gEdit->blockSignals(false);
    _bEdit->blockSignals(false);

    emit ColorChanged(Color());
    update();
}

void NyanColorPicker::UpdateUiFromColor()
{
    const QColor c = Color();

    _rEdit->blockSignals(true);
    _gEdit->blockSignals(true);
    _bEdit->blockSignals(true);
    _hexEdit->blockSignals(true);

    _rEdit->setText(QString::number(c.red()));
    _gEdit->setText(QString::number(c.green()));
    _bEdit->setText(QString::number(c.blue()));
    _hexEdit->setText(c.name().toUpper());

    _rEdit->blockSignals(false);
    _gEdit->blockSignals(false);
    _bEdit->blockSignals(false);
    _hexEdit->blockSignals(false);

    if (_alphaSlider != nullptr) {
        _alphaSlider->blockSignals(true);
        _alphaSlider->setValue(_alpha);
        _alphaSlider->blockSignals(false);
    }
}

void NyanColorPicker::StartEyedropper()
{
    // Simple implementation: grab color under cursor on next click.
    // For a full implementation, would need a transparent overlay window.
    // Here we just grab the current cursor position.
    const QPoint globalPos = QCursor::pos();
    QScreen* screen = QGuiApplication::screenAt(globalPos);
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen == nullptr) {
        return;
    }

    const QPixmap pixmap = screen->grabWindow(0, globalPos.x(), globalPos.y(), 1, 1);
    if (pixmap.isNull()) {
        return;
    }

    const QImage img = pixmap.toImage();
    if (img.width() > 0 && img.height() > 0) {
        const QColor pickedColor = img.pixelColor(0, 0);
        SetColor(pickedColor);
        emit ColorChanged(Color());
    }
}

auto NyanColorPicker::HsvWheelRect() const -> QRect
{
    return {10, 10, kWheelSize, kWheelSize};
}

auto NyanColorPicker::ValueSliderRect() const -> QRect
{
    return {10 + kWheelSize + 10, 10, kSliderWidth, kWheelSize};
}

auto NyanColorPicker::PreviewRect() const -> QRect
{
    return {10 + kWheelSize + 10 + kSliderWidth + 10, 10, kPreviewSize, kPreviewSize};
}

auto NyanColorPicker::HitTestHsvWheel(const QPoint& pos) const -> bool
{
    const QRect wheelRect = HsvWheelRect();
    const int cx = wheelRect.center().x();
    const int cy = wheelRect.center().y();
    const int radius = wheelRect.width() / 2;

    const int dx = pos.x() - cx;
    const int dy = pos.y() - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
}

auto NyanColorPicker::HitTestValueSlider(const QPoint& pos) const -> bool
{
    return ValueSliderRect().contains(pos);
}

} // namespace matcha::gui