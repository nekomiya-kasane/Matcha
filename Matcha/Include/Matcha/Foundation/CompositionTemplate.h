#pragma once

/**
 * @file CompositionTemplate.h
 * @brief Declarative composition templates for canonical page layouts.
 *
 * Describes structural presets like MasterDetail, HeaderDetailFooter,
 * Shell layout, and Dialog templates. Qt-free — produces slot descriptors
 * with tree structure that the UiNode layer materializes into widget trees.
 *
 * @see Matcha_Design_System_Specification.md §6.2
 */

#include "Matcha/Foundation/Macros.h"

#include <cstdint>
#include <string>
#include <vector>

namespace matcha::fw {

// ============================================================================
// Enums
// ============================================================================

/**
 * @brief Well-known composition template types.
 */
enum class TemplateKind : uint8_t {
    MasterDetail,         ///< Sidebar list + detail content
    HeaderDetailFooter,   ///< Header + scrollable detail + footer bar
    ShellLayout,          ///< Full application shell (TitleBar/MenuBar/ActionBar/Viewport/StatusBar)
    DialogConfirm,        ///< Confirmation dialog
    DialogInput,          ///< Single-input dialog
    DialogWizard,         ///< Multi-step wizard
    DialogSettings,       ///< Category-sidebar settings dialog
};

/**
 * @brief Dock edge for panels/bars.
 */
enum class DockEdge : uint8_t {
    Top,
    Bottom,
    Left,
    Right,
};

/**
 * @brief Split direction for a slot group container.
 */
enum class SplitDirection : uint8_t {
    Horizontal,  ///< Children laid out left-to-right
    Vertical,    ///< Children laid out top-to-bottom (default)
};

/**
 * @brief A named slot in a composition template (tree node).
 *
 * Each slot can be a leaf (content region) or a container (with children).
 * This models nested layouts like "titleBar | [viewport | propertyPanel] | statusBar".
 */
struct SlotDescriptor {
    std::string name;             ///< Unique slot identifier
    int         defaultWidth  = 0;  ///< Default width (0 = flexible)
    int         defaultHeight = 0;  ///< Default height (0 = flexible)
    int         minWidth      = 0;  ///< Minimum width constraint
    int         maxWidth      = 0;  ///< Maximum width (0 = unconstrained)
    int         minHeight     = 0;  ///< Minimum height constraint
    int         maxHeight     = 0;  ///< Maximum height (0 = unconstrained)
    bool        scrollable    = false; ///< Content scrolls vertically
    bool        collapsible   = false; ///< Can be collapsed/hidden
    bool        resizable     = false; ///< User can resize (splitter)
    DockEdge    edge          = DockEdge::Left; ///< Dock edge (for panels)
    int         flex          = 0;  ///< Flex factor (0 = fixed)

    SplitDirection splitDir = SplitDirection::Vertical; ///< Children layout direction
    std::vector<SlotDescriptor> children; ///< Child slots (empty = leaf node)

    [[nodiscard]] auto IsContainer() const -> bool { return !children.empty(); }
};

/**
 * @brief Complete template descriptor — a tree of named slots + metadata.
 */
struct TemplateDescriptor {
    TemplateKind kind {};
    std::string  name;                   ///< Human-readable name
    int          defaultWidth  = 0;      ///< Default container width
    int          defaultHeight = 0;      ///< Default container height
    int          minWidth      = 0;      ///< Minimum container width
    int          minHeight     = 0;      ///< Minimum container height
    SlotDescriptor root;                 ///< Root slot (container with children)
};

// ============================================================================
// TemplateBuilder — fluent API for constructing templates (Fix #11)
// ============================================================================

/**
 * @class TemplateBuilder
 * @brief Fluent builder for custom TemplateDescriptor construction.
 */
class MATCHA_EXPORT TemplateBuilder {
public:
    TemplateBuilder(TemplateKind kind, const std::string& name);

    auto DefaultSize(int w, int h) -> TemplateBuilder&;
    auto MinSize(int w, int h) -> TemplateBuilder&;

    auto RootVertical() -> TemplateBuilder&;
    auto RootHorizontal() -> TemplateBuilder&;

    auto AddSlot(const SlotDescriptor& slot) -> TemplateBuilder&;
    auto AddGroup(const std::string& name, SplitDirection dir,
                  std::vector<SlotDescriptor> children, int flex = 0) -> TemplateBuilder&;

    [[nodiscard]] auto Build() const -> TemplateDescriptor;

private:
    TemplateDescriptor _desc;
};

// ============================================================================
// CompositionTemplate — factory for canonical layouts
// ============================================================================

/**
 * @class CompositionTemplate
 * @brief Factory for well-known composition template descriptors.
 */
class MATCHA_EXPORT CompositionTemplate {
public:
    static auto MasterDetail(int masterWidth = 240,
                             DockEdge masterSide = DockEdge::Left) -> TemplateDescriptor;

    static auto HeaderDetailFooter(int headerHeight = 48,
                                   int footerHeight = 40) -> TemplateDescriptor;

    static auto ShellLayout() -> TemplateDescriptor;

    static auto DialogConfirm() -> TemplateDescriptor;
    static auto DialogInput() -> TemplateDescriptor;
    static auto DialogWizard() -> TemplateDescriptor;
    static auto DialogSettings() -> TemplateDescriptor;

    static auto Create(TemplateKind kind) -> TemplateDescriptor;

    /**
     * @brief Find a slot by name in the tree. Returns nullptr if not found.
     */
    static auto FindSlot(const TemplateDescriptor& desc,
                         const std::string& name) -> const SlotDescriptor*;

    /**
     * @brief Collect all leaf slots from the tree (flattened, in DFS order).
     */
    static auto FlattenSlots(const TemplateDescriptor& desc) -> std::vector<const SlotDescriptor*>;
};

// ============================================================================
// ShellLayoutManager — consumer stub (Fix #12)
// ============================================================================

/**
 * @class ShellLayoutManager
 * @brief Interprets a TemplateDescriptor to manage shell panel visibility.
 *
 * This is a Foundation-level stub. The UiNode layer subclasses or wraps
 * this to create actual widget trees from the descriptor.
 */
class MATCHA_EXPORT ShellLayoutManager {
public:
    explicit ShellLayoutManager(TemplateDescriptor desc);

    [[nodiscard]] auto Descriptor() const -> const TemplateDescriptor& { return _desc; }

    /**
     * @brief Get the slot descriptor for a named region.
     */
    [[nodiscard]] auto GetSlot(const std::string& name) const -> const SlotDescriptor*;

    /**
     * @brief Check if a slot is currently visible (not collapsed).
     */
    [[nodiscard]] auto IsSlotVisible(const std::string& name) const -> bool;

    /**
     * @brief Set visibility of a collapsible slot.
     */
    void SetSlotVisible(const std::string& name, bool visible);

private:
    TemplateDescriptor _desc;
    struct SlotVisibility { std::string name; bool visible = true; };
    std::vector<SlotVisibility> _visibility;

    void InitVisibility(const SlotDescriptor& slot);
};

} // namespace matcha::fw
