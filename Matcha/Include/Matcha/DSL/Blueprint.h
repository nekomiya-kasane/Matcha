#pragma once

/**
 * @file Blueprint.h
 * @brief Declarative UI description layer — compile-time verified Blueprint DSL.
 *
 * Enables in-language declarative UI construction using C++23 features:
 * designated initializers, CTAD, operator overloading, and consteval
 * NodeType validation via FixedString NTTP.
 *
 * Zero Qt dependency. Part of the Matcha Foundation layer.
 *
 * @par Usage
 * @code
 *   using namespace matcha::dsl;
 *   auto panel = N<"Container">("settings")(
 *       N<"Label">("title") | Prop("text", "Settings"),
 *       N<"LineEdit">("name") | Prop("placeholder", "Enter name"),
 *       N<"PushButton">("ok") | Prop("text", "OK")
 *   );
 *   auto* root = Materialize(panel, parentNode);
 * @endcode
 */

#include "Matcha/Core/FixedString.h"
#include "Matcha/Tree/UiNode.h"

#include <string>
#include <utility>
#include <vector>

namespace matcha::dsl {

using matcha::fw::NodeType;
using matcha::fw::ParseNodeType;

// ============================================================================
// Prop helper
// ============================================================================

/// @brief Create a property key-value pair for Blueprint DSL.
inline auto Prop(std::string key, std::string value) -> std::pair<std::string, std::string> {
    return {std::move(key), std::move(value)};
}

// ============================================================================
// Blueprint — runtime tree description
// ============================================================================

/// @brief Runtime description of a UI subtree. Type-checked at compile time
/// via Node<> but stored as a plain data structure for runtime materialization.
struct Blueprint {
    NodeType type;
    std::string id;
    std::vector<std::pair<std::string, std::string>> properties;
    std::vector<Blueprint> children;

    /// @brief Identity conversion (allows uniform .Build() on both Node and Blueprint).
    [[nodiscard]] auto Build() const -> Blueprint { return *this; }
};

// ============================================================================
// Node<TypeName> — compile-time verified node descriptor
// ============================================================================

/// @brief Compile-time verified node descriptor.
/// @tparam TypeName A FixedString containing the NodeType name (e.g. "PushButton").
///         Validated at compile time via consteval ParseNodeType().
template <matcha::FixedString TypeName>
struct Node {
    static constexpr NodeType kType = ParseNodeType(TypeName.view());

    std::string id;
    std::vector<std::pair<std::string, std::string>> props;

    /// @brief Add children to produce a Blueprint with nested structure.
    /// Accepts both Node<> (has .Build()) and Blueprint (has .Build()) as children.
    template <typename... Children>
    auto operator()(Children&&... childNodes) const -> Blueprint {
        Blueprint bp{kType, id, props, {}};
        (bp.children.push_back(std::forward<Children>(childNodes).Build()), ...);
        return bp;
    }

    /// @brief Convert this leaf node to a Blueprint (no children).
    [[nodiscard]] auto Build() const -> Blueprint {
        return {kType, id, props, {}};
    }
};

/// @brief Pipe operator for fluent property setting on Node<>.
template <matcha::FixedString TypeName>
auto operator|(Node<TypeName> node, std::pair<std::string, std::string> prop) -> Node<TypeName> {
    node.props.push_back(std::move(prop));
    return node;
}

/// @brief Convenience factory: create a Node<TypeName> with an optional id.
template <matcha::FixedString TypeName>
auto N(std::string id = {}) -> Node<TypeName> {
    return {.id = std::move(id), .props = {}};
}

// ============================================================================
// Materialize — runtime Blueprint -> live UiNode tree
// ============================================================================

/// @brief Materialize a Blueprint into a live UiNode subtree.
/// @param bp     The blueprint to materialize.
/// @param parent The parent UiNode to attach to (children are AddNode'd).
/// @return Non-owning pointer to the root of the materialized subtree.
MATCHA_EXPORT auto Materialize(const Blueprint& bp, matcha::fw::UiNode* parent) -> matcha::fw::UiNode*;

} // namespace matcha::dsl
