#pragma once

/**
 * @file NyanTabItem.h
 * @brief Self-drawn widget representing a single tab in NyanTabBar.
 *
 * Each NyanTabItem is a QWidget child of NyanTabBar, responsible for its own
 * painting, hover tracking, close button, and drag initiation.
 *
 * Two visual styles are supported via TabStyle (TitleBar vs Floating).
 * The corresponding UiNode is TabItemNode, which holds a back-pointer
 * to its NyanTabItem via Widget().
 *
 * @see NyanTabBar for the parent container.
 * @see TabItemNode for the UiNode layer.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Core/StrongId.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

enum class TabStyle : uint8_t;

/**
 * @brief Self-drawn widget for a single document tab.
 *
 * Handles its own paintEvent, hover, close-button hit-test, and drag-out.
 * Emits signals that NyanTabBar forwards to its own public signals.
 */
class MATCHA_EXPORT NyanTabItem : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    NyanTabItem(TabStyle style, fw::PageId pageId, const QString& title,
                QWidget* parent = nullptr);
    ~NyanTabItem() override;

    NyanTabItem(const NyanTabItem&)            = delete;
    NyanTabItem& operator=(const NyanTabItem&) = delete;
    NyanTabItem(NyanTabItem&&)                 = delete;
    NyanTabItem& operator=(NyanTabItem&&)      = delete;

    // -- Accessors --

    [[nodiscard]] auto GetPageId() const -> fw::PageId { return _pageId; }
    [[nodiscard]] auto Title() const -> const QString& { return _title; }
    [[nodiscard]] auto IsActive() const -> bool { return _active; }
    [[nodiscard]] auto Style() const -> TabStyle { return _style; }

    void SetTitle(const QString& title);
    void SetActive(bool active);

Q_SIGNALS:
    void Pressed(matcha::fw::PageId pageId);
    void CloseRequested(matcha::fw::PageId pageId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;
    void OnThemeChanged() override;

private:
    void PaintTitleBar(QPainter& painter);
    void PaintFloating(QPainter& painter);
    void PaintCloseButton(QPainter& painter, const QRect& closeRect,
                          bool darkMode) const;

    [[nodiscard]] auto CloseButtonRect() const -> QRect;

    TabStyle    _style;
    fw::PageId  _pageId;
    QString     _title;
    bool        _active       = false;
    bool        _hovered      = false;
    bool        _closeHovered = false;
    bool        _dragArmed    = false;
    bool        _reordering   = false;
    QPoint      _dragStartPos;
    int         _dragStartX   = 0;   ///< X position at drag arm for reorder tracking

    // Metrics
    static constexpr int kTitleBarWidth   = 105;
    static constexpr int kTitleBarHeight  = 24;
    static constexpr int kFloatingWidth   = 120;
    static constexpr int kFloatingHeight  = 28;
    static constexpr int kRadius          = 3;
    static constexpr int kTextLeft        = 8;
    static constexpr int kTextRight       = 28;
    static constexpr int kCloseSize       = 16;
};

} // namespace matcha::gui
