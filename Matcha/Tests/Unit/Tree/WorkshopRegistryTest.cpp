#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Core/StringId.h>
#include <Matcha/Tree/Composition/Workbench/WorkbenchTypes.h>
#include <Matcha/Tree/Composition/Workbench/WorkshopRegistry.h>

using namespace matcha::fw;

// =========================================================================== //
// StringId<Tag> tests
// =========================================================================== //

TEST_SUITE("StringId") {

TEST_CASE("StringId::From constructs with correct value") {
    auto id = WorkshopId::From("mesh_workshop");
    CHECK(id.value == "mesh_workshop");
    CHECK(id.View() == "mesh_workshop");
    CHECK(id.IsValid());
}

TEST_CASE("StringId default is invalid") {
    WorkshopId id;
    CHECK_FALSE(id.IsValid());
    CHECK(id.value.empty());
}

TEST_CASE("StringId comparison operators") {
    auto a = WorkbenchId::From("alpha");
    auto b = WorkbenchId::From("beta");
    auto c = WorkbenchId::From("alpha");

    CHECK(a == c);
    CHECK(a != b);
    CHECK(a < b);
    CHECK(b > a);
}

TEST_CASE("StringId cross-type safety") {
    CHECK_FALSE((std::is_same_v<WorkshopId, WorkbenchId>));
    CHECK_FALSE((std::is_same_v<WorkshopId, CmdHeaderId>));
    CHECK_FALSE((std::is_same_v<WorkbenchId, CmdHeaderId>));
}

TEST_CASE("StringId::Hash works with unordered_map") {
    std::unordered_map<CmdHeaderId, int, CmdHeaderId::Hash> map;
    map[CmdHeaderId::From("cmd.pad")] = 42;
    CHECK(map[CmdHeaderId::From("cmd.pad")] == 42);
    CHECK(map.find(CmdHeaderId::From("cmd.missing")) == map.end());
}

} // TEST_SUITE("StringId")

// =========================================================================== //
// WorkshopRegistry tests
// =========================================================================== //

namespace {

CommandHeaderDescriptor MakeCmd(std::string_view id, std::string_view label) {
    CommandHeaderDescriptor desc;
    desc.id = CmdHeaderId::From(id);
    desc.label = std::string(label);
    return desc;
}

TabBlueprint MakeTab(std::string tabId, std::string label,
                     std::vector<CmdHeaderId> cmds)
{
    TabBlueprint bp;
    bp.tabId = std::move(tabId);
    bp.label = std::move(label);
    ToolbarBlueprint tb;
    tb.toolbarId = bp.tabId + "_toolbar";
    tb.label = bp.label + " Toolbar";
    tb.commands = std::move(cmds);
    bp.toolbars.push_back(std::move(tb));
    return bp;
}

} // namespace

TEST_SUITE("WorkshopRegistry") {

TEST_CASE("Register and find workshop") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part Workshop";
    ws.commands.push_back(MakeCmd("cmd.save", "Save"));

    reg.RegisterWorkshop(std::move(ws));

    CHECK(reg.WorkshopCount() == 1);

    auto* found = reg.FindWorkshop(WorkshopId::From("part"));
    REQUIRE(found != nullptr);
    CHECK(found->label == "Part Workshop");
    CHECK(found->commands.size() == 1);

    CHECK(reg.FindWorkshop(WorkshopId::From("missing")) == nullptr);
}

TEST_CASE("Register and find workbench") {
    WorkshopRegistry reg;

    WorkbenchDescriptor wb;
    wb.id = WorkbenchId::From("part_design");
    wb.label = "Part Design";
    wb.workshopId = WorkshopId::From("part");
    wb.commands.push_back(MakeCmd("cmd.pad", "Pad"));

    reg.RegisterWorkbench(std::move(wb));

    CHECK(reg.WorkbenchCount() == 1);

    auto* found = reg.FindWorkbench(WorkbenchId::From("part_design"));
    REQUIRE(found != nullptr);
    CHECK(found->label == "Part Design");
    CHECK(found->workshopId == WorkshopId::From("part"));

    CHECK(reg.FindWorkbench(WorkbenchId::From("missing")) == nullptr);
}

TEST_CASE("Register and find addins") {
    WorkshopRegistry reg;

    AddinDescriptor addin;
    addin.pluginId = "my_plugin";
    addin.target = WorkshopId::From("part");
    addin.commands.push_back(MakeCmd("cmd.custom", "Custom"));

    reg.RegisterAddin(std::move(addin));

    CHECK(reg.AddinCount() == 1);

    auto wsAddins = reg.FindAddins(WorkshopId::From("part"));
    CHECK(wsAddins.size() == 1);
    CHECK(wsAddins[0]->pluginId == "my_plugin");

    auto noAddins = reg.FindAddins(WorkshopId::From("other"));
    CHECK(noAddins.empty());

    auto wbAddins = reg.FindAddins(WorkbenchId::From("part_design"));
    CHECK(wbAddins.empty());
}

