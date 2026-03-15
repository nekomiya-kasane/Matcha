#pragma once

/**
 * @file NyanFormLayout.h
 * @brief Theme-aware label-input pair form layout.
 *
 * Two-column form: label column (right-aligned) + input column.
 * Consistent label widths. Section headers. Replaces manual
 * QFormLayout usage in property panels.
 *
 * @see 05_Greenfield_Plan.md ss 3.3, widget #44.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QWidget>

#include <string_view>
#include <vector>

class QFormLayout;
class QLabel;

namespace matcha::gui {

class SimpleWidgetEventFilter;

/**
 * @brief Theme-aware label-input pair form layout.
 *
 * A11y role: Form.
 */
class MATCHA_EXPORT NyanFormLayout : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanFormLayout(QWidget* parent = nullptr);
    ~NyanFormLayout() override;

    NyanFormLayout(const NyanFormLayout&)            = delete;
    NyanFormLayout& operator=(const NyanFormLayout&) = delete;
    NyanFormLayout(NyanFormLayout&&)                 = delete;
    NyanFormLayout& operator=(NyanFormLayout&&)      = delete;

    /// @brief Add a label-widget row.
    void AddRow(std::string_view label, QWidget* widget);

    /// @brief Add a section header.
    void AddSection(std::string_view title);

    /// @brief Set the label column width (pixels).
    void SetLabelWidth(int width);

    /// @brief Get the label column width.
    [[nodiscard]] auto LabelWidth() const -> int { return _labelWidth; }

    /// @brief Number of rows.
    [[nodiscard]] auto RowCount() const -> int;

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    void SetupUi();

    QFormLayout* _formLayout = nullptr;
    int _labelWidth = 120;
    std::vector<QLabel*> _labels;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
