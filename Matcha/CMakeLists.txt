cmake_minimum_required(VERSION 3.28)

project(Matcha
    VERSION 0.1.0
    DESCRIPTION "Matcha —— Nyan GUI Framework"
    LANGUAGES CXX
)

# --------------------------------------------------------------------------- #
# Global C++ standard
# --------------------------------------------------------------------------- #
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------------------------- #
# Qt 6
# --------------------------------------------------------------------------- #
find_package(Qt6 6.7 REQUIRED COMPONENTS Widgets Svg Test)
qt_standard_project_setup()

# --------------------------------------------------------------------------- #
# Compiler warnings
# --------------------------------------------------------------------------- #
add_library(Matcha_CompilerWarnings INTERFACE)

if(MSVC)
    target_compile_options(Matcha_CompilerWarnings INTERFACE
        /W4
        $<$<CONFIG:Debug>:/WX>
    )
else()
    target_compile_options(Matcha_CompilerWarnings INTERFACE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-Werror>
    )
endif()

# --------------------------------------------------------------------------- #
# Matcha shared library
# --------------------------------------------------------------------------- #
add_library(Matcha SHARED
    Source/Core/Placeholder.cpp
    Source/Theming/ThemeAware.cpp
    Source/Theming/ThemeServiceGlobal.cpp
    Source/Animation/AnimationServiceGlobal.cpp
    Source/Interaction/Focus/MnemonicState.cpp
    Source/Interaction/Focus/MnemonicStateGlobal.cpp
    Include/Matcha/Interaction/Focus/MnemonicState.h
    Source/Theming/NyanTheme.cpp
    Source/Theming/Palette/TonalPaletteGenerator.cpp
    Source/Theming/SvgIconProvider.cpp
    Source/Animation/AnimationService.cpp
    Source/Animation/SpringAnimation.cpp
    Source/Animation/StateTransition.cpp
    Source/Theming/Palette/ContrastChecker.cpp
    Source/Theming/A11yAudit.cpp
    Source/Widgets/_Private/SimpleWidgetEventFilter.cpp
    Source/Widgets/_Private/PushButtonEventFilter.cpp
    Source/Widgets/_Private/LineEditEventFilter.cpp
    Source/Widgets/_Private/ComboBoxEventFilter.cpp
    Source/Widgets/_Private/TooltipEventFilter.cpp
    Source/Widgets/_Private/WhatsThisEventFilter.cpp
    Source/Widgets/_Private/FlowLayout.cpp
    Source/Widgets/Controls/NyanCheckBox.cpp
    Source/Widgets/Controls/NyanRadioButton.cpp
    Source/Widgets/Controls/NyanComboBox.cpp
    Source/Widgets/Controls/NyanToggleSwitch.cpp
    Include/Matcha/Widgets/Controls/NyanCheckBox.h
    Include/Matcha/Widgets/Controls/NyanRadioButton.h
    Include/Matcha/Widgets/Controls/NyanComboBox.h
    Include/Matcha/Widgets/Controls/NyanToggleSwitch.h
    Source/Widgets/Controls/NyanLabel.cpp
    Source/Widgets/Controls/NyanLine.cpp
    Include/Matcha/Widgets/Controls/NyanLabel.h
    Include/Matcha/Widgets/Controls/NyanLine.h
    Source/Widgets/Controls/NyanPushButton.cpp
    Include/Matcha/Widgets/Controls/NyanPushButton.h
    Source/Widgets/Controls/NyanToolButton.cpp
    Include/Matcha/Widgets/Controls/NyanToolButton.h
    Source/Widgets/Controls/NyanLineEdit.cpp
    Include/Matcha/Widgets/Controls/NyanLineEdit.h
    Source/Widgets/Controls/NyanSpinBox.cpp
    Source/Widgets/Controls/NyanDoubleSpinBox.cpp
    Include/Matcha/Widgets/Controls/NyanSpinBox.h
    Include/Matcha/Widgets/Controls/NyanDoubleSpinBox.h
    Source/Widgets/Controls/NyanSlider.cpp
    Source/Widgets/Controls/NyanRangeSlider.cpp
    Include/Matcha/Widgets/Controls/NyanSlider.h
    Include/Matcha/Widgets/Controls/NyanRangeSlider.h
    Source/Widgets/Controls/NyanBadge.cpp
    Source/Widgets/Controls/NyanBreadcrumb.cpp
    Include/Matcha/Widgets/Controls/NyanBadge.h
    Include/Matcha/Widgets/Controls/NyanBreadcrumb.h
    Source/Widgets/Controls/NyanTag.cpp
    Include/Matcha/Widgets/Controls/NyanTag.h
    Source/Widgets/Controls/NyanProgressBar.cpp
    Source/Widgets/Controls/NyanProgressRing.cpp
    Include/Matcha/Widgets/Controls/NyanProgressBar.h
    Include/Matcha/Widgets/Controls/NyanProgressRing.h
    Source/Widgets/Controls/NyanColorSwatch.cpp
    Source/Widgets/Controls/NyanLegend.cpp
    Source/Widgets/Controls/NyanLegendSlider.cpp
    Include/Matcha/Widgets/Controls/NyanColorSwatch.h
    Include/Matcha/Widgets/Controls/NyanLegend.h
    Include/Matcha/Widgets/Controls/NyanLegendSlider.h
    Source/Widgets/Controls/NyanDateTimePicker.cpp
    Include/Matcha/Widgets/Controls/NyanDateTimePicker.h
    Source/Widgets/Controls/NyanSearchBox.cpp
    Include/Matcha/Widgets/Controls/NyanSearchBox.h
    Source/Widgets/Menu/NyanPopConfirm.cpp
    Include/Matcha/Widgets/Menu/NyanPopConfirm.h
    Source/Widgets/Menu/NyanInputDialog.cpp
    Include/Matcha/Widgets/Menu/NyanInputDialog.h
    Source/Widgets/Controls/NyanRichTooltip.cpp
    Include/Matcha/Widgets/Controls/NyanRichTooltip.h
    Source/Widgets/Shell/NyanScrollBar.cpp
    Include/Matcha/Widgets/Shell/NyanScrollBar.h
    Source/Widgets/Shell/NyanScrollArea.cpp
    Include/Matcha/Widgets/Shell/NyanScrollArea.h
    Source/Widgets/Shell/NyanTabWidget.cpp
    Include/Matcha/Widgets/Shell/NyanTabWidget.h
    Source/Widgets/Shell/NyanStackedWidget.cpp
    Include/Matcha/Widgets/Shell/NyanStackedWidget.h
    Source/Widgets/Shell/NyanSplitter.cpp
    Include/Matcha/Widgets/Shell/NyanSplitter.h
    Source/Widgets/ActionBar/NyanPanel.cpp
    Include/Matcha/Widgets/ActionBar/NyanPanel.h
    Source/Widgets/ActionBar/NyanPanelProgress.cpp
    Include/Matcha/Widgets/ActionBar/NyanPanelProgress.h
    Source/Widgets/Controls/NyanGroupBox.cpp
    Include/Matcha/Widgets/Controls/NyanGroupBox.h
    Source/Widgets/Controls/NyanCollapsibleSection.cpp
    Include/Matcha/Widgets/Controls/NyanCollapsibleSection.h
    Source/Widgets/Controls/NyanDataTable.cpp
    Include/Matcha/Widgets/Controls/NyanDataTable.h
    Source/Widgets/Controls/NyanListWidget.cpp
    Include/Matcha/Widgets/Controls/NyanListWidget.h
    Source/Widgets/Controls/NyanTableWidget.cpp
    Include/Matcha/Widgets/Controls/NyanTableWidget.h
    Source/Widgets/Controls/NyanPaginator.cpp
    Include/Matcha/Widgets/Controls/NyanPaginator.h
    Source/Widgets/Controls/NyanPropertyGrid.cpp
    Include/Matcha/Widgets/Controls/NyanPropertyGrid.h
    Source/Widgets/Controls/NyanColorPicker.cpp
    Include/Matcha/Widgets/Controls/NyanColorPicker.h
    Source/Tree/UpdateGuard.cpp
    Include/Matcha/Tree/UpdateGuard.h
    Source/Widgets/Shell/NyanStatusBar.cpp
    Include/Matcha/Widgets/Shell/NyanStatusBar.h
    Source/Widgets/Shell/NyanTabBar.cpp
    Include/Matcha/Widgets/Shell/NyanTabBar.h
    Source/Widgets/Shell/NyanTabItem.cpp
    Include/Matcha/Widgets/Shell/NyanTabItem.h
    Source/Widgets/Menu/NyanMenuItem.cpp
    Include/Matcha/Widgets/Menu/NyanMenuItem.h
    Source/Widgets/Menu/NyanMenuSeparator.cpp
    Include/Matcha/Widgets/Menu/NyanMenuSeparator.h
    Source/Widgets/Menu/NyanMenuCheckItem.cpp
    Include/Matcha/Widgets/Menu/NyanMenuCheckItem.h
    Source/Widgets/Menu/NyanMenu.cpp
    Include/Matcha/Widgets/Menu/NyanMenu.h
    Source/Widgets/Menu/NyanMenuBar.cpp
    Include/Matcha/Widgets/Menu/NyanMenuBar.h
    Source/Widgets/Controls/NyanNotification.cpp
    Include/Matcha/Widgets/Controls/NyanNotification.h
    Source/Widgets/Controls/NyanSelectionInput.cpp
    Include/Matcha/Widgets/Controls/NyanSelectionInput.h
    Source/Widgets/Controls/NyanStructureTree.cpp
    Include/Matcha/Widgets/Controls/NyanStructureTree.h
    Source/Widgets/Shell/NyanMainTitleBar.cpp
    Include/Matcha/Widgets/Shell/NyanMainTitleBar.h
    Source/Widgets/Shell/NyanLogoButton.cpp
    Include/Matcha/Widgets/Shell/NyanLogoButton.h
    Source/Widgets/Shell/NyanDocumentToolBar.cpp
    Include/Matcha/Widgets/Shell/NyanDocumentToolBar.h
    Source/Widgets/Shell/NyanFloatingTitleBar.cpp
    Include/Matcha/Widgets/Shell/NyanFloatingTitleBar.h
    Source/Widgets/Menu/NyanDialog.cpp
    Include/Matcha/Widgets/Menu/NyanDialog.h
    Source/Widgets/Menu/NyanDialogTitleBar.cpp
    Include/Matcha/Widgets/Menu/NyanDialogTitleBar.h
    Source/Widgets/Menu/NyanDialogFootBar.cpp
    Include/Matcha/Widgets/Menu/NyanDialogFootBar.h
    Source/Widgets/Menu/NyanFileDialog.cpp
    Include/Matcha/Widgets/Menu/NyanFileDialog.h
    Source/Widgets/ActionBar/NyanActionBar.cpp
    Include/Matcha/Widgets/ActionBar/NyanActionBar.h
    Source/Widgets/Shell/NyanVerticalTabBar.cpp
    Include/Matcha/Widgets/Shell/NyanVerticalTabBar.h
    Source/Widgets/ActionBar/ActionBarFloatingFrame.cpp
    Include/Matcha/Widgets/ActionBar/ActionBarFloatingFrame.h
    Source/Widgets/ActionBar/TrapezoidHandle.cpp
    Include/Matcha/Widgets/ActionBar/TrapezoidHandle.h
    Source/Widgets/ActionBar/NyanActionTab.cpp
    Include/Matcha/Widgets/ActionBar/NyanActionTab.h
    Source/Widgets/ActionBar/NyanActionToolbar.cpp
    Include/Matcha/Widgets/ActionBar/NyanActionToolbar.h
    Source/Widgets/Shell/ViewportWidget.cpp
    Include/Matcha/Widgets/Shell/ViewportWidget.h
    Source/Widgets/Shell/ViewportHeaderBar.cpp
    Include/Matcha/Widgets/Shell/ViewportHeaderBar.h
    Source/Interaction/DropZoneOverlay.cpp
    Include/Matcha/Interaction/DropZoneOverlay.h
    Source/Widgets/Shell/ViewportFrame.cpp
    Include/Matcha/Widgets/Shell/ViewportFrame.h
    Source/Event/BaseObject.cpp
    Source/Event/Notification.cpp
    Source/Event/EventNode.cpp
    Source/Event/CommandNode.cpp
    Source/Event/NotificationQueue.cpp
    Source/Tree/UiNode.cpp
    Source/Interaction/Focus/FocusChain.cpp
    Source/Interaction/Focus/FocusManager.cpp
    Include/Matcha/Interaction/Focus/FocusManager.h
    Source/Interaction/Focus/MnemonicManager.cpp
    Source/Interaction/Focus/MnemonicManagerGlobal.cpp
    Include/Matcha/Interaction/Focus/MnemonicManager.h
    Include/Matcha/Tree/UiNode.h
    Source/Tree/WidgetWrapper.cpp
    Include/Matcha/Tree/WidgetWrapper.h
    Source/Tree/WidgetNode.cpp
    Include/Matcha/Tree/WidgetNode.h
    Source/DSL/Materializer.cpp
    Include/Matcha/DSL/Blueprint.h
    Include/Matcha/Tree/GridConstants.h
    Source/Tree/Controls/LineEditNode.cpp
    Include/Matcha/Tree/Controls/LineEditNode.h
    Source/Tree/Controls/ComboBoxNode.cpp
    Include/Matcha/Tree/Controls/ComboBoxNode.h
    Source/Tree/Controls/SpinBoxNode.cpp
    Include/Matcha/Tree/Controls/SpinBoxNode.h
    Source/Tree/Controls/CheckBoxNode.cpp
    Include/Matcha/Tree/Controls/CheckBoxNode.h
    Source/Tree/Controls/ToggleSwitchNode.cpp
    Include/Matcha/Tree/Controls/ToggleSwitchNode.h
    Source/Tree/Controls/PushButtonNode.cpp
    Include/Matcha/Tree/Controls/PushButtonNode.h
    Source/Tree/Controls/ToolButtonNode.cpp
    Include/Matcha/Tree/Controls/ToolButtonNode.h
    Source/Tree/Controls/RadioButtonNode.cpp
    Include/Matcha/Tree/Controls/RadioButtonNode.h
    Source/Tree/Controls/DoubleSpinBoxNode.cpp
    Include/Matcha/Tree/Controls/DoubleSpinBoxNode.h
    Include/Matcha/Tree/WidgetNotification.h
    Source/Tree/Controls/LabelNode.cpp
    Include/Matcha/Tree/Controls/LabelNode.h
    Source/Tree/Controls/ProgressBarNode.cpp
    Include/Matcha/Tree/Controls/ProgressBarNode.h
    Source/Tree/Controls/SliderNode.cpp
    Include/Matcha/Tree/Controls/SliderNode.h
    Source/Tree/Controls/ColorPickerNode.cpp
    Include/Matcha/Tree/Controls/ColorPickerNode.h
    Source/Tree/Controls/ColorSwatchNode.cpp
    Include/Matcha/Tree/Controls/ColorSwatchNode.h
    Source/Tree/Controls/PlainTextEditNode.cpp
    Include/Matcha/Tree/Controls/PlainTextEditNode.h
    Source/Tree/Controls/LineNode.cpp
    Include/Matcha/Tree/Controls/LineNode.h
    Source/Tree/Controls/BadgeNode.cpp
    Include/Matcha/Tree/Controls/BadgeNode.h
    Source/Tree/Controls/MessageNode.cpp
    Include/Matcha/Tree/Controls/MessageNode.h
    Source/Tree/Controls/ProgressRingNode.cpp
    Include/Matcha/Tree/Controls/ProgressRingNode.h
    Source/Tree/Controls/RangeSliderNode.cpp
    Include/Matcha/Tree/Controls/RangeSliderNode.h
    Source/Tree/Controls/DateTimePickerNode.cpp
    Include/Matcha/Tree/Controls/DateTimePickerNode.h
    Source/Tree/Controls/PaginatorNode.cpp
    Include/Matcha/Tree/Controls/PaginatorNode.h
    Source/Tree/Controls/CollapsibleSectionNode.cpp
    Include/Matcha/Tree/Controls/CollapsibleSectionNode.h
    Source/Tree/Controls/NotificationNode.cpp
    Include/Matcha/Tree/Controls/NotificationNode.h
    Source/Tree/Controls/ToastStackNode.cpp
    Include/Matcha/Tree/Controls/ToastStackNode.h
    Source/Widgets/Controls/DragGhostOverlay.cpp
    Include/Matcha/Widgets/Controls/DragGhostOverlay.h
    Source/Tree/Composition/Shell/Shell.cpp
    Include/Matcha/Tree/Composition/Shell/Shell.h
    Source/Tree/Composition/Shell/Application.cpp
    Include/Matcha/Tree/Composition/Shell/Application.h
    Source/Tree/Composition/Shell/WindowNode.cpp
    Include/Matcha/Tree/Composition/Shell/WindowNode.h
    Source/Tree/Composition/Shell/TitleBarNode.cpp
    Include/Matcha/Tree/Composition/Shell/TitleBarNode.h
    Source/Tree/Composition/Shell/MainTitleBarNode.cpp
    Include/Matcha/Tree/Composition/Shell/MainTitleBarNode.h
    Source/Tree/Composition/Shell/DocumentToolBarNode.cpp
    Include/Matcha/Tree/Composition/Shell/DocumentToolBarNode.h
    Source/Tree/Composition/Shell/LogoButtonNode.cpp
    Include/Matcha/Tree/Composition/Shell/LogoButtonNode.h
    Source/Tree/Composition/Shell/FloatingTitleBarNode.cpp
    Include/Matcha/Tree/Composition/Shell/FloatingTitleBarNode.h
    Source/Tree/Composition/Document/TabBarNode.cpp
    Include/Matcha/Tree/Composition/Document/TabBarNode.h
    Source/Tree/Composition/Document/TabItemNode.cpp
    Include/Matcha/Tree/Composition/Document/TabItemNode.h
    Source/Tree/Composition/Shell/FloatingWindowNode.cpp
    Include/Matcha/Tree/Composition/Shell/FloatingWindowNode.h
    Source/Tree/Composition/Shell/FloatingTabWindowNode.cpp
    Include/Matcha/Tree/Composition/Shell/FloatingTabWindowNode.h
    Include/Matcha/Services/IDocumentManager.h
    Source/Services/DocumentManager.cpp
    Include/Matcha/Services/DocumentManager.h
    Source/Tree/Composition/ActionBar/ActionBarNode.cpp
    Include/Matcha/Tree/Composition/ActionBar/ActionBarNode.h
    Source/Tree/Composition/Shell/StatusBarNode.cpp
    Include/Matcha/Tree/Composition/Shell/StatusBarNode.h
    Source/Tree/Composition/Shell/StatusItemNode.cpp
    Include/Matcha/Tree/Composition/Shell/StatusItemNode.h
    Source/Tree/Composition/ActionBar/ActionTabNode.cpp
    Include/Matcha/Tree/Composition/ActionBar/ActionTabNode.h
    Source/Tree/Composition/ActionBar/ActionToolbarNode.cpp
    Include/Matcha/Tree/Composition/ActionBar/ActionToolbarNode.h
    Source/Tree/Composition/ActionBar/ActionButtonNode.cpp
    Include/Matcha/Tree/Composition/ActionBar/ActionButtonNode.h
    Source/Tree/Composition/Workbench/WorkshopRegistry.cpp
    Include/Matcha/Tree/Composition/Workbench/WorkshopRegistry.h
    Include/Matcha/Tree/Composition/Workbench/WorkbenchTypes.h
    Source/Tree/Composition/Workbench/WorkbenchManager.cpp
    Include/Matcha/Tree/Composition/Workbench/WorkbenchManager.h
    Source/Theming/Token/TokenRegistryGlobal.cpp
    Source/Tree/ContainerNode.cpp
    Include/Matcha/Tree/ContainerNode.h
    Source/Tree/PopupNode.cpp
    Include/Matcha/Tree/PopupNode.h
    Source/Tree/Composition/Menu/DialogNode.cpp
    Include/Matcha/Tree/Composition/Menu/DialogNode.h
    Source/Tree/Composition/Menu/PopConfirmNode.cpp
    Include/Matcha/Tree/Composition/Menu/PopConfirmNode.h
    Source/Interaction/ContextMenuRequest.cpp
    Source/Tree/Composition/Menu/MenuBarNode.cpp
    Include/Matcha/Tree/Composition/Menu/MenuBarNode.h
    Source/Tree/Composition/Menu/MenuNode.cpp
    Include/Matcha/Tree/Composition/Menu/MenuNode.h
    Source/Tree/Composition/Menu/MenuItemNode.cpp
    Include/Matcha/Tree/Composition/Menu/MenuItemNode.h
    Source/Tree/Controls/DataTableNode.cpp
    Include/Matcha/Tree/Controls/DataTableNode.h
    Source/Tree/Controls/PropertyGridNode.cpp
    Include/Matcha/Tree/Controls/PropertyGridNode.h
    Source/Tree/Controls/ListWidgetNode.cpp
    Include/Matcha/Tree/Controls/ListWidgetNode.h
    Source/Tree/Controls/TreeItemNode.cpp
    Include/Matcha/Tree/Controls/TreeItemNode.h
    Source/Tree/Controls/TreeWidgetNode.cpp
    Include/Matcha/Tree/Controls/TreeWidgetNode.h
    Source/Tree/Controls/SearchBoxNode.cpp
    Include/Matcha/Tree/Controls/SearchBoxNode.h
    Source/Widgets/Controls/NyanCascader.cpp
    Include/Matcha/Widgets/Controls/NyanCascader.h
    Source/Widgets/Controls/NyanTransfer.cpp
    Include/Matcha/Widgets/Controls/NyanTransfer.h
    Source/Widgets/Controls/NyanFormLayout.cpp
    Include/Matcha/Widgets/Controls/NyanFormLayout.h
    Source/Widgets/Controls/NyanMessage.cpp
    Include/Matcha/Widgets/Controls/NyanMessage.h
    Source/Widgets/Controls/NyanAlert.cpp
    Include/Matcha/Widgets/Controls/NyanAlert.h
    Source/Widgets/Controls/NyanAvatar.cpp
    Include/Matcha/Widgets/Controls/NyanAvatar.h
    Include/Matcha/Theming/IThemeService.h
    Include/Matcha/Theming/NyanTheme.h
    Source/Tree/Composition/Document/DocumentPage.cpp
    Include/Matcha/Tree/Composition/Document/DocumentPage.h
    Source/Tree/Composition/Document/SplitTreeNode.cpp
    Include/Matcha/Tree/Composition/Document/SplitTreeNode.h
    Source/Tree/Composition/Document/ViewportGroup.cpp
    Include/Matcha/Tree/Composition/Document/ViewportGroup.h
    Source/Tree/Composition/Document/Viewport.cpp
    Include/Matcha/Tree/Composition/Document/Viewport.h
    Source/Tree/UiNodeQuery.cpp
    Include/Matcha/Tree/UiNodeQuery.h
    Source/Interaction/Selection/SelectionModel.cpp
    Include/Matcha/Interaction/Selection/SelectionModel.h
    Source/Interaction/PopupPositioner.cpp
    Include/Matcha/Interaction/PopupPositioner.h
    Source/Tree/Layout/LayoutEngine.cpp
    Include/Matcha/Tree/Layout/LayoutEngine.h
    Source/Feedback/ContentStateModel.cpp
    Include/Matcha/Feedback/ContentStateModel.h
    Source/Tree/Layout/BreakpointObserver.cpp
    Include/Matcha/Tree/Layout/BreakpointObserver.h
    Source/Animation/CompositionTemplate.cpp
    Include/Matcha/Animation/CompositionTemplate.h
    Source/Animation/InteractionTimingRegistry.cpp
    Include/Matcha/Animation/InteractionTimingRegistry.h
    Source/Feedback/ErrorBoundary.cpp
    Include/Matcha/Feedback/ErrorBoundary.h
    Source/Interaction/FormValidator.cpp
    Include/Matcha/Interaction/FormValidator.h
    Source/Interaction/Input/ScrollPhysics.cpp
    Include/Matcha/Interaction/Input/ScrollPhysics.h
    Source/Interaction/Input/ShortcutManager.cpp
    Include/Matcha/Interaction/Input/ShortcutManager.h
    Source/Feedback/NotificationStackManager.cpp
    Include/Matcha/Feedback/NotificationStackManager.h
    Source/Interaction/ContextMenuComposer.cpp
    Include/Matcha/Interaction/ContextMenuComposer.h
    Source/Interaction/ContextualHelpService.cpp
    Include/Matcha/Interaction/ContextualHelpService.h
    Source/Interaction/DragDropVisual.cpp
    Include/Matcha/Interaction/DragDropVisual.h
    Source/Animation/ChoreographyEngine.cpp
    Include/Matcha/Animation/ChoreographyEngine.h
    Source/Animation/SharedElementTransition.cpp
    Include/Matcha/Animation/SharedElementTransition.h
    Source/Interaction/Input/GestureMotionTracker.cpp
    Include/Matcha/Interaction/Input/GestureMotionTracker.h
    Source/Animation/AnimationBlendLayer.cpp
    Include/Matcha/Animation/AnimationBlendLayer.h
    Source/Theming/Token/DtcgTokenModel.cpp
    Include/Matcha/Theming/Token/DtcgTokenModel.h
    Source/Theming/Token/VariantNameRegistry.cpp
    Include/Matcha/Theming/Token/VariantNameRegistry.h
    Include/Matcha/Tree/FSM/WidgetFsm.h
    Include/Matcha/Tree/FSM/WidgetFsmBridge.h
    Source/Interaction/Input/KeyboardContract.cpp
    Include/Matcha/Interaction/Input/KeyboardContract.h
    Source/Feedback/DestructiveActionPolicy.cpp
    Include/Matcha/Feedback/DestructiveActionPolicy.h
    Source/Feedback/EdgeCaseGuard.cpp
    Include/Matcha/Feedback/EdgeCaseGuard.h
    Source/Feedback/FeedbackPolicy.cpp
    Include/Matcha/Feedback/FeedbackPolicy.h
    Include/Matcha/Core/FixedString.h
    Source/Tree/Composition/Document/DocumentArea.cpp
    Include/Matcha/Tree/Composition/Document/DocumentArea.h
    Source/Tree/Composition/Shell/WorkspaceFrame.cpp
    Include/Matcha/Tree/Composition/Shell/WorkspaceFrame.h
    Source/Tree/Composition/Shell/ControlBar.cpp
    Include/Matcha/Tree/Composition/Shell/ControlBar.h
    Include/Matcha/Services/IExpansionPlugin.h
    Include/Matcha/Services/IViewportRenderer.h
    Include/Matcha/Services/PluginHost.h
    Source/Services/PluginHost.cpp
    Include/Matcha/CApi/NyanCApi.h
    Source/CApi/NyanCApi.cpp
)

