#pragma once

/**
 * @file UiNodeQuery.h
 * @brief Compile-time UiNode path query system.
 *
 * Provides a type-safe, zero-runtime-parsing query mechanism for navigating
 * the UiNode tree. Path strings are parsed at compile time into a fixed
 * sequence of segment descriptors; at runtime only direct child/parent
 * traversal executes.
 *
 * @par Path Syntax
 * @code
 *   path      := segment ('/' segment)*
 *   segment   := '..'                              // parent
 *              | '**'                               // recursive descend (DFS)
 *              | '^^'                               // recursive ascend (ancestors)
 *              | matcher ('[' index ']')?
 *   matcher   := '*'                                // any single child
 *              | typeMatch? idMatch? nameMatch?
 *   typeMatch := identifier                         // NodeType name
 *   idMatch   := '#' identifier
 *   nameMatch := '@' identifier
 *   index     := integer                            // 0-based
 * @endcode
 *
 * @par Sibling Navigation (P2)
 * @code
 *   '>TypeName'  -- next sibling of type
 *   '<TypeName'  -- prev sibling of type
 * @endcode
 *
 * @par Negation (P2)
 * @code
 *   '!TypeName'  -- exclude type (matches any child NOT of this type)
 * @endcode
 *
 * @par Examples
 *   - `shell | Q<"WindowNode/TitleBar/MenuBar">`          -- by NodeType chain
 *   - `shell | Q<"** /Viewport[2]">`                      -- recursive + index
 *   - `node  | Q<"^^/WindowNode">`                        -- nearest Window ancestor
 *   - `shell | Q<"WindowNode#mainWin/TitleBar">`          -- combined type+id
 *   - `shell | Q<"** /Viewport"> | All`                   -- collect all matches
 *   - `shell | Q<"WindowNode/TitleBar/MenuBar"> | As<T>`  -- typed result
 */

#include <Matcha/Core/FixedString.h>
#include <Matcha/Core/Types.h>
#include <Matcha/Tree/UiNode.h>

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

// =========================================================================== //
// Predicate concept and ResolveQueryTarget helper
// =========================================================================== //

/** @brief Concept for a callable that filters UiNode* query results. */
template <typename F>
concept NodePredicate = std::invocable<F, UiNode*>
    && std::convertible_to<std::invoke_result_t<F, UiNode*>, bool>;

/** @brief Default predicate that accepts all nodes. */
inline constexpr auto AcceptAll = [](UiNode*) -> bool { return true; };

/** @brief Apply ResolveQueryTarget() on a matched node. Returns nullptr if resolved to null. */
inline auto Resolve(UiNode* node) -> UiNode* {
    return node ? node->ResolveQueryTarget() : nullptr;
}

// =========================================================================== //
// Part 1: Compile-time segment parsing
// =========================================================================== //

/** @brief How a single path segment matches nodes. */
enum class MatchMode : uint8_t {
    ByType,         ///< NodeType name match
    ById,           ///< #id match
    ByName,         ///< @name match
    Wildcard,       ///< * -- any single child
    RecursiveDown,  ///< ** -- DFS descendants
    RecursiveUp,    ///< ^^ -- ancestor chain
    Parent,         ///< .. -- single parent step
    NextSibling,    ///< >Type -- next sibling (P2)
    PrevSibling,    ///< <Type -- prev sibling (P2)
    Negation,       ///< !Type -- not this type (P2)
};

/** @brief Compile-time description of one path segment. */
struct SegmentDescriptor {
    MatchMode        mode       = MatchMode::Wildcard;
    NodeType         type       = NodeType::Custom;      ///< Valid when mode uses type
    std::string_view id         = {};                     ///< Valid for ById / combined
    std::string_view name       = {};                     ///< Valid for ByName / combined
    int              index      = -1;                     ///< [N] selector, -1 = first
    bool             hasType    = false;
    bool             hasId      = false;
    bool             hasName    = false;
    bool             hasIndex   = false;
};

