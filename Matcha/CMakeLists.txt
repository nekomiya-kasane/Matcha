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
    Source/Foundation/Placeholder.cpp
    Source/Widgets/Core/ThemeAware.cpp
    Source/Widgets/Core/ThemeServiceGlobal.cpp
    Source/Widgets/Core/AnimationServiceGlobal.cpp
    Source/Widgets/Core/MnemonicState.cpp
    Source/Widgets/Core/MnemonicStateGlobal.cpp
    Include/Matcha/Widgets/Core/MnemonicState.h
    Source/Widgets/Core/NyanTheme.cpp
    Source/Widgets/Core/TonalPaletteGenerator.cpp
    Source/Widgets/Core/SvgIconProvider.cpp
    Source/Widgets/Core/AnimationService.cpp
    Source/Widgets/Core/SpringAnimation.cpp
    Source/Widgets/Core/StateTransition.cpp
    Source/Widgets/Core/ContrastChecker.cpp
    Source/Widgets/Core/A11yAudit.cpp
    Source/Widgets/Core/InteractionEventFilter.cpp
    Source/Widgets/Core/TooltipEventFilter.cpp
    Source/Widgets/Core/WhatsThisEventFilter.cpp
    Source/Widgets/Core/FlowLayout.cpp
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
    Source/Widgets/Core/UpdateGuard.cpp
    Include/Matcha/Widgets/Core/UpdateGuard.h
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
    Source/Widgets/Core/DropZoneOverlay.cpp
    Include/Matcha/Widgets/Core/DropZoneOverlay.h
    Source/Widgets/Shell/ViewportFrame.cpp
    Include/Matcha/Widgets/Shell/ViewportFrame.h
    Source/UiNodes/Core/BaseObject.cpp
    Source/UiNodes/Core/Notification.cpp
    Source/UiNodes/Core/EventNode.cpp
    Source/UiNodes/Core/CommandNode.cpp
    Source/UiNodes/Core/NotificationQueue.cpp
    Source/UiNodes/Core/UiNode.cpp
    Source/UiNodes/Core/FocusChain.cpp
    Source/UiNodes/Core/FocusManager.cpp
    Include/Matcha/UiNodes/Core/FocusManager.h
    Source/UiNodes/Core/MnemonicManager.cpp
    Source/UiNodes/Core/MnemonicManagerGlobal.cpp
    Include/Matcha/UiNodes/Core/MnemonicManager.h
    Include/Matcha/UiNodes/Core/UiNode.h
    Source/UiNodes/Core/WidgetWrapper.cpp
    Include/Matcha/UiNodes/Core/WidgetWrapper.h
    Source/UiNodes/Core/WidgetNode.cpp
    Include/Matcha/UiNodes/Core/WidgetNode.h
    Include/Matcha/UiNodes/Core/GridConstants.h
    Source/UiNodes/Controls/LineEditNode.cpp
    Include/Matcha/UiNodes/Controls/LineEditNode.h
    Source/UiNodes/Controls/ComboBoxNode.cpp
    Include/Matcha/UiNodes/Controls/ComboBoxNode.h
    Source/UiNodes/Controls/SpinBoxNode.cpp
    Include/Matcha/UiNodes/Controls/SpinBoxNode.h
    Source/UiNodes/Controls/CheckBoxNode.cpp
    Include/Matcha/UiNodes/Controls/CheckBoxNode.h
    Source/UiNodes/Controls/ToggleSwitchNode.cpp
    Include/Matcha/UiNodes/Controls/ToggleSwitchNode.h
    Source/UiNodes/Controls/PushButtonNode.cpp
    Include/Matcha/UiNodes/Controls/PushButtonNode.h
    Source/UiNodes/Controls/ToolButtonNode.cpp
    Include/Matcha/UiNodes/Controls/ToolButtonNode.h
    Source/UiNodes/Controls/RadioButtonNode.cpp
    Include/Matcha/UiNodes/Controls/RadioButtonNode.h
    Source/UiNodes/Controls/DoubleSpinBoxNode.cpp
    Include/Matcha/UiNodes/Controls/DoubleSpinBoxNode.h
    Include/Matcha/UiNodes/Core/WidgetNotification.h
    Source/UiNodes/Controls/LabelNode.cpp
    Include/Matcha/UiNodes/Controls/LabelNode.h
    Source/UiNodes/Controls/ProgressBarNode.cpp
    Include/Matcha/UiNodes/Controls/ProgressBarNode.h
    Source/UiNodes/Controls/SliderNode.cpp
    Include/Matcha/UiNodes/Controls/SliderNode.h
    Source/UiNodes/Controls/ColorPickerNode.cpp
    Include/Matcha/UiNodes/Controls/ColorPickerNode.h
    Source/UiNodes/Controls/ColorSwatchNode.cpp
    Include/Matcha/UiNodes/Controls/ColorSwatchNode.h
    Source/UiNodes/Controls/PlainTextEditNode.cpp
    Include/Matcha/UiNodes/Controls/PlainTextEditNode.h
    Source/UiNodes/Controls/LineNode.cpp
    Include/Matcha/UiNodes/Controls/LineNode.h
    Source/UiNodes/Controls/BadgeNode.cpp
    Include/Matcha/UiNodes/Controls/BadgeNode.h
    Source/UiNodes/Controls/MessageNode.cpp
    Include/Matcha/UiNodes/Controls/MessageNode.h
    Source/UiNodes/Controls/ProgressRingNode.cpp
    Include/Matcha/UiNodes/Controls/ProgressRingNode.h
    Source/UiNodes/Controls/RangeSliderNode.cpp
    Include/Matcha/UiNodes/Controls/RangeSliderNode.h
    Source/UiNodes/Controls/DateTimePickerNode.cpp
    Include/Matcha/UiNodes/Controls/DateTimePickerNode.h
    Source/UiNodes/Controls/PaginatorNode.cpp
    Include/Matcha/UiNodes/Controls/PaginatorNode.h
    Source/UiNodes/Controls/CollapsibleSectionNode.cpp
    Include/Matcha/UiNodes/Controls/CollapsibleSectionNode.h
    Source/UiNodes/Controls/NotificationNode.cpp
    Include/Matcha/UiNodes/Controls/NotificationNode.h
    Source/UiNodes/Shell/Shell.cpp
    Include/Matcha/UiNodes/Shell/Shell.h
    Source/UiNodes/Shell/Application.cpp
    Include/Matcha/UiNodes/Shell/Application.h
    Source/UiNodes/Shell/WindowNode.cpp
    Include/Matcha/UiNodes/Shell/WindowNode.h
    Source/UiNodes/Shell/TitleBarNode.cpp
    Include/Matcha/UiNodes/Shell/TitleBarNode.h
    Source/UiNodes/Shell/MainTitleBarNode.cpp
    Include/Matcha/UiNodes/Shell/MainTitleBarNode.h
    Source/UiNodes/Shell/DocumentToolBarNode.cpp
    Include/Matcha/UiNodes/Shell/DocumentToolBarNode.h
    Source/UiNodes/Shell/LogoButtonNode.cpp
    Include/Matcha/UiNodes/Shell/LogoButtonNode.h
    Source/UiNodes/Shell/FloatingTitleBarNode.cpp
    Include/Matcha/UiNodes/Shell/FloatingTitleBarNode.h
    Source/UiNodes/Document/TabBarNode.cpp
    Include/Matcha/UiNodes/Document/TabBarNode.h
    Source/UiNodes/Document/TabItemNode.cpp
    Include/Matcha/UiNodes/Document/TabItemNode.h
    Source/UiNodes/Shell/FloatingWindowNode.cpp
    Include/Matcha/UiNodes/Shell/FloatingWindowNode.h
    Source/UiNodes/Shell/FloatingTabWindowNode.cpp
    Include/Matcha/UiNodes/Shell/FloatingTabWindowNode.h
    Include/Matcha/Services/IDocumentManager.h
    Source/Services/DocumentManager.cpp
    Include/Matcha/Services/DocumentManager.h
    Source/UiNodes/ActionBar/ActionBarNode.cpp
    Include/Matcha/UiNodes/ActionBar/ActionBarNode.h
    Source/UiNodes/Shell/StatusBarNode.cpp
    Include/Matcha/UiNodes/Shell/StatusBarNode.h
    Source/UiNodes/Shell/StatusItemNode.cpp
    Include/Matcha/UiNodes/Shell/StatusItemNode.h
    Source/UiNodes/ActionBar/ActionTabNode.cpp
    Include/Matcha/UiNodes/ActionBar/ActionTabNode.h
    Source/UiNodes/ActionBar/ActionToolbarNode.cpp
    Include/Matcha/UiNodes/ActionBar/ActionToolbarNode.h
    Source/UiNodes/ActionBar/ActionButtonNode.cpp
    Include/Matcha/UiNodes/ActionBar/ActionButtonNode.h
    Source/UiNodes/Workbench/WorkshopRegistry.cpp
    Include/Matcha/UiNodes/Workbench/WorkshopRegistry.h
    Include/Matcha/UiNodes/Workbench/WorkbenchTypes.h
    Source/UiNodes/Workbench/WorkbenchManager.cpp
    Include/Matcha/UiNodes/Workbench/WorkbenchManager.h
    Source/UiNodes/Core/TokenRegistryGlobal.cpp
    Source/UiNodes/Core/ContainerNode.cpp
    Include/Matcha/UiNodes/Core/ContainerNode.h
    Source/UiNodes/Menu/DialogNode.cpp
    Include/Matcha/UiNodes/Menu/DialogNode.h
    Source/UiNodes/Core/ContextMenuRequest.cpp
    Source/UiNodes/Menu/MenuBarNode.cpp
    Include/Matcha/UiNodes/Menu/MenuBarNode.h
    Source/UiNodes/Menu/MenuNode.cpp
    Include/Matcha/UiNodes/Menu/MenuNode.h
    Source/UiNodes/Menu/MenuItemNode.cpp
    Include/Matcha/UiNodes/Menu/MenuItemNode.h
    Source/UiNodes/Controls/DataTableNode.cpp
    Include/Matcha/UiNodes/Controls/DataTableNode.h
    Source/UiNodes/Controls/PropertyGridNode.cpp
    Include/Matcha/UiNodes/Controls/PropertyGridNode.h
    Source/UiNodes/Controls/ListWidgetNode.cpp
    Include/Matcha/UiNodes/Controls/ListWidgetNode.h
    Source/UiNodes/Controls/TreeItemNode.cpp
    Include/Matcha/UiNodes/Controls/TreeItemNode.h
    Source/UiNodes/Controls/TreeWidgetNode.cpp
    Include/Matcha/UiNodes/Controls/TreeWidgetNode.h
    Source/UiNodes/Controls/SearchBoxNode.cpp
    Include/Matcha/UiNodes/Controls/SearchBoxNode.h
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
    Include/Matcha/Widgets/Core/IThemeService.h
    Include/Matcha/Widgets/Core/NyanTheme.h
    Source/UiNodes/Document/DocumentPage.cpp
    Include/Matcha/UiNodes/Document/DocumentPage.h
    Source/UiNodes/Document/SplitTreeNode.cpp
    Include/Matcha/UiNodes/Document/SplitTreeNode.h
    Source/UiNodes/Document/ViewportGroup.cpp
    Include/Matcha/UiNodes/Document/ViewportGroup.h
    Source/UiNodes/Document/Viewport.cpp
    Include/Matcha/UiNodes/Document/Viewport.h
    Source/UiNodes/Core/UiNodeQuery.cpp
    Include/Matcha/UiNodes/Core/UiNodeQuery.h
    Include/Matcha/Foundation/FixedString.h
    Source/UiNodes/Document/DocumentArea.cpp
    Include/Matcha/UiNodes/Document/DocumentArea.h
    Source/UiNodes/Shell/WorkspaceFrame.cpp
    Include/Matcha/UiNodes/Shell/WorkspaceFrame.h
    Source/UiNodes/Shell/ControlBar.cpp
    Include/Matcha/UiNodes/Shell/ControlBar.h
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
    Tests/Unit/Foundation/StrongIdTest.cpp
    Tests/Unit/Foundation/ErrorCodeTest.cpp
    Tests/Unit/Foundation/ObserverPtrTest.cpp
    Tests/Unit/Core/CommandNodeTest.cpp
    Tests/Unit/Core/DesignTokenTest.cpp
    Tests/Unit/Core/TokenEnumsTest.cpp
    Tests/Unit/Core/PhaseBTest.cpp
    Tests/Unit/Core/PhaseCTest.cpp
    Tests/Unit/Core/PhaseDTest.cpp
    Tests/Unit/Core/ITokenRegistryTest.cpp
    Tests/Unit/Core/TonalPaletteTest.cpp
    Tests/Unit/Core/WidgetStyleSheetTest.cpp
    Tests/Unit/Core/PaletteLoadTest.cpp
    Tests/Unit/Core/CoreWidgetEnumTest.cpp
    Tests/Unit/Core/ContainerWidgetEnumTest.cpp
    Tests/Unit/Core/ApplicationWidgetEnumTest.cpp
    Tests/Unit/Core/UiNodeTest.cpp
    Tests/Unit/Document/DocumentManagerTest.cpp
    Tests/Unit/Shell/WorkspaceFrameTest.cpp
    Tests/Unit/Document/DocumentAreaTest.cpp
    Tests/Unit/Document/ViewportGroupTest.cpp
    Tests/Unit/Foundation/InputEventTest.cpp
    Tests/Unit/Document/ViewportBindTest.cpp
    Tests/Unit/Document/ViewportGroupLayoutTest.cpp
    Tests/Unit/Document/SplitTreeNodeTest.cpp
    Tests/Unit/Services/PluginHostTest.cpp
    Tests/Unit/CApi/CApiRoundTripTest.cpp
    Tests/Unit/CApi/CApiSafetyTest.cpp
    Tests/Unit/Query/UiNodeQueryTest.cpp
    Tests/Unit/Workbench/WorkshopRegistryTest.cpp
    Tests/Unit/Core/MnemonicStateTest.cpp
    Tests/Unit/Core/MnemonicManagerTest.cpp
)

target_include_directories(MatchaUnitTests PRIVATE
    Tests/TestUtils
    Include
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
