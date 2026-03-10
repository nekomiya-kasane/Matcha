#pragma once

/**
 * @file NyanVerticalTabBar.h
 * @brief QTabBar subclass with vertical text rendering for ActionBar side-docking.
 *
 * When the ActionBar is docked to Left or Right edges, the tab bar must render
 * text vertically. This class handles:
 * - Latin characters: rotated 90 degrees (Left dock: top-to-bottom, Right dock: bottom-to-top)
 * - CJK characters: stacked vertically (one char per line, top-to-bottom for both sides)
 * - Mixed text: each run of characters is rendered according to its script
 *
 * For horizontal orientation this class behaves identically to QTabBar.
 */

#include <Matcha/Foundation/Macros.h>

#include <QTabBar>

namespace matcha::gui {

/**
 * @brief QTabBar subclass supporting vertical text rendering.
 *
 * Use SetVertical(true) to enable vertical tab painting.
 * SetRotationDirection controls Latin text rotation:
 *   +1 = clockwise 90 deg (Left dock: read top-to-bottom)
 *   -1 = counter-clockwise 90 deg (Right dock: read top-to-bottom)
 */
class MATCHA_EXPORT NyanVerticalTabBar : public QTabBar {
    Q_OBJECT

public:
    explicit NyanVerticalTabBar(QWidget* parent = nullptr);
    ~NyanVerticalTabBar() override;

    NyanVerticalTabBar(const NyanVerticalTabBar&)            = delete;
    NyanVerticalTabBar& operator=(const NyanVerticalTabBar&) = delete;
    NyanVerticalTabBar(NyanVerticalTabBar&&)                 = delete;
    NyanVerticalTabBar& operator=(NyanVerticalTabBar&&)      = delete;

    /// @brief Enable or disable vertical text rendering.
    void SetVertical(bool vertical);

    /// @brief Whether vertical text rendering is active.
    [[nodiscard]] auto IsVertical() const -> bool;

    /// @brief Set Latin text rotation direction.
    /// @param cw true = clockwise (Left dock), false = counter-clockwise (Right dock).
    void SetRotateClockwise(bool cw);

    /// @brief Whether Latin rotation is clockwise.
    [[nodiscard]] auto IsRotateClockwise() const -> bool;

protected:
    [[nodiscard]] auto tabSizeHint(int index) const -> QSize override;
    void paintEvent(QPaintEvent* event) override;

private:
    /// @brief Check if a character is CJK.
    [[nodiscard]] static auto IsCjk(char32_t ch) -> bool;

    /// @brief Measure vertical text size for a given string.
    [[nodiscard]] static auto MeasureVerticalText(const QString& text, const QFontMetrics& fm) -> QSize;

    /// @brief Paint vertical text in the given rect.
    void PaintVerticalText(QPainter& painter, const QRect& rect,
                           const QString& text, const QFont& font) const;

    /// @brief Paint a single CJK character centered in content rect, advancing yOff.
    static void PaintCjkChar(QPainter& painter, const QRect& content,
                             const QString& ch, const QFontMetrics& fm, int& yOff);

    /// @brief Paint a rotated Latin run in content rect, advancing yOff.
    void PaintLatinRun(QPainter& painter, const QRect& content,
                       const QString& run, const QFontMetrics& fm, int& yOff) const;

    bool _vertical        = false;
    bool _rotateClockwise = true; // Left dock default
};

} // namespace matcha::gui