namespace detail {

// -- consteval helpers for string parsing --

consteval auto IsDigit(char c) -> bool {
    return c >= '0' && c <= '9';
}

consteval auto ParseInt(std::string_view sv) -> int {
    int result = 0;
    for (char c : sv) {
        result = (result * 10) + (c - '0');
    }
    return result;
}

/** @brief Count '/' separated segments in a path. */
consteval auto CountSegments(std::string_view path) -> std::size_t {
    if (path.empty()) { return 0; }
    std::size_t count = 1;
    for (char c : path) {
        if (c == '/') { ++count; }
    }
    return count;
}

/** @brief Extract the Nth '/' separated segment from a path. */
constexpr auto GetSegment(std::string_view path, std::size_t idx) -> std::string_view {
    std::size_t start = 0;
    std::size_t current = 0;
    for (std::size_t i = 0; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            if (current == idx) {
                return path.substr(start, i - start);
            }
            ++current;
            start = i + 1;
        }
    }
    return {};
}

/** @brief Parse a single segment string into a SegmentDescriptor. */
consteval auto ParseOneSegment(std::string_view seg) -> SegmentDescriptor {
    SegmentDescriptor d{};

    // Special tokens
    if (seg == "..") { d.mode = MatchMode::Parent; return d; }
    if (seg == "**") { d.mode = MatchMode::RecursiveDown; return d; }
    if (seg == "^^") { d.mode = MatchMode::RecursiveUp; return d; }
    if (seg == "*")  { d.mode = MatchMode::Wildcard; return d; }

    // Strip [N] index suffix
    auto body = seg;
    if (auto bracket = seg.find('['); bracket != std::string_view::npos) {
        auto close = seg.find(']', bracket);
        if (close != std::string_view::npos) {
            auto numStr = seg.substr(bracket + 1, close - bracket - 1);
            d.index = ParseInt(numStr);
            d.hasIndex = true;
        }
        body = seg.substr(0, bracket);
    }

    // Wildcard with optional index: *[N]
    if (body == "*") {
        d.mode = MatchMode::Wildcard;
        return d;
    }

    // P2: sibling navigation >Type / <Type
    if (!body.empty() && body[0] == '>') {
        d.mode = MatchMode::NextSibling;
        auto typeName = body.substr(1);
        d.type = ParseNodeType(typeName);
        d.hasType = true;
        return d;
    }
    if (!body.empty() && body[0] == '<') {
        d.mode = MatchMode::PrevSibling;
        auto typeName = body.substr(1);
        d.type = ParseNodeType(typeName);
        d.hasType = true;
        return d;
    }

    // P2: negation !Type
    if (!body.empty() && body[0] == '!') {
        d.mode = MatchMode::Negation;
        auto typeName = body.substr(1);
        d.type = ParseNodeType(typeName);
        d.hasType = true;
        return d;
    }

    // Parse combined: TypeName#id@name
    // Find # and @ positions
    auto hashPos = body.find('#');
    auto atPos = body.find('@');

    // Extract type part (before # or @)
    std::size_t typeEnd = body.size();
    if (hashPos != std::string_view::npos) { typeEnd = std::min(typeEnd, hashPos); }
    if (atPos != std::string_view::npos) { typeEnd = std::min(typeEnd, atPos); }

    if (typeEnd > 0) {
        auto typePart = body.substr(0, typeEnd);
        d.type = ParseNodeType(typePart);
        d.hasType = true;
    }

    // Extract #id part
    if (hashPos != std::string_view::npos) {
        auto idStart = hashPos + 1;
        auto idEnd = (atPos != std::string_view::npos && atPos > hashPos)
                     ? atPos : body.size();
        d.id = body.substr(idStart, idEnd - idStart);
        d.hasId = true;
    }

    // Extract @name part
    if (atPos != std::string_view::npos) {
        d.name = body.substr(atPos + 1);
        d.hasName = true;
    }

    // Determine primary mode
    if (d.hasType) {
        d.mode = MatchMode::ByType;
    } else if (d.hasId) {
        d.mode = MatchMode::ById;
    } else if (d.hasName) {
        d.mode = MatchMode::ByName;
    }

    return d;
}

// =========================================================================== //
// Part 2: Runtime node matching
// =========================================================================== //

/** @brief Check if a single UiNode matches a SegmentDescriptor. */
inline auto NodeMatchesSegment(UiNode* node, const SegmentDescriptor& seg) -> bool {
    if (!node) { return false; }

    if (seg.mode == MatchMode::Wildcard) { return true; }

    if (seg.mode == MatchMode::Negation) {
        return !seg.hasType || node->Type() != seg.type;
    }

    // Check type constraint
    if (seg.hasType && node->Type() != seg.type) { return false; }

    // Check id constraint
    if (seg.hasId && node->Id() != seg.id) { return false; }

    // Check name constraint
    if (seg.hasName && node->Name() != seg.name) { return false; }

    return true;
}

