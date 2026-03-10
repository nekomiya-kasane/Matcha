#pragma once

/**
 * @file WorkbenchTypes.h
 * @brief Declarative descriptor types for the Workshop/Workbench architecture.
 *
 * These are pure value types (no Qt dependency). They describe the UI layout
 * that WorkbenchManager will materialize into live UiNode subtrees.
 *
 * @see docs/Matcha_Design_System_Specification.md Appendix C.7
 */

#include "Matcha/Foundation/Macros.h"
#include "Matcha/Foundation/StringId.h"
#include "Matcha/UiNodes/Core/TokenEnums.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace matcha { class CommandNode; }

namespace matcha::fw {

class UiNode;
class Shell;

// --------------------------------------------------------------------------- //
// CommandHeaderDescriptor — lazy command proxy
// --------------------------------------------------------------------------- //

/**
 * @brief Metadata for a single command. The factory is invoked only on first
 *        user click (lazy loading).
 *
 * A nullptr factory indicates a metadata-only entry (separator, label, etc.).
 */
/**
 * @brief Factory that creates a CommandNode on first invocation.
 *
 * Shared (not move-only) so the same factory can be referenced by
 * multiple descriptors or registry lookups without ownership issues.
 */
using CommandFactory = std::shared_ptr<
    std::move_only_function<std::unique_ptr<matcha::CommandNode>() const>>;

/**
 * @brief Optional callback queried each tick to determine command availability.
 *
 * Return true if the command should be enabled in the current context.
 * If nullptr, the command is always enabled.
 */
using CommandAvailabilityFn = std::function<bool()>;

struct CommandHeaderDescriptor {
    CmdHeaderId   id;
    std::string   label;
    std::string   iconId;
    std::string   tooltip;
    std::string   shortcut;      ///< Qt key sequence string, e.g. "Ctrl+R"
    std::string   libraryHint;   ///< DLL name hint for lazy loading
    IconSize iconSize = IconSize::Sm;

    CommandFactory         factory;       ///< Lazy command instantiation (nullptr = metadata-only)
    CommandAvailabilityFn  availability;  ///< Dynamic enable/disable (nullptr = always enabled)
};

// --------------------------------------------------------------------------- //
// TabBlueprint / ToolbarBlueprint — declarative UI layout
// --------------------------------------------------------------------------- //

/**
 * @brief Describes one toolbar within a tab. Pure data — no UiNode created
 *        until materialization.
 */
struct ToolbarBlueprint {
    std::string                toolbarId;
    std::string                label;
    std::vector<CmdHeaderId>   commands;
};

/**
 * @brief Describes one tab within the ActionBar. Pure data — no UiNode created
 *        until materialization.
 */
struct TabBlueprint {
    std::string                    tabId;
    std::string                    label;
    std::vector<ToolbarBlueprint>  toolbars;
};

// --------------------------------------------------------------------------- //
// MenuBlueprint — declarative menu layout
// --------------------------------------------------------------------------- //

/**
 * @brief Describes one item within a menu. Pure data.
 *
 * Set isSeparator=true for a menu separator (other fields ignored).
 */
struct MenuItemBlueprint {
    CmdHeaderId   commandId;     ///< References a CommandHeaderDescriptor
    std::string   submenuId;     ///< Non-empty = this item opens a submenu
    bool          isSeparator = false;
};

/**
 * @brief Describes one top-level menu contributed by a Workshop/Workbench.
 */
struct MenuBlueprint {
    std::string                     menuId;
    std::string                     label;     ///< e.g. "&Edit"
    std::vector<MenuItemBlueprint>  items;
};

// --------------------------------------------------------------------------- //
// IWorkbenchLifecycle — activation/deactivation protocol
// --------------------------------------------------------------------------- //

/**
 * @brief Optional lifecycle hooks for workshop/workbench activation.
 *
 * OnActivate is called after UI is materialized.
 * OnDeactivate is called before UI is torn down.
 */
class MATCHA_EXPORT IWorkbenchLifecycle {
public:
    virtual ~IWorkbenchLifecycle() = default;

    virtual void OnActivate(Shell& shell)   = 0;
    virtual void OnDeactivate(Shell& shell) = 0;
};

// --------------------------------------------------------------------------- //
// WorkshopDescriptor
// --------------------------------------------------------------------------- //

/**
 * @brief Describes a Workshop (document-type-level UI configuration).
 *
 * A Workshop defines the base tabs visible when a document type is active.
 * It contains one or more Workbenches representing task modes.
 */
struct WorkshopDescriptor {
    WorkshopId                               id;
    std::string                              label;

    std::vector<CommandHeaderDescriptor>      commands;
    std::vector<TabBlueprint>                baseTabs;
    std::vector<MenuBlueprint>               menus;

    WorkbenchId                              defaultWorkbenchId;
    std::vector<WorkbenchId>                 workbenchIds;

    std::shared_ptr<IWorkbenchLifecycle>     lifecycle;
};

// --------------------------------------------------------------------------- //
// WorkbenchDescriptor
// --------------------------------------------------------------------------- //

/**
 * @brief Describes a Workbench (task-mode-level UI configuration within a Workshop).
 *
 * A Workbench adds task-specific tabs and optionally hides workshop base tabs.
 */
struct WorkbenchDescriptor {
    WorkbenchId                              id;
    std::string                              label;
    WorkshopId                               workshopId;

    std::vector<CommandHeaderDescriptor>      commands;
    std::vector<TabBlueprint>                taskTabs;
    std::vector<MenuBlueprint>               menus;
    std::vector<std::string>                 hiddenBaseTabIds;

    std::shared_ptr<IWorkbenchLifecycle>     lifecycle;
};

// --------------------------------------------------------------------------- //
// AddinDescriptor
// --------------------------------------------------------------------------- //

/**
 * @brief Describes a plugin extension to an existing Workshop or Workbench.
 *
 * A single AddinDescriptor can target either a WorkshopId (visible across
 * all workbenches in that workshop) or a specific WorkbenchId.
 */
struct AddinDescriptor {
    std::string                              pluginId;
    std::variant<WorkshopId, WorkbenchId>    target;

    std::vector<CommandHeaderDescriptor>      commands;
    std::vector<TabBlueprint>                addinTabs;
};

// --------------------------------------------------------------------------- //
// IWorkshopContributor — plugin declares contributions
// --------------------------------------------------------------------------- //

/**
 * @brief Interface for plugins that contribute Workshop/Workbench addins.
 *
 * Plugins implement this alongside IExpansionPlugin. During Start(),
 * they register addins with the WorkshopRegistry.
 */
class MATCHA_EXPORT IWorkshopContributor {
public:
    virtual ~IWorkshopContributor() = default;

    virtual auto GetAddins() -> std::vector<AddinDescriptor> = 0;
};

} // namespace matcha::fw
