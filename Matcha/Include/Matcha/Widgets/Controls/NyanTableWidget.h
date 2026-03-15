#pragma once

/**
 * @file NyanTableWidget.h
 * @brief Theme-aware table widget with checkable vertical headers.
 *
 * Inherits QTableWidget for Qt table semantics and ThemeAware for design
 * token integration. Provides vertical header checkboxes for batch selection.
 *
 * @par Old project reference
 * No old NyanGuis equivalent. New widget.
 *
 * @par Visual specification
 * - Selection: PrimaryNormal background
 * - Cell: Foreground1 text, alternating Background1/Background2
 * - Header: Background3, Foreground1 text
 * - Vertical header checkbox: for batch row selection
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QTableWidget>

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware table widget with checkable vertical headers.
 *
 * A11y role: Table.
 */
class MATCHA_EXPORT NyanTableWidget : public QTableWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a table widget.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanTableWidget(QWidget* parent = nullptr);

    /**
     * @brief Construct a table widget with dimensions.
     * @param theme Theme service reference.
     * @param rows Initial row count.
     * @param columns Initial column count.
     * @param parent Optional parent widget.
     */
    NyanTableWidget(int rows, int columns, QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanTableWidget() override;

    NyanTableWidget(const NyanTableWidget&)            = delete;
    NyanTableWidget& operator=(const NyanTableWidget&) = delete;
    NyanTableWidget(NyanTableWidget&&)                 = delete;
    NyanTableWidget& operator=(NyanTableWidget&&)      = delete;

    /// @brief Enable or disable checkable vertical headers.
    void SetCheckableHeaders(bool enabled);

    /// @brief Whether vertical headers are checkable.
    [[nodiscard]] auto HasCheckableHeaders() const -> bool;

    /// @brief Get list of checked row indices.
    [[nodiscard]] auto CheckedRows() const -> QList<int>;

    /// @brief Set checked state for a row.
    void SetRowChecked(int row, bool checked);

Q_SIGNALS:
    /// @brief Emitted when a row's checked state changes.
    void RowCheckedChanged(int row, bool checked);

protected:
    /// @brief Trigger style update on theme change.
    void OnThemeChanged() override;

private:
    void ApplyStyle();
    void SetupCheckableHeaders();

    bool _checkableHeaders = false;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