target_include_directories(Matcha
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
        $<INSTALL_INTERFACE:include>
)

target_include_directories(Matcha
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_compile_definitions(Matcha PRIVATE MATCHA_LIB_EXPORT)

target_link_libraries(Matcha
    PUBLIC
        Qt6::Widgets
    PRIVATE
        Qt6::Svg
        Matcha_CompilerWarnings
)

# --------------------------------------------------------------------------- #
# NyanCad demo executable
# --------------------------------------------------------------------------- #
add_executable(NyanCad
    Demos/NyanCad/main.cpp
    Demos/NyanCad/NyanCadApp.h
    Demos/NyanCad/NyanCadApp.cpp
    Demos/NyanCad/NyanCadMainWindow.h
    Demos/NyanCad/NyanCadMainWindow.cpp
    Demos/NyanCad/NyanCadWorkshopSetup.h
    Demos/NyanCad/NyanCadWorkshopSetup.cpp
    Demos/NyanCad/LayerDemo.h
    Demos/NyanCad/LayerDemo.cpp
    Demos/NyanCad/DocumentView.h
    Demos/NyanCad/DocumentView.cpp
    Demos/NyanCad/NyanCadDocument.h
    Demos/NyanCad/NyanCadDocument.cpp
    Demos/NyanCad/MeshEditors/NyanFwSurfaceMeshEditor.h
    Demos/NyanCad/MeshEditors/NyanFwSurfaceMeshEditor.cpp
    Demos/NyanCad/MeshEditors/NyanFwTetMeshEditor.h
    Demos/NyanCad/MeshEditors/NyanFwTetMeshEditor.cpp
    Demos/NyanCad/MeshEditors/NyanFwSketchEditor.h
    Demos/NyanCad/MeshEditors/NyanFwSketchEditor.cpp
    Demos/NyanCad/DetectionFreedomEdge.h
    Demos/NyanCad/DetectionFreedomEdge.cpp
    Demos/NyanCad/DetectionInterference.h
    Demos/NyanCad/DetectionInterference.cpp
    Demos/NyanCad/DetectionRepeat.h
    Demos/NyanCad/DetectionRepeat.cpp
    Demos/NyanCad/DetectionFailureArea.h
    Demos/NyanCad/DetectionFailureArea.cpp
    Demos/NyanCad/Diagnostics/LayoutBoundsOverlay.h
    Demos/NyanCad/Diagnostics/LayoutBoundsOverlay.cpp
    Demos/NyanCad/Diagnostics/WidgetPickerFilter.h
    Demos/NyanCad/Diagnostics/WidgetPickerFilter.cpp
    Demos/NyanCad/Diagnostics/NyanCadDevToolsWindow.h
    Demos/NyanCad/Diagnostics/NyanCadDevToolsWindow.cpp
)

target_include_directories(NyanCad PRIVATE
    Include
    Demos/NyanCad
)

target_compile_definitions(NyanCad PRIVATE
    MATCHA_PALETTE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources/Themes"
    MATCHA_PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}/Plugins"
)

