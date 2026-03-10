#pragma once

/**
 * @file NyanScrollArea.h
 * @brief Theme-aware scrollable container using NyanScrollBar.
 *
 * Inherits QScrollArea for Qt scroll semantics and ThemeAware for design
 * token integration. Replaces default QScrollBars with NyanScrollBar
 * instances for consistent themed appearance.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Uses NyanScrollBar (thin, auto-hide, hover-expand) for both orientations
 * - Smooth scrolling via configurable wheel step
 * - Background: transparent by default (content determines appearance)
 *
 * @see NyanScrollBar for scrollbar visual specification.
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QScrollArea>

namespace matcha::gui {

class NyanScrollBar;

/**
 * @brief Theme-aware scroll area with custom NyanScrollBar scrollbars.
 *
 * Provides SetScrollBarPolicy() and SetWheelStep() convenience API.
 * Both vertical and horizontal scrollbars are NyanScrollBar instances.
 */
class MATCHA_EXPORT NyanScrollArea : public QScrollArea, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a scroll area.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanScrollArea(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanScrollArea() override;

    NyanScrollArea(const NyanScrollArea&)            = delete;
    NyanScrollArea& operator=(const NyanScrollArea&) = delete;
    NyanScrollArea(NyanScrollArea&&)                 = delete;
    NyanScrollArea& operator=(NyanScrollArea&&)      = delete;

    /// @brief Set scrollbar visibility policy for both orientations.
    void SetScrollBarPolicy(Qt::ScrollBarPolicy horizontal, Qt::ScrollBarPolicy vertical);

    /// @brief Set the wheel scroll step in pixels. Default: 40.
    void SetWheelStep(int pixels);

    /// @brief Get the current wheel scroll step.
    [[nodiscard]] auto WheelStep() const -> int;

    /// @brief Access the vertical NyanScrollBar.
    [[nodiscard]] auto VerticalBar() const -> NyanScrollBar*;

    /// @brief Access the horizontal NyanScrollBar.
    [[nodiscard]] auto HorizontalBar() const -> NyanScrollBar*;

    /// @brief Size hint: 200x200.
    [[nodiscard]] auto sizeHint() const -> QSize override;

protected:
    /// @brief Custom wheel event for configurable step.
    void wheelEvent(QWheelEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kDefaultWheelStep = 40;
    static constexpr int kDefaultSize      = 200;

    NyanScrollBar* _vBar = nullptr;
    NyanScrollBar* _hBar = nullptr;
    int _wheelStep = kDefaultWheelStep;
};

} // namespace matcha::gui
