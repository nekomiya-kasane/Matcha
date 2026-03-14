#include "Matcha/Foundation/ContentStateModel.h"

#include <algorithm>

namespace matcha::fw {

void ContentStateModel::SetLoading(bool loading)
{
    _loading = loading;
    Resolve();
}

void ContentStateModel::SetError(const ErrorInfo& info)
{
    _hasError = true;
    _errorInfo = info;
    Resolve();
}

void ContentStateModel::SetError(const std::string& message)
{
    SetError({.message = message, .detail = {}, .severity = ErrorSeverity::Error, .retryable = true});
}

void ContentStateModel::ClearError()
{
    _hasError = false;
    _errorInfo = {};
    Resolve();
}

void ContentStateModel::SetEmpty(const EmptyInfo& info)
{
    _isEmpty = true;
    _emptyInfo = info;
    Resolve();
}

void ContentStateModel::ClearEmpty()
{
    _isEmpty = false;
    _emptyInfo = {};
    Resolve();
}

auto ContentStateModel::ResolvedState() const -> ContentState
{
    if (_loading) { return ContentState::Loading; }
    if (_hasError) { return ContentState::Error; }
    if (_isEmpty) { return ContentState::Empty; }
    return ContentState::Content;
}

auto ContentStateModel::OnStateChanged(StateChangedCallback cb) -> CallbackId
{
    const auto id = _nextId++;
    _callbacks.push_back({id, std::move(cb)});
    return id;
}

void ContentStateModel::RemoveCallback(CallbackId id)
{
    std::erase_if(_callbacks, [id](const CallbackEntry& e) { return e.id == id; });
}

void ContentStateModel::Resolve()
{
    const auto newState = ResolvedState();
    if (newState != _resolved) {
        _resolved = newState;
        for (const auto& entry : _callbacks) {
            entry.cb(_resolved);
        }
    }
}

} // namespace matcha::fw