target_link_libraries(NyanCad
    PRIVATE
        Matcha
        Qt6::Widgets
        Matcha_CompilerWarnings
)

# --------------------------------------------------------------------------- #
# Deploy Qt runtime DLLs next to executables (Windows only, dev convenience)
# --------------------------------------------------------------------------- #
if(WIN32)
    # Copy Qt DLLs + plugins needed at runtime into the build output directory.
    # Uses qt_generate_deploy_app_script (Qt 6.5+) for correct platform/plugin resolution.
    qt_generate_deploy_app_script(
        TARGET NyanCad
        OUTPUT_SCRIPT NYANCAD_DEPLOY_SCRIPT
        NO_UNSUPPORTED_PLATFORM_ERROR
        NO_TRANSLATIONS
    )
    install(SCRIPT ${NYANCAD_DEPLOY_SCRIPT})

    # For F5/debug convenience: post-build copy of essential Qt DLLs via $<TARGET_RUNTIME_DLLS>
    add_custom_command(TARGET NyanCad POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:NyanCad> $<TARGET_FILE_DIR:NyanCad>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying Qt runtime DLLs to build output directory..."
    )
endif()

# --------------------------------------------------------------------------- #
# NyanCad plugin DLLs (6 expansion plugins)
# --------------------------------------------------------------------------- #
function(add_nyancad_plugin TARGET_NAME PLUGIN_DIR PLUGIN_SOURCE)
    add_library(${TARGET_NAME} SHARED
        Demos/NyanCad/Plugins/${PLUGIN_DIR}/${PLUGIN_SOURCE}.h
        Demos/NyanCad/Plugins/${PLUGIN_DIR}/${PLUGIN_SOURCE}.cpp
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        Demos/NyanCad/Plugins/${PLUGIN_DIR}
        Include
    )
    target_link_libraries(${TARGET_NAME} PRIVATE Matcha Matcha_CompilerWarnings)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Plugins"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Plugins"
    )
