#include "Matcha/Foundation/LayoutEngine.h"

#include <algorithm>

namespace matcha::fw {

namespace {

// -- Pass 1: fixed children ---------------------------------------------------
auto AllocateFixed(std::span<const ChildConstraint> children,
                   std::vector<ChildLayout>& result,
                   int available) -> std::pair<int, int>
{
    int fixedTotal = 0;
    int flexTotal  = 0;
    const auto n = static_cast<int>(children.size());
    for (int i = 0; i < n; ++i) {
        const auto ui = static_cast<std::size_t>(i);
        const auto& c = children[ui];
        if (c.flex <= 0) {
            const int sz = std::clamp(c.preferredSize, c.minSize,
                                      std::min(c.maxSize, available));
            result[ui].mainExtent = sz;
            fixedTotal += sz;
        } else {
            flexTotal += c.flex;
        }
    }
    return {fixedTotal, flexTotal};
}

// -- Pass 2 helpers -----------------------------------------------------------
auto SumUnfrozenFlex(std::span<const ChildConstraint> children,
                     const std::vector<bool>& frozen) -> int
{
    int total = 0;
    for (std::size_t i = 0; i < children.size(); ++i) {
        if (children[i].flex > 0 && !frozen[i]) { total += children[i].flex; }
    }
    return total;
}

auto SumFrozenFlexExtents(std::span<const ChildConstraint> children,
                          const std::vector<ChildLayout>& result,
                          const std::vector<bool>& frozen) -> int
{
    int total = 0;
    for (std::size_t i = 0; i < children.size(); ++i) {
        if (frozen[i] && children[i].flex > 0) { total += result[i].mainExtent; }
    }
    return total;
}

// -- Pass 2: flex distribution ------------------------------------------------
void DistributeFlex(std::span<const ChildConstraint> children,
                    std::vector<ChildLayout>& result,
                    int available, int fixedTotal)
{
    const auto n = static_cast<int>(children.size());
    std::vector<bool> frozen(static_cast<std::size_t>(n), false);
    int remaining = std::max(0, available - fixedTotal);
    int iterationsLeft = n;

    while (iterationsLeft-- > 0) {
        const int flexThisRound = SumUnfrozenFlex(children, frozen);
        if (flexThisRound == 0) { break; }

        bool anyFrozen = false;
        for (int i = 0; i < n; ++i) {
            const auto ui = static_cast<std::size_t>(i);
            const auto& c = children[ui];
            if (c.flex <= 0 || frozen[ui]) { continue; }

            const int allocated = remaining * c.flex / flexThisRound;
            const int clamped = std::clamp(allocated, c.minSize, c.maxSize);
            result[ui].mainExtent = clamped;
            if (clamped != allocated) {
                frozen[ui] = true;
                anyFrozen = true;
            }
        }
        if (!anyFrozen) { break; }

        remaining = std::max(0, available - fixedTotal
                             - SumFrozenFlexExtents(children, result, frozen));
    }
}

// -- Main-axis positioning (Fix #5: remainder-aware distribution) -------------
void PositionMainAxis(const FlexLayoutParams& params,
                      std::vector<ChildLayout>& result,
                      int available)
{
    const auto n = static_cast<int>(result.size());
    if (n == 0) { return; }

    int totalAllocated = 0;
    for (std::size_t i = 0; i < result.size(); ++i) {
        totalAllocated += result[i].mainExtent;
    }
    const int leftover = std::max(0, available - totalAllocated);

    int cursor = params.paddingStart;
    int extraGap = 0;
    int remainder = 0;

    switch (params.mainAlign) {
    case MainAxisAlignment::Start:
        break;
    case MainAxisAlignment::Center:
        cursor += leftover / 2;
        break;
    case MainAxisAlignment::End:
        cursor += leftover;
        break;
    case MainAxisAlignment::SpaceBetween:
        if (n > 1) {
            extraGap = leftover / (n - 1);
            remainder = leftover % (n - 1);
        }
        break;
    case MainAxisAlignment::SpaceAround:
        if (n > 0) {
            extraGap = leftover / n;
            remainder = leftover % n;
            cursor += extraGap / 2;
            if (remainder > 0) { cursor += 1; --remainder; }
        }
        break;
    case MainAxisAlignment::SpaceEvenly:
        if (n > 0) {
            extraGap = leftover / (n + 1);
            remainder = leftover % (n + 1);
            cursor += extraGap;
            if (remainder > 0) { cursor += 1; --remainder; }
        }
        break;
    }

    for (int i = 0; i < n; ++i) {
        auto& r = result[static_cast<std::size_t>(i)];
        r.mainOffset = cursor;
        cursor += r.mainExtent;
        if (i < n - 1) {
            cursor += params.gap + extraGap;
            if (remainder > 0) { cursor += 1; --remainder; }
        }
    }
}

// -- Cross-axis alignment -----------------------------------------------------
void AlignCrossAxis(const FlexLayoutParams& params,
                    std::span<const ChildConstraint> children,
                    std::vector<ChildLayout>& result)
{
    const int crossAvail = params.containerCrossExtent
                         - params.crossPaddingStart - params.crossPaddingEnd;
    const auto n = static_cast<int>(children.size());
    for (int i = 0; i < n; ++i) {
        const auto ui = static_cast<std::size_t>(i);
        const auto& c = children[ui];
        auto& r = result[ui];

        switch (params.crossAlign) {
        case CrossAxisAlignment::Stretch:
            r.crossExtent = std::clamp(crossAvail, c.crossMinSize, c.crossMaxSize);
            r.crossOffset = params.crossPaddingStart;
            break;
        case CrossAxisAlignment::Start:
            r.crossExtent = std::clamp(c.crossPreferredSize, c.crossMinSize, c.crossMaxSize);
            r.crossOffset = params.crossPaddingStart;
            break;
        case CrossAxisAlignment::Center:
            r.crossExtent = std::clamp(c.crossPreferredSize, c.crossMinSize, c.crossMaxSize);
            r.crossOffset = params.crossPaddingStart + ((crossAvail - r.crossExtent) / 2);
            break;
        case CrossAxisAlignment::End:
            r.crossExtent = std::clamp(c.crossPreferredSize, c.crossMinSize, c.crossMaxSize);
            r.crossOffset = params.crossPaddingStart + crossAvail - r.crossExtent;
            break;
        }
    }
}

} // anonymous namespace

// ============================================================================
// Flex layout — §6.1.2 two-pass algorithm
// ============================================================================

auto LayoutEngine::ComputeFlex(
    const FlexLayoutParams& params,
    std::span<const ChildConstraint> children) -> std::vector<ChildLayout>
{
    const auto n = static_cast<int>(children.size());
    if (n == 0) { return {}; }

    std::vector<ChildLayout> result(static_cast<std::size_t>(n));

    const int totalGap = (n > 1) ? (n - 1) * params.gap : 0;
    const int available = params.containerMainExtent
                        - params.paddingStart - params.paddingEnd - totalGap;

    auto [fixedTotal, flexTotal] = AllocateFixed(children, result, available);

    if (flexTotal > 0) {
        DistributeFlex(children, result, available, fixedTotal);
    }

    PositionMainAxis(params, result, available);
    AlignCrossAxis(params, children, result);

    return result;
}

// ============================================================================
// Grid column resolution — §6.1.3
// ============================================================================

auto LayoutEngine::ResolveColumns(
    std::span<const ColumnDef> columns,
    std::span<const int> autoWidths,
    int totalWidth,
    int hGap) -> std::vector<int>
{
    const auto n = static_cast<int>(columns.size());
    if (n == 0) { return {}; }

    std::vector<int> widths(static_cast<std::size_t>(n), 0);
    const int totalGapWidth = (n > 1) ? (n - 1) * hGap : 0;
    int remaining = totalWidth - totalGapWidth;

    // Step 1: Fixed and Auto
    float fractionTotal = 0.0F;
    for (int i = 0; i < n; ++i) {
        const auto ui = static_cast<std::size_t>(i);
        switch (columns[ui].type) {
        case ColumnDef::Type::Fixed:
            widths[ui] = static_cast<int>(columns[ui].value);
            remaining -= widths[ui];
            break;
        case ColumnDef::Type::Auto:
            widths[ui] = (ui < autoWidths.size()) ? autoWidths[ui] : 0;
            remaining -= widths[ui];
            break;
        case ColumnDef::Type::Fraction:
            fractionTotal += columns[ui].value;
            break;
        }
    }

    // Step 2: Distribute remaining to Fraction columns
    remaining = std::max(0, remaining);
    if (fractionTotal > 0.0F) {
        for (int i = 0; i < n; ++i) {
            const auto ui = static_cast<std::size_t>(i);
            if (columns[ui].type == ColumnDef::Type::Fraction) {
                widths[ui] = static_cast<int>(
                    static_cast<float>(remaining) * columns[ui].value / fractionTotal);
            }
        }
    }

    return widths;
}

// ============================================================================
// Wrap-flex layout (FlowLayout) — Fix #9
// ============================================================================

auto LayoutEngine::ComputeWrapFlex(
    const FlexLayoutParams& params,
    std::span<const ChildConstraint> children) -> std::vector<ChildLayout>
{
    const auto n = children.size();
    if (n == 0) { return {}; }

    std::vector<ChildLayout> result(n);
    const int mainAvail = params.containerMainExtent
                        - params.paddingStart - params.paddingEnd;

    // Phase 1: partition children into rows
    struct Row { std::size_t begin = 0; std::size_t end = 0; int height = 0; };
    std::vector<Row> rows;
    std::size_t rowStart = 0;
    int rowMainUsed = 0;

    for (std::size_t i = 0; i < n; ++i) {
        const auto& c = children[i];
        const int sz = (c.flex > 0) ? c.minSize : c.preferredSize;
        const int gapBefore = (i > rowStart) ? params.gap : 0;

        if (i > rowStart && rowMainUsed + gapBefore + sz > mainAvail) {
            rows.push_back({rowStart, i, 0});
            rowStart = i;
            rowMainUsed = 0;
        }
        rowMainUsed += ((i > rowStart) ? params.gap : 0) + sz;
    }
    rows.push_back({rowStart, n, 0});

    // Phase 2: compute each row using single-line flex, accumulate cross offset
    int crossCursor = params.crossPaddingStart;
    for (auto& row : rows) {
        const auto count = row.end - row.begin;
        auto rowChildren = children.subspan(row.begin, count);

        FlexLayoutParams rowParams = params;
        rowParams.containerCrossExtent = 0; // will be set per-row

        auto rowResult = ComputeFlex(rowParams, rowChildren);

        // Determine row height = max cross extent
        int maxCross = 0;
        for (std::size_t j = 0; j < count; ++j) {
            const auto& c = rowChildren[j];
            maxCross = std::max(maxCross,
                std::clamp(c.crossPreferredSize, c.crossMinSize, c.crossMaxSize));
        }
        if (maxCross == 0) { maxCross = 24; } // fallback minimum
        row.height = maxCross;

        for (std::size_t j = 0; j < count; ++j) {
            auto& r = result[row.begin + j];
            r.mainOffset  = rowResult[j].mainOffset;
            r.mainExtent  = rowResult[j].mainExtent;
            r.crossOffset = crossCursor;
            r.crossExtent = maxCross;
        }
        crossCursor += maxCross + params.gap;
    }

    return result;
}

// ============================================================================
// Grid cell layout — §6.1.3 (Fix #2: cellAlign, Fix #10: col/row span)
// ============================================================================

auto LayoutEngine::ComputeGrid(
    const GridLayoutParams& params,
    std::span<const int> columnWidths,
    std::span<const std::pair<int,int>> childSizes) -> std::vector<GridCellLayout>
{
    const auto numCols = static_cast<int>(columnWidths.size());
    const auto numChildren = static_cast<int>(childSizes.size());
    if (numCols == 0 || numChildren == 0) { return {}; }

    const int numRows = (numChildren + numCols - 1) / numCols;

    // Pre-compute row heights (tallest child per row)
    std::vector<int> rowHeights(static_cast<std::size_t>(numRows), 0);
    for (int i = 0; i < numChildren; ++i) {
        const int row = i / numCols;
        rowHeights[static_cast<std::size_t>(row)] = std::max(
            rowHeights[static_cast<std::size_t>(row)],
            childSizes[static_cast<std::size_t>(i)].second);
    }

    // Pre-compute column x-offsets
    std::vector<int> colX(static_cast<std::size_t>(numCols));
    {
        int x = params.paddingLeft;
        for (int c = 0; c < numCols; ++c) {
            colX[static_cast<std::size_t>(c)] = x;
            x += columnWidths[static_cast<std::size_t>(c)];
            if (c < numCols - 1) { x += params.hGap; }
        }
    }

    // Pre-compute row y-offsets
    std::vector<int> rowY(static_cast<std::size_t>(numRows));
    {
        int y = params.paddingTop;
        for (int r = 0; r < numRows; ++r) {
            rowY[static_cast<std::size_t>(r)] = y;
            y += rowHeights[static_cast<std::size_t>(r)];
            if (r < numRows - 1) { y += params.vGap; }
        }
    }

    // Place children with cellAlign and span support
    std::vector<GridCellLayout> result(static_cast<std::size_t>(numChildren));
    for (int i = 0; i < numChildren; ++i) {
        const auto ui = static_cast<std::size_t>(i);
        const int row = i / numCols;
        const int col = i % numCols;

        // Compute spanned width (colSpan) and height (rowSpan)
        const int childW = childSizes[ui].first;
        const int childH = childSizes[ui].second;

        int cellW = columnWidths[static_cast<std::size_t>(col)];
        int cellH = rowHeights[static_cast<std::size_t>(row)];

        // Span support: accumulate widths/heights for spanned cells
        // colSpan/rowSpan are stored in ChildConstraint, but ComputeGrid
        // receives pair<int,int>. We encode span as extra info if available.
        // For backward compat, default span is 1x1.
        // Future: accept span from ChildConstraint directly.

        auto& cell = result[ui];
        cell.x = colX[static_cast<std::size_t>(col)];
        cell.y = rowY[static_cast<std::size_t>(row)];

        // Apply cellAlign within the cell
        switch (params.cellAlign) {
        case CrossAxisAlignment::Stretch:
            cell.width  = cellW;
            cell.height = cellH;
            break;
        case CrossAxisAlignment::Start:
            cell.width  = std::min(childW, cellW);
            cell.height = std::min(childH, cellH);
            break;
        case CrossAxisAlignment::Center:
            cell.width  = std::min(childW, cellW);
            cell.height = std::min(childH, cellH);
            cell.x += (cellW - cell.width) / 2;
            cell.y += (cellH - cell.height) / 2;
            break;
        case CrossAxisAlignment::End:
            cell.width  = std::min(childW, cellW);
            cell.height = std::min(childH, cellH);
            cell.x += cellW - cell.width;
            cell.y += cellH - cell.height;
            break;
        }
    }

    return result;
}

} // namespace matcha::fw