TEST_CASE("Addin targeting workbench") {
    WorkshopRegistry reg;

    AddinDescriptor addin;
    addin.pluginId = "wb_plugin";
    addin.target = WorkbenchId::From("sketcher");
    reg.RegisterAddin(std::move(addin));

    auto wbAddins = reg.FindAddins(WorkbenchId::From("sketcher"));
    CHECK(wbAddins.size() == 1);

    auto wsAddins = reg.FindAddins(WorkshopId::From("part"));
    CHECK(wsAddins.empty());
}

TEST_CASE("AllWorkshopIds enumeration") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws1;
    ws1.id = WorkshopId::From("part");
    ws1.label = "Part";
    reg.RegisterWorkshop(std::move(ws1));

    WorkshopDescriptor ws2;
    ws2.id = WorkshopId::From("assembly");
    ws2.label = "Assembly";
    reg.RegisterWorkshop(std::move(ws2));

    auto ids = reg.AllWorkshopIds();
    CHECK(ids.size() == 2);
}

TEST_CASE("WorkbenchIdsFor enumeration") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    ws.workbenchIds = {WorkbenchId::From("design"), WorkbenchId::From("sketcher")};
    reg.RegisterWorkshop(std::move(ws));

    auto ids = reg.WorkbenchIdsFor(WorkshopId::From("part"));
    CHECK(ids.size() == 2);

    auto empty = reg.WorkbenchIdsFor(WorkshopId::From("missing"));
    CHECK(empty.empty());
}

// -- Duplicate detection ------------------------------------------------- //

TEST_CASE("RegisterWorkshop rejects duplicate ID") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws1;
    ws1.id = WorkshopId::From("part");
    ws1.label = "Part v1";
    CHECK(reg.RegisterWorkshop(std::move(ws1)));

    WorkshopDescriptor ws2;
    ws2.id = WorkshopId::From("part");
    ws2.label = "Part v2";
    CHECK_FALSE(reg.RegisterWorkshop(std::move(ws2)));

    CHECK(reg.WorkshopCount() == 1);
    CHECK(reg.FindWorkshop(WorkshopId::From("part"))->label == "Part v1");
}

TEST_CASE("RegisterWorkbench rejects duplicate ID") {
    WorkshopRegistry reg;

    WorkbenchDescriptor wb1;
    wb1.id = WorkbenchId::From("design");
    wb1.label = "Design v1";
    wb1.workshopId = WorkshopId::From("part");
    CHECK(reg.RegisterWorkbench(std::move(wb1)));

    WorkbenchDescriptor wb2;
    wb2.id = WorkbenchId::From("design");
    wb2.label = "Design v2";
    wb2.workshopId = WorkshopId::From("part");
    CHECK_FALSE(reg.RegisterWorkbench(std::move(wb2)));

    CHECK(reg.WorkbenchCount() == 1);
    CHECK(reg.FindWorkbench(WorkbenchId::From("design"))->label == "Design v1");
}

// -- Unregistration ------------------------------------------------------ //

TEST_CASE("UnregisterWorkshop removes entry") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    reg.RegisterWorkshop(std::move(ws));

    CHECK(reg.WorkshopCount() == 1);
    CHECK(reg.UnregisterWorkshop(WorkshopId::From("part")));
    CHECK(reg.WorkshopCount() == 0);
    CHECK(reg.FindWorkshop(WorkshopId::From("part")) == nullptr);
}

TEST_CASE("UnregisterWorkshop returns false for missing ID") {
    WorkshopRegistry reg;
    CHECK_FALSE(reg.UnregisterWorkshop(WorkshopId::From("nonexistent")));
}

TEST_CASE("UnregisterWorkbench removes entry") {
    WorkshopRegistry reg;

    WorkbenchDescriptor wb;
    wb.id = WorkbenchId::From("design");
    wb.label = "Design";
    wb.workshopId = WorkshopId::From("part");
    reg.RegisterWorkbench(std::move(wb));

    CHECK(reg.WorkbenchCount() == 1);
    CHECK(reg.UnregisterWorkbench(WorkbenchId::From("design")));
    CHECK(reg.WorkbenchCount() == 0);
    CHECK(reg.FindWorkbench(WorkbenchId::From("design")) == nullptr);
}

TEST_CASE("UnregisterWorkbench returns false for missing ID") {
    WorkshopRegistry reg;
    CHECK_FALSE(reg.UnregisterWorkbench(WorkbenchId::From("nonexistent")));
}

// -- Command resolution -------------------------------------------------- //

