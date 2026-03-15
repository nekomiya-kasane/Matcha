#pragma once

/**
 * @file MnemonicManager.h
 * @brief Centralized mnemonic (access key) dispatch and scope management.
 *
 * MnemonicManager is the UiNode-layer coordinator for mnemonic keystrokes.
 * It maintains a scope stack and a registry of mnemonic handlers, dispatching
 * keypresses to the appropriate handler based on the active scope.
 *
 * Lifecycle: Created by Application::Initialize(), destroyed by Application::Shutdown().
 * Global accessor follows the same pattern as FocusManager.
 *
 * @see MnemonicState (gui layer) for rendering and Alt visibility.
 * @see Matcha_Design_System_Specification.md Section 18.11.18 for architecture.
 */

#include "Matcha/Core/Macros.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

class QChar;

namespace matcha::fw {

// --------------------------------------------------------------------------- //
// Mnemonic scope
// --------------------------------------------------------------------------- //

/**
 * @brief Scope in which a mnemonic registration is active.
 *
 * Scopes form a stack. Only registrations matching the top-of-stack scope
 * are eligible for dispatch. Scope transitions mirror focus scope changes:
 * - Global: main window (menu bar mnemonics)
 * - Menu: inside an open menu (direct letter, no Alt)
 * - Dialog: inside a modal/modeless dialog (Alt+Letter)
 * - Panel: side panel / embedded form with SetFocusScope(true)
 */
enum class MnemonicScope : uint8_t {
    Global = 0,
    Menu   = 1,
    Dialog = 2,
    Panel  = 3,
};

// --------------------------------------------------------------------------- //
// Registration descriptor
// --------------------------------------------------------------------------- //

/**
 * @brief Descriptor for a single mnemonic registration.
 */
struct MnemonicRegistration {
    MnemonicScope         scope;
    char16_t              character;   ///< Mnemonic character (stored as char16_t for Qt-free header). Case-insensitive matching.
    std::function<void()> handler;     ///< Invoked when the mnemonic is dispatched.
    std::weak_ptr<void>   aliveToken;  ///< Lifetime guard from EventNode::AliveToken(). Expired => skip.
};

// --------------------------------------------------------------------------- //
// MnemonicManager
// --------------------------------------------------------------------------- //

/**
 * @brief Centralized mnemonic dispatch and scope management.
 *
 * Plain C++ class (not a QObject). Lives in the fw layer (no Qt widget dependency).
 */
class MATCHA_EXPORT MnemonicManager {
public:
    MnemonicManager();
    ~MnemonicManager();

    MnemonicManager(const MnemonicManager&) = delete;
    auto operator=(const MnemonicManager&) -> MnemonicManager& = delete;
    MnemonicManager(MnemonicManager&&) = delete;
    auto operator=(MnemonicManager&&) -> MnemonicManager& = delete;

    // -- Registration --

    /**
     * @brief Register a mnemonic handler.
     *
     * The registration is active only when the scope stack's top matches
     * the registration's scope.
     *
     * @param reg Registration descriptor (scope, character, handler, aliveToken).
     * @return Unique registration ID for later unregistration.
     */
    auto Register(MnemonicRegistration reg) -> uint64_t;

    /**
     * @brief Unregister a mnemonic handler by its ID.
     */
    void Unregister(uint64_t id);

    // -- Dispatch --

    /**
     * @brief Attempt to dispatch a mnemonic character in the current active scope.
     *
     * Searches registrations matching the active scope and the given character
     * (case-insensitive). If exactly one live registration matches, its handler
     * is invoked. If multiple match, the first is invoked (caller may implement
     * cycling externally).
     *
     * @param ch The mnemonic character (will be compared case-insensitively).
     * @return true if a matching handler was found and invoked.
     */
    auto Dispatch(char16_t ch) -> bool;

    // -- Scope stack --

    /**
     * @brief Push a new scope onto the stack.
     *
     * The pushed scope becomes the active scope for subsequent Dispatch() calls.
     */
    void PushScope(MnemonicScope scope);

    /**
     * @brief Pop the top scope from the stack.
     *
     * If the stack becomes empty, the active scope reverts to Global.
     */
    void PopScope();

    /**
     * @brief Get the currently active scope (top of stack, or Global if empty).
     */
    [[nodiscard]] auto ActiveScope() const -> MnemonicScope;

    // -- Bulk operations --

    /**
     * @brief Remove all registrations for a given scope.
     *
     * Useful when a menu closes or a dialog hides — bulk-clear all its registrations.
     */
    void ClearScope(MnemonicScope scope);

    /**
     * @brief Remove all registrations and reset the scope stack.
     */
    void Reset();

    // -- Query --

    /**
     * @brief Count of active (non-expired) registrations in the given scope.
     */
    [[nodiscard]] auto RegistrationCount(MnemonicScope scope) const -> size_t;

private:
    struct Entry {
        uint64_t              id = 0;
        MnemonicRegistration  reg;
    };

    std::vector<Entry>          _entries;
    std::vector<MnemonicScope>  _scopeStack;
    uint64_t                    _nextId = 1;
};

// ============================================================================
// Global MnemonicManager Accessor
// ============================================================================

/// @brief Set the global MnemonicManager instance. Called once by Application::Initialize().
MATCHA_EXPORT void SetMnemonicManager(MnemonicManager* mgr);

/// @brief Get the global MnemonicManager instance (may be nullptr before Initialize).
[[nodiscard]] MATCHA_EXPORT auto GetMnemonicManager() -> MnemonicManager*;

/// @brief Check if a global MnemonicManager has been set.
[[nodiscard]] MATCHA_EXPORT auto HasMnemonicManager() -> bool;

} // namespace matcha::fw
