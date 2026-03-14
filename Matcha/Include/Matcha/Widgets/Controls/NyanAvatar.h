#pragma once

/**
 * @file NyanAvatar.h
 * @brief Theme-aware user avatar display (image or initials).
 *
 * Circular clip. Fallback to initials on missing image.
 * Optional online status indicator dot.
 *
 * @see 05_Greenfield_Plan.md ss 3.4, widget #68.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QPixmap>
#include <QWidget>

#include <cstdint>
#include <string>
#include <string_view>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/** @brief Avatar display size. */
enum class AvatarSize : uint8_t {
    Small  = 24,
    Medium = 32,
    Large  = 48,
};

/** @brief Online status indicator. */
enum class OnlineStatus : uint8_t {
    None,
    Online,
    Away,
    Busy,
    Offline,
};

/**
 * @brief Theme-aware user avatar display.
 *
 * A11y role: Button.
 */
class MATCHA_EXPORT NyanAvatar : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanAvatar(QWidget* parent = nullptr);
    ~NyanAvatar() override;

    NyanAvatar(const NyanAvatar&)            = delete;
    NyanAvatar& operator=(const NyanAvatar&) = delete;
    NyanAvatar(NyanAvatar&&)                 = delete;
    NyanAvatar& operator=(NyanAvatar&&)      = delete;

    /// @brief Set the avatar image (circular clipped).
    void SetImage(const QPixmap& image);

    /// @brief Set initials fallback text (shown when no image).
    void SetInitials(std::string_view initials);

    /// @brief Set the avatar display size.
    void SetSize(AvatarSize size);

    /// @brief Get the current size.
    [[nodiscard]] auto Size() const -> AvatarSize { return _size; }

    /// @brief Set the online status indicator.
    void SetOnlineStatus(OnlineStatus status);

    /// @brief Get the online status.
    [[nodiscard]] auto Status() const -> OnlineStatus { return _status; }

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the avatar is clicked.
    void Clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void OnThemeChanged() override;

private:
    QPixmap _image;
    std::string _initials;
    AvatarSize _size = AvatarSize::Medium;
    OnlineStatus _status = OnlineStatus::None;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
