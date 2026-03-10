#pragma once

/**
 * @file UiNode.h
 * @brief Base class for all nodes in the UiNode tree.
 *
 * UiNode is the fundamental building block of the Matcha UI framework's
 * declarative tree. Business layer assembles UI by creating UiNode subclass
 * instances and adding them as children to container nodes.
 *
 * @par Ownership
 * Parent owns children via std::unique_ptr. UiNode stores a non-owning
 * back-pointer to its parent.
 *
 * @par Qt Independence
 * UiNode base has NO Qt dependency. Subclasses that wrap QWidgets override
 * Widget() to return their concrete QWidget pointer.
 */

#include "Matcha/UiNodes/Core/CommandNode.h"

#include <cstdint>
#include <functional>
#include <generator>
#include <memory>
#include <string>
#include <string_view>

class QWidget;

namespace matcha::fw {

/**
 * @brief Enumerates all node types in the UiNode tree.
 *
 * Each framework-provided container and widget node has a dedicated entry.
 * Custom user widgets use NodeType::Custom via WidgetWrapper.
 */
enum class NodeType : uint8_t {
    // -- Root --
    Shell,

    // -- Window abstraction (multi-window support) --
    WindowNode,
    TitleBar,
    DocumentToolBar,
    LogoButton,

    // -- ActionBar hierarchy --
    ActionBar,
    ActionTab,
    ActionToolbar,
    ActionButton,

    // -- Dialog --
    Dialog,

    // -- StatusBar --
    StatusBar,
    StatusBarItem,

    // -- ContextMenu --
    ContextMenu,

    // -- Menu system --
    MenuBar,
    Menu,
    MenuItem,

    // -- WorkspaceFrame layout --
    WorkspaceFrame,
    ControlBar,
    DocumentArea,

    // -- Tab bar --
    TabBar,
    TabItem,
    DocumentBar,  // legacy alias — will be removed

    // -- Document/Viewport 3-layer model --
    DocumentPage,
    ViewportGroup,
    Viewport,

    // -- WidgetNode typed nodes (Scheme D) --
    // Input widgets
    PushButton,
    ToolButton,
    RadioButton,
    LineEdit,
    ComboBox,
    SpinBox,
    DoubleSpinBox,
    CheckBox,
    ToggleSwitch,
    // Display widgets
    Label,
    ProgressBar,
    Slider,
    ColorPicker,
    ColorSwatch,
    // Display widgets (continued)
    ProgressRing,
    Badge,
    Line,
    // Data widgets
    DataTable,
    PropertyGrid,
    ListWidget,
    TreeWidget,
    SearchBox,
    PlainTextEdit,
    // Composite input widgets
    RangeSlider,
    DateTimePicker,
    // Navigation
    Paginator,
    // Feedback
    Message,
    Notification,
    // Layout
    CollapsibleSection,

    // -- Layout container --
    Container,

    // -- Escape hatch --
    WidgetWrapper,
    Custom,