/** @brief Find matching child at segment, respecting [index]. Returns nullptr on failure. */
inline auto StepChild(UiNode* node, const SegmentDescriptor& seg) -> UiNode* {
    if (!node) { return nullptr; }
    int matchCount = 0;
    int target = seg.hasIndex ? seg.index : 0;
    for (std::size_t i = 0; i < node->NodeCount(); ++i) {
        auto* child = node->NodeAt(i);
        if (NodeMatchesSegment(child, seg)) {
            if (matchCount == target) { return child; }
            ++matchCount;
        }
    }
    return nullptr;
}

/** @brief Collect ALL matching children at segment. */
inline auto StepChildAll(UiNode* node, const SegmentDescriptor& seg,
                         std::vector<UiNode*>& out) {
    if (!node) { return; }
    for (std::size_t i = 0; i < node->NodeCount(); ++i) {
        auto* child = node->NodeAt(i);
        if (NodeMatchesSegment(child, seg)) {
            out.push_back(child);
        }
    }
}

/** @brief DFS search for first descendant matching seg, respecting [index]. */
inline auto StepRecursiveDown(UiNode* node, const SegmentDescriptor& seg) -> UiNode* {
    if (!node) { return nullptr; }
    int matchCount = 0;
    int target = seg.hasIndex ? seg.index : 0;
    // DFS pre-order
    for (auto* desc : node->Descendants()) {
        if (NodeMatchesSegment(desc, seg)) {
            if (matchCount == target) { return desc; }
            ++matchCount;
        }
    }
    return nullptr;
}

/** @brief DFS search collecting ALL descendants matching seg. */
inline auto StepRecursiveDownAll(UiNode* node, const SegmentDescriptor& seg,
                                 std::vector<UiNode*>& out) {
    if (!node) { return; }
    for (auto* desc : node->Descendants()) {
        if (NodeMatchesSegment(desc, seg)) {
            out.push_back(desc);
        }
    }
}

/** @brief Ancestor search for first ancestor matching seg. */
inline auto StepRecursiveUp(UiNode* node, const SegmentDescriptor& seg) -> UiNode* {
    if (!node) { return nullptr; }
    int matchCount = 0;
    int target = seg.hasIndex ? seg.index : 0;
    auto* cur = node->ParentNode();
    while (cur) {
        if (NodeMatchesSegment(cur, seg)) {
            if (matchCount == target) { return cur; }
            ++matchCount;
        }
        cur = cur->ParentNode();
    }
    return nullptr;
}

/** @brief Ancestor search collecting ALL ancestors matching seg. */
inline auto StepRecursiveUpAll(UiNode* node, const SegmentDescriptor& seg,
                               std::vector<UiNode*>& out) {
    if (!node) { return; }
    auto* cur = node->ParentNode();
    while (cur) {
        if (NodeMatchesSegment(cur, seg)) {
            out.push_back(cur);
        }
        cur = cur->ParentNode();
    }
}

/** @brief Find sibling index of node in its parent's children. Returns -1 if not found. */
inline auto SiblingIndex(UiNode* node) -> int {
    if (!node || !node->ParentNode()) { return -1; }
    auto* parent = node->ParentNode();
    for (std::size_t i = 0; i < parent->NodeCount(); ++i) {
        if (parent->NodeAt(i) == node) { return static_cast<int>(i); }
    }
    return -1;
}

/** @brief Next sibling matching seg. */
inline auto StepNextSibling(UiNode* node, const SegmentDescriptor& seg) -> UiNode* {
    if (!node || !node->ParentNode()) { return nullptr; }
    auto* parent = node->ParentNode();
    int idx = SiblingIndex(node);
    if (idx < 0) { return nullptr; }
    int matchCount = 0;
    int target = seg.hasIndex ? seg.index : 0;
    for (std::size_t i = static_cast<std::size_t>(idx) + 1; i < parent->NodeCount(); ++i) {
        auto* sib = parent->NodeAt(i);
        if (NodeMatchesSegment(sib, seg)) {
            if (matchCount == target) { return sib; }
            ++matchCount;
        }
    }
    return nullptr;
}