TEST_CASE("ResolveCommand context-free") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    ws.commands.push_back(MakeCmd("cmd.save", "Save"));
    reg.RegisterWorkshop(std::move(ws));

    auto* found = reg.ResolveCommand(CmdHeaderId::From("cmd.save"));
    REQUIRE(found != nullptr);
    CHECK(found->label == "Save");

    CHECK(reg.ResolveCommand(CmdHeaderId::From("cmd.missing")) == nullptr);
}

TEST_CASE("ResolveCommand context-aware priority") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    ws.commands.push_back(MakeCmd("cmd.shared", "WS-shared"));
    reg.RegisterWorkshop(std::move(ws));

    WorkbenchDescriptor wb;
    wb.id = WorkbenchId::From("design");
    wb.label = "Design";
    wb.workshopId = WorkshopId::From("part");
    wb.commands.push_back(MakeCmd("cmd.shared", "WB-shared"));
    reg.RegisterWorkbench(std::move(wb));

    // Workbench commands take priority over workshop commands
    auto* found = reg.ResolveCommand(
        CmdHeaderId::From("cmd.shared"),
        WorkshopId::From("part"),
        WorkbenchId::From("design"));
    REQUIRE(found != nullptr);
    CHECK(found->label == "WB-shared");
}

// -- Validation ---------------------------------------------------------- //

TEST_CASE("Validate: valid configuration") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    ws.commands.push_back(MakeCmd("cmd.save", "Save"));
    ws.baseTabs.push_back(MakeTab("base_tab", "Base",
        {CmdHeaderId::From("cmd.save")}));
    ws.defaultWorkbenchId = WorkbenchId::From("design");
    ws.workbenchIds = {WorkbenchId::From("design")};
    reg.RegisterWorkshop(std::move(ws));

    WorkbenchDescriptor wb;
    wb.id = WorkbenchId::From("design");
    wb.label = "Design";
    wb.workshopId = WorkshopId::From("part");
    wb.commands.push_back(MakeCmd("cmd.pad", "Pad"));
    wb.taskTabs.push_back(MakeTab("task_tab", "Task",
        {CmdHeaderId::From("cmd.pad")}));
    reg.RegisterWorkbench(std::move(wb));

    auto result = reg.Validate();
    CHECK(result.valid);
    CHECK(result.errors.empty());
}

TEST_CASE("Validate: orphan workbench") {
    WorkshopRegistry reg;

    WorkbenchDescriptor wb;
    wb.id = WorkbenchId::From("orphan");
    wb.label = "Orphan";
    wb.workshopId = WorkshopId::From("nonexistent");
    reg.RegisterWorkbench(std::move(wb));

    auto result = reg.Validate();
    CHECK_FALSE(result.valid);
    REQUIRE(result.errors.size() >= 1);
    CHECK(result.errors[0].find("Orphan workbench") != std::string::npos);
}

TEST_CASE("Validate: missing default workbench") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    ws.defaultWorkbenchId = WorkbenchId::From("nonexistent");
    reg.RegisterWorkshop(std::move(ws));

    auto result = reg.Validate();
    CHECK_FALSE(result.valid);
    REQUIRE(result.errors.size() >= 1);
    CHECK(result.errors[0].find("defaultWorkbenchId") != std::string::npos);
}

TEST_CASE("Validate: dangling command ref in TabBlueprint") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    ws.baseTabs.push_back(MakeTab("tab", "Tab",
        {CmdHeaderId::From("cmd.nonexistent")}));
    reg.RegisterWorkshop(std::move(ws));

    auto result = reg.Validate();
    CHECK_FALSE(result.valid);
    REQUIRE(result.errors.size() >= 1);
    CHECK(result.errors[0].find("Dangling command ref") != std::string::npos);
}

TEST_CASE("Validate: cycle detection") {
    WorkshopRegistry reg;

    WorkshopDescriptor ws;
    ws.id = WorkshopId::From("part");
    ws.label = "Part";
    ws.defaultWorkbenchId = WorkbenchId::From("design");
    reg.RegisterWorkshop(std::move(ws));

    WorkbenchDescriptor wb;
    wb.id = WorkbenchId::From("design");
    wb.label = "Design";
    wb.workshopId = WorkshopId::From("assembly");  // belongs to different workshop
    reg.RegisterWorkbench(std::move(wb));

    WorkshopDescriptor ws2;
    ws2.id = WorkshopId::From("assembly");
    ws2.label = "Assembly";
    reg.RegisterWorkshop(std::move(ws2));

    auto result = reg.Validate();
    CHECK_FALSE(result.valid);
    REQUIRE(result.errors.size() >= 1);
    CHECK(result.errors[0].find("Cycle") != std::string::npos);
}

} // TEST_SUITE("WorkshopRegistry")

#ifdef __clang__
#pragma clang diagnostic pop
#endif