endfunction()

add_nyancad_plugin(NyanSurfaceMeshPlugin       NyanSurfaceMesh            NyanSurfaceMeshPlugin)
add_nyancad_plugin(NyanMeshEditPlugin          NyanMeshEdit               NyanMeshEditPlugin)
add_nyancad_plugin(NyanMeshVerifRepairPlugin   NyanMeshVerificationRepair NyanMeshVerificationRepairPlugin)
add_nyancad_plugin(NyanTetrahedronMeshPlugin   NyanTetrahedronMesh        NyanTetrahedronMeshPlugin)
add_nyancad_plugin(NyanEntityBuildPlugin       NyanEntityBuild            NyanEntityBuildPlugin)
add_nyancad_plugin(NyanOneDimMeshPlugin        NyanOneDimensionalMesh     NyanOneDimensionalMeshPlugin)

# --------------------------------------------------------------------------- #
# Mock plugin shared library (for plugin system tests)
# --------------------------------------------------------------------------- #
add_library(MatchaMockPlugin SHARED
    Tests/TestUtils/MockPlugin.cpp
    Tests/TestUtils/MockPlugin.h
)

target_include_directories(MatchaMockPlugin PRIVATE
    Tests/TestUtils
    Include
)

target_link_libraries(MatchaMockPlugin
    PRIVATE
        Matcha
        Matcha_CompilerWarnings
)

