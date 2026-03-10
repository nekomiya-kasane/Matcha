#pragma once

/**
 * @file WorkspaceFrame.h
 * @brief WorkspaceFrame UiNode -- central layout container within a WindowNode.
 *
 * Provides access to the main content area components:
 * DocumentArea, ActionBar, ControlBar, and overlay layout.
 *
 * @see 05_Greenfield_Plan.md ss 8.2.3 for the API specification.
 */

#include <Matcha/Foundation/Macros.h>
#include <Matcha/Foundation/Types.h>
#include <Matcha/UiNodes/Core/UiNode.h>

class QWidget;

namespace matcha::fw {

class DocumentArea;
class ActionBarNode;
class ControlBar;
class DialogNode;

/**
 * @brief Central workspace layout container.
 *
 * **Children** (managed internally):
 * - ActionBar (top)
 * - DocumentArea (center)
 * - ControlBar (bottom, optional)
 * - Overlay layout (for floating panels)
 *
 * **Ownership**: WindowNode owns WorkspaceFrame.
 */
class MATCHA_EXPORT WorkspaceFrame : public UiNode {
public:
    /// @brief Construct with a parent UiNode whose Widget() is the container (not owned).
    WorkspaceFrame(std::string name, UiNode* parentHint = nullptr);
    ~WorkspaceFrame() override;

    WorkspaceFrame(const WorkspaceFrame&)            = delete;
    WorkspaceFrame& operator=(const WorkspaceFrame&) = delete;
    WorkspaceFrame(WorkspaceFrame&&)                 = delete;
    WorkspaceFrame& operator=(WorkspaceFrame&&)      = delete;

    // -- Widget --

    [[nodiscard]] auto Widget() -> QWidget* override;

    // -- Child access --

    [[nodiscard]] auto GetDocumentArea() -> observer_ptr<DocumentArea>;
    [[nodiscard]] auto GetActionBar() -> observer_ptr<ActionBarNode>;
    [[nodiscard]] auto GetControlBar() -> observer_ptr<ControlBar>;

    // -- Dialog overlay management --

    /// @brief Show a modeless dialog embedded within this WorkspaceFrame.
    /// The dialog is reparented to the container widget and constrained to its bounds.
    /// DialogNode ownership is transferred to WorkspaceFrame's UiNode tree.
    void ShowDialog(std::unique_ptr<DialogNode> dialog);

    /// @brief Close and remove an embedded dialog by pointer.
    void CloseDialog(DialogNode* dialog);

private:
    friend class WindowNode;
    friend class FloatingWindowNode;
    friend class FloatingTabWindowNode;

    /// @brief Set the container widget. @internal Only called by WindowNode/FloatingWindowNode.
    void SetContainerWidget(QWidget* container) { _container = container; }

    QWidget* _container = nullptr;
};

} // namespace matcha::fw
