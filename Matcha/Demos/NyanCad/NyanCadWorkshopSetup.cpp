/**
 * @file NyanCadWorkshopSetup.cpp
 * @brief Registers NyanCad Workshop/Workbench descriptors.
 */

#include "NyanCadWorkshopSetup.h"

#include "Matcha/UiNodes/Workbench/WorkbenchTypes.h"
#include "Matcha/UiNodes/Workbench/WorkshopRegistry.h"

namespace nyancad {

using matcha::fw::CmdHeaderId;
using matcha::fw::CommandHeaderDescriptor;
using matcha::fw::TabBlueprint;
using matcha::fw::ToolbarBlueprint;
using matcha::fw::WorkbenchDescriptor;
using matcha::fw::WorkbenchId;
using matcha::fw::WorkshopDescriptor;
using matcha::fw::WorkshopId;
using matcha::fw::WorkshopRegistry;

namespace {

// --------------------------------------------------------------------------- //
// Helper: build a CommandHeaderDescriptor from minimal fields
// --------------------------------------------------------------------------- //

auto MakeCmd(std::string_view id, std::string_view label,
             std::string_view icon = {},
             std::string_view tooltip = {}) -> CommandHeaderDescriptor
{
    CommandHeaderDescriptor desc;
    desc.id      = CmdHeaderId::From(id);
    desc.label   = std::string(label);
    desc.iconId  = std::string(icon);
    desc.tooltip = std::string(tooltip);
    return desc;
}

// --------------------------------------------------------------------------- //
// Helper: build a TabBlueprint with one toolbar containing given commands
// --------------------------------------------------------------------------- //

auto MakeTab(std::string tabId, std::string label,
                    std::string toolbarId, std::string toolbarLabel,
                    std::vector<CmdHeaderId> cmds) -> TabBlueprint
{
    TabBlueprint tab;
    tab.tabId = std::move(tabId);
    tab.label = std::move(label);

    ToolbarBlueprint tb;
    tb.toolbarId = std::move(toolbarId);
    tb.label     = std::move(toolbarLabel);
    tb.commands  = std::move(cmds);
    tab.toolbars.push_back(std::move(tb));

    return tab;
}

} // anonymous namespace

// --------------------------------------------------------------------------- //
// Registration
// --------------------------------------------------------------------------- //

void RegisterNyanCadWorkshops(WorkshopRegistry& registry)
{
    // =======================================================================
    // Workshop: "mesh" — the Mesh document-type workshop
    // =======================================================================

    WorkshopDescriptor meshWs;
    meshWs.id    = WorkshopId::From("mesh");
    meshWs.label = "Mesh";

    // -- Workshop-level commands (available in all workbenches) ------------ //
    meshWs.commands.push_back(MakeCmd("cmd.new",    "New",    "new-file",  "New Document"));
    meshWs.commands.push_back(MakeCmd("cmd.open",   "Open",   "open",      "Open Document"));
    meshWs.commands.push_back(MakeCmd("cmd.save",   "Save",   "save",      "Save Document"));
    meshWs.commands.push_back(MakeCmd("cmd.undo",   "Undo",   "undo",      "Undo"));
    meshWs.commands.push_back(MakeCmd("cmd.redo",   "Redo",   "redo",      "Redo"));
    meshWs.commands.push_back(MakeCmd("cmd.cut",    "Cut",    "cut",       "Cut"));
    meshWs.commands.push_back(MakeCmd("cmd.copy",   "Copy",   "copy",      "Copy"));
    meshWs.commands.push_back(MakeCmd("cmd.paste",  "Paste",  "paste",     "Paste"));
    meshWs.commands.push_back(MakeCmd("cmd.generate", "Generate", "refresh", "Generate Mesh"));
    meshWs.commands.push_back(MakeCmd("cmd.refine",   "Refine",   "edit",    "Refine Mesh"));
    meshWs.commands.push_back(MakeCmd("cmd.verify",   "Verify",   "check",   "Verify Mesh"));

    // -- Base tabs (always visible when this workshop is active) ---------- //
    meshWs.baseTabs.push_back(MakeTab(
        "file", "File", "file_ops", "File Operations",
        {CmdHeaderId::From("cmd.new"),
         CmdHeaderId::From("cmd.open"),
         CmdHeaderId::From("cmd.save")}));

    meshWs.baseTabs.push_back(MakeTab(
        "edit", "Edit", "edit_ops", "Edit Operations",
        {CmdHeaderId::From("cmd.undo"),
         CmdHeaderId::From("cmd.redo"),
         CmdHeaderId::From("cmd.cut"),
         CmdHeaderId::From("cmd.copy"),
         CmdHeaderId::From("cmd.paste")}));

    meshWs.baseTabs.push_back(MakeTab(
        "mesh", "Mesh", "mesh_ops", "Mesh Operations",
        {CmdHeaderId::From("cmd.generate"),
         CmdHeaderId::From("cmd.refine"),
         CmdHeaderId::From("cmd.verify")}));

    // -- Workbench IDs belonging to this workshop ------------------------- //
    meshWs.workbenchIds = {
        WorkbenchId::From("surface_mesh"),
        WorkbenchId::From("tet_mesh"),
        WorkbenchId::From("sketch"),
    };
    meshWs.defaultWorkbenchId = WorkbenchId::From("surface_mesh");

    registry.RegisterWorkshop(std::move(meshWs));

    // =======================================================================
    // Workbench: "surface_mesh" — Surface Mesh task mode
    // =======================================================================

    {
        WorkbenchDescriptor wb;
        wb.id         = WorkbenchId::From("surface_mesh");
        wb.label      = "Surface Mesh";
        wb.workshopId = WorkshopId::From("mesh");

        wb.commands.push_back(MakeCmd("cmd.sm.triangulate", "Triangulate", "refresh", "Triangulate Surface"));
        wb.commands.push_back(MakeCmd("cmd.sm.optimize",    "Optimize",    "check",   "Optimize Surface Mesh"));
        wb.commands.push_back(MakeCmd("cmd.sm.export",      "Export STL",  "save",    "Export as STL"));

        wb.taskTabs.push_back(MakeTab(
            "surface_mesh", "Surface Mesh", "sm_ops", "Surface Mesh Operations",
            {CmdHeaderId::From("cmd.sm.triangulate"),
             CmdHeaderId::From("cmd.sm.optimize"),
             CmdHeaderId::From("cmd.sm.export")}));

        registry.RegisterWorkbench(std::move(wb));
    }

    // =======================================================================
    // Workbench: "tet_mesh" — Tetrahedral Mesh task mode
    // =======================================================================

    {
        WorkbenchDescriptor wb;
        wb.id         = WorkbenchId::From("tet_mesh");
        wb.label      = "Tet Mesh";
        wb.workshopId = WorkshopId::From("mesh");

        wb.commands.push_back(MakeCmd("cmd.tm.generate", "Generate Tets", "refresh", "Generate Tetrahedral Mesh"));
        wb.commands.push_back(MakeCmd("cmd.tm.quality",  "Quality Check", "check",   "Check Tet Quality"));
        wb.commands.push_back(MakeCmd("cmd.tm.coarsen",  "Coarsen",       "edit",    "Coarsen Tet Mesh"));

        wb.taskTabs.push_back(MakeTab(
            "tet_mesh", "Tet Mesh", "tm_ops", "Tet Mesh Operations",
            {CmdHeaderId::From("cmd.tm.generate"),
             CmdHeaderId::From("cmd.tm.quality"),
             CmdHeaderId::From("cmd.tm.coarsen")}));

        registry.RegisterWorkbench(std::move(wb));
    }

    // =======================================================================
    // Workbench: "sketch" — Sketch task mode
    // =======================================================================

    {
        WorkbenchDescriptor wb;
        wb.id         = WorkbenchId::From("sketch");
        wb.label      = "Sketch";
        wb.workshopId = WorkshopId::From("mesh");

        wb.commands.push_back(MakeCmd("cmd.sk.line",   "Line",   "edit",    "Draw Line"));
        wb.commands.push_back(MakeCmd("cmd.sk.arc",    "Arc",    "refresh", "Draw Arc"));
        wb.commands.push_back(MakeCmd("cmd.sk.circle", "Circle", "check",   "Draw Circle"));

        wb.taskTabs.push_back(MakeTab(
            "sketch", "Sketch", "sk_ops", "Sketch Operations",
            {CmdHeaderId::From("cmd.sk.line"),
             CmdHeaderId::From("cmd.sk.arc"),
             CmdHeaderId::From("cmd.sk.circle")}));

        registry.RegisterWorkbench(std::move(wb));
    }
}

} // namespace nyancad
