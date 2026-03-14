#pragma once

/**
 * @file LayoutEngine.h
 * @brief Qt-free layout calculation engine.
 *
 * Implements the two-pass flex distribution algorithm from §6.1.2,
 * grid column resolution from §6.1.3, and axis alignment semantics.
 *
 * All calculations operate on plain int geometry — no Qt dependency.
 * `ContainerNode` translates between Qt widgets and this engine.
 *
 * @see Matcha_Design_System_Specification.md §6.1
 */

#include "Matcha/Foundation/Macros.h"

#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace matcha::fw {

// ============================================================================
// Enums
// ============================================================================

/**
 * @brief Main-axis alignment for HBox / VBox children.
 * @see §6.1.1
 */
enum class MainAxisAlignment : uint8_t {
    Start,          ///< Pack children at the start
    Center,         ///< Center children
    End,            ///< Pack children at the end
    SpaceBetween,   ///< Even space between children, none at edges
    SpaceAround,    ///< Even space around each child (half at edges)
    SpaceEvenly,    ///< Equal space between and at edges
};

/**
 * @brief Cross-axis alignment for children.
 * @see §6.1.1
 */
enum class CrossAxisAlignment : uint8_t {
    Start,    ///< Align to cross-axis start
    Center,   ///< Center along cross-axis
    End,      ///< Align to cross-axis end
    Stretch,  ///< Stretch to fill cross-axis (default)
};

/**
 * @brief Overflow strategy when children exceed container extent.
 * @see §6.1.1
 */
enum class LayoutOverflow : uint8_t {
    Clip,    ///< Children are clipped at container boundary (default)
    Scroll,  ///< Container becomes scrollable
    Wrap,    ///< Children wrap to next row/column
};

// ============================================================================
// Layout input/output structs
// ============================================================================

/**
 * @brief Per-child layout constraints along the main axis.
 */
struct ChildConstraint {
    int minSize       = 0;    ///< Minimum extent along main axis (px)
    int maxSize       = std::numeric_limits<int>::max(); ///< Maximum extent
    int preferredSize = 0;    ///< Preferred extent (sizeHint)
    int flex          = 0;    ///< Flex factor (0 = fixed, >0 = proportional)

    int crossMinSize       = 0;
    int crossMaxSize       = std::numeric_limits<int>::max();
    int crossPreferredSize = 0;

    int colSpan = 1;  ///< Grid: number of columns this child spans (≥1)
    int rowSpan = 1;  ///< Grid: number of rows this child spans (≥1)
};

/**
 * @brief Per-child computed layout result.
 */
struct ChildLayout {
    int mainOffset    = 0;    ///< Offset along main axis (relative to container start)
    int mainExtent    = 0;    ///< Allocated extent along main axis
    int crossOffset   = 0;    ///< Offset along cross axis
    int crossExtent   = 0;    ///< Allocated extent along cross axis
};

/**
 * @brief Container-level layout parameters.
 */
struct FlexLayoutParams {
    int containerMainExtent  = 0;   ///< Total available main-axis size
    int containerCrossExtent = 0;   ///< Total available cross-axis size
    int gap                  = 0;   ///< Spacing between children (px)
    int paddingStart         = 0;   ///< Padding at main-axis start
    int paddingEnd           = 0;   ///< Padding at main-axis end
    int crossPaddingStart    = 0;   ///< Padding at cross-axis start
    int crossPaddingEnd      = 0;   ///< Padding at cross-axis end
    MainAxisAlignment  mainAlign  = MainAxisAlignment::Start;
    CrossAxisAlignment crossAlign = CrossAxisAlignment::Stretch;
    LayoutOverflow     overflow   = LayoutOverflow::Clip; ///< Overflow strategy
};

// ============================================================================
// Grid column definition
// ============================================================================

/**
 * @brief Grid column size definition.
 * @see §6.1.3
 */
struct ColumnDef {
    enum class Type : uint8_t { Fixed, Fraction, Auto };
    Type  type  = Type::Auto;
    float value = 0.0F;  ///< px for Fixed, fr for Fraction, ignored for Auto
};

/**
 * @brief Grid layout parameters.
 */
struct GridLayoutParams {
    int containerWidth    = 0;
    int containerHeight   = 0;
    int hGap              = 0;   ///< Horizontal gap between cells
    int vGap              = 0;   ///< Vertical gap between rows
    int paddingLeft       = 0;
    int paddingTop        = 0;
    int paddingRight      = 0;
    int paddingBottom     = 0;
    CrossAxisAlignment cellAlign = CrossAxisAlignment::Stretch;
};

/**
 * @brief Per-child grid cell layout result.
 */
struct GridCellLayout {
    int x      = 0;   ///< Left offset in container
    int y      = 0;   ///< Top offset in container
    int width  = 0;   ///< Cell width
    int height = 0;   ///< Cell height
};

// ============================================================================
// LayoutEngine — stateless calculator
// ============================================================================

/**
 * @class LayoutEngine
 * @brief Stateless layout calculator.
 *
 * All methods are static, pure functions with no side effects.
 */
class MATCHA_EXPORT LayoutEngine {
public:
    // ====================================================================
    // Flex layout (HBox / VBox)
    // ====================================================================

    /**
     * @brief Compute flex layout for a list of children.
     *
     * Implements the two-pass algorithm from §6.1.2:
     * - Pass 1: allocate fixed-size children
     * - Pass 2: distribute remaining space to flex children
     * - Apply main-axis alignment for any leftover space
     * - Apply cross-axis alignment per child
     *
     * @param params Container-level parameters.
     * @param children Per-child constraints (input).
     * @return Per-child layout results.
     */
    [[nodiscard]] static auto ComputeFlex(
        const FlexLayoutParams& params,
        std::span<const ChildConstraint> children) -> std::vector<ChildLayout>;

    /**
     * @brief Compute wrap-flex layout (FlowLayout).
     *
     * When overflow == Wrap, children that exceed the main-axis extent
     * wrap onto new rows. Each row is independently flex-distributed.
     *
     * @return Per-child layout results with cross offsets reflecting row.
     */
    [[nodiscard]] static auto ComputeWrapFlex(
        const FlexLayoutParams& params,
        std::span<const ChildConstraint> children) -> std::vector<ChildLayout>;

    // ====================================================================
    // Grid layout
    // ====================================================================

    /**
     * @brief Resolve column widths from column definitions.
     *
     * Fixed → literal px, Auto → use autoWidths, Fraction → proportional
     * from remaining space.
     *
     * @param columns Column definitions.
     * @param autoWidths Measured content width per column (for Auto columns).
     * @param totalWidth Available width after padding.
     * @param hGap Horizontal gap between columns.
     * @return Resolved width per column (px).
     */
    [[nodiscard]] static auto ResolveColumns(
        std::span<const ColumnDef> columns,
        std::span<const int> autoWidths,
        int totalWidth,
        int hGap) -> std::vector<int>;

    /**
     * @brief Compute grid cell positions for children.
     *
     * Children are placed left-to-right, top-to-bottom. Each row is
     * auto-sized to the tallest child in that row.
     *
     * @param params Grid parameters.
     * @param columnWidths Pre-resolved column widths.
     * @param childSizes Preferred (width, height) per child.
     * @return Per-child cell layout.
     */
    [[nodiscard]] static auto ComputeGrid(
        const GridLayoutParams& params,
        std::span<const int> columnWidths,
        std::span<const std::pair<int,int>> childSizes) -> std::vector<GridCellLayout>;
};

} // namespace matcha::fw