/** @brief Previous sibling matching seg. */
inline auto StepPrevSibling(UiNode* node, const SegmentDescriptor& seg) -> UiNode* {
    if (!node || !node->ParentNode()) { return nullptr; }
    auto* parent = node->ParentNode();
    int idx = SiblingIndex(node);
    if (idx <= 0) { return nullptr; }
    int matchCount = 0;
    int target = seg.hasIndex ? seg.index : 0;
    for (int i = idx - 1; i >= 0; --i) {
        auto* sib = parent->NodeAt(static_cast<std::size_t>(i));
        if (NodeMatchesSegment(sib, seg)) {
            if (matchCount == target) { return sib; }
            ++matchCount;
        }
    }
    return nullptr;
}

// =========================================================================== //
// Part 3: Query error type
// =========================================================================== //

/** @brief Error information from a failed query. */
struct QueryError {
    std::size_t segmentIndex = 0;
    std::string segmentText;
    std::string nodeId;
    std::string nodeType;
    std::string message;

    [[nodiscard]] auto What() const -> std::string {
        return std::format("Query failed at segment {} '{}': {} [at node id='{}' type={}]",
                           segmentIndex, segmentText, message, nodeId, nodeType);
    }
};

template <typename T>
using QueryResult = std::expected<T, QueryError>;

inline auto MakeQueryError(std::size_t segIdx, std::string_view segText,
                           UiNode* node, std::string_view msg) -> QueryError {
    return QueryError{
        .segmentIndex = segIdx,
        .segmentText  = std::string(segText),
        .nodeId       = node ? std::string(node->Id()) : "(null)",
        .nodeType     = node ? std::string(NodeTypeName(node->Type())) : "(null)",
        .message      = std::string(msg),
    };
}

// =========================================================================== //
// Part 4: Segment array holder (constexpr, passed as NTTP)
// =========================================================================== //

template <std::size_t N>
struct SegmentArray {
    SegmentDescriptor segs[N]{};  // NOLINT(modernize-avoid-c-arrays)
    std::size_t count = N;

    [[nodiscard]] constexpr auto operator[](std::size_t i) const -> const SegmentDescriptor& {
        return segs[i];
    }
};

/** @brief Parse a FixedString path into a SegmentArray at compile time. */
template <FixedString Path>
consteval auto ParsePath() {
    constexpr std::string_view sv = Path;
    constexpr std::size_t n = CountSegments(sv);
    SegmentArray<n> result{};
    for (std::size_t i = 0; i < n; ++i) {
        result.segs[i] = ParseOneSegment(GetSegment(sv, i));
    }
    return result;
}

// =========================================================================== //
// Part 5: Runtime query execution engine
// =========================================================================== //

/** @brief Execute one segment step. Returns the next node or error. */
inline auto ExecuteSegment(UiNode* node, const SegmentDescriptor& seg,
                           std::size_t segIdx, std::string_view segText)
    -> QueryResult<UiNode*>
{
    UiNode* result = nullptr;

    switch (seg.mode) {
    case MatchMode::Parent:
        result = node->ParentNode();
        if (!result) {
            return std::unexpected(MakeQueryError(segIdx, segText, node, "parent is null"));
        }
        return result;

    case MatchMode::RecursiveDown:
        // ** must be followed by another segment -- handled by caller.
        // If ** is the last segment, it's a wildcard over all descendants.
        // Return the node itself so the caller can iterate descendants.
        return node;

    case MatchMode::RecursiveUp:
        // ^^ similarly handled by caller.
        return node;

    case MatchMode::NextSibling:
        result = StepNextSibling(node, seg);
        if (!result) {
            return std::unexpected(MakeQueryError(segIdx, segText, node, "no matching next sibling"));
        }
        return result;

    case MatchMode::PrevSibling:
        result = StepPrevSibling(node, seg);
        if (!result) {
            return std::unexpected(MakeQueryError(segIdx, segText, node, "no matching prev sibling"));
        }
        return result;

    default: // ByType, ById, ByName, Wildcard, Negation
        result = StepChild(node, seg);
        if (!result) {
            return std::unexpected(MakeQueryError(segIdx, segText, node, "no matching child"));
        }
        return result;
    }
}

/**
 * @brief Execute a full parsed query on a root node.
 *
 * Handles **, ^^, and normal segments with proper lookahead for recursive modes.
 */
