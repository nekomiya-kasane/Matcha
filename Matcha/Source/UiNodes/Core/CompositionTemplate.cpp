#include "Matcha/Foundation/CompositionTemplate.h"

#include <utility>

namespace matcha::fw {

// ============================================================================
// Helper: leaf slot factory (reduces boilerplate)
// ============================================================================

namespace {

auto Leaf(const std::string& n, int dw, int dh,
          int minW, int maxW, int minH, int maxH,
          bool scroll, bool collapse, bool resize,
          DockEdge e, int f) -> SlotDescriptor
{
    return {.name = n, .defaultWidth = dw, .defaultHeight = dh,
            .minWidth = minW, .maxWidth = maxW, .minHeight = minH, .maxHeight = maxH,
            .scrollable = scroll, .collapsible = collapse, .resizable = resize,
            .edge = e, .flex = f,
            .splitDir = SplitDirection::Vertical, .children = {}};
}

auto Group(const std::string& n, SplitDirection dir,
           std::vector<SlotDescriptor> kids, int f = 0) -> SlotDescriptor
{
    SlotDescriptor s;
    s.name = n;
    s.flex = f;
    s.splitDir = dir;
    s.children = std::move(kids);
    return s;
}

const SlotDescriptor* FindInTree(const SlotDescriptor& slot, const std::string& name)
{
    if (slot.name == name) { return &slot; }
    for (const auto& child : slot.children) {
        if (const auto* found = FindInTree(child, name)) { return found; }
    }
    return nullptr;
}

void CollectLeaves(const SlotDescriptor& slot, std::vector<const SlotDescriptor*>& out)
{
    if (slot.children.empty()) {
        if (!slot.name.empty()) { out.push_back(&slot); }
    } else {
        for (const auto& child : slot.children) { CollectLeaves(child, out); }
    }
}

} // anonymous namespace

// ============================================================================
// TemplateBuilder
// ============================================================================

TemplateBuilder::TemplateBuilder(TemplateKind kind, const std::string& name)
{
    _desc.kind = kind;
    _desc.name = name;
    _desc.root.name = "root";
    _desc.root.splitDir = SplitDirection::Vertical;
}

auto TemplateBuilder::DefaultSize(int w, int h) -> TemplateBuilder&
{
    _desc.defaultWidth = w;
    _desc.defaultHeight = h;
    return *this;
}

auto TemplateBuilder::MinSize(int w, int h) -> TemplateBuilder&
{
    _desc.minWidth = w;
    _desc.minHeight = h;
    return *this;
}

auto TemplateBuilder::RootVertical() -> TemplateBuilder&
{
    _desc.root.splitDir = SplitDirection::Vertical;
    return *this;
}

auto TemplateBuilder::RootHorizontal() -> TemplateBuilder&
{
    _desc.root.splitDir = SplitDirection::Horizontal;
    return *this;
}

auto TemplateBuilder::AddSlot(const SlotDescriptor& slot) -> TemplateBuilder&
{
    _desc.root.children.push_back(slot);
    return *this;
}

auto TemplateBuilder::AddGroup(const std::string& name, SplitDirection dir,
                               std::vector<SlotDescriptor> children, int flex) -> TemplateBuilder&
{
    _desc.root.children.push_back(Group(name, dir, std::move(children), flex));
    return *this;
}

auto TemplateBuilder::Build() const -> TemplateDescriptor { return _desc; }

// ============================================================================
// Factory methods — tree-structured
// ============================================================================

auto CompositionTemplate::MasterDetail(int masterWidth,
                                       DockEdge masterSide) -> TemplateDescriptor
{
    return TemplateBuilder(TemplateKind::MasterDetail, "MasterDetail")
        .MinSize(400, 300)
        .RootHorizontal()
        .AddSlot(Leaf("master", masterWidth, 0, 150, 400, 0, 0,
                       true, true, true, masterSide, 0))
        .AddSlot(Leaf("detail", 0, 0, 200, 0, 0, 0,
                       true, false, false, DockEdge::Right, 1))
        .Build();
}

auto CompositionTemplate::HeaderDetailFooter(int headerHeight,
                                             int footerHeight) -> TemplateDescriptor
{
    return TemplateBuilder(TemplateKind::HeaderDetailFooter, "HeaderDetailFooter")
        .MinSize(300, 200)
        .RootVertical()
        .AddSlot(Leaf("header", 0, headerHeight, 0, 0, headerHeight, headerHeight,
                       false, false, false, DockEdge::Top, 0))
        .AddSlot(Leaf("detail", 0, 0, 0, 0, 100, 0,
                       true, false, false, DockEdge::Top, 1))
        .AddSlot(Leaf("footer", 0, footerHeight, 0, 0, footerHeight, footerHeight,
                       false, false, false, DockEdge::Bottom, 0))
        .Build();
}

auto CompositionTemplate::ShellLayout() -> TemplateDescriptor
{
    return TemplateBuilder(TemplateKind::ShellLayout, "ShellLayout")
        .DefaultSize(1280, 800)
        .MinSize(800, 600)
        .RootVertical()
        .AddSlot(Leaf("titleBar", 0, 64, 0, 0, 64, 64,
                       false, false, false, DockEdge::Top, 0))
        .AddSlot(Leaf("menuBar", 0, 28, 0, 0, 28, 28,
                       false, true, false, DockEdge::Top, 0))
        .AddSlot(Leaf("actionBar", 0, 36, 0, 0, 0, 0,
                       false, true, false, DockEdge::Top, 0))
        .AddGroup("centerArea", SplitDirection::Horizontal, {
            Leaf("viewport", 0, 0, 300, 0, 300, 0,
                 false, false, false, DockEdge::Left, 1),
            Leaf("propertyPanel", 280, 0, 200, 400, 0, 0,
                 true, true, true, DockEdge::Right, 0),
        }, 1)
        .AddSlot(Leaf("statusBar", 0, 24, 0, 0, 24, 24,
                       false, false, false, DockEdge::Bottom, 0))
        .Build();
}

auto CompositionTemplate::DialogConfirm() -> TemplateDescriptor
{
    return TemplateBuilder(TemplateKind::DialogConfirm, "DialogConfirm")
        .DefaultSize(400, 0)
        .MinSize(300, 150)
        .RootVertical()
        .AddGroup("body", SplitDirection::Horizontal, {
            Leaf("icon", 48, 48, 48, 48, 48, 48,
                 false, false, false, DockEdge::Left, 0),
            Leaf("message", 0, 0, 0, 0, 0, 0,
                 false, false, false, DockEdge::Top, 1),
        })
        .AddSlot(Leaf("buttons", 0, 40, 0, 0, 40, 40,
                       false, false, false, DockEdge::Bottom, 0))
        .Build();
}

auto CompositionTemplate::DialogInput() -> TemplateDescriptor
{
    return TemplateBuilder(TemplateKind::DialogInput, "DialogInput")
        .DefaultSize(360, 0)
        .MinSize(280, 120)
        .RootVertical()
        .AddSlot(Leaf("label", 0, 0, 0, 0, 0, 0,
                       false, false, false, DockEdge::Top, 0))
        .AddSlot(Leaf("input", 0, 32, 0, 0, 32, 32,
                       false, false, false, DockEdge::Top, 0))
        .AddSlot(Leaf("buttons", 0, 40, 0, 0, 40, 40,
                       false, false, false, DockEdge::Bottom, 0))
        .Build();
}

auto CompositionTemplate::DialogWizard() -> TemplateDescriptor
{
    return TemplateBuilder(TemplateKind::DialogWizard, "DialogWizard")
        .DefaultSize(600, 400)
        .MinSize(500, 400)
        .RootVertical()
        .AddSlot(Leaf("stepIndicator", 0, 32, 0, 0, 32, 32,
                       false, false, false, DockEdge::Top, 0))
        .AddSlot(Leaf("content", 0, 0, 0, 0, 200, 0,
                       true, false, false, DockEdge::Top, 1))
        .AddSlot(Leaf("buttons", 0, 40, 0, 0, 40, 40,
                       false, false, false, DockEdge::Bottom, 0))
        .Build();
}

auto CompositionTemplate::DialogSettings() -> TemplateDescriptor
{
    return TemplateBuilder(TemplateKind::DialogSettings, "DialogSettings")
        .DefaultSize(720, 500)
        .MinSize(600, 400)
        .RootVertical()
        .AddGroup("body", SplitDirection::Horizontal, {
            Leaf("categoryList", 180, 0, 140, 240, 0, 0,
                 true, false, true, DockEdge::Left, 0),
            Leaf("content", 0, 0, 300, 0, 0, 0,
                 true, false, false, DockEdge::Right, 1),
        }, 1)
        .AddSlot(Leaf("buttons", 0, 40, 0, 0, 40, 40,
                       false, false, false, DockEdge::Bottom, 0))
        .Build();
}

auto CompositionTemplate::Create(TemplateKind kind) -> TemplateDescriptor
{
    switch (kind) {
    case TemplateKind::MasterDetail:       return MasterDetail();
    case TemplateKind::HeaderDetailFooter: return HeaderDetailFooter();
    case TemplateKind::ShellLayout:        return ShellLayout();
    case TemplateKind::DialogConfirm:      return DialogConfirm();
    case TemplateKind::DialogInput:        return DialogInput();
    case TemplateKind::DialogWizard:       return DialogWizard();
    case TemplateKind::DialogSettings:     return DialogSettings();
    }
    return MasterDetail();
}

auto CompositionTemplate::FindSlot(const TemplateDescriptor& desc,
                                   const std::string& name) -> const SlotDescriptor*
{
    return FindInTree(desc.root, name);
}

auto CompositionTemplate::FlattenSlots(const TemplateDescriptor& desc) -> std::vector<const SlotDescriptor*>
{
    std::vector<const SlotDescriptor*> result;
    CollectLeaves(desc.root, result);
    return result;
}

// ============================================================================
// ShellLayoutManager
// ============================================================================

ShellLayoutManager::ShellLayoutManager(TemplateDescriptor desc)
    : _desc(std::move(desc))
{
    InitVisibility(_desc.root);
}

void ShellLayoutManager::InitVisibility(const SlotDescriptor& slot)
{
    if (slot.children.empty()) {
        _visibility.push_back({.name = slot.name, .visible = true});
    } else {
        for (const auto& child : slot.children) { InitVisibility(child); }
    }
}

auto ShellLayoutManager::GetSlot(const std::string& name) const -> const SlotDescriptor*
{
    return FindInTree(_desc.root, name);
}

auto ShellLayoutManager::IsSlotVisible(const std::string& name) const -> bool
{
    for (const auto& v : _visibility) {
        if (v.name == name) { return v.visible; }
    }
    return false;
}

void ShellLayoutManager::SetSlotVisible(const std::string& name, bool visible)
{
    for (auto& v : _visibility) {
        if (v.name == name) { v.visible = visible; return; }
    }
}

} // namespace matcha::fw
