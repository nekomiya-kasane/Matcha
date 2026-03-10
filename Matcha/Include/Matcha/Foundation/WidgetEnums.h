#pragma once

/**
 * @file WidgetEnums.h
 * @brief Shared enums used by both Widget and UiNode layers.
 *
 * These enums are deliberately placed in Foundation so that application-layer
 * code can reference them without including any Widget or Qt headers.
 */

#include <cstddef>
#include <cstdint>

namespace matcha::gui {

/// @brief Property type for auto-creating editor widgets in NyanPropertyGrid.
enum class PropertyType : uint8_t {
    Text,     ///< NyanLineEdit
    Integer,  ///< QSpinBox (themed)
    Double,   ///< QDoubleSpinBox (themed)
    Bool,     ///< QCheckBox (themed)
    Choice,   ///< QComboBox (themed)
    Color,    ///< Color swatch button

    Count_    ///< Sentinel for array sizing
};

/// @brief Compile-time count of PropertyType values.
inline constexpr std::size_t kPropertyTypeCount = static_cast<std::size_t>(PropertyType::Count_);

/// @brief Visual role for NyanLabel text styling.
enum class LabelRole : uint8_t {
    Title,   ///< Section header / dialog title
    Name,    ///< Property name / field label
    Body,    ///< Default body text
    Caption, ///< Small annotation text

    Count_   ///< Sentinel for array sizing
};

/// @brief Visual variant of a push button.
enum class ButtonVariant : uint8_t {
    Primary,    ///< Filled primary color (CTA)
    Secondary,  ///< Filled neutral background
    Ghost,      ///< Outlined with border, transparent bg
    Danger,     ///< Filled error/destructive color

    Count_
};

/// @brief Status bar item side.
enum class StatusBarSide : uint8_t {
    Left,
    Right,
};

/// @brief Sort order for DataTable columns.
enum class SortOrder : uint8_t {
    None,       ///< No sorting
    Ascending,  ///< A-Z, 0-9
    Descending  ///< Z-A, 9-0
};

/// @brief Selection mode for DataTable rows.
enum class SelectionMode : uint8_t {
    SingleRow,  ///< Only one row can be selected
    MultiRow    ///< Multiple rows can be selected (Ctrl+click, Shift+click)
};

/// @brief Qt-free horizontal alignment for cells/items.
///
/// Values are chosen to map directly to Qt::AlignmentFlag combinations
/// via ToQtAlignment() in the widget layer.
enum class HAlign : uint8_t {
    Left,    ///< Left-aligned (default)
    Center,  ///< Horizontally centered
    Right    ///< Right-aligned
};

} // namespace matcha::gui
