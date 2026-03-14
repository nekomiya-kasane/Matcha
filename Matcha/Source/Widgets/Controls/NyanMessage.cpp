/**
 * @file NyanMessage.cpp
 * @brief Implementation of NyanMessage inline message bar.
 */

#include <Matcha/Widgets/Controls/NyanMessage.h>

#include "../Core/SimpleWidgetEventFilter.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>

namespace matcha::gui {

NyanMessage::NyanMessage(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::Message)
{
    SetupUi();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanMessage::~NyanMessage() = default;

void NyanMessage::SetType(MessageType type)
{
    _type = type;
    UpdateStyle();
}

void NyanMessage::SetText(std::string_view text)
{
    _textLabel->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
}

auto NyanMessage::Text() const -> std::string
{
    return _textLabel->text().toStdString();
}

void NyanMessage::SetClosable(bool closable)
{
    _closable = closable;
    _closeBtn->setVisible(closable);
}

void NyanMessage::SetAction(std::string_view text, std::function<void()> callback)
{
    _actionCallback = std::move(callback);
    _actionBtn->setText(QString::fromUtf8(text.data(), static_cast<int>(text.size())));
    _actionBtn->setVisible(true);
}

auto NyanMessage::sizeHint() const -> QSize { return {400, 40}; }
auto NyanMessage::minimumSizeHint() const -> QSize { return {200, 32}; }

void NyanMessage::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter p(this);

    // Semantic bg color depends on message type — not in variant matrix
    ColorToken bgToken = ColorToken::PrimaryBgHover;
    switch (_type) {
    case MessageType::Info:    bgToken = ColorToken::PrimaryBgHover; break;
    case MessageType::Success: bgToken = ColorToken::SuccessBgHover; break;
    case MessageType::Warning: bgToken = ColorToken::WarningBgHover; break;
    case MessageType::Error:   bgToken = ColorToken::ErrorBgHover;   break;
    }
    p.fillRect(rect(), Theme().Color(bgToken));
}

void NyanMessage::OnThemeChanged()
{
    UpdateStyle();
    update();
}

void NyanMessage::SetupUi()
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    _iconLabel = new QLabel(this);
    _iconLabel->setFixedSize(16, 16);
    layout->addWidget(_iconLabel);

    _textLabel = new QLabel(this);
    _textLabel->setWordWrap(true);
    layout->addWidget(_textLabel, 1);

    _actionBtn = new QPushButton(this);
    _actionBtn->setVisible(false);
    layout->addWidget(_actionBtn);
    connect(_actionBtn, &QPushButton::clicked, this, [this] {
        if (_actionCallback) _actionCallback();
        Q_EMIT ActionClicked();
    });

    _closeBtn = new QPushButton(QStringLiteral("\u2715"), this);
    _closeBtn->setFixedSize(20, 20);
    _closeBtn->setVisible(false);
    layout->addWidget(_closeBtn);
    connect(_closeBtn, &QPushButton::clicked, this, [this] {
        hide();
        Q_EMIT Closed();
    });
}

void NyanMessage::UpdateStyle()
{
    update();
}

} // namespace matcha::gui