/**
 * @file VariantNameRegistry.cpp
 * @brief Implementation of VariantNameRegistry.
 */

#include <Matcha/Theming/Token/VariantNameRegistry.h>

namespace matcha::fw {

void VariantNameRegistry::RegisterKind(std::string_view widgetKind,
                                        std::vector<std::string> variantNames)
{
    KindEntry entry;
    for (int i = 0; i < static_cast<int>(variantNames.size()); ++i) {
        entry.nameToIndex[variantNames[static_cast<std::size_t>(i)]] = i;
    }
    entry.names = std::move(variantNames);
    _kinds[std::string(widgetKind)] = std::move(entry);
}

auto VariantNameRegistry::IndexOf(std::string_view widgetKind,
                                   std::string_view variantName) const -> std::optional<int>
{
    auto kindIt = _kinds.find(std::string(widgetKind));
    if (kindIt == _kinds.end()) {
        return std::nullopt;
    }
    auto nameIt = kindIt->second.nameToIndex.find(std::string(variantName));
    if (nameIt == kindIt->second.nameToIndex.end()) {
        return std::nullopt;
    }
    return nameIt->second;
}

auto VariantNameRegistry::NameOf(std::string_view widgetKind,
                                  int index) const -> std::optional<std::string_view>
{
    auto kindIt = _kinds.find(std::string(widgetKind));
    if (kindIt == _kinds.end()) {
        return std::nullopt;
    }
    if (index < 0 || index >= static_cast<int>(kindIt->second.names.size())) {
        return std::nullopt;
    }
    return kindIt->second.names[static_cast<std::size_t>(index)];
}

auto VariantNameRegistry::VariantNames(std::string_view widgetKind) const
    -> const std::vector<std::string>*
{
    auto kindIt = _kinds.find(std::string(widgetKind));
    if (kindIt == _kinds.end()) {
        return nullptr;
    }
    return &kindIt->second.names;
}

auto VariantNameRegistry::VariantCount(std::string_view widgetKind) const -> int
{
    auto kindIt = _kinds.find(std::string(widgetKind));
    if (kindIt == _kinds.end()) {
        return 0;
    }
    return static_cast<int>(kindIt->second.names.size());
}

auto VariantNameRegistry::HasKind(std::string_view widgetKind) const -> bool
{
    return _kinds.contains(std::string(widgetKind));
}

auto VariantNameRegistry::RegisteredKinds() const -> std::vector<std::string_view>
{
    std::vector<std::string_view> result;
    result.reserve(_kinds.size());
    for (const auto& [key, entry] : _kinds) {
        result.emplace_back(key);
    }
    return result;
}

auto VariantNameRegistry::UnregisterKind(std::string_view widgetKind) -> bool
{
    return _kinds.erase(std::string(widgetKind)) > 0;
}

void VariantNameRegistry::Clear()
{
    _kinds.clear();
}

} // namespace matcha::fw
