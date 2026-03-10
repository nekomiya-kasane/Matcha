#pragma once

/**
 * @file A11yRole.h
 * @brief Semantic accessibility role enum (S5, fw layer, zero Qt).
 *
 * Maps conceptually to QAccessible::Role but is Qt-free so it can live
 * in the fw layer. The gui layer maps A11yRole -> QAccessible::Role
 * when setting widget accessibility properties.
 */

#include <cstddef>
#include <cstdint>

namespace matcha::fw {

/**
 * @brief Semantic role for accessibility annotation.
 *
 * Each interactive WidgetNode should have a non-None A11yRole set.
 * A11yAudit checks this at test time.
 */
enum class A11yRole : uint8_t {
    None = 0,

    // Input controls
    Button,
    CheckBox,
    RadioButton,
    Slider,
    SpinBox,
    ComboBox,
    LineEdit,
    Toggle,

    // Navigation / containers
    Tab,
    TabPanel,
    Menu,
    MenuItem,
    MenuBar,

    // Overlays
    Dialog,
    Alert,
    Tooltip,

    // Status / feedback
    Status,
    ProgressBar,
    Badge,

    // Data / list
    TreeItem,
    ListItem,
    TableCell,
    Table,

    // Layout
    Group,
    Separator,
    ScrollBar,

    // Application-level
    Toolbar,
    Window,
    Label,

    Count_
};

inline constexpr auto kA11yRoleCount = static_cast<std::size_t>(A11yRole::Count_);

} // namespace matcha::fw
