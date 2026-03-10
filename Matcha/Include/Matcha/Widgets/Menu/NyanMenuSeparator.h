#pragma once

/**
 * @file NyanMenuSeparator.h
 * @brief Menu separator line (3px height).
 *
 * @par Old project reference
 * Replaces old NyanMenuStyle separator drawing.
 *
 * @par Visual specification
 * - Height: 3px
 * - Line: Line4 color, 1px
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

namespace matcha::gui {

/**
 * @brief Menu separator line.
 *
 * A11y role: Separator.
 */
class MATCHA_EXPORT NyanMenuSeparator : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a menu separator.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanMenuSeparator(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanMenuSeparator() override;

    NyanMenuSeparator(const NyanMenuSeparator&)            = delete;
    NyanMenuSeparator& operator=(const NyanMenuSeparator&) = delete;
    NyanMenuSeparator(NyanMenuSeparator&&)                 = delete;
    NyanMenuSeparator& operator=(NyanMenuSeparator&&)      = delete;

    // -- Size hints --

    /// @brief Size hint: full width, 3px height.
    [[nodiscard]] auto sizeHint() const -> QSize override;

    /// @brief Minimum size hint.
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    /// @brief Custom paint for separator line.
    void paintEvent(QPaintEvent* event) override;

    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

private:
    static constexpr int kHeight = 3;
};

} // namespace matcha::gui
