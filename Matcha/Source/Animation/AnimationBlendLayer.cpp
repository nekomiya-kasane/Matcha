/**
 * @file AnimationBlendLayer.cpp
 * @brief Implementation of AnimationBlendLayer.
 */

#include <Matcha/Animation/AnimationBlendLayer.h>

#include <algorithm>

namespace matcha::fw {

// ============================================================================
// Entry management
// ============================================================================

void AnimationBlendLayer::SetEntry(const std::string& propertyKey, BlendEntry entry)
{
    _entries[propertyKey] = entry;
}

auto AnimationBlendLayer::UpdateValue(const std::string& propertyKey,
                                       double value, double progress) -> bool
{
    auto it = _entries.find(propertyKey);
    if (it == _entries.end()) {
        return false;
    }
    it->second.value = value;
    it->second.progress = progress;
    return true;
}

auto AnimationBlendLayer::Remove(const std::string& propertyKey) -> bool
{
    return _entries.erase(propertyKey) > 0;
}

auto AnimationBlendLayer::GetEntry(const std::string& propertyKey) const -> const BlendEntry*
{
    auto it = _entries.find(propertyKey);
    if (it == _entries.end()) {
        return nullptr;
    }
    return &it->second;
}

auto AnimationBlendLayer::HasActive(const std::string& propertyKey) const -> bool
{
    const auto* e = GetEntry(propertyKey);
    return e != nullptr && e->active;
}

// ============================================================================
// Blending
// ============================================================================

auto AnimationBlendLayer::Apply(const std::string& propertyKey,
                                 double baseValue) const -> double
{
    const auto* e = GetEntry(propertyKey);
    if (e == nullptr || !e->active) {
        return baseValue;
    }
    const double w = std::clamp(e->weight, 0.0, 1.0);
    return (baseValue * (1.0 - w)) + (e->value * w);
}

// ============================================================================
// Bulk operations
// ============================================================================

void AnimationBlendLayer::DeactivateAll()
{
    for (auto& [key, entry] : _entries) {
        entry.active = false;
    }
}

void AnimationBlendLayer::PurgeInactive()
{
    std::erase_if(_entries, [](const auto& pair) {
        return !pair.second.active;
    });
}

void AnimationBlendLayer::Clear()
{
    _entries.clear();
}

auto AnimationBlendLayer::ActiveCount() const -> int
{
    int count = 0;
    for (const auto& [key, entry] : _entries) {
        if (entry.active) {
            ++count;
        }
    }
    return count;
}

} // namespace matcha::fw
