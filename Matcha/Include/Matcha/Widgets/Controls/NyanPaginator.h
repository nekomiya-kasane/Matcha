#pragma once

/**
 * @file NyanPaginator.h
 * @brief Theme-aware prev/next page navigation widget.
 *
 * Custom-painted paginator with prev/next buttons and page indicator.
 * Replaces old `NyanBrowseCheck`.
 *
 * @par Old project reference
 * - `old/NyanGuis/PublicInterfaces/NyanBrowseCheck.h`
 * - SetBrowseCount -> SetCount
 * - GetBrowseCount -> Count
 * - SetCurrentIndex -> SetCurrent
 * - GetCurrentIndex -> Current
 * - CurrentIndexChanged -> PageChanged
 * - ResetBrowseAll -> ResetClicked
 *
 * @par Visual specification
 * - Height: 24px (matches old NyanBrowseCheck)
 * - Prev/Next buttons: icon-only, 20x20
 * - Page indicator: "1/10" format, editable
 * - Reset button: optional
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware prev/next page navigation widget.
 *
 * A11y role: Toolbar.
 */
class MATCHA_EXPORT NyanPaginator : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a paginator.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanPaginator(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanPaginator() override;

    NyanPaginator(const NyanPaginator&)            = delete;
    NyanPaginator& operator=(const NyanPaginator&) = delete;
    NyanPaginator(NyanPaginator&&)                 = delete;
    NyanPaginator& operator=(NyanPaginator&&)      = delete;

    /// @brief Set the total page count.
    void SetCount(int count);

    /// @brief Get the total page count.
    [[nodiscard]] auto Count() const -> int;

    /// @brief Set the current page (0-indexed).
    void SetCurrent(int page);

    /// @brief Get the current page (0-indexed).
    [[nodiscard]] auto Current() const -> int;

    /// @brief Enable or disable the reset button.
    void SetResetButtonVisible(bool visible);

    /// @brief Whether the reset button is visible.
    [[nodiscard]] auto IsResetButtonVisible() const -> bool;

    /// @brief Size hint: width based on content, height 24px.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Minimum size hint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when the current page changes.
    void PageChanged(int page);

    /// @brief Emitted when the reset button is clicked.
    void ResetClicked();

protected:
    /// @brief Custom paint: buttons + page indicator.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Handle mouse press for buttons.
    void mousePressEvent(QMouseEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    [[nodiscard]] auto PrevButtonRect() const -> QRect;
    [[nodiscard]] auto NextButtonRect() const -> QRect;
    [[nodiscard]] auto ResetButtonRect() const -> QRect;
    [[nodiscard]] auto IndicatorRect() const -> QRect;

    static constexpr int kHeight = 24;
    static constexpr int kButtonSize = 20;
    static constexpr int kSpacing = 4;

    int _count = 0;
    int _current = -1;
    bool _resetButtonVisible = false;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
