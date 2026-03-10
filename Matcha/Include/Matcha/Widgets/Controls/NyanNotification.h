#pragma once

/**
 * @file NyanNotification.h
 * @brief Toast notification popup with auto-dismiss.
 *
 * NyanNotification provides:
 * - Slide-in animation from top-right corner
 * - Auto-dismiss after configurable duration
 * - Optional action button (e.g., "Undo", "View")
 * - Queue management: max 3 visible, FIFO
 *
 * @par Visual specification
 * - Width: 320px, Height: auto (min 48px)
 * - Rounded corners: 6px
 * - Icon based on NotificationType
 * - Close button on right
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QTimer>
#include <QWidget>

#include <chrono>
#include <functional>

class QHBoxLayout;
class QLabel;
class QPushButton;
class QPropertyAnimation;

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Notification type determines icon and color scheme.
 */
enum class NotificationType : uint8_t {
    Info,     ///< Blue info icon
    Success,  ///< Green checkmark
    Warning,  ///< Yellow warning triangle
    Error     ///< Red error circle
};

/**
 * @brief Toast notification popup with auto-dismiss.
 *
 * A11y role: Alert.
 */
class MATCHA_EXPORT NyanNotification : public QWidget, public ThemeAware {
    Q_OBJECT
    Q_PROPERTY(int slideOffset READ SlideOffset WRITE SetSlideOffset)

public:
    /// @brief Default auto-dismiss duration.
    static constexpr auto kDefaultDuration = std::chrono::milliseconds(5000);

    /// @brief Maximum visible notifications.
    static constexpr int kMaxVisible = 3;

    /**
     * @brief Construct a notification.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanNotification(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanNotification() override;

    NyanNotification(const NyanNotification&)            = delete;
    NyanNotification& operator=(const NyanNotification&) = delete;
    NyanNotification(NyanNotification&&)                 = delete;
    NyanNotification& operator=(NyanNotification&&)      = delete;

    // -- Configuration --

    /// @brief Set the notification message.
    void SetMessage(const QString& message);

    /// @brief Get the notification message.
    [[nodiscard]] auto Message() const -> QString;

    /// @brief Set the notification type.
    void SetType(NotificationType type);

    /// @brief Get the notification type.
    [[nodiscard]] auto Type() const -> NotificationType;

    /// @brief Set the auto-dismiss duration.
    void SetDuration(std::chrono::milliseconds duration);

    /// @brief Get the auto-dismiss duration.
    [[nodiscard]] auto Duration() const -> std::chrono::milliseconds;

    /// @brief Set an action button.
    /// @param text Button text.
    /// @param callback Callback when button is clicked.
    void SetAction(const QString& text, std::function<void()> callback);

    /// @brief Remove the action button.
    void ClearAction();

    // -- Display --

    /// @brief Show the notification with slide-in animation.
    /// @param globalPos Top-right corner position.
    void ShowAt(const QPoint& globalPos);

    /// @brief Dismiss the notification with slide-out animation.
    void Dismiss();

    // -- Animation Property --

    [[nodiscard]] auto SlideOffset() const -> int;
    void SetSlideOffset(int offset);

    // -- Size hints --

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the action button is clicked.
    void ActionClicked();

    /// @brief Emitted when the notification is dismissed.
    void Dismissed();

protected:
    /// @brief Custom paint for themed notification.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    void InitLayout();
    void UpdateIcon();
    void StartShowAnimation();
    void StartDismissAnimation();
    void OnDismissTimeout();
    void OnCloseClicked();

    [[nodiscard]] auto TypeColor() const -> QColor;
    [[nodiscard]] auto TypeIcon() const -> QString;

    static constexpr int kWidth           = 320;
    static constexpr int kMinHeight       = 48;
    static constexpr int kRadius          = 6;
    static constexpr int kIconSize        = 20;
    static constexpr int kPadding         = 12;
    static constexpr int kAnimationDuration = 200;

    QHBoxLayout*        _layout         = nullptr;
    QLabel*             _iconLabel      = nullptr;
    QLabel*             _messageLabel   = nullptr;
    QPushButton*        _actionButton   = nullptr;
    QPushButton*        _closeButton    = nullptr;
    QTimer*             _dismissTimer   = nullptr;
    QPropertyAnimation* _slideAnimation = nullptr;

    QString                    _message;
    NotificationType           _type     = NotificationType::Info;
    std::chrono::milliseconds  _duration = kDefaultDuration;
    std::function<void()>      _actionCallback;
    int                        _slideOffset = 0;
    InteractionEventFilter* _interactionFilter = nullptr;
};

/**
 * @brief Manages a queue of notifications.
 *
 * Use this class to show notifications with proper queue management.
 */
class MATCHA_EXPORT NyanNotificationManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Construct a notification manager.
     * @param theme Theme service reference.
     * @param parent Parent widget for notifications.
     */
    explicit NyanNotificationManager(QWidget* parent);

    /// @brief Destructor.
    ~NyanNotificationManager() override;

    NyanNotificationManager(const NyanNotificationManager&)            = delete;
    NyanNotificationManager& operator=(const NyanNotificationManager&) = delete;
    NyanNotificationManager(NyanNotificationManager&&)                 = delete;
    NyanNotificationManager& operator=(NyanNotificationManager&&)      = delete;

    /// @brief Show a notification.
    /// @return Pointer to the notification (may be queued).
    auto Show(const QString& message,
              NotificationType type = NotificationType::Info,
              std::chrono::milliseconds duration = NyanNotification::kDefaultDuration) -> NyanNotification*;

    /// @brief Show a notification with an action.
    auto ShowWithAction(const QString& message,
                        const QString& actionText,
                        std::function<void()> actionCallback,
                        NotificationType type = NotificationType::Info,
                        std::chrono::milliseconds duration = NyanNotification::kDefaultDuration) -> NyanNotification*;

    /// @brief Dismiss all notifications.
    void DismissAll();

    /// @brief Get the number of visible notifications.
    [[nodiscard]] auto VisibleCount() const -> int;

private:
    void OnNotificationDismissed();
    void RepositionNotifications();
    void ShowNextQueued();

        QWidget* _parent;
    QVector<NyanNotification*> _visible;
    QVector<NyanNotification*> _queued;
};

} // namespace matcha::gui
