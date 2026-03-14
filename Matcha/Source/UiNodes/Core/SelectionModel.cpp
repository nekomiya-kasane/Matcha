/**
 * @file SelectionModel.cpp
 * @brief Implementation of the generic selection model (§7.1).
 */

#include <Matcha/UiNodes/Core/SelectionModel.h>

#include <algorithm>
#include <utility>

namespace matcha::fw {

// ============================================================================
// Construction
// ============================================================================

SelectionModel::SelectionModel(SelectionMode mode)
    : _mode(mode)
{
}

// ============================================================================
// Configuration
// ============================================================================

void SelectionModel::SetItemCount(int count)
{
    count = std::max(count, 0);
    if (count == _itemCount) { return; }
    _itemCount = count;
    // Clear selection when item count changes
    _selected.clear();
    _anchor = -1;
    _focus  = -1;
    NotifyChanged();
}

void SelectionModel::SetMode(SelectionMode mode)
{
    if (mode == _mode) { return; }
    _mode = mode;
    // Trim selection to match new mode constraints
    if (_mode == SelectionMode::None) {
        _selected.clear();
        _anchor = -1;
        _focus  = -1;
        NotifyChanged();
    } else if (_mode == SelectionMode::Single && _selected.size() > 1) {
        // Keep only the focus item (or last selected)
        const int keep = _focus >= 0 ? _focus : _selected.back();
        _selected.clear();
        _selected.push_back(keep);
        _anchor = keep;
        _focus  = keep;
        NotifyChanged();
    }
}

// ============================================================================
// Interaction API
// ============================================================================

void SelectionModel::Click(int index, ClickModifier mod)
{
    if (_mode == SelectionMode::None) { return; }
    if (!ValidIndex(index)) { return; }

    const bool ctrl  = (mod & ClickModifier::Ctrl) != ClickModifier::None;
    const bool shift = (mod & ClickModifier::Shift) != ClickModifier::None;

    if (_mode == SelectionMode::Single) {
        // Single mode ignores modifiers
        _selected.clear();
        _selected.push_back(index);
        _anchor = index;
        _focus  = index;
        NotifyChanged();
        return;
    }

    // Multi / Range modes
    if (shift && _anchor >= 0) {
        // Shift+Click: range selection [anchor, index]
        SetRange(_anchor, index);
        _focus = index;
        // anchor stays unchanged
        NotifyChanged();
    } else if (ctrl) {
        // Ctrl+Click: toggle
        auto it = std::find(_selected.begin(), _selected.end(), index);
        if (it != _selected.end()) {
            _selected.erase(it);
        } else {
            _selected.push_back(index);
            std::sort(_selected.begin(), _selected.end());
        }
        _anchor = index;
        _focus  = index;
        NotifyChanged();
    } else {
        // Plain click: clear + select
        _selected.clear();
        _selected.push_back(index);
        _anchor = index;
        _focus  = index;
        NotifyChanged();
    }
}

void SelectionModel::SelectAll()
{
    if (_mode == SelectionMode::None || _mode == SelectionMode::Single) { return; }
    if (_itemCount <= 0) { return; }

    _selected.resize(static_cast<std::size_t>(_itemCount));
    for (int i = 0; i < _itemCount; ++i) {
        _selected[static_cast<std::size_t>(i)] = i;
    }
    _anchor = 0;
    _focus  = _itemCount - 1;
    NotifyChanged();
}

void SelectionModel::Clear()
{
    if (_selected.empty() && _anchor == -1 && _focus == -1) { return; }
    _selected.clear();
    _anchor = -1;
    _focus  = -1;
    NotifyChanged();
}

void SelectionModel::MoveFocus(int delta, bool extend)
{
    if (_mode == SelectionMode::None) { return; }
    if (_itemCount <= 0) { return; }

    int newFocus = _focus + delta;
    newFocus = std::clamp(newFocus, 0, _itemCount - 1);
    if (newFocus == _focus) { return; }

    if (extend && _mode != SelectionMode::Single && _anchor >= 0) {
        // Shift+Arrow: extend range
        SetRange(_anchor, newFocus);
        _focus = newFocus;
        NotifyChanged();
    } else {
        // Arrow without shift: move + select single
        _selected.clear();
        _selected.push_back(newFocus);
        _anchor = newFocus;
        _focus  = newFocus;
        NotifyChanged();
    }
}

// ============================================================================
// Query API
// ============================================================================

auto SelectionModel::SelectedIndices() const -> const std::vector<int>&
{
    return _selected;
}

auto SelectionModel::IsSelected(int index) const -> bool
{
    return std::find(_selected.begin(), _selected.end(), index) != _selected.end();
}

// ============================================================================
// Observer
// ============================================================================

void SelectionModel::OnChanged(ChangedCallback cb)
{
    _callback = std::move(cb);
}

// ============================================================================
// Private helpers
// ============================================================================

void SelectionModel::SetRange(int from, int to)
{
    const int lo = std::min(from, to);
    const int hi = std::max(from, to);
    _selected.clear();
    const auto rangeLen = static_cast<std::size_t>(hi - lo + 1);
    _selected.reserve(rangeLen);
    for (int i = lo; i <= hi; ++i) {
        _selected.push_back(i);
    }
}

void SelectionModel::NotifyChanged()
{
    if (_callback) { _callback(*this); }
}

} // namespace matcha::fw
