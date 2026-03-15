#pragma once

/**
 * @file FocusManager.h
 * @brief Centralized focus tracking and cross-region focus flow.
 *
 * FocusManager is a lightweight service that:
 * 1. Tracks which WidgetNode currently has keyboard focus
 * 2. Provides cross-region focus transfer (e.g. Toolbar -> Viewport -> PropertyPanel)
 * 3. Dispatches FocusChanged notifications via the UiNode event system
 * 4. Integrates with FocusChain for Tab/Shift+Tab within a focus scope
 *
 * Lifecycle: Created by Application::Initialize(), destroyed by Application::Shutdown().
 * The global accessor follows the same pattern as IThemeService / IAnimationService.
 */

#include "Matcha/Core/Macros.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace matcha::fw {

class EventNode;
class UiNode;
class WidgetNode;

/**
 * @brief Named focus region for cross-region navigation.
 *
 * A focus region is a named UiNode subtree that participates in
 * region-level focus flow (e.g. F6 key cycling between regions).
 */
struct FocusRegion {
    std::string          id;        ///< Unique region ID (e.g. "toolbar", "viewport", "property_panel")
    UiNode*              root;      ///< Root node of this region's subtree
    std::weak_ptr<void>  rootToken; ///< Lifetime token from EventNode::AliveToken(). Expired => region is stale.
    int                  order;     ///< Sort order for F6 cycling (ascending)
};

/**
 * @brief Centralized focus management service.
 */
class MATCHA_EXPORT FocusManager {
public:
    FocusManager();
    ~FocusManager();

    FocusManager(const FocusManager&) = delete;
    auto operator=(const FocusManager&) -> FocusManager& = delete;
    FocusManager(FocusManager&&) = delete;
    auto operator=(FocusManager&&) -> FocusManager& = delete;

    // -- Focus tracking --

    /// @brief Notify the manager that a WidgetNode gained focus.
    /// Called automatically by FocusTabEventFilter after SetFocus().
    /// Also callable externally when focus changes via mouse click.
    void NotifyFocusGained(WidgetNode* node);

    /// @brief Notify the manager that a WidgetNode lost focus.
    void NotifyFocusLost(WidgetNode* node);

    /// @brief Get the currently focused WidgetNode (may be nullptr).
    [[nodiscard]] auto FocusedNode() const -> WidgetNode*;

    /// @brief Get the previously focused WidgetNode (may be nullptr).
    [[nodiscard]] auto PreviousFocusedNode() const -> WidgetNode*;

    // -- Focus scope integration --

    /// @brief Move focus to the next focusable widget within the enclosing scope.
    /// @param current The currently focused node (uses FocusedNode() if nullptr).
    /// @return The node that received focus, or nullptr if no focusable node found.
    auto FocusNext(WidgetNode* current = nullptr) -> WidgetNode*;

    /// @brief Move focus to the previous focusable widget within the enclosing scope.
    /// @param current The currently focused node (uses FocusedNode() if nullptr).
    /// @return The node that received focus, or nullptr if no focusable node found.
    auto FocusPrevious(WidgetNode* current = nullptr) -> WidgetNode*;

    // -- Cross-region focus flow --

    /// @brief Register a named focus region for F6/Shift+F6 cycling.
    /// @param region Region descriptor. Ownership of the UiNode is NOT transferred.
    void RegisterRegion(FocusRegion region);

    /// @brief Unregister a focus region by ID.
    void UnregisterRegion(std::string_view regionId);

    /// @brief Move focus to the next registered region (F6 behavior).
    /// @return The region ID that received focus, or empty if no regions registered.
    auto FocusNextRegion() -> std::string;

    /// @brief Move focus to the previous registered region (Shift+F6 behavior).
    /// @return The region ID that received focus, or empty if no regions registered.
    auto FocusPreviousRegion() -> std::string;

    /// @brief Move focus to a specific region by ID.
    /// @return true if the region was found and focus was moved.
    auto FocusRegionById(std::string_view regionId) -> bool;

    /// @brief Get the ID of the region that currently has focus.
    [[nodiscard]] auto ActiveRegionId() const -> std::string_view;

    /// @brief Get all registered regions, sorted by order.
    [[nodiscard]] auto Regions() const -> const std::vector<FocusRegion>&;

    // -- Focus restore --

    /// @brief Push current focus state onto the restore stack.
    /// Each call pushes one level (supports nested dialogs).
    void PushFocusState();

    /// @brief Pop and restore the most recently pushed focus state.
    /// No-op if the stack is empty.
    void PopFocusState();

    /// @brief Current depth of the focus restore stack.
    [[nodiscard]] auto FocusRestoreDepth() const -> size_t;

    /// @brief Legacy single-level save (calls PushFocusState).
    void SaveFocusState() { PushFocusState(); }

    /// @brief Legacy single-level restore (calls PopFocusState).
    void RestoreFocusState() { PopFocusState(); }

private:
    void PurgeStaleRegions();

    WidgetNode*                        _focused         = nullptr;
    WidgetNode*                        _previousFocused = nullptr;
    std::vector<WidgetNode*>           _focusStack;
    std::vector<FocusRegion>           _regions;
    std::string                        _activeRegionId;
};

// ============================================================================
// Global Focus Manager Accessor
// ============================================================================

/// @brief Set the global FocusManager instance. Called once by Application::Initialize().
MATCHA_EXPORT void SetFocusManager(FocusManager* mgr);

/// @brief Get the global FocusManager instance (may be nullptr before Initialize).
[[nodiscard]] MATCHA_EXPORT auto GetFocusManager() -> FocusManager*;

/// @brief Check if a global FocusManager has been set.
[[nodiscard]] MATCHA_EXPORT auto HasFocusManager() -> bool;

} // namespace matcha::fw
