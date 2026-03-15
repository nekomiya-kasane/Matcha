/**
 * @file NyanFwTetMeshEditor.cpp
 * @brief Tetrahedral mesh editor implementation.
 */

#include "NyanFwTetMeshEditor.h"

#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"

namespace nyancad {

void NyanFwTetMeshEditor::Start(matcha::fw::Application& app)
{
    if (_started) { return; }
    auto actionBarPtr = app.GetShell().GetActionBar();
    if (actionBarPtr.get() != nullptr) {
        (void)actionBarPtr->AddTab("tet_mesh", "Tet Mesh");
    }
    _started = true;
}

void NyanFwTetMeshEditor::Stop()
{
    _started = false;
}

} // namespace nyancad
