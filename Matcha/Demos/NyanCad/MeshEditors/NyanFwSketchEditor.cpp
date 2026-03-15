/**
 * @file NyanFwSketchEditor.cpp
 * @brief Sketch editor implementation.
 */

#include "NyanFwSketchEditor.h"

#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Tree/Composition/Shell/Shell.h"

namespace nyancad {

void NyanFwSketchEditor::Start(matcha::fw::Application& app)
{
    if (_started) { return; }
    auto actionBarPtr = app.GetShell().GetActionBar();
    if (actionBarPtr.get() != nullptr) {
        (void)actionBarPtr->AddTab("sketch", "Sketch");
    }
    _started = true;
}

void NyanFwSketchEditor::Stop()
{
    _started = false;
}

} // namespace nyancad
