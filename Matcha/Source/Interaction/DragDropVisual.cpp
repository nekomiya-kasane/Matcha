/**
 * @file DragDropVisual.cpp
 * @brief Implementation of DragDropVisualManager.
 */

#include <Matcha/Interaction/DragDropVisual.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Drag preview
// ============================================================================

void DragDropVisualManager::SetPreviewConfig(DragPreviewConfig config)
{
    _preview = std::move(config);
}

void DragDropVisualManager::SetDragActive(bool active)
{
    _dragActive = active;
    if (!active) {
        ResetAllZones();
    }
}

// ============================================================================
// Drop zones
// ============================================================================

auto DragDropVisualManager::RegisterZone(DropZoneConfig zone) -> bool
{
    if (FindZoneMut(zone.zoneId) != nullptr) {
        return false;
    }
    _zones.push_back(std::move(zone));
    return true;
}

auto DragDropVisualManager::UnregisterZone(std::string_view zoneId) -> bool
{
    const auto it = std::ranges::find_if(_zones, [zoneId](const DropZoneConfig& z) {
        return z.zoneId == zoneId;
    });
    if (it == _zones.end()) {
        return false;
    }
    _zones.erase(it);
    return true;
}

auto DragDropVisualManager::SetZoneHighlight(std::string_view zoneId,
                                              DropZoneHighlight hl) -> bool
{
    auto* z = FindZoneMut(zoneId);
    if (z == nullptr) {
        return false;
    }
    z->highlight = hl;
    return true;
}

auto DragDropVisualManager::SetZoneInsertion(std::string_view zoneId,
                                              InsertionPosition pos, int index) -> bool
{
    auto* z = FindZoneMut(zoneId);
    if (z == nullptr) {
        return false;
    }
    z->insertion = pos;
    z->insertIndex = index;
    return true;
}

auto DragDropVisualManager::ZoneAccepts(std::string_view zoneId,
                                         std::string_view mimeType) const -> bool
{
    const auto* z = FindZone(zoneId);
    if (z == nullptr || !z->enabled) {
        return false;
    }
    return std::ranges::any_of(z->acceptedMimeTypes, [mimeType](const std::string& mt) {
        return mt == mimeType;
    });
}

auto DragDropVisualManager::FindZone(std::string_view zoneId) const -> const DropZoneConfig*
{
    for (const auto& z : _zones) {
        if (z.zoneId == zoneId) {
            return &z;
        }
    }
    return nullptr;
}

// ============================================================================
// Bulk operations
// ============================================================================

void DragDropVisualManager::ResetAllZones()
{
    for (auto& z : _zones) {
        z.highlight = DropZoneHighlight::None;
        z.insertion = InsertionPosition::None;
        z.insertIndex = -1;
    }
}

void DragDropVisualManager::Clear()
{
    _preview = {};
    _dragActive = false;
    _zones.clear();
}

// ============================================================================
// Private
// ============================================================================

auto DragDropVisualManager::FindZoneMut(std::string_view zoneId) -> DropZoneConfig*
{
    for (auto& z : _zones) {
        if (z.zoneId == zoneId) {
            return &z;
        }
    }
    return nullptr;
}

} // namespace matcha::fw
