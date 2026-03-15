#pragma once

/**
 * @file LayerDemo.h
 * @brief Demo page showing all 8 UI layer types in Matcha's semantic layering system.
 *
 * Demonstrates:
 *  1. FloatingPanel  — Qt::Tool, non-modal, no auto-close
 *  2. Scrim          — Qt native modal backdrop (via DialogNode)
 *  3. Dialog         — Modal dialog with focus trap
 *  4. Popover        — Click-away popup with auto-positioning (PopupNode::Dropdown)
 *  5. Menu           — Context menu / dropdown menu
 *  6. Tooltip        — Non-interactive hover tip (PopupNode::Tooltip)
 *  7. Toast          — Auto-dismiss notification (NotificationStackManager)
 *  8. DragGhost      — Drag preview visual config
 *
 * Each layer type is triggered by a button in a grid layout.
 */

#include "Matcha/Tree/ContainerNode.h"

#include <memory>

namespace matcha::fw {
class Application;
class NotificationStackManager;
class PopupNode;
class DialogNode;
class MenuNode;
} // namespace matcha::fw

namespace nyancad {

/**
 * @brief Build the "Layer Demo" page as a ContainerNode subtree.
 * @param app Application reference for dialog parent etc.
 * @return Root ContainerNode for the demo page.
 */
auto BuildLayerDemoPage(matcha::fw::Application& app)
    -> std::unique_ptr<matcha::fw::ContainerNode>;

} // namespace nyancad
