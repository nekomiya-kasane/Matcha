/**
 * @file NyanPropertyGrid.cpp
 * @brief Implementation of NyanPropertyGrid themed property editor panel.
 */

#include <Matcha/Widgets/Controls/NyanPropertyGrid.h>

#include "../Core/SimpleWidgetEventFilter.h"
#include <Matcha/Widgets/Controls/NyanCollapsibleSection.h>

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace matcha::gui {

// ============================================================================
// Construction
// ============================================================================

NyanPropertyGrid::NyanPropertyGrid(QWidget* parent)
    : QWidget(parent)
    , ThemeAware(WidgetKind::PropertyGrid)
{
    _mainLayout = new QVBoxLayout(this);
    _mainLayout->setContentsMargins(0, 0, 0, 0);
    _mainLayout->setSpacing(0);

    _defaultContainer = new QWidget(this);
    _defaultLayout = new QVBoxLayout(_defaultContainer);
    _defaultLayout->setContentsMargins(0, 0, 0, 0);
    _defaultLayout->setSpacing(2);
    _mainLayout->addWidget(_defaultContainer);
    _mainLayout->addStretch();

    ApplyStyle();
    _swFilter = new SimpleWidgetEventFilter(this, nullptr);
}

NyanPropertyGrid::~NyanPropertyGrid() = default;

// ============================================================================
// Property API
// ============================================================================

void NyanPropertyGrid::AddProperty(const QString& name, PropertyType type,
                                   const QVariant& defaultValue,
                                   const QStringList& choices)
{
    PropertyEntry entry;
    entry.name = name;
    entry.type = type;
    entry.groupName = _currentGroup;

    // Create row widget.
    entry.rowWidget = new QWidget();
    auto* rowLayout = new QHBoxLayout(entry.rowWidget);
    rowLayout->setContentsMargins(4, 2, 4, 2);
    rowLayout->setSpacing(8);

    // Label.
    auto* label = new QLabel(name);
    label->setFixedWidth(kLabelWidth);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    entry.label = label;
    rowLayout->addWidget(label);

    // Editor.
    entry.editor = CreateEditor(type, defaultValue, choices);
    entry.editor->setFixedHeight(kRowHeight - 4);
    rowLayout->addWidget(entry.editor, 1);

    entry.rowWidget->setFixedHeight(kRowHeight);

    // Add to appropriate container.
    if (_currentGroup.isEmpty()) {
        _defaultLayout->addWidget(entry.rowWidget);
    } else {
        for (auto& group : _groups) {
            if (group.name == _currentGroup && group.contentLayout != nullptr) {
                group.contentLayout->addWidget(entry.rowWidget);
                break;
            }
        }
    }

    ConnectEditorSignals(entry.editor, name, type);
    _properties.push_back(std::move(entry));
}

void NyanPropertyGrid::AddGroup(const QString& name)
{
    _currentGroup = name;

    GroupEntry group;
    group.name = name;
    group.section = new NyanCollapsibleSection(this);
    group.section->SetTitle(name);
    group.section->SetExpanded(true);

    group.contentWidget = new QWidget();
    group.contentLayout = new QVBoxLayout(group.contentWidget);
    group.contentLayout->setContentsMargins(0, 0, 0, 0);
    group.contentLayout->setSpacing(2);

    group.section->SetContent(group.contentWidget);

    // Insert before the stretch.
    _mainLayout->insertWidget(_mainLayout->count() - 1, group.section);

    _groups.push_back(std::move(group));
}

void NyanPropertyGrid::SetReadOnly(const QString& name, bool readOnly)
{
    for (auto& prop : _properties) {
        if (prop.name == name && prop.editor != nullptr) {
            prop.editor->setEnabled(!readOnly);
            break;
        }
    }
}

auto NyanPropertyGrid::Value(const QString& name) const -> QVariant
{
    for (const auto& prop : _properties) {
        if (prop.name != name || prop.editor == nullptr) {
            continue;
        }

        switch (prop.type) {
        case PropertyType::Text:
            if (auto* edit = qobject_cast<QLineEdit*>(prop.editor)) {
                return edit->text();
            }
            break;
        case PropertyType::Integer:
            if (auto* spin = qobject_cast<QSpinBox*>(prop.editor)) {
                return spin->value();
            }
            break;
        case PropertyType::Double:
            if (auto* spin = qobject_cast<QDoubleSpinBox*>(prop.editor)) {
                return spin->value();
            }
            break;
        case PropertyType::Bool:
            if (auto* check = qobject_cast<QCheckBox*>(prop.editor)) {
                return check->isChecked();
            }
            break;
        case PropertyType::Choice:
            if (auto* combo = qobject_cast<QComboBox*>(prop.editor)) {
                return combo->currentText();
            }
            break;
        case PropertyType::Color:
            if (auto* btn = qobject_cast<QPushButton*>(prop.editor)) {
                return btn->property("color");
            }
            break;
        case PropertyType::Count_:
            break;
        }
    }
    return {};
}

void NyanPropertyGrid::SetValue(const QString& name, const QVariant& value)
{
    for (auto& prop : _properties) {
        if (prop.name != name || prop.editor == nullptr) {
            continue;
        }

        switch (prop.type) {
        case PropertyType::Text:
            if (auto* edit = qobject_cast<QLineEdit*>(prop.editor)) {
                edit->setText(value.toString());
            }
            break;
        case PropertyType::Integer:
            if (auto* spin = qobject_cast<QSpinBox*>(prop.editor)) {
                spin->setValue(value.toInt());
            }
            break;
        case PropertyType::Double:
            if (auto* spin = qobject_cast<QDoubleSpinBox*>(prop.editor)) {
                spin->setValue(value.toDouble());
            }
            break;
        case PropertyType::Bool:
            if (auto* check = qobject_cast<QCheckBox*>(prop.editor)) {
                check->setChecked(value.toBool());
            }
            break;
        case PropertyType::Choice:
            if (auto* combo = qobject_cast<QComboBox*>(prop.editor)) {
                combo->setCurrentText(value.toString());
            }
            break;
        case PropertyType::Color:
            if (auto* btn = qobject_cast<QPushButton*>(prop.editor)) {
                const QColor color = value.value<QColor>();
                btn->setProperty("color", color);
                btn->setStyleSheet(QStringLiteral("background: %1;").arg(color.name()));
            }
            break;
        case PropertyType::Count_:
            break;
        }
        break;
    }
}

auto NyanPropertyGrid::HasProperty(const QString& name) const -> bool
{
    for (const auto& prop : _properties) {
        if (prop.name == name) {
            return true;
        }
    }
    return false;
}

void NyanPropertyGrid::Clear()
{
    for (auto& prop : _properties) {
        if (prop.rowWidget != nullptr) {
            prop.rowWidget->deleteLater();
        }
    }
    _properties.clear();

    for (auto& group : _groups) {
        if (group.section != nullptr) {
            group.section->deleteLater();
        }
    }
    _groups.clear();

    _currentGroup.clear();
}

// ============================================================================
// Theme
// ============================================================================

void NyanPropertyGrid::OnThemeChanged()
{
    ApplyStyle();
}

void NyanPropertyGrid::ApplyStyle()
{
    const auto style = Theme().Resolve(WidgetKind::PropertyGrid, 0, InteractionState::Normal);
    setFont(style.font);

    setStyleSheet(QStringLiteral("QLabel { color: %1; } QWidget { background: %2; }")
        .arg(style.foreground.name(), style.background.name()));
}

// ============================================================================
// Private Helpers
// ============================================================================

auto NyanPropertyGrid::CreateEditor(PropertyType type, const QVariant& defaultValue,
                                    const QStringList& choices) -> QWidget*
{
    const auto editorStyle = Theme().Resolve(WidgetKind::PropertyGrid, 0, InteractionState::Hovered);
    const auto normalStyle = Theme().Resolve(WidgetKind::PropertyGrid, 0, InteractionState::Normal);

    const QString baseStyle = QStringLiteral(
        "background: %1; color: %2; border: 1px solid %3; border-radius: %4px; padding: 2px 4px;"
    ).arg(editorStyle.background.name(), normalStyle.foreground.name(),
          normalStyle.border.name(), QString::number(static_cast<int>(normalStyle.radiusPx)));

    switch (type) {
    case PropertyType::Text: {
        auto* edit = new QLineEdit();
        edit->setText(defaultValue.toString());
        edit->setStyleSheet(baseStyle);
        return edit;
    }
    case PropertyType::Integer: {
        auto* spin = new QSpinBox();
        spin->setRange(-999999, 999999);
        spin->setValue(defaultValue.toInt());
        spin->setStyleSheet(baseStyle);
        return spin;
    }
    case PropertyType::Double: {
        auto* spin = new QDoubleSpinBox();
        spin->setRange(-999999.0, 999999.0);
        spin->setDecimals(3);
        spin->setValue(defaultValue.toDouble());
        spin->setStyleSheet(baseStyle);
        return spin;
    }
    case PropertyType::Bool: {
        auto* check = new QCheckBox();
        check->setChecked(defaultValue.toBool());
        return check;
    }
    case PropertyType::Choice: {
        auto* combo = new QComboBox();
        combo->addItems(choices);
        if (!defaultValue.toString().isEmpty()) {
            combo->setCurrentText(defaultValue.toString());
        }
        combo->setStyleSheet(baseStyle);
        return combo;
    }
    case PropertyType::Color: {
        auto* btn = new QPushButton();
        const QColor color = defaultValue.value<QColor>();
        btn->setProperty("color", color);
        btn->setStyleSheet(QStringLiteral("background: %1; border: 1px solid %2; border-radius: %3px;")
            .arg(color.isValid() ? color.name() : QStringLiteral("#ffffff"),
                 normalStyle.border.name(),
                 QString::number(static_cast<int>(normalStyle.radiusPx))));
        btn->setFixedWidth(60);

        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            const QColor current = btn->property("color").value<QColor>();
            const QColor newColor = QColorDialog::getColor(current, this, tr("Select Color"));
            if (newColor.isValid()) {
                btn->setProperty("color", newColor);
                btn->setStyleSheet(QStringLiteral("background: %1;").arg(newColor.name()));
                // Find property name and emit signal.
                for (const auto& prop : _properties) {
                    if (prop.editor == btn) {
                        emit PropertyChanged(prop.name, newColor);
                        break;
                    }
                }
            }
        });
        return btn;
    }
    case PropertyType::Count_:
        break;
    }

    return new QWidget();
}

