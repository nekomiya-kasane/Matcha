#pragma once

/**
 * @file NyanListWidget.h
 * @brief Theme-aware list widget with styled selection and optional drag reorder.
 *
 * Inherits QListWidget for Qt list semantics and ThemeAware for design
 * token integration. Provides themed selection colors and optional drag reorder.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Selection: PrimaryNormal background
 * - Item: Foreground1 text, alternating Background1/Background2
 * - Hover: Background3
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QListWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware list widget with styled selection and optional drag reorder.
 *
 * A11y role: List.
 */
class MATCHA_EXPORT NyanListWidget : public QListWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a list widget.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanListWidget(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanListWidget() override;

    NyanListWidget(const NyanListWidget&)            = delete;
    NyanListWidget& operator=(const NyanListWidget&) = delete;
    NyanListWidget(NyanListWidget&&)                 = delete;
    NyanListWidget& operator=(NyanListWidget&&)      = delete;

    /// @brief Enable or disable drag reorder of items.
    void SetDragReorderEnabled(bool enabled);

    /// @brief Whether drag reorder is enabled.
    [[nodiscard]] auto IsDragReorderEnabled() const -> bool;

protected:
    /// @brief Trigger style update on theme change.
    void OnThemeChanged() override;

private:
    void ApplyStyle();

    bool _dragReorderEnabled = false;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
