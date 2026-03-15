/**
 * @file MnemonicStateGlobal.cpp
 * @brief Global MnemonicState accessor implementation.
 */

#include "Matcha/Interaction/Focus/MnemonicState.h"

namespace matcha::gui {

namespace {
MnemonicState* g_mnemonicState = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
} // anonymous namespace

void SetMnemonicState(MnemonicState* state)
{
    g_mnemonicState = state;
}

auto GetMnemonicState() -> MnemonicState*
{
    return g_mnemonicState;
}

auto HasMnemonicState() -> bool
{
    return g_mnemonicState != nullptr;
}

} // namespace matcha::gui
