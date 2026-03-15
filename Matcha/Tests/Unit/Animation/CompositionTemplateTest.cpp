#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include "doctest.h"

#include <Matcha/Animation/CompositionTemplate.h>

using namespace matcha::fw;

TEST_SUITE("fw::CompositionTemplate") {

// ============================================================================
// MasterDetail
// ============================================================================

TEST_CASE("MasterDetail: default") {
    auto desc = CompositionTemplate::MasterDetail();
    CHECK(desc.kind == TemplateKind::MasterDetail);
    CHECK(desc.name == "MasterDetail");

    const auto* master = CompositionTemplate::FindSlot(desc, "master");
    REQUIRE(master != nullptr);
    CHECK(master->defaultWidth == 240);
    CHECK(master->edge == DockEdge::Left);
    CHECK(master->collapsible);
    CHECK(master->resizable);

    const auto* detail = CompositionTemplate::FindSlot(desc, "detail");
    REQUIRE(detail != nullptr);
    CHECK(detail->flex == 1);
}

TEST_CASE("MasterDetail: custom width and side") {
    auto desc = CompositionTemplate::MasterDetail(300, DockEdge::Right);
    const auto* master = CompositionTemplate::FindSlot(desc, "master");
    REQUIRE(master != nullptr);
    CHECK(master->defaultWidth == 300);
    CHECK(master->edge == DockEdge::Right);
}

// ============================================================================
// HeaderDetailFooter
// ============================================================================

TEST_CASE("HeaderDetailFooter: default") {
    auto desc = CompositionTemplate::HeaderDetailFooter();
    CHECK(desc.kind == TemplateKind::HeaderDetailFooter);

    const auto* header = CompositionTemplate::FindSlot(desc, "header");
    REQUIRE(header != nullptr);
    CHECK(header->defaultHeight == 48);

    const auto* detail = CompositionTemplate::FindSlot(desc, "detail");
    REQUIRE(detail != nullptr);
    CHECK(detail->scrollable);
    CHECK(detail->flex == 1);

    const auto* footer = CompositionTemplate::FindSlot(desc, "footer");
    REQUIRE(footer != nullptr);
    CHECK(footer->defaultHeight == 40);
}

TEST_CASE("HeaderDetailFooter: custom heights") {
    auto desc = CompositionTemplate::HeaderDetailFooter(60, 32);
    const auto* header = CompositionTemplate::FindSlot(desc, "header");
    REQUIRE(header != nullptr);
    CHECK(header->defaultHeight == 60);

    const auto* footer = CompositionTemplate::FindSlot(desc, "footer");
    REQUIRE(footer != nullptr);
    CHECK(footer->defaultHeight == 32);
}

// ============================================================================
// ShellLayout — §6.2.1 (now with menuBar and tree structure)
// ============================================================================

TEST_CASE("ShellLayout: slots match §6.2.1") {
    auto desc = CompositionTemplate::ShellLayout();
    CHECK(desc.kind == TemplateKind::ShellLayout);
    CHECK(desc.minWidth == 800);
    CHECK(desc.minHeight == 600);

    const auto* titleBar = CompositionTemplate::FindSlot(desc, "titleBar");
    REQUIRE(titleBar != nullptr);
    CHECK(titleBar->defaultHeight == 64);

    const auto* menuBar = CompositionTemplate::FindSlot(desc, "menuBar");
    REQUIRE(menuBar != nullptr);
    CHECK(menuBar->defaultHeight == 28);
    CHECK(menuBar->collapsible);

    const auto* actionBar = CompositionTemplate::FindSlot(desc, "actionBar");
    REQUIRE(actionBar != nullptr);
    CHECK(actionBar->collapsible);

    const auto* vp = CompositionTemplate::FindSlot(desc, "viewport");
    REQUIRE(vp != nullptr);
    CHECK(vp->flex == 1);
    CHECK(vp->minWidth == 300);

    const auto* prop = CompositionTemplate::FindSlot(desc, "propertyPanel");
    REQUIRE(prop != nullptr);
    CHECK(prop->defaultWidth == 280);
    CHECK(prop->minWidth == 200);
    CHECK(prop->maxWidth == 400);
    CHECK(prop->edge == DockEdge::Right);
    CHECK(prop->collapsible);
    CHECK(prop->resizable);

    const auto* statusBar = CompositionTemplate::FindSlot(desc, "statusBar");
    REQUIRE(statusBar != nullptr);
    CHECK(statusBar->defaultHeight == 24);
}

TEST_CASE("ShellLayout: tree structure — centerArea is container") {
    auto desc = CompositionTemplate::ShellLayout();
    const auto* center = CompositionTemplate::FindSlot(desc, "centerArea");
    REQUIRE(center != nullptr);
    CHECK(center->IsContainer());
    CHECK(center->splitDir == SplitDirection::Horizontal);
    CHECK(center->children.size() == 2);
}

// ============================================================================
// Dialog templates — §6.2.3
// ============================================================================

TEST_CASE("DialogConfirm: width 400, has icon+message+buttons") {
    auto desc = CompositionTemplate::DialogConfirm();
    CHECK(desc.kind == TemplateKind::DialogConfirm);
    CHECK(desc.defaultWidth == 400);

    auto leaves = CompositionTemplate::FlattenSlots(desc);
    REQUIRE(leaves.size() == 3);
    CHECK(leaves[0]->name == "icon");
    CHECK(leaves[1]->name == "message");
    CHECK(leaves[2]->name == "buttons");
}

TEST_CASE("DialogInput: width 360, has label+input+buttons") {
    auto desc = CompositionTemplate::DialogInput();
    CHECK(desc.kind == TemplateKind::DialogInput);
    CHECK(desc.defaultWidth == 360);

    auto leaves = CompositionTemplate::FlattenSlots(desc);
    REQUIRE(leaves.size() == 3);
    CHECK(leaves[0]->name == "label");
    CHECK(leaves[1]->name == "input");
    CHECK(leaves[2]->name == "buttons");
}

TEST_CASE("DialogWizard: 600x400, has stepIndicator+content+buttons") {
    auto desc = CompositionTemplate::DialogWizard();
    CHECK(desc.kind == TemplateKind::DialogWizard);
    CHECK(desc.defaultWidth == 600);
    CHECK(desc.minHeight == 400);

    const auto* content = CompositionTemplate::FindSlot(desc, "content");
    REQUIRE(content != nullptr);
    CHECK(content->scrollable);

    auto leaves = CompositionTemplate::FlattenSlots(desc);
    REQUIRE(leaves.size() == 3);
    CHECK(leaves[0]->name == "stepIndicator");
    CHECK(leaves[1]->name == "content");
    CHECK(leaves[2]->name == "buttons");
}

TEST_CASE("DialogSettings: 720x500, has categoryList+content+buttons") {
    auto desc = CompositionTemplate::DialogSettings();
    CHECK(desc.kind == TemplateKind::DialogSettings);
    CHECK(desc.defaultWidth == 720);
    CHECK(desc.defaultHeight == 500);

    const auto* catList = CompositionTemplate::FindSlot(desc, "categoryList");
    REQUIRE(catList != nullptr);
    CHECK(catList->defaultWidth == 180);

    const auto* content = CompositionTemplate::FindSlot(desc, "content");
    REQUIRE(content != nullptr);
    CHECK(content->flex == 1);
}

// ============================================================================
// Create (dispatch by kind)
// ============================================================================

TEST_CASE("Create dispatches to correct factory") {
    auto md = CompositionTemplate::Create(TemplateKind::MasterDetail);
    CHECK(md.kind == TemplateKind::MasterDetail);
    CHECK(md.name == "MasterDetail");

    auto hdf = CompositionTemplate::Create(TemplateKind::HeaderDetailFooter);
    CHECK(hdf.kind == TemplateKind::HeaderDetailFooter);

    auto shell = CompositionTemplate::Create(TemplateKind::ShellLayout);
    CHECK(shell.kind == TemplateKind::ShellLayout);
}

// ============================================================================
// FindSlot (tree search)
// ============================================================================

TEST_CASE("FindSlot returns nullptr for unknown name") {
    auto desc = CompositionTemplate::MasterDetail();
    CHECK(CompositionTemplate::FindSlot(desc, "nonexistent") == nullptr);
}

TEST_CASE("FindSlot finds nested slot in tree") {
    auto desc = CompositionTemplate::ShellLayout();
    const auto* vp = CompositionTemplate::FindSlot(desc, "viewport");
    REQUIRE(vp != nullptr);
    CHECK(vp->minWidth == 300);
}

// ============================================================================
// FlattenSlots
// ============================================================================

TEST_CASE("FlattenSlots returns all leaves in DFS order") {
    auto desc = CompositionTemplate::ShellLayout();
    auto leaves = CompositionTemplate::FlattenSlots(desc);
    CHECK(leaves.size() == 6); // titleBar, menuBar, actionBar, viewport, propertyPanel, statusBar
    CHECK(leaves[0]->name == "titleBar");
    CHECK(leaves[1]->name == "menuBar");
    CHECK(leaves[2]->name == "actionBar");
    CHECK(leaves[3]->name == "viewport");
    CHECK(leaves[4]->name == "propertyPanel");
    CHECK(leaves[5]->name == "statusBar");
}

// ============================================================================
// TemplateBuilder fluent API
// ============================================================================

TEST_CASE("TemplateBuilder creates custom template") {
    auto desc = TemplateBuilder(TemplateKind::MasterDetail, "Custom")
        .DefaultSize(800, 600)
        .MinSize(400, 300)
        .RootHorizontal()
        .AddSlot({.name = "left", .flex = 1, .splitDir = SplitDirection::Vertical, .children = {}})
        .AddSlot({.name = "right", .flex = 2, .splitDir = SplitDirection::Vertical, .children = {}})
        .Build();

    CHECK(desc.name == "Custom");
    CHECK(desc.defaultWidth == 800);
    CHECK(desc.root.splitDir == SplitDirection::Horizontal);
    auto leaves = CompositionTemplate::FlattenSlots(desc);
    REQUIRE(leaves.size() == 2);
    CHECK(leaves[0]->name == "left");
    CHECK(leaves[1]->name == "right");
}

// ============================================================================
// ShellLayoutManager
// ============================================================================

TEST_CASE("ShellLayoutManager: default visibility") {
    ShellLayoutManager mgr(CompositionTemplate::ShellLayout());
    CHECK(mgr.IsSlotVisible("titleBar"));
    CHECK(mgr.IsSlotVisible("menuBar"));
    CHECK(mgr.IsSlotVisible("viewport"));
    CHECK(mgr.IsSlotVisible("propertyPanel"));
    CHECK(mgr.IsSlotVisible("statusBar"));
}

TEST_CASE("ShellLayoutManager: toggle visibility") {
    ShellLayoutManager mgr(CompositionTemplate::ShellLayout());
    mgr.SetSlotVisible("propertyPanel", false);
    CHECK_FALSE(mgr.IsSlotVisible("propertyPanel"));
    mgr.SetSlotVisible("propertyPanel", true);
    CHECK(mgr.IsSlotVisible("propertyPanel"));
}

TEST_CASE("ShellLayoutManager: GetSlot delegates to FindSlot") {
    ShellLayoutManager mgr(CompositionTemplate::ShellLayout());
    const auto* vp = mgr.GetSlot("viewport");
    REQUIRE(vp != nullptr);
    CHECK(vp->flex == 1);
    CHECK(mgr.GetSlot("nonexistent") == nullptr);
}

} // TEST_SUITE
