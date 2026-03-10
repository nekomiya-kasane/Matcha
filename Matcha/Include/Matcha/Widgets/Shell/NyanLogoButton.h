#pragma once

/**
 * @file NyanLogoButton.h
 * @brief Clickable logo button that spans two shell rows.
 *
 * NyanLogoButton is a 56x56 button that sits in the left column of the
 * shell QGridLayout, spanning TitleBar (row 0) and DocumentToolBar (row 1).
 *
 * It paints a two-color background (top half follows TitleBar bg, bottom
 * half follows DocumentToolBar bg) and draws the logo pixmap centered.
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QPixmap>
#include <QWidget>

namespace matcha::gui {

/**
 * @brief Clickable logo button spanning two shell rows.
 *
 * A11y role: Button.
 */
class MATCHA_EXPORT NyanLogoButton : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanLogoButton(QWidget* parent = nullptr);
    ~NyanLogoButton() override;

    NyanLogoButton(const NyanLogoButton&)            = delete;
    NyanLogoButton& operator=(const NyanLogoButton&) = delete;
    NyanLogoButton(NyanLogoButton&&)                 = delete;
    NyanLogoButton& operator=(NyanLogoButton&&)      = delete;

    /// @brief Set the logo pixmap (scaled to fit).
    void SetLogo(const QPixmap& logo);

    /// @brief Get the current logo pixmap.
    [[nodiscard]] auto Logo() const -> const QPixmap&;

    /// @brief Set the vertical split point (Row 1 height in pixels).
    void SetSplitY(int row1Height);

    /// @brief Set the top background color (follows TitleBar).
    void SetTopColor(const QColor& color);

    /// @brief Set the bottom background color (follows DocumentToolBar).
    void SetBottomColor(const QColor& color);

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the logo button is clicked.
    void Clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void OnThemeChanged() override;

private:
    static constexpr int kLogoSize   = 56;
    static constexpr int kLogoMargin = 64;

    QPixmap _logo;
    QColor  _topColor;
    QColor  _bottomColor;
    int     _splitY  = 28;
    bool    _hovered = false;
};

} // namespace matcha::gui
