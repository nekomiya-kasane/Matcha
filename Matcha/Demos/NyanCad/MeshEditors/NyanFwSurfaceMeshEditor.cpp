/**
 * @file NyanFwSurfaceMeshEditor.cpp
 * @brief Surface mesh editor implementation.
 */

#include "NyanFwSurfaceMeshEditor.h"

#include "Matcha/UiNodes/ActionBar/ActionBarNode.h"
#include "Matcha/UiNodes/Shell/Application.h"
#include "Matcha/UiNodes/Shell/Shell.h"

namespace nyancad {

void NyanFwSurfaceMeshEditor::Start(matcha::fw::Application& app)
{
    if (_started) { return; }
    auto actionBarPtr = app.GetShell().GetActionBar();
    if (actionBarPtr.get() != nullptr) {
        (void)actionBarPtr->AddTab("surface_mesh", "Surface Mesh");
    }
    _started = true;
}

void NyanFwSurfaceMeshEditor::Stop()
{
    _started = false;
}

} // namespace nyancad