# --------------------------------------------------------------------------- #
# Testing
# --------------------------------------------------------------------------- #
enable_testing()

# -- MatchaUnitTests (doctest only, no Qt event loop) ---------------------- #
add_executable(MatchaUnitTests
    Tests/Unit/main.cpp
    Tests/Unit/Core/StrongIdTest.cpp
    Tests/Unit/Core/ErrorCodeTest.cpp
    Tests/Unit/Core/ObserverPtrTest.cpp
    Tests/Unit/Core/StateMachineTest.cpp
    Tests/Unit/Tree/LayoutEngineTest.cpp
    Tests/Unit/Feedback/ContentStateModelTest.cpp
    Tests/Unit/Tree/BreakpointObserverTest.cpp
    Tests/Unit/Animation/CompositionTemplateTest.cpp
    Tests/Unit/Animation/InteractionTimingRegistryTest.cpp
    Tests/Unit/Feedback/ErrorBoundaryTest.cpp
    Tests/Unit/Interaction/FormValidatorTest.cpp
    Tests/Unit/Interaction/ScrollPhysicsTest.cpp
    Tests/Unit/Interaction/ShortcutManagerTest.cpp
    Tests/Unit/Feedback/NotificationStackManagerTest.cpp
    Tests/Unit/Interaction/ContextMenuComposerTest.cpp
    Tests/Unit/Interaction/ContextualHelpServiceTest.cpp
    Tests/Unit/Interaction/DragDropVisualTest.cpp
    Tests/Unit/Animation/ChoreographyEngineTest.cpp
    Tests/Unit/Animation/SharedElementTransitionTest.cpp
    Tests/Unit/Interaction/GestureMotionTrackerTest.cpp
    Tests/Unit/Animation/AnimationBlendLayerTest.cpp
    Tests/Unit/Theming/DtcgTokenModelTest.cpp
    Tests/Unit/Theming/VariantNameRegistryTest.cpp
    Tests/Unit/Tree/WidgetFsmTest.cpp
    Tests/Unit/Interaction/KeyboardContractTest.cpp
    Tests/Unit/Feedback/PhaseFTest.cpp
    Tests/Unit/Feedback/CrossPhaseIntegrationTest.cpp
    Tests/Unit/Core/ObservableTest.cpp
    Tests/Unit/DSL/BlueprintTest.cpp
    Tests/Unit/Event/CommandNodeTest.cpp
    Tests/Unit/Theming/DesignTokenTest.cpp
    Tests/Unit/Theming/TokenEnumsTest.cpp
    Tests/Unit/Interaction/SelectionModelTest.cpp
    Tests/Unit/Interaction/PopupPositionerTest.cpp
    Tests/Unit/Tree/PhaseBTest.cpp
    Tests/Unit/Tree/PhaseCTest.cpp
    Tests/Unit/Tree/PhaseDTest.cpp
    Tests/Unit/Theming/ITokenRegistryTest.cpp
    Tests/Unit/Theming/TonalPaletteTest.cpp
    Tests/Unit/Theming/WidgetStyleSheetTest.cpp
    Tests/Unit/Theming/PaletteLoadTest.cpp
    Tests/Unit/Tree/CoreWidgetEnumTest.cpp
    Tests/Unit/Tree/ContainerWidgetEnumTest.cpp
    Tests/Unit/Tree/ApplicationWidgetEnumTest.cpp
    Tests/Unit/Tree/UiNodeTest.cpp
    Tests/Unit/Services/DocumentManagerTest.cpp
    Tests/Unit/Tree/WorkspaceFrameTest.cpp
    Tests/Unit/Tree/DocumentAreaTest.cpp
    Tests/Unit/Tree/ViewportGroupTest.cpp
    Tests/Unit/Core/InputEventTest.cpp
    Tests/Unit/Tree/ViewportBindTest.cpp
    Tests/Unit/Tree/ViewportGroupLayoutTest.cpp
    Tests/Unit/Tree/SplitTreeNodeTest.cpp
    Tests/Unit/Services/PluginHostTest.cpp
    Tests/Unit/CApi/CApiRoundTripTest.cpp
    Tests/Unit/CApi/CApiSafetyTest.cpp
    Tests/Unit/Tree/UiNodeQueryTest.cpp
    Tests/Unit/Tree/WorkshopRegistryTest.cpp
    Tests/Unit/Interaction/MnemonicStateTest.cpp
    Tests/Unit/Interaction/MnemonicManagerTest.cpp
    Tests/Unit/Tree/PopupNodeTest.cpp
)

