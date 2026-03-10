#pragma once

/**
 * @file NyanSplitter.h
 * @brief Theme-aware splitter with themed handle and collapse support.
 *
 * Inherits QSplitter for Qt splitter semantics and ThemeAware for design
 * token integration. Themed handle with hover highlight and optional
 * collapse button.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Handle: Border2 color, 4px width (vertical) / height (horizontal)
 * - Handle hover: PrimaryHover background
 * - Collapse button: centered chevron icon on hover
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QSplitter>

namespace matcha::gui {

/**
 * @brief Theme-aware splitter with themed handle and collapse support.
 */
class MATCHA_EXPORT NyanSplitter : public QSplitter, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a horizontal splitter.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanSplitter(QWidget* parent = nullptr);

    /**
     * @brief Construct a splitter with specified orientation.
     * @param theme Theme service reference.
     * @param orientation Qt::Horizontal or Qt::Vertical.
     * @param parent Optional parent widget.
     */
    NyanSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanSplitter() override;

    NyanSplitter(const NyanSplitter&)            = delete;
    NyanSplitter& operator=(const NyanSplitter&) = delete;
    NyanSplitter(NyanSplitter&&)                 = delete;
    NyanSplitter& operator=(NyanSplitter&&)      = delete;

    /// @brief Enable or disable collapse button on handle.
    void SetCollapseButtonVisible(bool visible);

    /// @brief Whether collapse button is shown.
    [[nodiscard]] auto CollapseButtonVisible() const -> bool;

    /// @brief Collapse the panel at given index (set size to 0).
    void CollapsePanel(int index);

    /// @brief Restore the panel at given index to its previous size.
    void RestorePanel(int index);

protected:
    /// @brief Trigger repaint on theme change.
    void OnThemeChanged() override;

    /// @brief Create themed splitter handle.
    [[nodiscard]] auto createHandle() -> QSplitterHandle* override;

private:
    void ApplyHandleStyle();

    static constexpr int kHandleWidth = 4;

    bool _collapseButtonVisible = false;
    QList<int> _savedSizes;
};

} // namespace matcha::gui