template <std::size_t N>
inline auto ExecuteQuery(UiNode* root, const SegmentArray<N>& segs,
                         std::string_view fullPath)
    -> QueryResult<UiNode*>
{
    auto* current = root;
    if (!current) {
        return std::unexpected(QueryError{0, std::string(fullPath), "", "", "root node is null"});
    }

    for (std::size_t i = 0; i < N; ++i) {
        const auto& seg = segs[i];

        // Reconstruct segment text for error messages
        auto segText = detail::GetSegment(fullPath, i);

        if (seg.mode == MatchMode::RecursiveDown) {
            // ** : need to look at the NEXT segment and DFS for it
            if (i + 1 < N) {
                const auto& nextSeg = segs[i + 1];
                auto* found = StepRecursiveDown(current, nextSeg);
                if (!found) {
                    auto nextText = detail::GetSegment(fullPath, i + 1);
                    return std::unexpected(MakeQueryError(i + 1, nextText, current,
                        "no descendant matches after **"));
                }
                current = found;
                ++i; // skip the next segment (already consumed)
            }
            // ** at end of path: return current (for RunAll, means "all descendants")
            continue;
        }

        if (seg.mode == MatchMode::RecursiveUp) {
            // ^^ : look at the NEXT segment and search ancestors
            if (i + 1 < N) {
                const auto& nextSeg = segs[i + 1];
                auto* found = StepRecursiveUp(current, nextSeg);
                if (!found) {
                    auto nextText = detail::GetSegment(fullPath, i + 1);
                    return std::unexpected(MakeQueryError(i + 1, nextText, current,
                        "no ancestor matches after ^^"));
                }
                current = found;
                ++i; // skip
            }
            continue;
        }

        auto result = ExecuteSegment(current, seg, i, segText);
        if (!result) { return std::unexpected(result.error()); }
        current = result.value();
    }

    return Resolve(current);
}

/**
 * @brief Execute a full parsed query collecting ALL matches.
 *
 * For ** at end: collects all descendants.
 * For ** + next: collects all descendants matching next.
 * For normal paths: collects all matching at the last segment.
 */
template <std::size_t N>
inline auto ExecuteQueryAll(UiNode* root, const SegmentArray<N>& segs,
                            std::string_view fullPath)
    -> std::vector<UiNode*>
{
    std::vector<UiNode*> results;
    if (!root) { return results; }

    if constexpr (N == 0) {
        results.push_back(root);
        return results;
    }

    // Navigate to the node BEFORE the last segment(s)
    auto* current = root;

    // Find the final collecting segment(s)
    // If the last segment is **, collect all descendants of the pre-last node
    // If second-to-last is ** and last is a matcher, collect all matching descendants

    // First, navigate all segments except those involved in the final collection
    std::size_t collectStart = N; // which segment starts the collection phase

    // Detect ** near end
    if (N >= 2 && segs[N - 2].mode == MatchMode::RecursiveDown) {
        collectStart = N - 2;
    } else if (N >= 2 && segs[N - 2].mode == MatchMode::RecursiveUp) {
        collectStart = N - 2;
    } else if (N >= 1 && segs[N - 1].mode == MatchMode::RecursiveDown) {
        collectStart = N - 1;
    } else if (N >= 1 && segs[N - 1].mode == MatchMode::RecursiveUp) {
        collectStart = N - 1;
    } else {
        // Normal: collect at last segment
        collectStart = N - 1;
    }

    // Navigate to collectStart
    for (std::size_t i = 0; i < collectStart; ++i) {
        const auto& seg = segs[i];
        auto segText = detail::GetSegment(fullPath, i);

        if (seg.mode == MatchMode::RecursiveDown && i + 1 < collectStart) {
            const auto& nextSeg = segs[i + 1];
            auto* found = StepRecursiveDown(current, nextSeg);
            if (!found) { return results; }
            current = found;
            ++i;
            continue;
        }
        if (seg.mode == MatchMode::RecursiveUp && i + 1 < collectStart) {
            const auto& nextSeg = segs[i + 1];
            auto* found = StepRecursiveUp(current, nextSeg);
            if (!found) { return results; }
            current = found;
            ++i;
            continue;
        }

        auto r = ExecuteSegment(current, seg, i, segText);
        if (!r) { return results; }
        current = r.value();
    }

    // Collection phase (into raw, then resolve)
    std::vector<UiNode*> raw;
    if (collectStart < N) {
        const auto& cseg = segs[collectStart];
        if (cseg.mode == MatchMode::RecursiveDown) {
            if (collectStart + 1 < N) {
                StepRecursiveDownAll(current, segs[collectStart + 1], raw);
            } else {
                for (auto* d : current->Descendants()) { raw.push_back(d); }
            }
        } else if (cseg.mode == MatchMode::RecursiveUp) {
            if (collectStart + 1 < N) {
                StepRecursiveUpAll(current, segs[collectStart + 1], raw);
            } else {
                auto* p = current->ParentNode();
                while (p) { raw.push_back(p); p = p->ParentNode(); }
            }
        } else {
            StepChildAll(current, cseg, raw);
        }
    }

    // Apply ResolveQueryTarget on each result
    results.reserve(raw.size());
    for (auto* n : raw) {
        auto* resolved = Resolve(n);
        if (resolved) { results.push_back(resolved); }
    }

    return results;
}

