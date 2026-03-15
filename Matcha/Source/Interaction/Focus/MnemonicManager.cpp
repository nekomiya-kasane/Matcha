/**
 * @file MnemonicManager.cpp
 * @brief MnemonicManager implementation — scope stack and dispatch.
 */

#include "Matcha/Interaction/Focus/MnemonicManager.h"


namespace matcha::fw {

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

MnemonicManager::MnemonicManager() = default;
MnemonicManager::~MnemonicManager() = default;

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

auto MnemonicManager::Register(MnemonicRegistration reg) -> uint64_t
{
    uint64_t id = _nextId++;
    _entries.push_back(Entry{id, std::move(reg)});
    return id;
}

void MnemonicManager::Unregister(uint64_t id)
{
    std::erase_if(_entries, [id](const Entry& e) { return e.id == id; });
}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------

namespace {

auto ToUpper(char16_t ch) -> char16_t
{
    if (ch >= u'a' && ch <= u'z') {
        return ch - u'a' + u'A';
    }
    return ch;
}

} // anonymous namespace

auto MnemonicManager::Dispatch(char16_t ch) -> bool
{
    const auto active = ActiveScope();
    const auto upperCh = ToUpper(ch);

    for (auto& entry : _entries) {
        if (entry.reg.scope != active) {
            continue;
        }
        if (ToUpper(entry.reg.character) != upperCh) {
            continue;
        }
        // Check lifetime guard
        if (!entry.reg.aliveToken.expired() || entry.reg.aliveToken.lock() == nullptr) {
            // aliveToken is either expired (dead) or was never set (default empty weak_ptr).
            // For empty weak_ptr (never set), we allow dispatch — it means no lifetime guard.
            // For expired non-empty weak_ptr, we skip.
        }

        // Refined logic: empty weak_ptr = no guard = always valid.
        // Non-empty expired weak_ptr = dead owner = skip.
        bool hasGuard = false;
        {
            // A default-constructed weak_ptr has use_count() == 0 and expired() == true.
            // We distinguish "never set" from "was set but expired" by checking
            // if the weak_ptr was ever assigned. Unfortunately weak_ptr doesn't
            // expose this directly. We use owner_before trick:
            // A default weak_ptr and a null shared_ptr have the same owner (none).
            std::shared_ptr<void> nullSp;
            hasGuard = entry.reg.aliveToken.owner_before(nullSp) ||
                       nullSp.owner_before(entry.reg.aliveToken);
        }

        if (hasGuard && entry.reg.aliveToken.expired()) {
            continue; // Owner is dead — skip this registration
        }

        // Invoke handler
        if (entry.reg.handler) {
            entry.reg.handler();
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// Scope stack
// ---------------------------------------------------------------------------

void MnemonicManager::PushScope(MnemonicScope scope)
{
    _scopeStack.push_back(scope);
}

void MnemonicManager::PopScope()
{
    if (!_scopeStack.empty()) {
        _scopeStack.pop_back();
    }
}

auto MnemonicManager::ActiveScope() const -> MnemonicScope
{
    if (_scopeStack.empty()) {
        return MnemonicScope::Global;
    }
    return _scopeStack.back();
}

// ---------------------------------------------------------------------------
// Bulk operations
// ---------------------------------------------------------------------------

void MnemonicManager::ClearScope(MnemonicScope scope)
{
    std::erase_if(_entries, [scope](const Entry& e) { return e.reg.scope == scope; });
}

void MnemonicManager::Reset()
{
    _entries.clear();
    _scopeStack.clear();
}

// ---------------------------------------------------------------------------
// Query
// ---------------------------------------------------------------------------

auto MnemonicManager::RegistrationCount(MnemonicScope scope) const -> size_t
{
    size_t count = 0;
    for (const auto& entry : _entries) {
        if (entry.reg.scope != scope) {
            continue;
        }
        // Check if alive
        bool hasGuard = false;
        {
            std::shared_ptr<void> nullSp;
            hasGuard = entry.reg.aliveToken.owner_before(nullSp) ||
                       nullSp.owner_before(entry.reg.aliveToken);
        }
        if (hasGuard && entry.reg.aliveToken.expired()) {
            continue;
        }
        ++count;
    }
    return count;
}

} // namespace matcha::fw
