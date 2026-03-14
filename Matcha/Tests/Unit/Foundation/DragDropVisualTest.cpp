#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file DragDropVisualTest.cpp
 * @brief Unit tests for DragDropVisualManager.
 */

#include "doctest.h"

#include <Matcha/Foundation/DragDropVisual.h>

using namespace matcha::fw;

namespace {

auto MakeZone(std::string id, std::vector<std::string> mimes = {}) -> DropZoneConfig
{
    return {
        .zoneId = std::move(id), .acceptedMimeTypes = std::move(mimes),
        .highlight = DropZoneHighlight::None, .insertion = InsertionPosition::None,
        .insertIndex = -1, .enabled = true,
    };
}

} // namespace

TEST_SUITE("DragDropVisualManager") {

// ============================================================================
// Drag preview
// ============================================================================

TEST_CASE("Default preview config") {
    DragDropVisualManager mgr;
    CHECK(mgr.PreviewConfig().style == DragPreviewStyle::Ghost);
    CHECK(mgr.PreviewConfig().opacity == doctest::Approx(0.7));
    CHECK_FALSE(mgr.IsDragActive());
}

TEST_CASE("SetPreviewConfig and SetDragActive") {
    DragDropVisualManager mgr;
    DragPreviewConfig cfg;
    cfg.style = DragPreviewStyle::Compact;
    cfg.opacity = 0.5;
    cfg.badgeCount = 3;
    cfg.label = "3 items";
    mgr.SetPreviewConfig(std::move(cfg));

    CHECK(mgr.PreviewConfig().style == DragPreviewStyle::Compact);
    CHECK(mgr.PreviewConfig().opacity == doctest::Approx(0.5));
    CHECK(mgr.PreviewConfig().badgeCount == 3);

    mgr.SetDragActive(true);
    CHECK(mgr.IsDragActive());
}

// ============================================================================
// Drop zones: registration
// ============================================================================

TEST_CASE("RegisterZone and FindZone") {
    DragDropVisualManager mgr;
    CHECK(mgr.RegisterZone(MakeZone("tree", {"application/x-matcha-tab"})));
    CHECK(mgr.ZoneCount() == 1);

    const auto* z = mgr.FindZone("tree");
    REQUIRE(z != nullptr);
    CHECK(z->zoneId == "tree");
    CHECK(z->acceptedMimeTypes.size() == 1);
}

TEST_CASE("RegisterZone rejects duplicate ID") {
    DragDropVisualManager mgr;
    CHECK(mgr.RegisterZone(MakeZone("z1")));
    CHECK_FALSE(mgr.RegisterZone(MakeZone("z1")));
    CHECK(mgr.ZoneCount() == 1);
}

TEST_CASE("UnregisterZone removes zone") {
    DragDropVisualManager mgr;
    mgr.RegisterZone(MakeZone("z1"));
    CHECK(mgr.UnregisterZone("z1"));
    CHECK(mgr.ZoneCount() == 0);
    CHECK(mgr.FindZone("z1") == nullptr);
}

TEST_CASE("UnregisterZone returns false for unknown") {
    DragDropVisualManager mgr;
    CHECK_FALSE(mgr.UnregisterZone("nonexistent"));
}

// ============================================================================
// Zone highlight and insertion
// ============================================================================

TEST_CASE("SetZoneHighlight updates state") {
    DragDropVisualManager mgr;
    mgr.RegisterZone(MakeZone("z1"));

    CHECK(mgr.SetZoneHighlight("z1", DropZoneHighlight::Hover));
    CHECK(mgr.FindZone("z1")->highlight == DropZoneHighlight::Hover);

    CHECK(mgr.SetZoneHighlight("z1", DropZoneHighlight::Accepted));
    CHECK(mgr.FindZone("z1")->highlight == DropZoneHighlight::Accepted);
}

TEST_CASE("SetZoneHighlight returns false for unknown zone") {
    DragDropVisualManager mgr;
    CHECK_FALSE(mgr.SetZoneHighlight("unknown", DropZoneHighlight::Hover));
}

TEST_CASE("SetZoneInsertion updates position and index") {
    DragDropVisualManager mgr;
    mgr.RegisterZone(MakeZone("z1"));

    CHECK(mgr.SetZoneInsertion("z1", InsertionPosition::Before, 5));
    CHECK(mgr.FindZone("z1")->insertion == InsertionPosition::Before);
    CHECK(mgr.FindZone("z1")->insertIndex == 5);
}

// ============================================================================
// ZoneAccepts
// ============================================================================

TEST_CASE("ZoneAccepts checks MIME types") {
    DragDropVisualManager mgr;
    mgr.RegisterZone(MakeZone("tree", {"text/plain", "application/x-matcha-tab"}));

    CHECK(mgr.ZoneAccepts("tree", "text/plain"));
    CHECK(mgr.ZoneAccepts("tree", "application/x-matcha-tab"));
    CHECK_FALSE(mgr.ZoneAccepts("tree", "image/png"));
}

TEST_CASE("ZoneAccepts returns false for disabled zone") {
    DragDropVisualManager mgr;
    auto zone = MakeZone("z1", {"text/plain"});
    zone.enabled = false;
    mgr.RegisterZone(std::move(zone));

    CHECK_FALSE(mgr.ZoneAccepts("z1", "text/plain"));
}

TEST_CASE("ZoneAccepts returns false for unknown zone") {
    DragDropVisualManager mgr;
    CHECK_FALSE(mgr.ZoneAccepts("unknown", "text/plain"));
}

// ============================================================================
// ResetAllZones
// ============================================================================

TEST_CASE("ResetAllZones clears highlights and insertion") {
    DragDropVisualManager mgr;
    mgr.RegisterZone(MakeZone("z1"));
    mgr.RegisterZone(MakeZone("z2"));
    mgr.SetZoneHighlight("z1", DropZoneHighlight::Accepted);
    mgr.SetZoneInsertion("z2", InsertionPosition::After, 3);

    mgr.ResetAllZones();
    CHECK(mgr.FindZone("z1")->highlight == DropZoneHighlight::None);
    CHECK(mgr.FindZone("z2")->insertion == InsertionPosition::None);
    CHECK(mgr.FindZone("z2")->insertIndex == -1);
}

TEST_CASE("SetDragActive(false) resets all zones") {
    DragDropVisualManager mgr;
    mgr.RegisterZone(MakeZone("z1"));
    mgr.SetDragActive(true);
    mgr.SetZoneHighlight("z1", DropZoneHighlight::Hover);

    mgr.SetDragActive(false);
    CHECK_FALSE(mgr.IsDragActive());
    CHECK(mgr.FindZone("z1")->highlight == DropZoneHighlight::None);
}

// ============================================================================
// Clear
// ============================================================================

TEST_CASE("Clear removes everything") {
    DragDropVisualManager mgr;
    mgr.RegisterZone(MakeZone("z1"));
    mgr.SetDragActive(true);
    mgr.Clear();

    CHECK(mgr.ZoneCount() == 0);
    CHECK_FALSE(mgr.IsDragActive());
}

} // TEST_SUITE