    _Count  ///< Sentinel for array sizing. Must be last.
};

// --------------------------------------------------------------------------- //
// NodeType <-> string constexpr mapping
// --------------------------------------------------------------------------- //

struct NodeTypeEntry {
    std::string_view name;
    NodeType         type;
};

inline constexpr NodeTypeEntry kNodeTypeTable[] = {
    {"Shell",          NodeType::Shell},
    {"WindowNode",     NodeType::WindowNode},
    {"TitleBar",          NodeType::TitleBar},
    {"DocumentToolBar",   NodeType::DocumentToolBar},
    {"LogoButton",        NodeType::LogoButton},
    {"ActionBar",         NodeType::ActionBar},
    {"ActionTab",      NodeType::ActionTab},
    {"ActionToolbar",  NodeType::ActionToolbar},
    {"ActionButton",   NodeType::ActionButton},
    {"Dialog",         NodeType::Dialog},
    {"StatusBar",      NodeType::StatusBar},
    {"StatusBarItem",  NodeType::StatusBarItem},
    {"ContextMenu",    NodeType::ContextMenu},
    {"MenuBar",        NodeType::MenuBar},
    {"Menu",           NodeType::Menu},
    {"MenuItem",       NodeType::MenuItem},
    {"WorkspaceFrame", NodeType::WorkspaceFrame},
    {"ControlBar",     NodeType::ControlBar},
    {"DocumentArea",   NodeType::DocumentArea},
    {"TabBar",         NodeType::TabBar},
    {"TabItem",        NodeType::TabItem},
    {"DocumentBar",    NodeType::DocumentBar},
    {"DocumentPage",   NodeType::DocumentPage},
    {"ViewportGroup",  NodeType::ViewportGroup},
    {"Viewport",       NodeType::Viewport},
    {"PushButton",     NodeType::PushButton},
    {"ToolButton",     NodeType::ToolButton},
    {"RadioButton",    NodeType::RadioButton},
    {"LineEdit",       NodeType::LineEdit},
    {"ComboBox",       NodeType::ComboBox},
    {"SpinBox",        NodeType::SpinBox},
    {"DoubleSpinBox",  NodeType::DoubleSpinBox},
    {"CheckBox",       NodeType::CheckBox},
    {"ToggleSwitch",   NodeType::ToggleSwitch},
    {"Label",          NodeType::Label},
    {"ProgressBar",    NodeType::ProgressBar},
    {"Slider",         NodeType::Slider},
    {"ColorPicker",    NodeType::ColorPicker},
    {"ColorSwatch",    NodeType::ColorSwatch},
    {"DataTable",      NodeType::DataTable},
    {"PropertyGrid",   NodeType::PropertyGrid},
    {"ListWidget",     NodeType::ListWidget},
    {"TreeWidget",     NodeType::TreeWidget},
    {"SearchBox",      NodeType::SearchBox},
    {"PlainTextEdit", NodeType::PlainTextEdit},
    {"RangeSlider",    NodeType::RangeSlider},
    {"DateTimePicker", NodeType::DateTimePicker},
    {"Paginator",      NodeType::Paginator},
    {"ProgressRing",   NodeType::ProgressRing},
    {"Badge",          NodeType::Badge},
    {"Message",        NodeType::Message},
    {"Notification",   NodeType::Notification},
    {"Line",           NodeType::Line},
    {"CollapsibleSection", NodeType::CollapsibleSection},
    {"Container",      NodeType::Container},
    {"WidgetWrapper",  NodeType::WidgetWrapper},
    {"Custom",         NodeType::Custom},
};

/** @brief Compile-time lookup: string -> NodeType. Fails to compile on unknown name. */
consteval auto ParseNodeType(std::string_view name) -> NodeType {
    for (const auto& e : kNodeTypeTable) {
        if (e.name == name) { return e.type; }
    }
    throw "Unknown NodeType name"; // consteval: compile error
}

/** @brief Runtime/constexpr reverse lookup: NodeType -> string. */
constexpr auto NodeTypeName(NodeType type) -> std::string_view {
    for (const auto& e : kNodeTypeTable) {
        if (e.type == type) { return e.name; }
    }
    return "?";
}

/**
 * @brief Base class for all nodes in the Matcha UiNode tree.
 *
 * Provides:
 * - Node identity: Id(), Type(), Name()
 * - Parent/child tree structure with unique_ptr ownership
 * - Virtual Widget() for subclasses that wrap QWidgets
 * - MetaClass RTTI via BaseObject inheritance
 */
class MATCHA_EXPORT UiNode : public matcha::CommandNode {
    MATCHA_DECLARE_CLASS

public:
    /**
     * @brief Construct a UiNode.
     * @param id Unique identifier for this node.
     * @param type The node type.
     * @param name Human-readable name (defaults to id if empty).
     */
    explicit UiNode(std::string id, NodeType type, std::string name = "");

    ~UiNode() override;

    UiNode(const UiNode&) = delete;
    auto operator=(const UiNode&) -> UiNode& = delete;
    UiNode(UiNode&&) = delete;
    auto operator=(UiNode&&) -> UiNode& = delete;

    // -- Identity --
    // Id() is inherited from CommandNode.

    /// @brief Node type enum.
    [[nodiscard]] auto Type() const -> NodeType { return _type; }

    /// @brief Human-readable name.
    [[nodiscard]] auto Name() const -> std::string_view { return _name; }

    /// @brief Set the human-readable name.
    void SetName(std::string_view name) { _name = std::string(name); }

