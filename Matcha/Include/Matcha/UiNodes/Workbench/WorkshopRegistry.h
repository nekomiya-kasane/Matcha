#pragma once

/**
 * @file WorkshopRegistry.h
 * @brief Centralized registry for Workshop/Workbench/Addin descriptors.
 *
 * All descriptors are registered here during plugin loading. The registry
 * provides lookup, command resolution, and structural validation.
 *
 * @see docs/Matcha_Design_System_Specification.md Appendix C.9
 */

#include "Matcha/Foundation/Macros.h"
#include "Matcha/Foundation/StringId.h"
#include "WorkbenchTypes.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace matcha::fw {

/**
 * @brief Centralized registry of all Workshop, Workbench, and Addin descriptors.
 *
 * Created by Application::Initialize() and accessible via
 * Application::GetWorkshopRegistry().
 */
class MATCHA_EXPORT WorkshopRegistry {
public:
    WorkshopRegistry()  = default;
    ~WorkshopRegistry() = default;

    WorkshopRegistry(const WorkshopRegistry&)            = delete;
    auto operator=(const WorkshopRegistry&) -> WorkshopRegistry& = delete;
    WorkshopRegistry(WorkshopRegistry&&)                 = default;
    auto operator=(WorkshopRegistry&&) -> WorkshopRegistry& = default;

    // -- Registration -------------------------------------------------------- //

    /// @return false if a workshop with the same id is already registered.
    auto RegisterWorkshop(WorkshopDescriptor desc) -> bool;
    /// @return false if a workbench with the same id is already registered.
    auto RegisterWorkbench(WorkbenchDescriptor desc) -> bool;
    void RegisterAddin(AddinDescriptor desc);

    // -- Unregistration ------------------------------------------------------ //

    /// @return false if the workshop was not found.
    auto UnregisterWorkshop(const WorkshopId& id) -> bool;
    /// @return false if the workbench was not found.
    auto UnregisterWorkbench(const WorkbenchId& id) -> bool;

    // -- Lookup -------------------------------------------------------------- //

    [[nodiscard]] auto FindWorkshop(const WorkshopId& id) -> WorkshopDescriptor*;
    [[nodiscard]] auto FindWorkshop(const WorkshopId& id) const -> const WorkshopDescriptor*;

    [[nodiscard]] auto FindWorkbench(const WorkbenchId& id) -> WorkbenchDescriptor*;
    [[nodiscard]] auto FindWorkbench(const WorkbenchId& id) const -> const WorkbenchDescriptor*;

    // -- Addin query --------------------------------------------------------- //

    [[nodiscard]] auto FindAddins(const WorkshopId& id) const
        -> std::vector<const AddinDescriptor*>;
    [[nodiscard]] auto FindAddins(const WorkbenchId& id) const
        -> std::vector<const AddinDescriptor*>;

    // -- Command resolution -------------------------------------------------- //

    /**
     * @brief Resolve a CmdHeaderId to its descriptor.
     *
     * Resolution order:
     * 1. Active Workbench's commands[]
     * 2. Active Workshop's commands[]
     * 3. Active Workbench Addins' commands[]
     * 4. Active Workshop Addins' commands[]
     *
     * For context-free resolution (no active state), searches all descriptors.
     */
    [[nodiscard]] auto ResolveCommand(const CmdHeaderId& id) const
        -> const CommandHeaderDescriptor*;

    /**
     * @brief Context-aware command resolution given active workshop/workbench.
     */
    [[nodiscard]] auto ResolveCommand(const CmdHeaderId& id,
                                       const WorkshopId& wsId,
                                       const WorkbenchId& wbId) const
        -> const CommandHeaderDescriptor*;

    // -- Enumeration --------------------------------------------------------- //

    [[nodiscard]] auto AllWorkshopIds() const -> std::vector<WorkshopId>;
    [[nodiscard]] auto WorkbenchIdsFor(const WorkshopId& id) const -> std::vector<WorkbenchId>;

    [[nodiscard]] auto WorkshopCount()  const -> size_t { return _workshops.size(); }
    [[nodiscard]] auto WorkbenchCount() const -> size_t { return _workbenches.size(); }
    [[nodiscard]] auto AddinCount()     const -> size_t { return _addins.size(); }

    // -- Validation ---------------------------------------------------------- //

    struct ValidationResult {
        bool                     valid = true;
        std::vector<std::string> errors;
    };

    /**
     * @brief Perform structural validation of all registered descriptors.
     *
     * Checks:
     * - Orphan workbench (workshopId not found)
     * - Missing default workbench
     * - Dangling command ref in TabBlueprint
     * - Duplicate ID
     * - Cycle detection (workshop A's default belongs to workshop B)
     */
    [[nodiscard]] auto Validate() const -> ValidationResult;

private:
    std::unordered_map<std::string, WorkshopDescriptor>  _workshops;
    std::unordered_map<std::string, WorkbenchDescriptor> _workbenches;
    std::vector<AddinDescriptor>                         _addins;

    [[nodiscard]] auto FindCommandIn(const CmdHeaderId& id,
                                      const std::vector<CommandHeaderDescriptor>& cmds) const
        -> const CommandHeaderDescriptor*;
};

} // namespace matcha::fw
