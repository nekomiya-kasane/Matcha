#pragma once

/**
 * @file ControlBar.h
 * @brief ControlBar UiNode -- bottom bar for context-specific controls.
 *
 * The ControlBar sits below the DocumentArea in the WorkspaceFrame.
 * Business layer populates it with tool-specific controls (e.g., mesh
 * quality sliders, coordinate readouts) via AddChild.
 *
 * @see 05_Greenfield_Plan.md ss 8.2.3 for the API specification.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/UiNodes/Core/UiNode.h>

namespace matcha::fw {

/**
 * @brief Bottom toolbar for context-sensitive controls.
 *
 * **Usage**: Business layer clears and repopulates on document/mode switch.
 * Accepts any UiNode children (typically WidgetWrapper or typed WidgetNodes).
 *
 * **Ownership**: WorkspaceFrame owns ControlBar.
 */
class MATCHA_EXPORT ControlBar : public UiNode {
public:
    explicit ControlBar(std::string name);
    ~ControlBar() override;

    ControlBar(const ControlBar&)            = delete;
    ControlBar& operator=(const ControlBar&) = delete;
    ControlBar(ControlBar&&)                 = delete;
    ControlBar& operator=(ControlBar&&)      = delete;

    /** @brief Remove all child controls. */
    void Clear();
};

} // namespace matcha::fw
