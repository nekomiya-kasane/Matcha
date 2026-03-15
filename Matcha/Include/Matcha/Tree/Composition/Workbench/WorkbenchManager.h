#pragma once

/**
 * @file WorkbenchManager.h
 * @brief State machine for Workshop/Workbench activation and UI materialization.
 *
 * Orchestrates UI reconfiguration: materializes TabBlueprints into live
 * ActionBar UiNode subtrees, manages lifecycle hooks, and supports a
 * push/pop stack for nested workbench scenarios (e.g., Sketcher).
 *
 * @see docs/Matcha_Design_System_Specification.md Appendix C.10
 */

#include "Matcha/Core/Macros.h"
#include "Matcha/Core/StringId.h"
#include "Matcha/Tree/Composition/Workbench/WorkbenchNotification.h"

#include <memory>
#include <string>
#include <vector>

class QShortcut;

namespace matcha { class CommandNode; }

namespace matcha::fw {

class Shell;
class WorkshopRegistry;
class ActionBarNode;
class ActionButtonNode;
class MenuBarNode;
struct TabBlueprint;
struct MenuBlueprint;
struct CommandHeaderDescriptor;

/**
 * @brief Orchestrator for Workshop/Workbench UI reconfiguration.
 *
 * Created by Application::Initialize() alongside WorkshopRegistry.
 * All public methods assert they are called on the Qt main thread.
 */
class MATCHA_EXPORT WorkbenchManager {
public:
    struct Notification {
        using WorkshopActivated   = matcha::fw::WorkshopActivated;
        using WorkshopDeactivated = matcha::fw::WorkshopDeactivated;
        using WorkbenchActivated  = matcha::fw::WorkbenchActivated;
        using WorkbenchDeactivated = matcha::fw::WorkbenchDeactivated;
        using CommandInvoked      = matcha::fw::CommandInvoked;
    };

    explicit WorkbenchManager(Shell& shell, WorkshopRegistry& registry);
    ~WorkbenchManager();

    WorkbenchManager(const WorkbenchManager&)            = delete;
    auto operator=(const WorkbenchManager&) -> WorkbenchManager& = delete;
    WorkbenchManager(WorkbenchManager&&)                 = delete;
    auto operator=(WorkbenchManager&&) -> WorkbenchManager& = delete;

    // -- Activation ---------------------------------------------------------- //

    /**
     * @brief Activate a workshop (document-type switch).
     *
     * Deactivates the current workbench and workshop, then activates the
     * new workshop and its default workbench.
     * @return false if the id is already active or not found.
     */
    auto ActivateWorkshop(const WorkshopId& id) -> bool;

    /**
     * @brief Activate a workbench within the current workshop (task-mode switch).
     *
     * Deactivates the current workbench, then activates the new one.
     * @return false if the id is already active or not found.
     */
    auto ActivateWorkbench(const WorkbenchId& id) -> bool;

    // -- Push/Pop stack ------------------------------------------------------ //

    /**
     * @brief Push current state and activate a new workbench.
     *
     * If the target workbench belongs to a different workshop, a full
     * workshop switch is performed. Use WorkbenchGuard for RAII cleanup.
     */
    void PushWorkbench(const WorkbenchId& id);

    /**
     * @brief Pop the stack and restore the previous workshop/workbench.
     *
     * Asserts the stack is non-empty.
     */
    void PopWorkbench();

    // -- Query --------------------------------------------------------------- //

    [[nodiscard]] auto ActiveWorkshopId()    const -> const WorkshopId&  { return _activeWorkshopId; }
    [[nodiscard]] auto ActiveWorkbenchId()   const -> const WorkbenchId& { return _activeWorkbenchId; }
    [[nodiscard]] auto WorkbenchStackDepth() const -> size_t { return _stack.size(); }

private:
    void AssertMainThread() const;

    void DeactivateWorkbench();
    void DeactivateWorkshop();

    void MaterializeTabs(const std::vector<TabBlueprint>& tabs,
                         ActionBarNode* bar,
                         std::vector<std::string>& outTabIds);

    void MaterializeAddins(const WorkshopId& wsId,
                           std::vector<std::string>& outTabIds);
    void MaterializeAddins(const WorkbenchId& wbId,
                           std::vector<std::string>& outTabIds);

    void RemoveTabsById(const std::vector<std::string>& tabIds, ActionBarNode* bar);
    void HideTabsById(const std::vector<std::string>& tabIds, ActionBarNode* bar);
    void ShowTabsById(const std::vector<std::string>& tabIds, ActionBarNode* bar);

    void RegisterShortcuts(const std::vector<CommandHeaderDescriptor>& cmds);
    void UnregisterShortcuts();

    void OnButtonClicked(ActionButtonNode* btn, const std::string& cmdId);

    void DispatchNotification(matcha::Notification& notif);

    Shell&             _shell;
    WorkshopRegistry&  _registry;

    WorkshopId         _activeWorkshopId;
    WorkbenchId        _activeWorkbenchId;

    struct StackEntry {
        WorkshopId   workshopId;
        WorkbenchId  workbenchId;
    };
    std::vector<StackEntry> _stack;

    std::vector<std::string> _workshopTabIds;
    std::vector<std::string> _workbenchTabIds;
    std::vector<std::string> _workshopAddinTabIds;
    std::vector<std::string> _workbenchAddinTabIds;
    std::vector<std::string> _hiddenBaseTabIds;

    std::unique_ptr<matcha::CommandNode>   _activeCommand;
    std::vector<QShortcut*>        _shortcuts;        ///< Qt parent-owned, cleared in UnregisterShortcuts
};

// --------------------------------------------------------------------------- //
// WorkbenchGuard — RAII for PushWorkbench / PopWorkbench
// --------------------------------------------------------------------------- //

/**
 * @brief RAII guard that calls PushWorkbench() on construction and PopWorkbench()
 *        on destruction. Atomic — no gap between push and guard ownership.
 *
 * Usage:
 * @code
 *   WorkbenchGuard guard(mgr, WorkbenchId::From("sketcher"));
 *   // ... sketcher operations ...
 *   // guard destructor -> PopWorkbench()
 * @endcode
 */
class MATCHA_EXPORT WorkbenchGuard {
public:
    WorkbenchGuard(WorkbenchManager& mgr, const WorkbenchId& id)
        : _mgr(&mgr) { _mgr->PushWorkbench(id); }
    ~WorkbenchGuard() { if (_mgr) { _mgr->PopWorkbench(); } }

    WorkbenchGuard(const WorkbenchGuard&)            = delete;
    auto operator=(const WorkbenchGuard&) -> WorkbenchGuard& = delete;

    WorkbenchGuard(WorkbenchGuard&& other) noexcept : _mgr(other._mgr) { other._mgr = nullptr; }
    auto operator=(WorkbenchGuard&& other) noexcept -> WorkbenchGuard& {
        if (this != &other) {
            if (_mgr) { _mgr->PopWorkbench(); }
            _mgr = other._mgr;
            other._mgr = nullptr;
        }
        return *this;
    }

private:
    WorkbenchManager* _mgr;
};

} // namespace matcha::fw
