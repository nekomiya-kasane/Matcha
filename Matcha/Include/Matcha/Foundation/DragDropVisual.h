#pragma once

/**
 * @file DragDropVisual.h
 * @brief Data model for drag preview and drop zone visual configuration.
 *
 * Defines the visual parameters for:
 * - DragPreview: ghost image, badge count, opacity, offset
 * - DropZone: highlight style, insertion indicator position
 * - DropFeedback: visual state of a drop target during drag
 *
 * This is a Foundation-layer component with zero Qt dependency.
 * The actual rendering is performed by Widget-layer code that reads these descriptors.
 *
 * @see Matcha_Design_System_Specification.md (Drag & Drop visuals)
 */

#include <Matcha/Foundation/Macros.h>

#include <cstdint>
#include <string>
#include <vector>

namespace matcha::fw {

// ============================================================================
// DragPreviewStyle
// ============================================================================

/**
 * @enum DragPreviewStyle
 * @brief Visual style of the drag ghost image.
 */
enum class DragPreviewStyle : uint8_t {
    Ghost    = 0,   ///< Semi-transparent copy of the dragged element
    Icon     = 1,   ///< Small icon representation
    Compact  = 2,   ///< Compact card showing label + badge
    Custom   = 3,   ///< Application-defined rendering
};

// ============================================================================
// DragPreviewConfig
// ============================================================================

/**
 * @struct DragPreviewConfig
 * @brief Configuration for the visual appearance of a drag operation.
 */
struct DragPreviewConfig {
    DragPreviewStyle style     = DragPreviewStyle::Ghost;
    double           opacity   = 0.7;      ///< Ghost opacity [0.0, 1.0]
    int              offsetX   = 10;       ///< Cursor offset X (px)
    int              offsetY   = 10;       ///< Cursor offset Y (px)
    int              badgeCount = 0;       ///< Multi-item badge (0 = no badge)
    std::string      iconId;               ///< Icon ID for Icon style
    std::string      label;                ///< Label for Compact style
    int              maxWidth  = 200;      ///< Max preview width (px)
    int              maxHeight = 100;      ///< Max preview height (px)
};

// ============================================================================
// DropZoneHighlight
// ============================================================================

/**
 * @enum DropZoneHighlight
 * @brief Visual state of a drop zone during a drag operation.
 */
enum class DropZoneHighlight : uint8_t {
    None     = 0,   ///< No highlight (not a valid target)
    Idle     = 1,   ///< Valid target but cursor not over it
    Hover    = 2,   ///< Cursor is over the drop zone
    Accepted = 3,   ///< Drop is valid and will be accepted
    Rejected = 4,   ///< Drop is not valid (visual: red border / X cursor)
};

// ============================================================================
// InsertionPosition
// ============================================================================

/**
 * @enum InsertionPosition
 * @brief Where the insertion indicator is shown relative to an item.
 */
enum class InsertionPosition : uint8_t {
    None   = 0,   ///< No insertion indicator
    Before = 1,   ///< Line above the item
    After  = 2,   ///< Line below the item
    On     = 3,   ///< Highlight the item itself (drop into)
};

// ============================================================================
// DropZoneConfig
// ============================================================================

/**
 * @struct DropZoneConfig
 * @brief Configuration for a drop target zone.
 */
struct DropZoneConfig {
    std::string                zoneId;
    std::vector<std::string>   acceptedMimeTypes;  ///< MIME types this zone accepts
    DropZoneHighlight          highlight   = DropZoneHighlight::None;
    InsertionPosition          insertion   = InsertionPosition::None;
    int                        insertIndex = -1;    ///< Index where drop would insert (-1 = end)
    bool                       enabled     = true;
};

// ============================================================================
// DragDropVisualManager
// ============================================================================

/**
 * @class DragDropVisualManager
 * @brief Manages drag preview config and drop zone states.
 *
 * Used by both UiNode and Widget layers:
 * - UiNode layer sets config and zone states based on DnD notifications
 * - Widget layer reads config to render overlays
 *
 * Thread safety: Not thread-safe. All calls from GUI thread.
 */
class MATCHA_EXPORT DragDropVisualManager {
public:
    DragDropVisualManager() = default;

    // ====================================================================
    // Drag preview
    // ====================================================================

    void SetPreviewConfig(DragPreviewConfig config);
    [[nodiscard]] auto PreviewConfig() const -> const DragPreviewConfig& { return _preview; }

    void SetDragActive(bool active);
    [[nodiscard]] auto IsDragActive() const -> bool { return _dragActive; }

    // ====================================================================
    // Drop zones
    // ====================================================================

    /**
     * @brief Register a drop zone. Returns false if zone ID already exists.
     */
    auto RegisterZone(DropZoneConfig zone) -> bool;

    /**
     * @brief Unregister a drop zone by ID.
     */
    auto UnregisterZone(std::string_view zoneId) -> bool;

    /**
     * @brief Update the highlight state of a zone.
     */
    auto SetZoneHighlight(std::string_view zoneId, DropZoneHighlight hl) -> bool;

    /**
     * @brief Update the insertion indicator of a zone.
     */
    auto SetZoneInsertion(std::string_view zoneId,
                          InsertionPosition pos, int index = -1) -> bool;

    /**
     * @brief Check if a zone accepts a given MIME type.
     */
    [[nodiscard]] auto ZoneAccepts(std::string_view zoneId,
                                    std::string_view mimeType) const -> bool;

    /**
     * @brief Find a zone by ID.
     */
    [[nodiscard]] auto FindZone(std::string_view zoneId) const -> const DropZoneConfig*;

    [[nodiscard]] auto ZoneCount() const -> int {
        return static_cast<int>(_zones.size());
    }

    // ====================================================================
    // Bulk operations
    // ====================================================================

    /**
     * @brief Reset all zones to None highlight (called when drag ends).
     */
    void ResetAllZones();

    /**
     * @brief Clear all zones and preview config.
     */
    void Clear();

private:
    auto FindZoneMut(std::string_view zoneId) -> DropZoneConfig*;

    DragPreviewConfig              _preview;
    bool                           _dragActive = false;
    std::vector<DropZoneConfig>    _zones;
};

} // namespace matcha::fw