/**
 * @brief Check if a node's ancestor path matches a pattern.
 *
 * Builds the root-to-node path and checks if the pattern matches.
 * Supports ** as wildcard spanning any number of ancestors.
 */
template <std::size_t N>
inline auto MatchesPattern(UiNode* node, const SegmentArray<N>& segs) -> bool {
    if (!node) { return false; }

    // Build ancestor path from root to node
    std::vector<UiNode*> chain;
    for (auto* cur = node; cur; cur = cur->ParentNode()) {
        chain.push_back(cur);
    }
    std::ranges::reverse(chain);

    // Match segments against chain using recursive backtracking
    // segIdx: current segment, chainIdx: current chain position
    std::function<bool(std::size_t, std::size_t)> match =
        [&](std::size_t segIdx, std::size_t chainIdx) -> bool {
        // Both exhausted: success
        if (segIdx >= N && chainIdx >= chain.size()) { return true; }
        // Segments exhausted but chain not: fail
        if (segIdx >= N) { return false; }

        const auto& seg = segs[segIdx];

        if (seg.mode == MatchMode::RecursiveDown) {
            // ** matches zero or more chain elements
            // Try matching next segment at every remaining chain position
            for (std::size_t ci = chainIdx; ci <= chain.size(); ++ci) {
                if (match(segIdx + 1, ci)) { return true; }
            }
            return false;
        }

        // Chain exhausted but segments remain: fail
        if (chainIdx >= chain.size()) { return false; }

        if (NodeMatchesSegment(chain[chainIdx], seg)) {
            return match(segIdx + 1, chainIdx + 1);
        }
        return false;
    };

    return match(0, 0);
}

} // namespace detail

// =========================================================================== //
// Part 6: UiQuery<Path> -- the lazy query descriptor
// =========================================================================== //

/**
 * @brief Lazy compile-time query descriptor.
 *
 * Constructed from a FixedString path. Does not execute until Run() is called
 * or a node is piped in via operator|.
 */
template <FixedString Path>
struct UiQuery {
    static constexpr auto kPath = Path;
    static constexpr auto kParsed = detail::ParsePath<Path>();
    static constexpr auto kSegCount = kParsed.count;

    // -- Single-result execution --

    [[nodiscard]] auto Run(UiNode* root) const -> detail::QueryResult<UiNode*> {
        return detail::ExecuteQuery(root, kParsed, Path.view());
    }

    template <NodePredicate Pred>
    [[nodiscard]] auto Run(UiNode* root, Pred&& pred) const -> detail::QueryResult<UiNode*> {
        auto r = detail::ExecuteQuery(root, kParsed, Path.view());
        if (!r) { return r; }
        if (!std::invoke(std::forward<Pred>(pred), r.value())) {
            return std::unexpected(detail::MakeQueryError(
                kSegCount > 0 ? kSegCount - 1 : 0,
                detail::GetSegment(Path.view(), kSegCount > 0 ? kSegCount - 1 : 0),
                r.value(), "predicate rejected result"));
        }
        return r;
    }

    template <typename T>
    [[nodiscard]] auto RunAs(UiNode* root) const -> detail::QueryResult<observer_ptr<T>> {
        auto r = Run(root);
        if (!r) { return std::unexpected(r.error()); }
        auto* typed = dynamic_cast<T*>(r.value());
        if (!typed) {
            return std::unexpected(detail::QueryError{
                kSegCount > 0 ? kSegCount - 1 : 0,
                std::string(Path.view()),
                r.value() ? std::string(r.value()->Id()) : "",
                r.value() ? std::string(NodeTypeName(r.value()->Type())) : "",
                "dynamic_cast failed to target type",
            });
        }
        return observer_ptr<T>(typed);
    }

