#include "Matcha/Tree/Composition/Workbench/WorkshopRegistry.h"

#include <format>

namespace matcha::fw {

// --------------------------------------------------------------------------- //
// Registration
// --------------------------------------------------------------------------- //

auto WorkshopRegistry::RegisterWorkshop(WorkshopDescriptor desc) -> bool {
    auto key = desc.id.value;
    if (_workshops.contains(key)) { return false; }
    _workshops.emplace(std::move(key), std::move(desc));
    return true;
}

auto WorkshopRegistry::RegisterWorkbench(WorkbenchDescriptor desc) -> bool {
    auto key = desc.id.value;
    if (_workbenches.contains(key)) { return false; }
    _workbenches.emplace(std::move(key), std::move(desc));
    return true;
}

void WorkshopRegistry::RegisterAddin(AddinDescriptor desc) {
    _addins.push_back(std::move(desc));
}

// --------------------------------------------------------------------------- //
// Unregistration
// --------------------------------------------------------------------------- //

auto WorkshopRegistry::UnregisterWorkshop(const WorkshopId& id) -> bool {
    return _workshops.erase(id.value) > 0;
}

auto WorkshopRegistry::UnregisterWorkbench(const WorkbenchId& id) -> bool {
    return _workbenches.erase(id.value) > 0;
}

// --------------------------------------------------------------------------- //
// Lookup
// --------------------------------------------------------------------------- //

auto WorkshopRegistry::FindWorkshop(const WorkshopId& id) -> WorkshopDescriptor* {
    auto it = _workshops.find(id.value);
    return it != _workshops.end() ? &it->second : nullptr;
}

auto WorkshopRegistry::FindWorkshop(const WorkshopId& id) const -> const WorkshopDescriptor* {
    auto it = _workshops.find(id.value);
    return it != _workshops.end() ? &it->second : nullptr;
}

auto WorkshopRegistry::FindWorkbench(const WorkbenchId& id) -> WorkbenchDescriptor* {
    auto it = _workbenches.find(id.value);
    return it != _workbenches.end() ? &it->second : nullptr;
}

auto WorkshopRegistry::FindWorkbench(const WorkbenchId& id) const -> const WorkbenchDescriptor* {
    auto it = _workbenches.find(id.value);
    return it != _workbenches.end() ? &it->second : nullptr;
}

// --------------------------------------------------------------------------- //
// Addin query
// --------------------------------------------------------------------------- //

auto WorkshopRegistry::FindAddins(const WorkshopId& id) const
    -> std::vector<const AddinDescriptor*>
{
    std::vector<const AddinDescriptor*> result;
    for (const auto& a : _addins) {
        if (const auto* wsId = std::get_if<WorkshopId>(&a.target)) {
            if (*wsId == id) {
                result.push_back(&a);
            }
        }
    }
    return result;
}

auto WorkshopRegistry::FindAddins(const WorkbenchId& id) const
    -> std::vector<const AddinDescriptor*>
{
    std::vector<const AddinDescriptor*> result;
    for (const auto& a : _addins) {
        if (const auto* wbId = std::get_if<WorkbenchId>(&a.target)) {
            if (*wbId == id) {
                result.push_back(&a);
            }
        }
    }
    return result;
}

// --------------------------------------------------------------------------- //
// Command resolution
// --------------------------------------------------------------------------- //

auto WorkshopRegistry::FindCommandIn(const CmdHeaderId& id,
                                      const std::vector<CommandHeaderDescriptor>& cmds) const
    -> const CommandHeaderDescriptor*
{
    for (const auto& cmd : cmds) {
        if (cmd.id == id) {
            return &cmd;
        }
    }
    return nullptr;
}

auto WorkshopRegistry::ResolveCommand(const CmdHeaderId& id) const
    -> const CommandHeaderDescriptor*
{
    // Search all workshops
    for (const auto& [_, ws] : _workshops) {
        if (const auto* found = FindCommandIn(id, ws.commands)) { return found; }
    }
    // Search all workbenches
    for (const auto& [_, wb] : _workbenches) {
        if (const auto* found = FindCommandIn(id, wb.commands)) { return found; }
    }
    // Search all addins
    for (const auto& a : _addins) {
        if (const auto* found = FindCommandIn(id, a.commands)) { return found; }
    }
    return nullptr;
}

auto WorkshopRegistry::ResolveCommand(const CmdHeaderId& id,
                                       const WorkshopId& wsId,
                                       const WorkbenchId& wbId) const
    -> const CommandHeaderDescriptor*
{
    // 1. Active Workbench's commands
    if (wbId.IsValid()) {
        if (const auto* wb = FindWorkbench(wbId)) {
            if (const auto* found = FindCommandIn(id, wb->commands)) { return found; }
        }
    }

    // 2. Active Workshop's commands
    if (wsId.IsValid()) {
        if (const auto* ws = FindWorkshop(wsId)) {
            if (const auto* found = FindCommandIn(id, ws->commands)) { return found; }
        }
    }

    // 3. Active Workbench Addins' commands
    if (wbId.IsValid()) {
        for (const auto* addin : FindAddins(wbId)) {
            if (const auto* found = FindCommandIn(id, addin->commands)) { return found; }
        }
    }

    // 4. Active Workshop Addins' commands
    if (wsId.IsValid()) {
        for (const auto* addin : FindAddins(wsId)) {
            if (const auto* found = FindCommandIn(id, addin->commands)) { return found; }
        }
    }

    return nullptr;
}

// --------------------------------------------------------------------------- //
// Enumeration
// --------------------------------------------------------------------------- //

auto WorkshopRegistry::AllWorkshopIds() const -> std::vector<WorkshopId> {
    std::vector<WorkshopId> result;
    result.reserve(_workshops.size());
    for (const auto& [key, _] : _workshops) {
        result.push_back(WorkshopId::From(key));
    }
    return result;
}

auto WorkshopRegistry::WorkbenchIdsFor(const WorkshopId& id) const -> std::vector<WorkbenchId> {
    const auto* ws = FindWorkshop(id);
    if (ws == nullptr) { return {}; }
    return ws->workbenchIds;
}

// --------------------------------------------------------------------------- //
// Validation
// --------------------------------------------------------------------------- //

auto WorkshopRegistry::Validate() const -> ValidationResult {
    ValidationResult result;

    // Check: orphan workbench (workshopId not found in registry)
    for (const auto& [key, wb] : _workbenches) {
        if (!_workshops.contains(wb.workshopId.value)) {
            result.valid = false;
            result.errors.push_back(
                std::format("Orphan workbench '{}': references unknown workshopId '{}'",
                            key, wb.workshopId.value));
        }
    }

    // Check: missing default workbench
    for (const auto& [key, ws] : _workshops) {
        if (ws.defaultWorkbenchId.IsValid()) {
            if (!_workbenches.contains(ws.defaultWorkbenchId.value)) {
                result.valid = false;
                result.errors.push_back(
                    std::format("Workshop '{}': defaultWorkbenchId '{}' not registered",
                                key, ws.defaultWorkbenchId.value));
            }
        }
    }

    // Check: cycle detection (workshop A's default workbench belongs to workshop B)
    for (const auto& [key, ws] : _workshops) {
        if (ws.defaultWorkbenchId.IsValid()) {
            auto wbIt = _workbenches.find(ws.defaultWorkbenchId.value);
            if (wbIt != _workbenches.end()) {
                if (wbIt->second.workshopId != ws.id) {
                    result.valid = false;
                    result.errors.push_back(
                        std::format("Cycle: workshop '{}' default workbench '{}' belongs to workshop '{}'",
                                    key, ws.defaultWorkbenchId.value, wbIt->second.workshopId.value));
                }
            }
        }
    }

    // Check: dangling command refs in TabBlueprints
    auto checkBlueprints = [&](const std::vector<TabBlueprint>& tabs, const std::string& owner) {
        for (const auto& tab : tabs) {
            for (const auto& toolbar : tab.toolbars) {
                for (const auto& cmdId : toolbar.commands) {
                    if (ResolveCommand(cmdId) == nullptr) {
                        result.valid = false;
                        result.errors.push_back(
                            std::format("Dangling command ref '{}' in '{}'/{}/{}",
                                        cmdId.value, owner, tab.tabId, toolbar.toolbarId));
                    }
                }
            }
        }
    };

    for (const auto& [key, ws] : _workshops) {
        checkBlueprints(ws.baseTabs, key);
    }
    for (const auto& [key, wb] : _workbenches) {
        checkBlueprints(wb.taskTabs, key);
    }
    for (size_t i = 0; i < _addins.size(); ++i) {
        checkBlueprints(_addins[i].addinTabs,
                        std::format("addin[{}]/{}", i, _addins[i].pluginId));
    }

    return result;
}

} // namespace matcha::fw
