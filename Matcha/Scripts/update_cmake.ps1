#!/usr/bin/env pwsh
# Update CMakeLists.txt file paths to match v4 directory structure
$ErrorActionPreference = 'Stop'
$cmakePath = Join-Path (Join-Path $PSScriptRoot "..") "CMakeLists.txt"

$content = Get-Content $cmakePath -Raw

# Build replacement map for CMakeLists paths (physical file paths, not include paths)
$replacements = [ordered]@{
    # --- Source/Foundation -> Source/Core ---
    "Source/Foundation/Placeholder.cpp"               = "Source/Core/Placeholder.cpp"

    # --- Source/Widgets/Core -> various ---
    "Source/Widgets/Core/ThemeAware.cpp"              = "Source/Theming/ThemeAware.cpp"
    "Source/Widgets/Core/ThemeServiceGlobal.cpp"      = "Source/Theming/ThemeServiceGlobal.cpp"
    "Source/Widgets/Core/AnimationServiceGlobal.cpp"  = "Source/Animation/AnimationServiceGlobal.cpp"
    "Source/Widgets/Core/MnemonicState.cpp"           = "Source/Interaction/Focus/MnemonicState.cpp"
    "Source/Widgets/Core/MnemonicStateGlobal.cpp"     = "Source/Interaction/Focus/MnemonicStateGlobal.cpp"
    "Include/Matcha/Widgets/Core/MnemonicState.h"     = "Include/Matcha/Interaction/Focus/MnemonicState.h"
    "Source/Widgets/Core/NyanTheme.cpp"               = "Source/Theming/NyanTheme.cpp"
    "Source/Widgets/Core/TonalPaletteGenerator.cpp"   = "Source/Theming/Palette/TonalPaletteGenerator.cpp"
    "Source/Widgets/Core/SvgIconProvider.cpp"         = "Source/Theming/SvgIconProvider.cpp"
    "Source/Widgets/Core/AnimationService.cpp"        = "Source/Animation/AnimationService.cpp"
    "Source/Widgets/Core/SpringAnimation.cpp"         = "Source/Animation/SpringAnimation.cpp"
    "Source/Widgets/Core/StateTransition.cpp"         = "Source/Animation/StateTransition.cpp"
    "Source/Widgets/Core/ContrastChecker.cpp"         = "Source/Theming/Palette/ContrastChecker.cpp"
    "Source/Widgets/Core/A11yAudit.cpp"               = "Source/Theming/A11yAudit.cpp"
    "Source/Widgets/Core/SimpleWidgetEventFilter.cpp" = "Source/Widgets/_Private/SimpleWidgetEventFilter.cpp"
    "Source/Widgets/Core/PushButtonEventFilter.cpp"   = "Source/Widgets/_Private/PushButtonEventFilter.cpp"
    "Source/Widgets/Core/LineEditEventFilter.cpp"     = "Source/Widgets/_Private/LineEditEventFilter.cpp"
    "Source/Widgets/Core/ComboBoxEventFilter.cpp"     = "Source/Widgets/_Private/ComboBoxEventFilter.cpp"
    "Source/Widgets/Core/TooltipEventFilter.cpp"      = "Source/Widgets/_Private/TooltipEventFilter.cpp"
    "Source/Widgets/Core/WhatsThisEventFilter.cpp"    = "Source/Widgets/_Private/WhatsThisEventFilter.cpp"
    "Source/Widgets/Core/FlowLayout.cpp"              = "Source/Widgets/_Private/FlowLayout.cpp"
    "Source/Widgets/Core/UpdateGuard.cpp"             = "Source/Tree/UpdateGuard.cpp"
    "Include/Matcha/Widgets/Core/UpdateGuard.h"       = "Include/Matcha/Tree/UpdateGuard.h"
    "Source/Widgets/Core/DropZoneOverlay.cpp"         = "Source/Interaction/DropZoneOverlay.cpp"
    "Include/Matcha/Widgets/Core/DropZoneOverlay.h"   = "Include/Matcha/Interaction/DropZoneOverlay.h"
    "Include/Matcha/Widgets/Core/IThemeService.h"     = "Include/Matcha/Theming/IThemeService.h"
    "Include/Matcha/Widgets/Core/NyanTheme.h"         = "Include/Matcha/Theming/NyanTheme.h"

    # --- Source/UiNodes/Core -> Source/Event ---
    "Source/UiNodes/Core/BaseObject.cpp"              = "Source/Event/BaseObject.cpp"
    "Source/UiNodes/Core/Notification.cpp"            = "Source/Event/Notification.cpp"
    "Source/UiNodes/Core/EventNode.cpp"               = "Source/Event/EventNode.cpp"
    "Source/UiNodes/Core/CommandNode.cpp"             = "Source/Event/CommandNode.cpp"
    "Source/UiNodes/Core/NotificationQueue.cpp"       = "Source/Event/NotificationQueue.cpp"

    # --- Source/UiNodes/Core -> Source/Tree ---
    "Source/UiNodes/Core/UiNode.cpp"                  = "Source/Tree/UiNode.cpp"
    "Source/UiNodes/Core/ContainerNode.cpp"           = "Source/Tree/ContainerNode.cpp"
    "Source/UiNodes/Core/WidgetNode.cpp"              = "Source/Tree/WidgetNode.cpp"
    "Source/UiNodes/Core/WidgetWrapper.cpp"           = "Source/Tree/WidgetWrapper.cpp"
    "Source/UiNodes/Core/UiNodeQuery.cpp"             = "Source/Tree/UiNodeQuery.cpp"

    # --- Source/UiNodes/Core -> Source/Interaction ---
    "Source/UiNodes/Core/FocusChain.cpp"              = "Source/Interaction/Focus/FocusChain.cpp"
    "Source/UiNodes/Core/FocusManager.cpp"            = "Source/Interaction/Focus/FocusManager.cpp"
    "Source/UiNodes/Core/MnemonicManager.cpp"         = "Source/Interaction/Focus/MnemonicManager.cpp"
    "Source/UiNodes/Core/MnemonicManagerGlobal.cpp"   = "Source/Interaction/Focus/MnemonicManagerGlobal.cpp"
    "Source/UiNodes/Core/SelectionModel.cpp"          = "Source/Interaction/Selection/SelectionModel.cpp"
    "Source/UiNodes/Core/PopupPositioner.cpp"         = "Source/Interaction/PopupPositioner.cpp"
    "Source/UiNodes/Core/ContextMenuComposer.cpp"     = "Source/Interaction/ContextMenuComposer.cpp"
    "Source/UiNodes/Core/ContextMenuRequest.cpp"      = "Source/Interaction/ContextMenuRequest.cpp"
    "Source/UiNodes/Core/ContextualHelpService.cpp"   = "Source/Interaction/ContextualHelpService.cpp"
    "Source/UiNodes/Core/FormValidator.cpp"           = "Source/Interaction/FormValidator.cpp"
    "Source/UiNodes/Core/DragDropVisual.cpp"          = "Source/Interaction/DragDropVisual.cpp"
    "Source/UiNodes/Core/ShortcutManager.cpp"         = "Source/Interaction/Input/ShortcutManager.cpp"
    "Source/UiNodes/Core/KeyboardContract.cpp"        = "Source/Interaction/Input/KeyboardContract.cpp"
    "Source/UiNodes/Core/GestureMotionTracker.cpp"    = "Source/Interaction/Input/GestureMotionTracker.cpp"
    "Source/UiNodes/Core/ScrollPhysics.cpp"           = "Source/Interaction/Input/ScrollPhysics.cpp"

    # --- Source/UiNodes/Core -> Source/Animation ---
    "Source/UiNodes/Core/ChoreographyEngine.cpp"      = "Source/Animation/ChoreographyEngine.cpp"
    "Source/UiNodes/Core/SharedElementTransition.cpp"  = "Source/Animation/SharedElementTransition.cpp"
    "Source/UiNodes/Core/AnimationBlendLayer.cpp"      = "Source/Animation/AnimationBlendLayer.cpp"
    "Source/UiNodes/Core/CompositionTemplate.cpp"      = "Source/Animation/CompositionTemplate.cpp"
    "Source/UiNodes/Core/InteractionTimingRegistry.cpp" = "Source/Animation/InteractionTimingRegistry.cpp"

    # --- Source/UiNodes/Core -> Source/Feedback ---
    "Source/UiNodes/Core/ErrorBoundary.cpp"           = "Source/Feedback/ErrorBoundary.cpp"
    "Source/UiNodes/Core/FeedbackPolicy.cpp"          = "Source/Feedback/FeedbackPolicy.cpp"
    "Source/UiNodes/Core/ContentStateModel.cpp"       = "Source/Feedback/ContentStateModel.cpp"
    "Source/UiNodes/Core/DestructiveActionPolicy.cpp"  = "Source/Feedback/DestructiveActionPolicy.cpp"
    "Source/UiNodes/Core/NotificationStackManager.cpp" = "Source/Feedback/NotificationStackManager.cpp"
    "Source/UiNodes/Core/EdgeCaseGuard.cpp"           = "Source/Feedback/EdgeCaseGuard.cpp"

    # --- Source/UiNodes/Core -> Source/Tree/Layout ---
    "Source/UiNodes/Core/LayoutEngine.cpp"            = "Source/Tree/Layout/LayoutEngine.cpp"
    "Source/UiNodes/Core/BreakpointObserver.cpp"      = "Source/Tree/Layout/BreakpointObserver.cpp"

    # --- Source/UiNodes/Core -> Source/DSL ---
    "Source/UiNodes/Core/Materializer.cpp"            = "Source/DSL/Materializer.cpp"

    # --- Source/UiNodes/Core -> Source/Theming/Token ---
    "Source/UiNodes/Core/TokenRegistryGlobal.cpp"     = "Source/Theming/Token/TokenRegistryGlobal.cpp"
    "Source/UiNodes/Core/DtfmTokenModel.cpp"          = "Source/Theming/Token/DtfmTokenModel.cpp"
    "Source/UiNodes/Core/VariantNameRegistry.cpp"     = "Source/Theming/Token/VariantNameRegistry.cpp"

    # --- Include/Matcha/UiNodes/Core -> Include/Matcha/Event ---
    # (these are listed in CMakeLists as explicit header entries)
    "Include/Matcha/UiNodes/Core/FocusManager.h"     = "Include/Matcha/Interaction/Focus/FocusManager.h"
    "Include/Matcha/UiNodes/Core/MnemonicManager.h"   = "Include/Matcha/Interaction/Focus/MnemonicManager.h"
    "Include/Matcha/UiNodes/Core/UiNode.h"            = "Include/Matcha/Tree/UiNode.h"
    "Include/Matcha/UiNodes/Core/WidgetWrapper.h"     = "Include/Matcha/Tree/WidgetWrapper.h"
    "Include/Matcha/UiNodes/Core/WidgetNode.h"        = "Include/Matcha/Tree/WidgetNode.h"
    "Include/Matcha/UiNodes/Core/GridConstants.h"     = "Include/Matcha/Tree/GridConstants.h"
    "Include/Matcha/UiNodes/Core/WidgetNotification.h" = "Include/Matcha/Tree/WidgetNotification.h"
    "Include/Matcha/UiNodes/Core/ContainerNode.h"     = "Include/Matcha/Tree/ContainerNode.h"
    "Include/Matcha/UiNodes/Core/UiNodeQuery.h"       = "Include/Matcha/Tree/UiNodeQuery.h"
    "Include/Matcha/UiNodes/Core/SelectionModel.h"    = "Include/Matcha/Interaction/Selection/SelectionModel.h"
    "Include/Matcha/UiNodes/Core/PopupPositioner.h"   = "Include/Matcha/Interaction/PopupPositioner.h"

    # --- Include/Matcha/Foundation -> various ---
    "Include/Matcha/Foundation/LayoutEngine.h"           = "Include/Matcha/Tree/Layout/LayoutEngine.h"
    "Include/Matcha/Foundation/ContentStateModel.h"      = "Include/Matcha/Feedback/ContentStateModel.h"
    "Include/Matcha/Foundation/BreakpointObserver.h"     = "Include/Matcha/Tree/Layout/BreakpointObserver.h"
    "Include/Matcha/Foundation/CompositionTemplate.h"    = "Include/Matcha/Animation/CompositionTemplate.h"
    "Include/Matcha/Foundation/InteractionTimingRegistry.h" = "Include/Matcha/Animation/InteractionTimingRegistry.h"
    "Include/Matcha/Foundation/ErrorBoundary.h"          = "Include/Matcha/Feedback/ErrorBoundary.h"
    "Include/Matcha/Foundation/FormValidator.h"          = "Include/Matcha/Interaction/FormValidator.h"
    "Include/Matcha/Foundation/ScrollPhysics.h"          = "Include/Matcha/Interaction/Input/ScrollPhysics.h"
    "Include/Matcha/Foundation/ShortcutManager.h"        = "Include/Matcha/Interaction/Input/ShortcutManager.h"
    "Include/Matcha/Foundation/NotificationStackManager.h" = "Include/Matcha/Feedback/NotificationStackManager.h"
    "Include/Matcha/Foundation/ContextMenuComposer.h"    = "Include/Matcha/Interaction/ContextMenuComposer.h"
    "Include/Matcha/Foundation/ContextualHelpService.h"  = "Include/Matcha/Interaction/ContextualHelpService.h"
    "Include/Matcha/Foundation/DragDropVisual.h"         = "Include/Matcha/Interaction/DragDropVisual.h"
    "Include/Matcha/Foundation/ChoreographyEngine.h"     = "Include/Matcha/Animation/ChoreographyEngine.h"
    "Include/Matcha/Foundation/SharedElementTransition.h" = "Include/Matcha/Animation/SharedElementTransition.h"
    "Include/Matcha/Foundation/GestureMotionTracker.h"   = "Include/Matcha/Interaction/Input/GestureMotionTracker.h"
    "Include/Matcha/Foundation/AnimationBlendLayer.h"    = "Include/Matcha/Animation/AnimationBlendLayer.h"
    "Include/Matcha/Foundation/DtfmTokenModel.h"         = "Include/Matcha/Theming/Token/DtfmTokenModel.h"
    "Include/Matcha/Foundation/VariantNameRegistry.h"    = "Include/Matcha/Theming/Token/VariantNameRegistry.h"
    "Include/Matcha/Foundation/WidgetFsm.h"              = "Include/Matcha/Tree/FSM/WidgetFsm.h"
    "Include/Matcha/Foundation/WidgetFsmBridge.h"        = "Include/Matcha/Tree/FSM/WidgetFsmBridge.h"
    "Include/Matcha/Foundation/KeyboardContract.h"       = "Include/Matcha/Interaction/Input/KeyboardContract.h"
    "Include/Matcha/Foundation/DestructiveActionPolicy.h" = "Include/Matcha/Feedback/DestructiveActionPolicy.h"
    "Include/Matcha/Foundation/EdgeCaseGuard.h"          = "Include/Matcha/Feedback/EdgeCaseGuard.h"
    "Include/Matcha/Foundation/FeedbackPolicy.h"         = "Include/Matcha/Feedback/FeedbackPolicy.h"
    "Include/Matcha/Foundation/FixedString.h"            = "Include/Matcha/Core/FixedString.h"
    "Include/Matcha/Foundation/Blueprint.h"              = "Include/Matcha/DSL/Blueprint.h"

    # --- Source/UiNodes/Controls -> Source/Tree/Controls ---
    "Source/UiNodes/Controls/LineEditNode.cpp"        = "Source/Tree/Controls/LineEditNode.cpp"
    "Source/UiNodes/Controls/ComboBoxNode.cpp"        = "Source/Tree/Controls/ComboBoxNode.cpp"
    "Source/UiNodes/Controls/SpinBoxNode.cpp"         = "Source/Tree/Controls/SpinBoxNode.cpp"
    "Source/UiNodes/Controls/CheckBoxNode.cpp"        = "Source/Tree/Controls/CheckBoxNode.cpp"
    "Source/UiNodes/Controls/ToggleSwitchNode.cpp"    = "Source/Tree/Controls/ToggleSwitchNode.cpp"
    "Source/UiNodes/Controls/PushButtonNode.cpp"      = "Source/Tree/Controls/PushButtonNode.cpp"
    "Source/UiNodes/Controls/ToolButtonNode.cpp"      = "Source/Tree/Controls/ToolButtonNode.cpp"
    "Source/UiNodes/Controls/RadioButtonNode.cpp"     = "Source/Tree/Controls/RadioButtonNode.cpp"
    "Source/UiNodes/Controls/DoubleSpinBoxNode.cpp"   = "Source/Tree/Controls/DoubleSpinBoxNode.cpp"
    "Source/UiNodes/Controls/LabelNode.cpp"           = "Source/Tree/Controls/LabelNode.cpp"
    "Source/UiNodes/Controls/ProgressBarNode.cpp"     = "Source/Tree/Controls/ProgressBarNode.cpp"
    "Source/UiNodes/Controls/SliderNode.cpp"          = "Source/Tree/Controls/SliderNode.cpp"
    "Source/UiNodes/Controls/ColorPickerNode.cpp"     = "Source/Tree/Controls/ColorPickerNode.cpp"
    "Source/UiNodes/Controls/ColorSwatchNode.cpp"     = "Source/Tree/Controls/ColorSwatchNode.cpp"
    "Source/UiNodes/Controls/PlainTextEditNode.cpp"   = "Source/Tree/Controls/PlainTextEditNode.cpp"
    "Source/UiNodes/Controls/LineNode.cpp"            = "Source/Tree/Controls/LineNode.cpp"
    "Source/UiNodes/Controls/BadgeNode.cpp"           = "Source/Tree/Controls/BadgeNode.cpp"
    "Source/UiNodes/Controls/MessageNode.cpp"         = "Source/Tree/Controls/MessageNode.cpp"
    "Source/UiNodes/Controls/ProgressRingNode.cpp"    = "Source/Tree/Controls/ProgressRingNode.cpp"
    "Source/UiNodes/Controls/RangeSliderNode.cpp"     = "Source/Tree/Controls/RangeSliderNode.cpp"
    "Source/UiNodes/Controls/DateTimePickerNode.cpp"  = "Source/Tree/Controls/DateTimePickerNode.cpp"
    "Source/UiNodes/Controls/PaginatorNode.cpp"       = "Source/Tree/Controls/PaginatorNode.cpp"
    "Source/UiNodes/Controls/CollapsibleSectionNode.cpp" = "Source/Tree/Controls/CollapsibleSectionNode.cpp"
    "Source/UiNodes/Controls/NotificationNode.cpp"    = "Source/Tree/Controls/NotificationNode.cpp"
    "Source/UiNodes/Controls/DataTableNode.cpp"       = "Source/Tree/Controls/DataTableNode.cpp"
    "Source/UiNodes/Controls/PropertyGridNode.cpp"    = "Source/Tree/Controls/PropertyGridNode.cpp"
    "Source/UiNodes/Controls/ListWidgetNode.cpp"      = "Source/Tree/Controls/ListWidgetNode.cpp"
    "Source/UiNodes/Controls/TreeItemNode.cpp"        = "Source/Tree/Controls/TreeItemNode.cpp"
    "Source/UiNodes/Controls/TreeWidgetNode.cpp"      = "Source/Tree/Controls/TreeWidgetNode.cpp"
    "Source/UiNodes/Controls/SearchBoxNode.cpp"       = "Source/Tree/Controls/SearchBoxNode.cpp"

    # --- Include/Matcha/UiNodes/Controls -> Include/Matcha/Tree/Controls ---
    "Include/Matcha/UiNodes/Controls/LineEditNode.h"  = "Include/Matcha/Tree/Controls/LineEditNode.h"
    "Include/Matcha/UiNodes/Controls/ComboBoxNode.h"  = "Include/Matcha/Tree/Controls/ComboBoxNode.h"
    "Include/Matcha/UiNodes/Controls/SpinBoxNode.h"   = "Include/Matcha/Tree/Controls/SpinBoxNode.h"
    "Include/Matcha/UiNodes/Controls/CheckBoxNode.h"  = "Include/Matcha/Tree/Controls/CheckBoxNode.h"
    "Include/Matcha/UiNodes/Controls/ToggleSwitchNode.h" = "Include/Matcha/Tree/Controls/ToggleSwitchNode.h"
    "Include/Matcha/UiNodes/Controls/PushButtonNode.h" = "Include/Matcha/Tree/Controls/PushButtonNode.h"
    "Include/Matcha/UiNodes/Controls/ToolButtonNode.h" = "Include/Matcha/Tree/Controls/ToolButtonNode.h"
    "Include/Matcha/UiNodes/Controls/RadioButtonNode.h" = "Include/Matcha/Tree/Controls/RadioButtonNode.h"
    "Include/Matcha/UiNodes/Controls/DoubleSpinBoxNode.h" = "Include/Matcha/Tree/Controls/DoubleSpinBoxNode.h"
    "Include/Matcha/UiNodes/Controls/LabelNode.h"     = "Include/Matcha/Tree/Controls/LabelNode.h"
    "Include/Matcha/UiNodes/Controls/ProgressBarNode.h" = "Include/Matcha/Tree/Controls/ProgressBarNode.h"
    "Include/Matcha/UiNodes/Controls/SliderNode.h"    = "Include/Matcha/Tree/Controls/SliderNode.h"
    "Include/Matcha/UiNodes/Controls/ColorPickerNode.h" = "Include/Matcha/Tree/Controls/ColorPickerNode.h"
    "Include/Matcha/UiNodes/Controls/ColorSwatchNode.h" = "Include/Matcha/Tree/Controls/ColorSwatchNode.h"
    "Include/Matcha/UiNodes/Controls/PlainTextEditNode.h" = "Include/Matcha/Tree/Controls/PlainTextEditNode.h"
    "Include/Matcha/UiNodes/Controls/LineNode.h"      = "Include/Matcha/Tree/Controls/LineNode.h"
    "Include/Matcha/UiNodes/Controls/BadgeNode.h"     = "Include/Matcha/Tree/Controls/BadgeNode.h"
    "Include/Matcha/UiNodes/Controls/MessageNode.h"   = "Include/Matcha/Tree/Controls/MessageNode.h"
    "Include/Matcha/UiNodes/Controls/ProgressRingNode.h" = "Include/Matcha/Tree/Controls/ProgressRingNode.h"
    "Include/Matcha/UiNodes/Controls/RangeSliderNode.h" = "Include/Matcha/Tree/Controls/RangeSliderNode.h"
    "Include/Matcha/UiNodes/Controls/DateTimePickerNode.h" = "Include/Matcha/Tree/Controls/DateTimePickerNode.h"
    "Include/Matcha/UiNodes/Controls/PaginatorNode.h" = "Include/Matcha/Tree/Controls/PaginatorNode.h"
    "Include/Matcha/UiNodes/Controls/CollapsibleSectionNode.h" = "Include/Matcha/Tree/Controls/CollapsibleSectionNode.h"
    "Include/Matcha/UiNodes/Controls/NotificationNode.h" = "Include/Matcha/Tree/Controls/NotificationNode.h"
    "Include/Matcha/UiNodes/Controls/DataTableNode.h" = "Include/Matcha/Tree/Controls/DataTableNode.h"
    "Include/Matcha/UiNodes/Controls/PropertyGridNode.h" = "Include/Matcha/Tree/Controls/PropertyGridNode.h"
    "Include/Matcha/UiNodes/Controls/ListWidgetNode.h" = "Include/Matcha/Tree/Controls/ListWidgetNode.h"
    "Include/Matcha/UiNodes/Controls/TreeItemNode.h"  = "Include/Matcha/Tree/Controls/TreeItemNode.h"
    "Include/Matcha/UiNodes/Controls/TreeWidgetNode.h" = "Include/Matcha/Tree/Controls/TreeWidgetNode.h"
    "Include/Matcha/UiNodes/Controls/SearchBoxNode.h" = "Include/Matcha/Tree/Controls/SearchBoxNode.h"

    # --- Source/UiNodes/Shell -> Source/Tree/Composition/Shell ---
    "Source/UiNodes/Shell/Shell.cpp"                  = "Source/Tree/Composition/Shell/Shell.cpp"
    "Source/UiNodes/Shell/Application.cpp"            = "Source/Tree/Composition/Shell/Application.cpp"
    "Source/UiNodes/Shell/WindowNode.cpp"             = "Source/Tree/Composition/Shell/WindowNode.cpp"
    "Source/UiNodes/Shell/TitleBarNode.cpp"           = "Source/Tree/Composition/Shell/TitleBarNode.cpp"
    "Source/UiNodes/Shell/MainTitleBarNode.cpp"       = "Source/Tree/Composition/Shell/MainTitleBarNode.cpp"
    "Source/UiNodes/Shell/DocumentToolBarNode.cpp"    = "Source/Tree/Composition/Shell/DocumentToolBarNode.cpp"
    "Source/UiNodes/Shell/LogoButtonNode.cpp"         = "Source/Tree/Composition/Shell/LogoButtonNode.cpp"
    "Source/UiNodes/Shell/FloatingTitleBarNode.cpp"   = "Source/Tree/Composition/Shell/FloatingTitleBarNode.cpp"
    "Source/UiNodes/Shell/FloatingWindowNode.cpp"     = "Source/Tree/Composition/Shell/FloatingWindowNode.cpp"
    "Source/UiNodes/Shell/FloatingTabWindowNode.cpp"  = "Source/Tree/Composition/Shell/FloatingTabWindowNode.cpp"
    "Source/UiNodes/Shell/StatusBarNode.cpp"          = "Source/Tree/Composition/Shell/StatusBarNode.cpp"
    "Source/UiNodes/Shell/StatusItemNode.cpp"         = "Source/Tree/Composition/Shell/StatusItemNode.cpp"
    "Source/UiNodes/Shell/WorkspaceFrame.cpp"         = "Source/Tree/Composition/Shell/WorkspaceFrame.cpp"
    "Source/UiNodes/Shell/ControlBar.cpp"             = "Source/Tree/Composition/Shell/ControlBar.cpp"

    # --- Include/Matcha/UiNodes/Shell -> Include/Matcha/Tree/Composition/Shell ---
    "Include/Matcha/UiNodes/Shell/Shell.h"            = "Include/Matcha/Tree/Composition/Shell/Shell.h"
    "Include/Matcha/UiNodes/Shell/Application.h"      = "Include/Matcha/Tree/Composition/Shell/Application.h"
    "Include/Matcha/UiNodes/Shell/WindowNode.h"       = "Include/Matcha/Tree/Composition/Shell/WindowNode.h"
    "Include/Matcha/UiNodes/Shell/TitleBarNode.h"     = "Include/Matcha/Tree/Composition/Shell/TitleBarNode.h"
    "Include/Matcha/UiNodes/Shell/MainTitleBarNode.h" = "Include/Matcha/Tree/Composition/Shell/MainTitleBarNode.h"
    "Include/Matcha/UiNodes/Shell/DocumentToolBarNode.h" = "Include/Matcha/Tree/Composition/Shell/DocumentToolBarNode.h"
    "Include/Matcha/UiNodes/Shell/LogoButtonNode.h"   = "Include/Matcha/Tree/Composition/Shell/LogoButtonNode.h"
    "Include/Matcha/UiNodes/Shell/FloatingTitleBarNode.h" = "Include/Matcha/Tree/Composition/Shell/FloatingTitleBarNode.h"
    "Include/Matcha/UiNodes/Shell/FloatingWindowNode.h" = "Include/Matcha/Tree/Composition/Shell/FloatingWindowNode.h"
    "Include/Matcha/UiNodes/Shell/FloatingTabWindowNode.h" = "Include/Matcha/Tree/Composition/Shell/FloatingTabWindowNode.h"
    "Include/Matcha/UiNodes/Shell/StatusBarNode.h"    = "Include/Matcha/Tree/Composition/Shell/StatusBarNode.h"
    "Include/Matcha/UiNodes/Shell/StatusItemNode.h"   = "Include/Matcha/Tree/Composition/Shell/StatusItemNode.h"
    "Include/Matcha/UiNodes/Shell/WorkspaceFrame.h"   = "Include/Matcha/Tree/Composition/Shell/WorkspaceFrame.h"
    "Include/Matcha/UiNodes/Shell/ControlBar.h"       = "Include/Matcha/Tree/Composition/Shell/ControlBar.h"

    # --- Source/UiNodes/ActionBar -> Source/Tree/Composition/ActionBar ---
    "Source/UiNodes/ActionBar/ActionBarNode.cpp"      = "Source/Tree/Composition/ActionBar/ActionBarNode.cpp"
    "Source/UiNodes/ActionBar/ActionTabNode.cpp"      = "Source/Tree/Composition/ActionBar/ActionTabNode.cpp"
    "Source/UiNodes/ActionBar/ActionToolbarNode.cpp"  = "Source/Tree/Composition/ActionBar/ActionToolbarNode.cpp"
    "Source/UiNodes/ActionBar/ActionButtonNode.cpp"   = "Source/Tree/Composition/ActionBar/ActionButtonNode.cpp"
    "Include/Matcha/UiNodes/ActionBar/ActionBarNode.h" = "Include/Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
    "Include/Matcha/UiNodes/ActionBar/ActionTabNode.h" = "Include/Matcha/Tree/Composition/ActionBar/ActionTabNode.h"
    "Include/Matcha/UiNodes/ActionBar/ActionToolbarNode.h" = "Include/Matcha/Tree/Composition/ActionBar/ActionToolbarNode.h"
    "Include/Matcha/UiNodes/ActionBar/ActionButtonNode.h" = "Include/Matcha/Tree/Composition/ActionBar/ActionButtonNode.h"

    # --- Source/UiNodes/Document -> Source/Tree/Composition/Document ---
    "Source/UiNodes/Document/DocumentPage.cpp"        = "Source/Tree/Composition/Document/DocumentPage.cpp"
    "Source/UiNodes/Document/SplitTreeNode.cpp"       = "Source/Tree/Composition/Document/SplitTreeNode.cpp"
    "Source/UiNodes/Document/ViewportGroup.cpp"       = "Source/Tree/Composition/Document/ViewportGroup.cpp"
    "Source/UiNodes/Document/Viewport.cpp"            = "Source/Tree/Composition/Document/Viewport.cpp"
    "Source/UiNodes/Document/TabBarNode.cpp"          = "Source/Tree/Composition/Document/TabBarNode.cpp"
    "Source/UiNodes/Document/TabItemNode.cpp"         = "Source/Tree/Composition/Document/TabItemNode.cpp"
    "Source/UiNodes/Document/DocumentArea.cpp"        = "Source/Tree/Composition/Document/DocumentArea.cpp"
    "Include/Matcha/UiNodes/Document/DocumentPage.h"  = "Include/Matcha/Tree/Composition/Document/DocumentPage.h"
    "Include/Matcha/UiNodes/Document/SplitTreeNode.h" = "Include/Matcha/Tree/Composition/Document/SplitTreeNode.h"
    "Include/Matcha/UiNodes/Document/ViewportGroup.h" = "Include/Matcha/Tree/Composition/Document/ViewportGroup.h"
    "Include/Matcha/UiNodes/Document/Viewport.h"      = "Include/Matcha/Tree/Composition/Document/Viewport.h"
    "Include/Matcha/UiNodes/Document/TabBarNode.h"    = "Include/Matcha/Tree/Composition/Document/TabBarNode.h"
    "Include/Matcha/UiNodes/Document/TabItemNode.h"   = "Include/Matcha/Tree/Composition/Document/TabItemNode.h"
    "Include/Matcha/UiNodes/Document/DocumentArea.h"  = "Include/Matcha/Tree/Composition/Document/DocumentArea.h"

    # --- Source/UiNodes/Menu -> Source/Tree/Composition/Menu ---
    "Source/UiNodes/Menu/DialogNode.cpp"              = "Source/Tree/Composition/Menu/DialogNode.cpp"
    "Source/UiNodes/Menu/MenuBarNode.cpp"             = "Source/Tree/Composition/Menu/MenuBarNode.cpp"
    "Source/UiNodes/Menu/MenuNode.cpp"                = "Source/Tree/Composition/Menu/MenuNode.cpp"
    "Source/UiNodes/Menu/MenuItemNode.cpp"            = "Source/Tree/Composition/Menu/MenuItemNode.cpp"
    "Include/Matcha/UiNodes/Menu/DialogNode.h"        = "Include/Matcha/Tree/Composition/Menu/DialogNode.h"
    "Include/Matcha/UiNodes/Menu/MenuBarNode.h"       = "Include/Matcha/Tree/Composition/Menu/MenuBarNode.h"
    "Include/Matcha/UiNodes/Menu/MenuNode.h"          = "Include/Matcha/Tree/Composition/Menu/MenuNode.h"
    "Include/Matcha/UiNodes/Menu/MenuItemNode.h"      = "Include/Matcha/Tree/Composition/Menu/MenuItemNode.h"

    # --- Source/UiNodes/Workbench -> Source/Tree/Composition/Workbench ---
    "Source/UiNodes/Workbench/WorkshopRegistry.cpp"   = "Source/Tree/Composition/Workbench/WorkshopRegistry.cpp"
    "Source/UiNodes/Workbench/WorkbenchManager.cpp"   = "Source/Tree/Composition/Workbench/WorkbenchManager.cpp"
    "Include/Matcha/UiNodes/Workbench/WorkshopRegistry.h" = "Include/Matcha/Tree/Composition/Workbench/WorkshopRegistry.h"
    "Include/Matcha/UiNodes/Workbench/WorkbenchTypes.h" = "Include/Matcha/Tree/Composition/Workbench/WorkbenchTypes.h"
    "Include/Matcha/UiNodes/Workbench/WorkbenchManager.h" = "Include/Matcha/Tree/Composition/Workbench/WorkbenchManager.h"

    # --- Tests ---
    "Tests/Unit/Foundation/StrongIdTest.cpp"           = "Tests/Unit/Core/StrongIdTest.cpp"
    "Tests/Unit/Foundation/ErrorCodeTest.cpp"           = "Tests/Unit/Core/ErrorCodeTest.cpp"
    "Tests/Unit/Foundation/ObserverPtrTest.cpp"         = "Tests/Unit/Core/ObserverPtrTest.cpp"
    "Tests/Unit/Foundation/StateMachineTest.cpp"        = "Tests/Unit/Core/StateMachineTest.cpp"
    "Tests/Unit/Foundation/LayoutEngineTest.cpp"        = "Tests/Unit/Tree/LayoutEngineTest.cpp"
    "Tests/Unit/Foundation/ContentStateModelTest.cpp"   = "Tests/Unit/Feedback/ContentStateModelTest.cpp"
    "Tests/Unit/Foundation/BreakpointObserverTest.cpp"  = "Tests/Unit/Tree/BreakpointObserverTest.cpp"
    "Tests/Unit/Foundation/CompositionTemplateTest.cpp" = "Tests/Unit/Animation/CompositionTemplateTest.cpp"
    "Tests/Unit/Foundation/InteractionTimingRegistryTest.cpp" = "Tests/Unit/Animation/InteractionTimingRegistryTest.cpp"
    "Tests/Unit/Foundation/ErrorBoundaryTest.cpp"       = "Tests/Unit/Feedback/ErrorBoundaryTest.cpp"
    "Tests/Unit/Foundation/FormValidatorTest.cpp"       = "Tests/Unit/Interaction/FormValidatorTest.cpp"
    "Tests/Unit/Foundation/ScrollPhysicsTest.cpp"       = "Tests/Unit/Interaction/ScrollPhysicsTest.cpp"
    "Tests/Unit/Foundation/ShortcutManagerTest.cpp"     = "Tests/Unit/Interaction/ShortcutManagerTest.cpp"
    "Tests/Unit/Foundation/NotificationStackManagerTest.cpp" = "Tests/Unit/Feedback/NotificationStackManagerTest.cpp"
    "Tests/Unit/Foundation/ContextMenuComposerTest.cpp" = "Tests/Unit/Interaction/ContextMenuComposerTest.cpp"
    "Tests/Unit/Foundation/ContextualHelpServiceTest.cpp" = "Tests/Unit/Interaction/ContextualHelpServiceTest.cpp"
    "Tests/Unit/Foundation/DragDropVisualTest.cpp"      = "Tests/Unit/Interaction/DragDropVisualTest.cpp"
    "Tests/Unit/Foundation/ChoreographyEngineTest.cpp"  = "Tests/Unit/Animation/ChoreographyEngineTest.cpp"
    "Tests/Unit/Foundation/SharedElementTransitionTest.cpp" = "Tests/Unit/Animation/SharedElementTransitionTest.cpp"
    "Tests/Unit/Foundation/GestureMotionTrackerTest.cpp" = "Tests/Unit/Interaction/GestureMotionTrackerTest.cpp"
    "Tests/Unit/Foundation/AnimationBlendLayerTest.cpp" = "Tests/Unit/Animation/AnimationBlendLayerTest.cpp"
    "Tests/Unit/Foundation/DtcgTokenModelTest.cpp"      = "Tests/Unit/Theming/DtcgTokenModelTest.cpp"
    "Tests/Unit/Foundation/VariantNameRegistryTest.cpp"  = "Tests/Unit/Theming/VariantNameRegistryTest.cpp"
    "Tests/Unit/Foundation/WidgetFsmTest.cpp"           = "Tests/Unit/Tree/WidgetFsmTest.cpp"
    "Tests/Unit/Foundation/KeyboardContractTest.cpp"    = "Tests/Unit/Interaction/KeyboardContractTest.cpp"
    "Tests/Unit/Foundation/PhaseFTest.cpp"              = "Tests/Unit/Feedback/PhaseFTest.cpp"
    "Tests/Unit/Foundation/CrossPhaseIntegrationTest.cpp" = "Tests/Unit/Feedback/CrossPhaseIntegrationTest.cpp"
    "Tests/Unit/Foundation/ObservableTest.cpp"          = "Tests/Unit/Core/ObservableTest.cpp"
    "Tests/Unit/Foundation/BlueprintTest.cpp"           = "Tests/Unit/DSL/BlueprintTest.cpp"
    "Tests/Unit/Foundation/InputEventTest.cpp"          = "Tests/Unit/Core/InputEventTest.cpp"
    "Tests/Unit/Core/CommandNodeTest.cpp"              = "Tests/Unit/Event/CommandNodeTest.cpp"
    "Tests/Unit/Core/DesignTokenTest.cpp"              = "Tests/Unit/Theming/DesignTokenTest.cpp"
    "Tests/Unit/Core/TokenEnumsTest.cpp"               = "Tests/Unit/Theming/TokenEnumsTest.cpp"
    "Tests/Unit/Core/SelectionModelTest.cpp"           = "Tests/Unit/Interaction/SelectionModelTest.cpp"
    "Tests/Unit/Core/PopupPositionerTest.cpp"          = "Tests/Unit/Interaction/PopupPositionerTest.cpp"
    "Tests/Unit/Core/ITokenRegistryTest.cpp"           = "Tests/Unit/Theming/ITokenRegistryTest.cpp"
    "Tests/Unit/Core/TonalPaletteTest.cpp"             = "Tests/Unit/Theming/TonalPaletteTest.cpp"
    "Tests/Unit/Core/WidgetStyleSheetTest.cpp"         = "Tests/Unit/Theming/WidgetStyleSheetTest.cpp"
    "Tests/Unit/Core/PaletteLoadTest.cpp"              = "Tests/Unit/Theming/PaletteLoadTest.cpp"
    "Tests/Unit/Core/MnemonicStateTest.cpp"            = "Tests/Unit/Interaction/MnemonicStateTest.cpp"
    "Tests/Unit/Core/MnemonicManagerTest.cpp"          = "Tests/Unit/Interaction/MnemonicManagerTest.cpp"
    "Tests/Unit/Core/PhaseBTest.cpp"                   = "Tests/Unit/Tree/PhaseBTest.cpp"
    "Tests/Unit/Core/PhaseCTest.cpp"                   = "Tests/Unit/Tree/PhaseCTest.cpp"
    "Tests/Unit/Core/PhaseDTest.cpp"                   = "Tests/Unit/Tree/PhaseDTest.cpp"
    "Tests/Unit/Core/CoreWidgetEnumTest.cpp"           = "Tests/Unit/Tree/CoreWidgetEnumTest.cpp"
    "Tests/Unit/Core/ContainerWidgetEnumTest.cpp"      = "Tests/Unit/Tree/ContainerWidgetEnumTest.cpp"
    "Tests/Unit/Core/ApplicationWidgetEnumTest.cpp"    = "Tests/Unit/Tree/ApplicationWidgetEnumTest.cpp"
    "Tests/Unit/Core/UiNodeTest.cpp"                   = "Tests/Unit/Tree/UiNodeTest.cpp"
    "Tests/Unit/Document/DocumentManagerTest.cpp"      = "Tests/Unit/Services/DocumentManagerTest.cpp"
    "Tests/Unit/Shell/WorkspaceFrameTest.cpp"          = "Tests/Unit/Tree/WorkspaceFrameTest.cpp"
    "Tests/Unit/Document/DocumentAreaTest.cpp"         = "Tests/Unit/Tree/DocumentAreaTest.cpp"
    "Tests/Unit/Document/ViewportGroupTest.cpp"        = "Tests/Unit/Tree/ViewportGroupTest.cpp"
    "Tests/Unit/Document/ViewportBindTest.cpp"         = "Tests/Unit/Tree/ViewportBindTest.cpp"
    "Tests/Unit/Document/ViewportGroupLayoutTest.cpp"  = "Tests/Unit/Tree/ViewportGroupLayoutTest.cpp"
    "Tests/Unit/Document/SplitTreeNodeTest.cpp"        = "Tests/Unit/Tree/SplitTreeNodeTest.cpp"
    "Tests/Unit/Query/UiNodeQueryTest.cpp"             = "Tests/Unit/Tree/UiNodeQueryTest.cpp"
    "Tests/Unit/Workbench/WorkshopRegistryTest.cpp"    = "Tests/Unit/Tree/WorkshopRegistryTest.cpp"
}

$count = 0
foreach ($kv in $replacements.GetEnumerator()) {
    $old = [regex]::Escape($kv.Key)
    if ($content -match $old) {
        $content = $content -replace $old, $kv.Value
        $count++
    }
}

# Also need to add Source/Widgets/_Private as a PRIVATE include dir
# The Widgets that remain under Source/Widgets/ need Source/Widgets/_Private on include path
# Add after the existing target_include_directories for Matcha
$content = $content -replace '(target_include_directories\(Matcha\s+PUBLIC\s+\$<BUILD_INTERFACE:\$\{CMAKE_CURRENT_SOURCE_DIR\}/Include>\s+\$<INSTALL_INTERFACE:include>\s*\))', @'
$1

target_include_directories(Matcha
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
'@

Set-Content -Path $cmakePath -Value $content -NoNewline
Write-Host "Updated $count paths in CMakeLists.txt" -ForegroundColor Green
