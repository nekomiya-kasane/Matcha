#include "Matcha/Tree/Layout/BreakpointObserver.h"

#include <algorithm>

namespace matcha::fw {

void BreakpointObserver::AddRule(const BreakpointRule& rule)
{
    _entries.push_back({.rule = rule, .collapsed = false});
    std::ranges::sort(_entries, [](const auto& a, const auto& b) {
        return a.rule.priority < b.rule.priority;
    });
    _lastWidth = -1;
    _lastHeight = -1;
}

void BreakpointObserver::RemoveRules(const std::string& elementId)
{
    std::erase_if(_entries, [&](const auto& e) { return e.rule.elementId == elementId; });
    _lastWidth = -1;
    _lastHeight = -1;
}

void BreakpointObserver::ClearRules()
{
    _entries.clear();
    _lastWidth = -1;
    _lastHeight = -1;
}

auto BreakpointObserver::Evaluate(int width, int height) -> std::vector<CollapseState>
{
    if (width == _lastWidth && height == _lastHeight) { return States(); }

    bool anyChanged = false;
    for (auto& entry : _entries) {
        const int dim = (entry.rule.axis == BreakpointAxis::Height) ? height : width;
        const bool nowCollapsed = dim < entry.rule.threshold;
        if (entry.collapsed != nowCollapsed) {
            entry.collapsed = nowCollapsed;
            anyChanged = true;
        }
    }

    _lastWidth = width;
    _lastHeight = height;

    auto snapshot = States();
    if (anyChanged && _callback) {
        _callback(snapshot);
    }

    return snapshot;
}

auto BreakpointObserver::States() const -> std::vector<CollapseState>
{
    std::vector<CollapseState> result;
    result.reserve(_entries.size());
    for (const auto& e : _entries) {
        result.push_back({
            .elementId = e.rule.elementId,
            .action    = e.rule.action,
            .collapsed = e.collapsed,
        });
    }
    return result;
}

auto BreakpointObserver::IsCollapsed(const std::string& elementId) const -> bool
{
    for (const auto& e : _entries) {
        if (e.rule.elementId == elementId) { return e.collapsed; }
    }
    return false;
}

} // namespace matcha::fw