target_include_directories(MatchaUnitTests PRIVATE
    Tests/TestUtils
    Include
    Source
)

target_compile_definitions(MatchaUnitTests PRIVATE
    MATCHA_TEST_PALETTE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources/Themes"
    MATCHA_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Tests/Fixtures"
    MATCHA_TEST_MOCK_PLUGIN_PATH="$<TARGET_FILE:MatchaMockPlugin>"
)

target_link_libraries(MatchaUnitTests
    PRIVATE
        Matcha
        Qt6::Widgets
        Matcha_CompilerWarnings
)

add_test(NAME MatchaUnitTests COMMAND MatchaUnitTests)
set_tests_properties(MatchaUnitTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;PATH=${CMAKE_PREFIX_PATH}/bin\;${CMAKE_CURRENT_BINARY_DIR}\;$ENV{PATH}"
)

# -- MatchaWidgetTests (Qt Test + WidgetTestFixture) ----------------------- #
add_executable(MatchaWidgetTests
    Tests/Widget/main.cpp
    Tests/Widget/MenuBarTest.h
    Tests/Widget/MenuBarTest.cpp
    Tests/Widget/ViewportGroupWidgetTest.h
    Tests/Widget/ViewportFrameTest.h
)

target_include_directories(MatchaWidgetTests PRIVATE
    Tests/TestUtils
    Tests/Widget
    Include
)

