#include "Matcha/Interaction/Focus/FocusManager.h"

#include "Matcha/Interaction/Focus/FocusChain.h"
#include "Matcha/Tree/UiNode.h"
#include "Matcha/Tree/UiNodeNotification.h"
#include "Matcha/Tree/WidgetNode.h"

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// FocusManager
// ============================================================================

FocusManager::FocusManager() = default;
FocusManager::~FocusManager() = default;

// -- Focus tracking --

void FocusManager::NotifyFocusGained(WidgetNode* node)
{
    if (node == _focused) {
        return;
    }
    _previousFocused = _focused;
    _focused = node;

    // Dispatch FocusChanged(true) notification via the UiNode event tree
    if (node != nullptr) {
        FocusChanged notif(true);
        node->SendNotification(node, notif);
    }

    // Update active region ID
    if (node != nullptr) {
        PurgeStaleRegions();
        for (const auto& region : _regions) {
            UiNode* walk = node;
            while (walk != nullptr) {
                if (walk == region.root) {
                    _activeRegionId = region.id;
                    break;
                }
                walk = walk->ParentNode();
            }
            if (walk != nullptr) { break; }
        }
    }
}

void FocusManager::NotifyFocusLost(WidgetNode* node)
{
    if (node == _focused) {
        _previousFocused = _focused;
        _focused = nullptr;

        // Dispatch FocusChanged(false) notification
        if (node != nullptr) {
            FocusChanged notif(false);
            node->SendNotification(node, notif);
        }
    }
}

auto FocusManager::FocusedNode() const -> WidgetNode*
{
    return _focused;
}

auto FocusManager::PreviousFocusedNode() const -> WidgetNode*
{
    return _previousFocused;
}

// -- Focus scope integration --

auto FocusManager::FocusNext(WidgetNode* current) -> WidgetNode*
{
    if (current == nullptr) {
        current = _focused;
    }
    if (current == nullptr) {
        return nullptr;
    }

    UiNode* scope = current->FindEnclosingFocusScope();
    if (scope == nullptr) {
        scope = current;
        while (scope->ParentNode() != nullptr) {
            scope = scope->ParentNode();
        }
    }

    auto chain = FocusChain::Collect(scope);
    if (chain.empty()) {
        return nullptr;
    }

    WidgetNode* next = FocusChain::Next(chain, current);
    if (next != nullptr) {
        next->SetFocus();
        NotifyFocusGained(next);
    }
    return next;
}

auto FocusManager::FocusPrevious(WidgetNode* current) -> WidgetNode*
{
    if (current == nullptr) {
        current = _focused;
    }
    if (current == nullptr) {
        return nullptr;
    }

    UiNode* scope = current->FindEnclosingFocusScope();
    if (scope == nullptr) {
        scope = current;
        while (scope->ParentNode() != nullptr) {
            scope = scope->ParentNode();
        }
    }

    auto chain = FocusChain::Collect(scope);
    if (chain.empty()) {
        return nullptr;
    }

    WidgetNode* prev = FocusChain::Previous(chain, current);
    if (prev != nullptr) {
        prev->SetFocus();
        NotifyFocusGained(prev);
    }
    return prev;
}

// -- Cross-region focus flow --

void FocusManager::RegisterRegion(FocusRegion region)
{
    // Remove existing region with same ID
    UnregisterRegion(region.id);
    _regions.push_back(std::move(region));
    std::sort(_regions.begin(), _regions.end(),
              [](const FocusRegion& a, const FocusRegion& b) {
                  return a.order < b.order;
              });
}

void FocusManager::UnregisterRegion(std::string_view regionId)
{
    std::erase_if(_regions,
                  [&](const FocusRegion& r) { return r.id == regionId; });
    if (_activeRegionId == regionId) {
        _activeRegionId.clear();
    }
}

void FocusManager::PurgeStaleRegions()
{
    std::erase_if(_regions,
                  [](const FocusRegion& r) { return r.rootToken.expired(); });
}

auto FocusManager::FocusNextRegion() -> std::string
{
    if (_regions.empty()) {
        return {};
    }

    // Find current region index
    size_t currentIdx = 0;
    bool found = false;
    for (size_t i = 0; i < _regions.size(); ++i) {
        if (_regions[i].id == _activeRegionId) {
            currentIdx = i;
            found = true;
            break;
        }
    }

    size_t nextIdx = found ? (currentIdx + 1) % _regions.size() : 0;
    return FocusRegionById(_regions[nextIdx].id)
        ? _regions[nextIdx].id
        : std::string{};
}

auto FocusManager::FocusPreviousRegion() -> std::string
{
    if (_regions.empty()) {
        return {};
    }

    size_t currentIdx = 0;
    bool found = false;
    for (size_t i = 0; i < _regions.size(); ++i) {
        if (_regions[i].id == _activeRegionId) {
            currentIdx = i;
            found = true;
            break;
        }
    }

    size_t prevIdx = found
        ? (currentIdx + _regions.size() - 1) % _regions.size()
        : _regions.size() - 1;
    return FocusRegionById(_regions[prevIdx].id)
        ? _regions[prevIdx].id
        : std::string{};
}

auto FocusManager::FocusRegionById(std::string_view regionId) -> bool
{
    PurgeStaleRegions();
    for (const auto& region : _regions) {
        if (region.id != regionId) {
            continue;
        }
        // Collect focusable nodes in the region and focus the first one
        auto chain = FocusChain::Collect(region.root);
        if (!chain.empty()) {
            chain.front()->SetFocus();
            NotifyFocusGained(chain.front());
            _activeRegionId = std::string(regionId);
            return true;
        }
        return false;
    }
    return false;
}

auto FocusManager::ActiveRegionId() const -> std::string_view
{
    return _activeRegionId;
}

auto FocusManager::Regions() const -> const std::vector<FocusRegion>&
{
    return _regions;
}

// -- Focus restore (stack) --

void FocusManager::PushFocusState()
{
    _focusStack.push_back(_focused);
}

void FocusManager::PopFocusState()
{
    if (_focusStack.empty()) {
        return;
    }
    WidgetNode* saved = _focusStack.back();
    _focusStack.pop_back();
    if (saved != nullptr) {
        saved->SetFocus();
        NotifyFocusGained(saved);
    }
}

auto FocusManager::FocusRestoreDepth() const -> size_t
{
    return _focusStack.size();
}

// ============================================================================
// Global accessor
// ============================================================================

namespace {
FocusManager* g_focusManager = nullptr;
} // anonymous namespace

void SetFocusManager(FocusManager* mgr)
{
    g_focusManager = mgr;
}

auto GetFocusManager() -> FocusManager*
{
    return g_focusManager;
}

auto HasFocusManager() -> bool
{
    return g_focusManager != nullptr;
}

} // namespace matcha::fw