void NyanPropertyGrid::ConnectEditorSignals(QWidget* editor, const QString& name, PropertyType type)
{
    switch (type) {
    case PropertyType::Text:
        if (auto* edit = qobject_cast<QLineEdit*>(editor)) {
            connect(edit, &QLineEdit::textChanged, this, [this, name](const QString& text) {
                emit PropertyChanged(name, text);
            });
        }
        break;
    case PropertyType::Integer:
        if (auto* spin = qobject_cast<QSpinBox*>(editor)) {
            connect(spin, &QSpinBox::valueChanged, this, [this, name](int value) {
                emit PropertyChanged(name, value);
            });
        }
        break;
    case PropertyType::Double:
        if (auto* spin = qobject_cast<QDoubleSpinBox*>(editor)) {
            connect(spin, &QDoubleSpinBox::valueChanged, this, [this, name](double value) {
                emit PropertyChanged(name, value);
            });
        }
        break;
    case PropertyType::Bool:
        if (auto* check = qobject_cast<QCheckBox*>(editor)) {
            connect(check, &QCheckBox::toggled, this, [this, name](bool checked) {
                emit PropertyChanged(name, checked);
            });
        }
        break;
    case PropertyType::Choice:
        if (auto* combo = qobject_cast<QComboBox*>(editor)) {
            connect(combo, &QComboBox::currentTextChanged, this, [this, name](const QString& text) {
                emit PropertyChanged(name, text);
            });
        }
        break;
    case PropertyType::Color:
        // Handled in CreateEditor.
        break;
    case PropertyType::Count_:
        break;
    }
}

void NyanPropertyGrid::RebuildLayout()
{
    // Currently not needed as we build incrementally.
}

} // namespace matcha::gui