target_compile_definitions(MatchaWidgetTests PRIVATE
    MATCHA_TEST_PALETTE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources/Themes"
)

target_link_libraries(MatchaWidgetTests
    PRIVATE
        Matcha
        Qt6::Widgets
        Qt6::Test
        Matcha_CompilerWarnings
)

add_test(NAME MatchaWidgetTests COMMAND MatchaWidgetTests)
set_tests_properties(MatchaWidgetTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;PATH=${CMAKE_PREFIX_PATH}/bin\;${CMAKE_CURRENT_BINARY_DIR}\;$ENV{PATH}"
)

# -- MatchaIntegrationTests (doctest + Qt) --------------------------------- #
add_executable(MatchaIntegrationTests
    Tests/Integration/main.cpp
    Tests/Integration/DocumentLifecycleTest.cpp
    Tests/Integration/ShellWiringTest.cpp
    Tests/Integration/ActionBarDockTest.cpp
    Tests/Integration/PluginIntegrationTest.cpp
    Tests/Integration/CApiIntegrationTest.cpp
    Tests/Integration/NyanCadDocumentTest.cpp
    Tests/Integration/NyanCadSmokeTest.cpp
    Tests/Integration/NyanCadPluginTest.cpp
    Tests/Integration/WidgetTreeTraverserTest.cpp
    Tests/Integration/WorkbenchManagerTest.cpp
    Demos/NyanCad/NyanCadDocument.h
    Demos/NyanCad/NyanCadDocument.cpp
    Demos/NyanCad/NyanCadMainWindow.h
    Demos/NyanCad/NyanCadMainWindow.cpp
    Demos/NyanCad/NyanCadWorkshopSetup.h
    Demos/NyanCad/NyanCadWorkshopSetup.cpp
    Demos/NyanCad/LayerDemo.h
    Demos/NyanCad/LayerDemo.cpp
    Demos/NyanCad/DocumentView.h
    Demos/NyanCad/DocumentView.cpp
    Demos/NyanCad/DetectionFreedomEdge.h
    Demos/NyanCad/DetectionFreedomEdge.cpp
    Demos/NyanCad/DetectionInterference.h
    Demos/NyanCad/DetectionInterference.cpp
    Demos/NyanCad/DetectionRepeat.h
    Demos/NyanCad/DetectionRepeat.cpp
    Demos/NyanCad/DetectionFailureArea.h
    Demos/NyanCad/DetectionFailureArea.cpp
)

