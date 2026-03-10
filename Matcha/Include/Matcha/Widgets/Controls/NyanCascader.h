#pragma once

/**
 * @file NyanCascader.h
 * @brief Theme-aware multi-level cascading selector.
 *
 * Multi-column drill-down for hierarchical data (material library,
 * coordinate systems, feature tree paths). Each column shows children
 * of the parent selection.
 *
 * @see 05_Greenfield_Plan.md ss 3.3, widget #42.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Widgets/Core/ThemeAware.h>

#include <QWidget>

#include <string>
#include <vector>

class QHBoxLayout;
class QListWidget;

namespace matcha::gui {

class InteractionEventFilter;

/**
 * @brief Data model node for NyanCascader.
 */
struct CascaderItem {
    std::string label;
    std::vector<CascaderItem> children;
};

/**
 * @brief Theme-aware multi-level cascading selector.
 *
 * A11y role: ComboBox.
 */
class MATCHA_EXPORT NyanCascader : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    explicit NyanCascader(QWidget* parent = nullptr);
    ~NyanCascader() override;

    NyanCascader(const NyanCascader&)            = delete;
    NyanCascader& operator=(const NyanCascader&) = delete;
    NyanCascader(NyanCascader&&)                 = delete;
    NyanCascader& operator=(NyanCascader&&)      = delete;

    /// @brief Set the hierarchical data model.
    void SetData(std::vector<CascaderItem> items);

    /// @brief Set placeholder text shown when no selection.
    void SetPlaceholder(std::string_view text);

    /// @brief Get the current selection path (labels from root to leaf).
    [[nodiscard]] auto Value() const -> std::vector<std::string>;

    /// @brief Clear the current selection.
    void ClearSelection();

    [[nodiscard]] auto sizeHint() const -> QSize override;
    [[nodiscard]] auto minimumSizeHint() const -> QSize override;

Q_SIGNALS:
    /// @brief Emitted when selection path changes.
    void SelectionChanged(const std::vector<std::string>& path);

protected:
    void paintEvent(QPaintEvent* event) override;
    void OnThemeChanged() override;

private:
    void SetupUi();
    void RebuildColumns();
    void OnColumnItemSelected(int columnIndex, int row);

    std::vector<CascaderItem> _rootItems;
    std::vector<std::string> _selectedPath;
    std::string _placeholder;

    QHBoxLayout* _columnsLayout = nullptr;
    std::vector<QListWidget*> _columns;
    InteractionEventFilter* _interactionFilter = nullptr;
};

} // namespace matcha::gui