    template <typename T, NodePredicate Pred>
    [[nodiscard]] auto RunAs(UiNode* root, Pred&& pred) const
        -> detail::QueryResult<observer_ptr<T>>
    {
        auto r = Run(root, std::forward<Pred>(pred));
        if (!r) { return std::unexpected(r.error()); }
        auto* typed = dynamic_cast<T*>(r.value());
        if (!typed) {
            return std::unexpected(detail::QueryError{
                kSegCount > 0 ? kSegCount - 1 : 0,
                std::string(Path.view()),
                r.value() ? std::string(r.value()->Id()) : "",
                r.value() ? std::string(NodeTypeName(r.value()->Type())) : "",
                "dynamic_cast failed to target type",
            });
        }
        return observer_ptr<T>(typed);
    }

    // -- Multi-result execution --

    [[nodiscard]] auto RunAll(UiNode* root) const -> std::vector<UiNode*> {
        return detail::ExecuteQueryAll(root, kParsed, Path.view());
    }

    template <NodePredicate Pred>
    [[nodiscard]] auto RunAll(UiNode* root, Pred&& pred) const -> std::vector<UiNode*> {
        auto all = detail::ExecuteQueryAll(root, kParsed, Path.view());
        std::erase_if(all, [&](UiNode* n) {
            return !std::invoke(std::forward<Pred>(pred), n);
        });
        return all;
    }

    // -- Convenience --

    [[nodiscard]] auto Exists(UiNode* root) const -> bool {
        return Run(root).has_value();
    }

    template <NodePredicate Pred>
    [[nodiscard]] auto Exists(UiNode* root, Pred&& pred) const -> bool {
        return Run(root, std::forward<Pred>(pred)).has_value();
    }

    [[nodiscard]] auto RunOr(UiNode* root, UiNode* fallback) const -> UiNode* {
        auto r = Run(root);
        return r.has_value() ? r.value() : fallback;
    }

    template <NodePredicate Pred>
    [[nodiscard]] auto RunOr(UiNode* root, UiNode* fallback, Pred&& pred) const -> UiNode* {
        auto r = Run(root, std::forward<Pred>(pred));
        return r.has_value() ? r.value() : fallback;
    }
};

// -- Global constexpr query instances --

template <FixedString Path>
inline constexpr UiQuery<Path> Q{};

// =========================================================================== //
// Part 7: Pipe operators
// =========================================================================== //

// -- node | Q<"Path"> → QueryResult<UiNode*> --

template <FixedString Path>
auto operator|(UiNode& node, UiQuery<Path> q) -> detail::QueryResult<UiNode*> {
    return q.Run(&node);
}

template <FixedString Path>
auto operator|(UiNode* node, UiQuery<Path> q) -> detail::QueryResult<UiNode*> {
    return q.Run(node);
}

// -- QueryResult | Q<"Path"> → QueryResult<UiNode*> (chaining) --

template <FixedString Path>
auto operator|(detail::QueryResult<UiNode*> prev, UiQuery<Path> q)
    -> detail::QueryResult<UiNode*>
{
    if (!prev) { return prev; }
    return q.Run(prev.value());
}

// =========================================================================== //
// Part 8: Pipe terminal tags
// =========================================================================== //

/** @brief Terminal: collect all matches into vector. */
struct AllTag {
    // Intentionally empty -- used as pipe terminator.
};
inline constexpr AllTag All{};

/** @brief Terminal: check existence. */
struct ExistsTag {};
inline constexpr ExistsTag Exist{};

/** @brief Terminal: typed result. */
template <typename T>
struct AsTag {};
template <typename T>
inline constexpr AsTag<T> As{};

// Q<"Path"> | All → AllQuery descriptor, then node | AllQuery → vector

template <FixedString Path>
struct AllQuery {
    [[nodiscard]] auto Run(UiNode* root) const -> std::vector<UiNode*> {
        return UiQuery<Path>{}.RunAll(root);
    }
};

template <FixedString Path>
auto operator|(UiQuery<Path> /*q*/, AllTag /*tag*/) -> AllQuery<Path> {
    return {};
}