target_include_directories(MatchaIntegrationTests PRIVATE
    Tests/TestUtils
    Include
    Demos/NyanCad
)

target_compile_definitions(MatchaIntegrationTests PRIVATE
    MATCHA_TEST_PALETTE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources/Themes"
    MATCHA_TEST_MOCK_PLUGIN_PATH="$<TARGET_FILE:MatchaMockPlugin>"
    MATCHA_TEST_PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}/Plugins"
)

target_link_libraries(MatchaIntegrationTests
    PRIVATE
        Matcha
        Qt6::Widgets
        Qt6::Test
        Matcha_CompilerWarnings
)

add_test(NAME MatchaIntegrationTests COMMAND MatchaIntegrationTests)
set_tests_properties(MatchaIntegrationTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;PATH=${CMAKE_PREFIX_PATH}/bin\;${CMAKE_CURRENT_BINARY_DIR}\;$ENV{PATH}"
)

# --------------------------------------------------------------------------- #
# Token Pipeline: build-time palette JSON validation (S6)
# --------------------------------------------------------------------------- #
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_FOUND)
    set(MATCHA_TOKEN_SCHEMA "${CMAKE_CURRENT_SOURCE_DIR}/Resources/tokens_schema.json")
    set(MATCHA_VALIDATE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/validate_tokens.py")

    file(GLOB MATCHA_PALETTE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Themes/*.json")

    add_custom_target(ValidateTokens
        COMMAND ${Python3_EXECUTABLE} ${MATCHA_VALIDATE_SCRIPT}
                ${MATCHA_TOKEN_SCHEMA} ${MATCHA_PALETTE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Validating palette JSON files against token schema..."
        SOURCES ${MATCHA_VALIDATE_SCRIPT} ${MATCHA_TOKEN_SCHEMA} ${MATCHA_PALETTE_FILES}
    )

    add_dependencies(Matcha ValidateTokens)
else()
    message(WARNING "Python3 not found — ValidateTokens target disabled. "
                    "Palette JSON files will NOT be validated at build time.")
endif()
