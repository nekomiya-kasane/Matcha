#pragma once

/**
 * @file NyanPropertyGrid.h
 * @brief Theme-aware key-value property editor panel.
 *
 * The single most-reused pattern in CAD Drawer panels. Left column = label,
 * right column = auto-created editor widget based on PropertyType.
 * Group headers use NyanCollapsibleSection for collapse.
 *
 * @par Old project reference
 * No direct old counterpart. New design based on CAD/CAE industry standard.
 *
 * @par Visual specification
 * - Row height: 28px
 * - Label: Foreground1, FontRole::Body
 * - Editor: auto-created based on PropertyType
 * - Group: NyanCollapsibleSection with title
 *
 * @see ThemeAware for mixin lifecycle.
 */

#include <Matcha/Core/Macros.h>
#include <Matcha/Tree/FSM/WidgetEnums.h>
#include <Matcha/Theming/ThemeAware.h>

#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

#include <vector>

namespace matcha::gui {

class SimpleWidgetEventFilter;

class NyanCollapsibleSection;

/**
 * @brief Theme-aware key-value property editor panel.
 *
 * A11y role: Table.
 */
class MATCHA_EXPORT NyanPropertyGrid : public QWidget, public ThemeAware {
    Q_OBJECT

public:
    /**
     * @brief Construct a property grid.
     * @param theme Theme service reference (must outlive this widget).
     * @param parent Optional parent widget.
     */
    explicit NyanPropertyGrid(QWidget* parent = nullptr);

    /// @brief Destructor.
    ~NyanPropertyGrid() override;

    NyanPropertyGrid(const NyanPropertyGrid&)            = delete;
    NyanPropertyGrid& operator=(const NyanPropertyGrid&) = delete;
    NyanPropertyGrid(NyanPropertyGrid&&)                 = delete;
    NyanPropertyGrid& operator=(NyanPropertyGrid&&)      = delete;

    // ========================================================================
    // Property API
    // ========================================================================

    /**
     * @brief Add a property to the grid.
     * @param name Unique property name (used as key).
     * @param type Property type (determines editor widget).
     * @param defaultValue Default value.
     * @param choices For Choice type: list of options.
     */
    void AddProperty(const QString& name, PropertyType type,
                     const QVariant& defaultValue = {},
                     const QStringList& choices = {});

    /**
     * @brief Add a collapsible group header.
     * @param name Group name (displayed as title).
     *
     * Subsequent AddProperty calls will be added to this group until
     * the next AddGroup call or end of properties.
     */
    void AddGroup(const QString& name);

    /**
     * @brief Set a property as read-only.
     * @param name Property name.
     * @param readOnly True to disable editing.
     */
    void SetReadOnly(const QString& name, bool readOnly);

    /**
     * @brief Get the current value of a property.
     * @param name Property name.
     * @return Current value, or invalid QVariant if not found.
     */
    [[nodiscard]] auto Value(const QString& name) const -> QVariant;

    /**
     * @brief Set the value of a property.
     * @param name Property name.
     * @param value New value.
     */
    void SetValue(const QString& name, const QVariant& value);

    /**
     * @brief Check if a property exists.
     * @param name Property name.
     * @return True if property exists.
     */
    [[nodiscard]] auto HasProperty(const QString& name) const -> bool;

    /**
     * @brief Clear all properties and groups.
     */
    void Clear();

Q_SIGNALS:
    /// @brief Emitted when a property value changes.
    void PropertyChanged(const QString& name, const QVariant& value);

protected:
    /// @brief Trigger style update on theme change.
    void OnThemeChanged() override;

private:
    struct PropertyEntry {
        QString name;
        PropertyType type = PropertyType::Text;
        QWidget* editor = nullptr;
        QWidget* label = nullptr;
        QWidget* rowWidget = nullptr;
        QString groupName;
    };

    struct GroupEntry {
        QString name;
        NyanCollapsibleSection* section = nullptr;
        QWidget* contentWidget = nullptr;
        QVBoxLayout* contentLayout = nullptr;
    };

    void RebuildLayout();
    auto CreateEditor(PropertyType type, const QVariant& defaultValue,
                      const QStringList& choices) -> QWidget*;
    void ConnectEditorSignals(QWidget* editor, const QString& name, PropertyType type);
    void ApplyStyle();

    static constexpr int kRowHeight = 28;
    static constexpr int kLabelWidth = 100;

    std::vector<PropertyEntry> _properties;
    std::vector<GroupEntry> _groups;
    QString _currentGroup;

    QVBoxLayout* _mainLayout = nullptr;
    QWidget* _defaultContainer = nullptr;
    QVBoxLayout* _defaultLayout = nullptr;
    SimpleWidgetEventFilter* _swFilter = nullptr;
};

} // namespace matcha::gui