template <FixedString Path>
auto operator|(UiNode& node, AllQuery<Path> q) -> std::vector<UiNode*> {
    return q.Run(&node);
}

template <FixedString Path>
auto operator|(UiNode* node, AllQuery<Path> q) -> std::vector<UiNode*> {
    return q.Run(node);
}

// -- Q<"Path"> | As<T> → AsQuery descriptor --

template <FixedString Path, typename T>
struct AsQuery {
    [[nodiscard]] auto Run(UiNode* root) const -> detail::QueryResult<observer_ptr<T>> {
        return UiQuery<Path>{}.template RunAs<T>(root);
    }
};

template <FixedString Path, typename T>
auto operator|(UiQuery<Path> /*q*/, AsTag<T> /*tag*/) -> AsQuery<Path, T> {
    return {};
}

template <FixedString Path, typename T>
auto operator|(UiNode& node, AsQuery<Path, T> q) -> detail::QueryResult<observer_ptr<T>> {
    return q.Run(&node);
}

template <FixedString Path, typename T>
auto operator|(UiNode* node, AsQuery<Path, T> q) -> detail::QueryResult<observer_ptr<T>> {
    return q.Run(node);
}

// -- Q<"Path"> | Exist → ExistsQuery descriptor --

template <FixedString Path>
struct ExistsQuery {
    [[nodiscard]] auto Run(UiNode* root) const -> bool {
        return UiQuery<Path>{}.Exists(root);
    }
};

template <FixedString Path>
auto operator|(UiQuery<Path> /*q*/, ExistsTag /*tag*/) -> ExistsQuery<Path> {
    return {};
}

template <FixedString Path>
auto operator|(UiNode& node, ExistsQuery<Path> q) -> bool {
    return q.Run(&node);
}

template <FixedString Path>
auto operator|(UiNode* node, ExistsQuery<Path> q) -> bool {
    return q.Run(node);
}

// =========================================================================== //
// Part 8b: Filter pipe terminal
// =========================================================================== //

/** @brief Wrapper holding a predicate for pipe-based filtering. */
template <NodePredicate Pred>
struct FilterTag {
    Pred pred;
};

/** @brief Create a Filter pipe terminal from a lambda. */
template <NodePredicate Pred>
auto Filter(Pred&& pred) -> FilterTag<std::decay_t<Pred>> {
    return {std::forward<Pred>(pred)};
}

// -- QueryResult | Filter(pred) -> QueryResult (single-result filtering) --

template <NodePredicate Pred>
auto operator|(detail::QueryResult<UiNode*> prev, FilterTag<Pred> f)
    -> detail::QueryResult<UiNode*>
{
    if (!prev) { return prev; }
    if (!std::invoke(f.pred, prev.value())) {
        return std::unexpected(detail::QueryError{
            0, "", prev.value() ? std::string(prev.value()->Id()) : "",
            prev.value() ? std::string(NodeTypeName(prev.value()->Type())) : "",
            "predicate rejected result",
        });
    }
    return prev;
}

// -- vector<UiNode*> | Filter(pred) -> vector<UiNode*> (multi-result filtering) --

template <NodePredicate Pred>
auto operator|(std::vector<UiNode*> prev, FilterTag<Pred> f)
    -> std::vector<UiNode*>
{
    std::erase_if(prev, [&](UiNode* n) { return !std::invoke(f.pred, n); });
    return prev;
}

// =========================================================================== //
// Part 9: Matches<"Pattern"> -- node predicate
// =========================================================================== //

/**
 * @brief Check if a node's root-to-self path matches a pattern.
 *
 * Supports ** as wildcard for any number of intermediate ancestors.
 */
template <FixedString Pattern>
auto Matches(UiNode* node) -> bool {
    static constexpr auto parsed = detail::ParsePath<Pattern>();
    return detail::MatchesPattern(node, parsed);
}

// =========================================================================== //
// Part 10: Path(node) -- runtime reverse path string (for debugging)
// =========================================================================== //

/**
 * @brief Build a human-readable path string from root to the given node.
 *
 * Format: "Shell/WindowNode[0]/TitleBar/MenuBar"
 * Includes [index] when there are multiple siblings of the same type.
 */
MATCHA_EXPORT auto NodePath(const UiNode* node) -> std::string;

} // namespace matcha::fw