    // -- Tree structure (typed wrappers over CommandNode) --

    /// @brief Non-owning pointer to parent UiNode. nullptr for root.
    [[nodiscard]] auto ParentNode() const -> UiNode*;

    /// @brief Add a UiNode child. Takes ownership.
    /// @return Non-owning pointer to the added child.
    /// @note Container subclasses override this to validate child type
    ///       and sync the Qt widget tree.
    virtual auto AddNode(std::unique_ptr<UiNode> child) -> UiNode*;

    /// @brief Remove a UiNode child. Returns ownership.
    /// @return Ownership of the removed child, or nullptr if not found.
    /// @note Container subclasses override this to detach the Qt widget.
    virtual auto RemoveNode(UiNode* child) -> std::unique_ptr<UiNode>;

    /// @brief Number of direct UiNode children.
    [[nodiscard]] auto NodeCount() const -> size_t { return CommandNode::ChildCount(); }

    /// @brief Get the i-th UiNode child (unchecked).
    [[nodiscard]] auto NodeAt(size_t index) const -> UiNode*;

    /// @brief Find a descendant by id (recursive depth-first search).
    /// @return Non-owning pointer, or nullptr if not found.
    [[nodiscard]] auto FindById(std::string_view id) const -> UiNode*;

    /// @brief Find a descendant by name (recursive depth-first search).
    /// @return Non-owning pointer, or nullptr if not found.
    [[nodiscard]] auto FindByName(std::string_view name) const -> UiNode*;

    // -- Traversal --

    /// @brief Lazy depth-first (pre-order) generator over all descendants.
    [[nodiscard]] auto Descendants() -> std::generator<UiNode*>;

    /// @brief Lazy generator over descendants matching a given NodeType.
    [[nodiscard]] auto DescendantsOfType(NodeType type) -> std::generator<UiNode*>;

    /// @brief Lazy generator over direct children matching a given NodeType.
    [[nodiscard]] auto ChildrenOfType(NodeType type) -> std::generator<UiNode*>;

    /// @brief Depth-first (pre-order) traversal calling visitor on each node.
    void TraverseDepthFirst(const std::function<void(UiNode*)>& visitor);

    /// @brief Convenience: call fn on this node and all descendants.
    void ForEachNode(std::function<void(UiNode*)> fn);

    // -- Query redirect --

    /// @brief Called by UiQuery when this node is matched by a query.
    /// Override in subclasses to redirect query results to another node
    /// (e.g. proxy nodes, wrapper nodes that should resolve to their inner node).
    /// @return `this` by default. Override to return a different UiNode.
    [[nodiscard]] virtual auto ResolveQueryTarget() -> UiNode* { return this; }

    // -- Widget bridge --

    /// @brief Return the underlying QWidget, if any.
    /// @return nullptr by default. Overridden by container/widget nodes.
    [[nodiscard]] virtual auto Widget() -> QWidget* { return nullptr; }

    /// @brief Resolve the parent QWidget by walking up the UiNode tree.
    /// @return The nearest ancestor's Widget(), or nullptr if none found.
    [[nodiscard]] auto ResolveParentWidget() -> QWidget*;

    /// @brief Re-parent this node's widget under a new UiNode's widget.
    /// @param newParent The UiNode whose Widget() becomes the Qt parent.
    ///                  Pass nullptr to unparent.
    void ReparentTo(UiNode* newParent);

    // -- Focus scope (S4/C6) --

    /// @brief Mark this node as a focus scope boundary.
    /// When a focus scope is active, Tab/Shift+Tab cycling is constrained
    /// to the subtree rooted at this node (focus trapping).
    void SetFocusScope(bool isScope) { _focusScope = isScope; }

    /// @brief Check if this node is a focus scope boundary.
    [[nodiscard]] auto IsFocusScope() const -> bool { return _focusScope; }

    /// @brief Walk up the tree and find the nearest ancestor (or self) that
    /// is marked as a focus scope. Returns nullptr if none found.
    [[nodiscard]] auto FindEnclosingFocusScope() -> UiNode*;

private:
    std::string _name;
    NodeType _type;
    bool _focusScope = false;
};

} // namespace matcha::fw
