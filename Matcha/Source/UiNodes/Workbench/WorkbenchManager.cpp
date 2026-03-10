#include "Matcha/UiNodes/Workbench/WorkbenchManager.h"

#include "Matcha/UiNodes/ActionBar/ActionBarNode.h"
#include "Matcha/UiNodes/ActionBar/ActionButtonNode.h"
#include "Matcha/UiNodes/ActionBar/ActionTabNode.h"
#include "Matcha/UiNodes/ActionBar/ActionToolbarNode.h"
#include "Matcha/UiNodes/Core/CommandNode.h"
#include "Matcha/UiNodes/Shell/Shell.h"
#include "Matcha/UiNodes/Shell/WindowNode.h"
#include "Matcha/UiNodes/Workbench/WorkbenchTypes.h"
#include "Matcha/UiNodes/Workbench/WorkshopRegistry.h"
#include "Matcha/Widgets/Core/UpdateGuard.h"

#include <QApplication>
#include <QShortcut>
#include <QThread>
#include <QWidget>

#include <cassert>

namespace matcha::fw {

// --------------------------------------------------------------------------- //
// Construction
// --------------------------------------------------------------------------- //

WorkbenchManager::WorkbenchManager(Shell& shell, WorkshopRegistry& registry)
    : _shell(shell)
    , _registry(registry)
{}

WorkbenchManager::~WorkbenchManager()
{
    UnregisterShortcuts();
    _activeCommand.reset();
}

// --------------------------------------------------------------------------- //
// Thread safety
// --------------------------------------------------------------------------- //

void WorkbenchManager::AssertMainThread() const
{
    assert(QThread::currentThread() == QApplication::instance()->thread()
           && "WorkbenchManager methods must be called on the main thread");
}

// --------------------------------------------------------------------------- //
// Notification dispatch — route through Shell for tree propagation
// --------------------------------------------------------------------------- //

void WorkbenchManager::DispatchNotification(matcha::Notification& notif)
{
    _shell.SendNotification(&_shell, notif);
}

// --------------------------------------------------------------------------- //
// ActivateWorkshop
// --------------------------------------------------------------------------- //

auto WorkbenchManager::ActivateWorkshop(const WorkshopId& id) -> bool {
    AssertMainThread();
    if (_activeWorkshopId == id) { return true; }

    auto* ws = _registry.FindWorkshop(id);
    if (ws == nullptr) { return false; }

    auto guard = _shell.FreezeUpdates();

    // Deactivate current state
    if (_activeWorkbenchId.IsValid()) {
        DeactivateWorkbench();
    }
    if (_activeWorkshopId.IsValid()) {
        DeactivateWorkshop();
    }

    // Activate new workshop
    _activeWorkshopId = id;

    auto actionBar = _shell.GetActionBar();
    if (actionBar.get() != nullptr) {
        MaterializeTabs(ws->baseTabs, actionBar.get(), _workshopTabIds);
        MaterializeAddins(id, _workshopAddinTabIds);
    }

    // Lifecycle hook
    if (ws->lifecycle) {
        ws->lifecycle->OnActivate(_shell);
    }

    // Notification
    WorkshopActivated notif(id.value);
    DispatchNotification(notif);

    // Activate default workbench
    if (ws->defaultWorkbenchId.IsValid()) {
        ActivateWorkbench(ws->defaultWorkbenchId);
    }

    return true;
}

// --------------------------------------------------------------------------- //
// ActivateWorkbench
// --------------------------------------------------------------------------- //

auto WorkbenchManager::ActivateWorkbench(const WorkbenchId& id) -> bool {
    AssertMainThread();
    if (_activeWorkbenchId == id) { return true; }

    auto* wb = _registry.FindWorkbench(id);
    if (wb == nullptr) { return false; }

    auto guard = _shell.FreezeUpdates();

    // Deactivate current workbench
    if (_activeWorkbenchId.IsValid()) {
        DeactivateWorkbench();
    }

    _activeWorkbenchId = id;

    auto actionBar = _shell.GetActionBar();
    if (actionBar.get() != nullptr) {
        // Hide workshop base tabs (visibility toggle, not destroy)
        HideTabsById(wb->hiddenBaseTabIds, actionBar.get());
        _hiddenBaseTabIds = wb->hiddenBaseTabIds;

        // Materialize workbench task tabs
        MaterializeTabs(wb->taskTabs, actionBar.get(), _workbenchTabIds);
        MaterializeAddins(id, _workbenchAddinTabIds);
    }

    // Register keyboard shortcuts for workbench commands
    RegisterShortcuts(wb->commands);

    // Lifecycle hook
    if (wb->lifecycle) {
        wb->lifecycle->OnActivate(_shell);
    }

    // Notification
    WorkbenchActivated notif(id.value, _activeWorkshopId.value);
    DispatchNotification(notif);

    return true;
}

// --------------------------------------------------------------------------- //
// PushWorkbench / PopWorkbench
// --------------------------------------------------------------------------- //

void WorkbenchManager::PushWorkbench(const WorkbenchId& id) {
    AssertMainThread();
    _stack.push_back({_activeWorkshopId, _activeWorkbenchId});

    auto* wb = _registry.FindWorkbench(id);
    if (wb != nullptr && wb->workshopId != _activeWorkshopId) {
        ActivateWorkshop(wb->workshopId);
    } else {
        ActivateWorkbench(id);
    }
}

void WorkbenchManager::PopWorkbench() {
    AssertMainThread();
    assert(!_stack.empty());
    auto [wsId, wbId] = _stack.back();
    _stack.pop_back();

    if (wsId != _activeWorkshopId) {
        ActivateWorkshop(wsId);
        ActivateWorkbench(wbId);
    } else {
        ActivateWorkbench(wbId);
    }
}

// --------------------------------------------------------------------------- //
// Deactivation
// --------------------------------------------------------------------------- //

void WorkbenchManager::DeactivateWorkbench() {
    if (!_activeWorkbenchId.IsValid()) { return; }

    auto deactivatingId = _activeWorkbenchId;

    // Destroy active command (if any)
    _activeCommand.reset();

    // Unregister keyboard shortcuts
    UnregisterShortcuts();

    // Lifecycle hook (before teardown)
    auto* wb = _registry.FindWorkbench(deactivatingId);
    if (wb != nullptr && wb->lifecycle) {
        wb->lifecycle->OnDeactivate(_shell);
    }

    auto actionBar = _shell.GetActionBar();
    if (actionBar.get() != nullptr) {
        // Remove workbench addin tabs
        RemoveTabsById(_workbenchAddinTabIds, actionBar.get());
        _workbenchAddinTabIds.clear();

        // Remove workbench task tabs
        RemoveTabsById(_workbenchTabIds, actionBar.get());
        _workbenchTabIds.clear();

        // Restore hidden base tabs (show, not re-create)
        if (!_hiddenBaseTabIds.empty()) {
            ShowTabsById(_hiddenBaseTabIds, actionBar.get());
        }
        _hiddenBaseTabIds.clear();
    }

    _activeWorkbenchId = {};

    // Notification
    WorkbenchDeactivated notif(deactivatingId.value);
    DispatchNotification(notif);
}

void WorkbenchManager::DeactivateWorkshop() {
    if (!_activeWorkshopId.IsValid()) { return; }

    auto deactivatingId = _activeWorkshopId;

    // Lifecycle hook (before teardown)
    auto* ws = _registry.FindWorkshop(deactivatingId);
    if (ws != nullptr && ws->lifecycle) {
        ws->lifecycle->OnDeactivate(_shell);
    }

    auto actionBar = _shell.GetActionBar();
    if (actionBar.get() != nullptr) {
        // Remove workshop addin tabs
        RemoveTabsById(_workshopAddinTabIds, actionBar.get());
        _workshopAddinTabIds.clear();

        // Remove workshop base tabs
        RemoveTabsById(_workshopTabIds, actionBar.get());
        _workshopTabIds.clear();
    }

    _activeWorkshopId = {};

    // Notification
    WorkshopDeactivated notif(deactivatingId.value);
    DispatchNotification(notif);
}

// --------------------------------------------------------------------------- //
// MaterializeTabs — convert blueprints to live UiNode subtree
// --------------------------------------------------------------------------- //

void WorkbenchManager::MaterializeTabs(const std::vector<TabBlueprint>& tabs,
                                        ActionBarNode* bar,
                                        std::vector<std::string>& outTabIds)
{
    for (const auto& tabBp : tabs) {
        auto* tabNode = bar->AddTab(tabBp.tabId, tabBp.label);
        if (tabNode == nullptr) { continue; }

        outTabIds.push_back(tabBp.tabId);

        for (const auto& toolbarBp : tabBp.toolbars) {
            auto* toolbarNode = tabNode->AddToolbar(toolbarBp.toolbarId, toolbarBp.label);
            if (toolbarNode == nullptr) { continue; }

            for (const auto& cmdId : toolbarBp.commands) {
                const auto* header = _registry.ResolveCommand(
                    cmdId, _activeWorkshopId, _activeWorkbenchId);
                if (header == nullptr) { continue; }

                auto* btn = toolbarNode->AddButton(
                    header->id.value, header->iconId, header->tooltip);
                if (btn == nullptr) { continue; }

                btn->SetText(header->label);
                btn->SetIcon(header->iconId, header->iconSize);
                btn->SetToolTip(header->tooltip);

                // Wire ButtonClicked -> command dispatch
                auto cmdIdCopy = header->id.value;
                btn->Subscribe(btn, "ButtonClicked",
                    [this, btn, cmdIdCopy](EventNode& /*sender*/, matcha::Notification& /*notif*/) {
                        OnButtonClicked(static_cast<ActionButtonNode*>(btn), cmdIdCopy);
                    });
            }
        }
    }
}

// --------------------------------------------------------------------------- //
// MaterializeAddins
// --------------------------------------------------------------------------- //

void WorkbenchManager::MaterializeAddins(const WorkshopId& wsId,
                                          std::vector<std::string>& outTabIds)
{
    auto actionBar = _shell.GetActionBar();
    if (actionBar.get() == nullptr) { return; }

    for (const auto* addin : _registry.FindAddins(wsId)) {
        MaterializeTabs(addin->addinTabs, actionBar.get(), outTabIds);
    }
}

void WorkbenchManager::MaterializeAddins(const WorkbenchId& wbId,
                                          std::vector<std::string>& outTabIds)
{
    auto actionBar = _shell.GetActionBar();
    if (actionBar.get() == nullptr) { return; }

    for (const auto* addin : _registry.FindAddins(wbId)) {
        MaterializeTabs(addin->addinTabs, actionBar.get(), outTabIds);
    }
}

// --------------------------------------------------------------------------- //
// Tab management
// --------------------------------------------------------------------------- //

void WorkbenchManager::RemoveTabsById(const std::vector<std::string>& tabIds,
                                       ActionBarNode* bar)
{
    for (const auto& tabId : tabIds) {
        bar->RemoveTab(tabId);
    }
}

void WorkbenchManager::HideTabsById(const std::vector<std::string>& tabIds,
                                     ActionBarNode* bar)
{
    for (const auto& tabId : tabIds) {
        auto* tab = bar->FindTab(tabId);
        if (tab != nullptr && tab->Widget() != nullptr) {
            tab->Widget()->setVisible(false);
        }
    }
}

void WorkbenchManager::ShowTabsById(const std::vector<std::string>& tabIds,
                                     ActionBarNode* bar)
{
    for (const auto& tabId : tabIds) {
        auto* tab = bar->FindTab(tabId);
        if (tab != nullptr && tab->Widget() != nullptr) {
            tab->Widget()->setVisible(true);
        }
    }
}

// --------------------------------------------------------------------------- //
// Shortcut registration
// --------------------------------------------------------------------------- //

void WorkbenchManager::RegisterShortcuts(
    const std::vector<CommandHeaderDescriptor>& cmds)
{
    UnregisterShortcuts();

    auto* mainWin = _shell.MainWindow();
    if (mainWin == nullptr) { return; }
    auto* parentWidget = mainWin->Widget();
    if (parentWidget == nullptr) { return; }

    for (const auto& cmd : cmds) {
        if (cmd.shortcut.empty()) { continue; }
        if (!cmd.factory) { continue; }

        auto* sc = new QShortcut(
            QKeySequence(QString::fromStdString(cmd.shortcut)),
            parentWidget);
        sc->setContext(Qt::ApplicationShortcut);

        auto cmdId = cmd.id.value;
        QObject::connect(sc, &QShortcut::activated, [this, cmdId]() {
            OnButtonClicked(nullptr, cmdId);
        });

        _shortcuts.push_back(sc);
    }
}

void WorkbenchManager::UnregisterShortcuts()
{
    for (auto* sc : _shortcuts) {
        delete sc;
    }
    _shortcuts.clear();
}

// --------------------------------------------------------------------------- //
// Command dispatch — the factory invocation closed loop
// --------------------------------------------------------------------------- //

void WorkbenchManager::OnButtonClicked(ActionButtonNode* /*btn*/,
                                        const std::string& cmdId)
{
    const auto* header = _registry.ResolveCommand(
        CmdHeaderId::From(cmdId), _activeWorkshopId, _activeWorkbenchId);
    if (header == nullptr) { return; }

    // If there's a factory, invoke it to create the command
    if (header->factory) {
        // Destroy previous active command
        _activeCommand.reset();

        auto cmd = (*header->factory)();
        if (cmd) {
            _activeCommand = std::move(cmd);
        }
    }

    // Dispatch CommandInvoked notification regardless of factory presence
    CommandInvoked notif(cmdId);
    DispatchNotification(notif);
}

} // namespace matcha::fw
