/**
 * @file MnemonicManagerGlobal.cpp
 * @brief Global MnemonicManager accessor implementation.
 */

#include "Matcha/UiNodes/Core/MnemonicManager.h"

namespace matcha::fw {

namespace {
MnemonicManager* g_mnemonicManager = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
} // anonymous namespace

void SetMnemonicManager(MnemonicManager* mgr)
{
    g_mnemonicManager = mgr;
}

auto GetMnemonicManager() -> MnemonicManager*
{
    return g_mnemonicManager;
}

auto HasMnemonicManager() -> bool
{
    return g_mnemonicManager != nullptr;
}

} // namespace matcha::fw
