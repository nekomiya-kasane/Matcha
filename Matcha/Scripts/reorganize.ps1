#!/usr/bin/env pwsh
# Matcha source tree reorganization script (v4 layout)
# Run from Matcha/ root directory.

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot | Split-Path

Write-Host "=== Matcha Source Reorganization ===" -ForegroundColor Cyan
Write-Host "Root: $root"

# ============================================================================
# STEP 0: Build include-path replacement map (old Matcha/... -> new Matcha/...)
# These are the include-path prefixes as they appear in #include directives.
# Format: @{ "old/path/prefix" = "new/path/prefix" }
# Order matters: longer (more specific) prefixes first.
# ============================================================================

$includeMap = [ordered]@{
    # --- Foundation -> Core (L0) ---
    "Matcha/Foundation/ErrorCode.h"              = "Matcha/Core/ErrorCode.h"
    "Matcha/Foundation/FixedString.h"            = "Matcha/Core/FixedString.h"
    "Matcha/Foundation/Macros.h"                 = "Matcha/Core/Macros.h"
    "Matcha/Foundation/Observable.h"             = "Matcha/Core/Observable.h"
    "Matcha/Foundation/PropertyBinding.h"        = "Matcha/Core/PropertyBinding.h"
    "Matcha/Foundation/StateMachine.h"           = "Matcha/Core/StateMachine.h"
    "Matcha/Foundation/StrongId.h"               = "Matcha/Core/StrongId.h"
    "Matcha/Foundation/StringId.h"               = "Matcha/Core/StringId.h"
    "Matcha/Foundation/Types.h"                  = "Matcha/Core/Types.h"

    # --- Foundation -> Animation (L2.5) ---
    "Matcha/Foundation/AnimationBlendLayer.h"    = "Matcha/Animation/AnimationBlendLayer.h"
    "Matcha/Foundation/ChoreographyEngine.h"     = "Matcha/Animation/ChoreographyEngine.h"
    "Matcha/Foundation/SharedElementTransition.h" = "Matcha/Animation/SharedElementTransition.h"
    "Matcha/Foundation/CompositionTemplate.h"    = "Matcha/Animation/CompositionTemplate.h"
    "Matcha/Foundation/InteractionTimingRegistry.h" = "Matcha/Animation/InteractionTimingRegistry.h"

    # --- Foundation -> Interaction (L2.5) ---
    "Matcha/Foundation/ShortcutManager.h"        = "Matcha/Interaction/Input/ShortcutManager.h"
    "Matcha/Foundation/KeyboardContract.h"       = "Matcha/Interaction/Input/KeyboardContract.h"
    "Matcha/Foundation/GestureMotionTracker.h"   = "Matcha/Interaction/Input/GestureMotionTracker.h"
    "Matcha/Foundation/ScrollPhysics.h"          = "Matcha/Interaction/Input/ScrollPhysics.h"
    "Matcha/Foundation/FormValidator.h"          = "Matcha/Interaction/FormValidator.h"
    "Matcha/Foundation/ContextMenuComposer.h"    = "Matcha/Interaction/ContextMenuComposer.h"
    "Matcha/Foundation/ContextualHelpService.h"  = "Matcha/Interaction/ContextualHelpService.h"
    "Matcha/Foundation/DragDropVisual.h"         = "Matcha/Interaction/DragDropVisual.h"

    # --- Foundation -> Feedback (L2.5) ---
    "Matcha/Foundation/ErrorBoundary.h"          = "Matcha/Feedback/ErrorBoundary.h"
    "Matcha/Foundation/FeedbackPolicy.h"         = "Matcha/Feedback/FeedbackPolicy.h"
    "Matcha/Foundation/ContentStateModel.h"      = "Matcha/Feedback/ContentStateModel.h"
    "Matcha/Foundation/DestructiveActionPolicy.h" = "Matcha/Feedback/DestructiveActionPolicy.h"
    "Matcha/Foundation/NotificationStackManager.h" = "Matcha/Feedback/NotificationStackManager.h"
    "Matcha/Foundation/EdgeCaseGuard.h"          = "Matcha/Feedback/EdgeCaseGuard.h"

    # --- Foundation -> Tree/Layout (L2) ---
    "Matcha/Foundation/LayoutEngine.h"           = "Matcha/Tree/Layout/LayoutEngine.h"
    "Matcha/Foundation/BreakpointObserver.h"     = "Matcha/Tree/Layout/BreakpointObserver.h"

    # --- Foundation -> Tree/FSM (L2) ---
    "Matcha/Foundation/WidgetFsm.h"              = "Matcha/Tree/FSM/WidgetFsm.h"
    "Matcha/Foundation/WidgetFsmBridge.h"        = "Matcha/Tree/FSM/WidgetFsmBridge.h"
    "Matcha/Foundation/WidgetEnums.h"            = "Matcha/Tree/FSM/WidgetEnums.h"

    # --- Foundation -> DSL (L3) ---
    "Matcha/Foundation/Blueprint.h"              = "Matcha/DSL/Blueprint.h"

    # --- Foundation -> Theming/Token ---
    "Matcha/Foundation/DtfmTokenModel.h"         = "Matcha/Theming/Token/DtfmTokenModel.h"
    "Matcha/Foundation/VariantNameRegistry.h"    = "Matcha/Theming/Token/VariantNameRegistry.h"

    # --- UiNodes/Core -> Event (L1) ---
    "Matcha/UiNodes/Core/BaseObject.h"           = "Matcha/Event/BaseObject.h"
    "Matcha/UiNodes/Core/MetaClass.h"            = "Matcha/Event/MetaClass.h"
    "Matcha/UiNodes/Core/EventNode.h"            = "Matcha/Event/EventNode.h"
    "Matcha/UiNodes/Core/CommandNode.h"          = "Matcha/Event/CommandNode.h"
    "Matcha/UiNodes/Core/Notification.h"         = "Matcha/Event/Notification.h"
    "Matcha/UiNodes/Core/NotificationQueue.h"    = "Matcha/Event/NotificationQueue.h"

    # --- UiNodes/Core -> Tree (L2) ---
    "Matcha/UiNodes/Core/UiNode.h"               = "Matcha/Tree/UiNode.h"
    "Matcha/UiNodes/Core/UiNodeQuery.h"          = "Matcha/Tree/UiNodeQuery.h"
    "Matcha/UiNodes/Core/ContainerNode.h"        = "Matcha/Tree/ContainerNode.h"
    "Matcha/UiNodes/Core/WidgetNode.h"           = "Matcha/Tree/WidgetNode.h"
    "Matcha/UiNodes/Core/WidgetWrapper.h"        = "Matcha/Tree/WidgetWrapper.h"
    "Matcha/UiNodes/Core/GridConstants.h"        = "Matcha/Tree/GridConstants.h"
    "Matcha/UiNodes/Core/A11yRole.h"             = "Matcha/Tree/A11yRole.h"
    "Matcha/UiNodes/Core/UiNodeNotification.h"   = "Matcha/Tree/UiNodeNotification.h"
    "Matcha/UiNodes/Core/WidgetNotification.h"   = "Matcha/Tree/WidgetNotification.h"
    "Matcha/UiNodes/Core/TooltipSpec.h"          = "Matcha/Tree/TooltipSpec.h"

    # --- UiNodes/Core -> Theming/Token ---
    "Matcha/UiNodes/Core/TokenEnums.h"           = "Matcha/Theming/Token/TokenEnums.h"
    "Matcha/UiNodes/Core/ITokenRegistry.h"       = "Matcha/Theming/Token/ITokenRegistry.h"
    "Matcha/UiNodes/Core/TokenRegistryGlobal.h"  = "Matcha/Theming/Token/TokenRegistryGlobal.h"

    # --- UiNodes/Core -> Interaction ---
    "Matcha/UiNodes/Core/FocusChain.h"           = "Matcha/Interaction/Focus/FocusChain.h"
    "Matcha/UiNodes/Core/FocusManager.h"         = "Matcha/Interaction/Focus/FocusManager.h"
    "Matcha/UiNodes/Core/MnemonicManager.h"      = "Matcha/Interaction/Focus/MnemonicManager.h"
    "Matcha/UiNodes/Core/SelectionModel.h"       = "Matcha/Interaction/Selection/SelectionModel.h"
    "Matcha/UiNodes/Core/PopupPositioner.h"      = "Matcha/Interaction/PopupPositioner.h"

    # --- UiNodes/Controls -> Tree/Controls ---
    "Matcha/UiNodes/Controls/"                   = "Matcha/Tree/Controls/"

    # --- UiNodes/Shell -> Tree/Composition/Shell ---
    "Matcha/UiNodes/Shell/"                      = "Matcha/Tree/Composition/Shell/"

    # --- UiNodes/ActionBar -> Tree/Composition/ActionBar ---
    "Matcha/UiNodes/ActionBar/"                  = "Matcha/Tree/Composition/ActionBar/"

    # --- UiNodes/Document -> Tree/Composition/Document ---
    "Matcha/UiNodes/Document/"                   = "Matcha/Tree/Composition/Document/"

    # --- UiNodes/Menu -> Tree/Composition/Menu ---
    "Matcha/UiNodes/Menu/"                       = "Matcha/Tree/Composition/Menu/"

    # --- UiNodes/Workbench -> Tree/Composition/Workbench ---
    "Matcha/UiNodes/Workbench/"                  = "Matcha/Tree/Composition/Workbench/"

    # --- Widgets/Core -> Theming (public headers) ---
    "Matcha/Widgets/Core/DesignTokens.h"         = "Matcha/Theming/DesignTokens.h"
    "Matcha/Widgets/Core/NyanTheme.h"            = "Matcha/Theming/NyanTheme.h"
    "Matcha/Widgets/Core/ResolvedStyle.h"        = "Matcha/Theming/ResolvedStyle.h"
    "Matcha/Widgets/Core/ThemeAware.h"           = "Matcha/Theming/ThemeAware.h"
    "Matcha/Widgets/Core/IThemeService.h"        = "Matcha/Theming/IThemeService.h"
    "Matcha/Widgets/Core/IAnimationService.h"    = "Matcha/Animation/IAnimationService.h"
    "Matcha/Widgets/Core/DefaultPalette.h"       = "Matcha/Theming/Palette/DefaultPalette.h"
    "Matcha/Widgets/Core/TonalPaletteGenerator.h" = "Matcha/Theming/Palette/TonalPaletteGenerator.h"
    "Matcha/Widgets/Core/ContrastChecker.h"      = "Matcha/Theming/Palette/ContrastChecker.h"
    "Matcha/Widgets/Core/A11yAudit.h"            = "Matcha/Theming/A11yAudit.h"
    "Matcha/Widgets/Core/MnemonicState.h"        = "Matcha/Interaction/Focus/MnemonicState.h"
    "Matcha/Widgets/Core/UpdateGuard.h"          = "Matcha/Tree/UpdateGuard.h"
    "Matcha/Widgets/Core/DropZoneOverlay.h"      = "Matcha/Interaction/DropZoneOverlay.h"
    "Matcha/Widgets/WidgetStyleSheet.h"          = "Matcha/Theming/WidgetStyleSheet.h"

    # --- Widgets/* headers stay but move to Source (private) ---
    # These will be handled by moving to Source/Widgets/ and adding a PRIVATE include dir
    # For include path replacement, we keep Matcha/Widgets/... but the physical files move.
    # Actually, since they're already under Include/Matcha/Widgets/ and many Source files
    # include them with "Matcha/Widgets/..." or <Matcha/Widgets/...>, we keep the include
    # path Matcha/Widgets/ but move the files to Source/_Private/Widgets/ and add that
    # as a PRIVATE include directory. This way include paths don't change for Widgets.
}

# ============================================================================
# STEP 1: Physical file moves
# ============================================================================

function MoveFile($src, $dst) {
    $srcFull = Join-Path $root $src
    $dstFull = Join-Path $root $dst
    if (-not (Test-Path $srcFull)) {
        Write-Warning "SKIP (not found): $src"
        return
    }
    $dstDir = Split-Path $dstFull
    if (-not (Test-Path $dstDir)) {
        New-Item -ItemType Directory -Path $dstDir -Force | Out-Null
    }
    Move-Item -Path $srcFull -Destination $dstFull -Force
    Write-Host "  $src -> $dst" -ForegroundColor DarkGray
}

Write-Host "`n--- Moving Include/ headers ---" -ForegroundColor Yellow

# Foundation -> Core
$foundationToCore = @(
    "ErrorCode.h", "FixedString.h", "Macros.h", "Observable.h",
    "PropertyBinding.h", "StateMachine.h", "StrongId.h", "StringId.h", "Types.h"
)
foreach ($f in $foundationToCore) {
    MoveFile "Include/Matcha/Foundation/$f" "Include/Matcha/Core/$f"
}

# Foundation -> Animation
$foundationToAnimation = @(
    "AnimationBlendLayer.h", "ChoreographyEngine.h", "SharedElementTransition.h",
    "CompositionTemplate.h", "InteractionTimingRegistry.h"
)
foreach ($f in $foundationToAnimation) {
    MoveFile "Include/Matcha/Foundation/$f" "Include/Matcha/Animation/$f"
}

# Foundation -> Interaction/Input
$foundationToInteractionInput = @(
    "ShortcutManager.h", "KeyboardContract.h", "GestureMotionTracker.h", "ScrollPhysics.h"
)
foreach ($f in $foundationToInteractionInput) {
    MoveFile "Include/Matcha/Foundation/$f" "Include/Matcha/Interaction/Input/$f"
}

# Foundation -> Interaction (root)
$foundationToInteraction = @(
    "FormValidator.h", "ContextMenuComposer.h", "ContextualHelpService.h", "DragDropVisual.h"
)
foreach ($f in $foundationToInteraction) {
    MoveFile "Include/Matcha/Foundation/$f" "Include/Matcha/Interaction/$f"
}

# Foundation -> Feedback
$foundationToFeedback = @(
    "ErrorBoundary.h", "FeedbackPolicy.h", "ContentStateModel.h",
    "DestructiveActionPolicy.h", "NotificationStackManager.h", "EdgeCaseGuard.h"
)
foreach ($f in $foundationToFeedback) {
    MoveFile "Include/Matcha/Foundation/$f" "Include/Matcha/Feedback/$f"
}

# Foundation -> Tree/Layout
MoveFile "Include/Matcha/Foundation/LayoutEngine.h" "Include/Matcha/Tree/Layout/LayoutEngine.h"
MoveFile "Include/Matcha/Foundation/BreakpointObserver.h" "Include/Matcha/Tree/Layout/BreakpointObserver.h"

# Foundation -> Tree/FSM
MoveFile "Include/Matcha/Foundation/WidgetFsm.h" "Include/Matcha/Tree/FSM/WidgetFsm.h"
MoveFile "Include/Matcha/Foundation/WidgetFsmBridge.h" "Include/Matcha/Tree/FSM/WidgetFsmBridge.h"
MoveFile "Include/Matcha/Foundation/WidgetEnums.h" "Include/Matcha/Tree/FSM/WidgetEnums.h"

# Foundation -> DSL
MoveFile "Include/Matcha/Foundation/Blueprint.h" "Include/Matcha/DSL/Blueprint.h"

# Foundation -> Theming/Token
MoveFile "Include/Matcha/Foundation/DtfmTokenModel.h" "Include/Matcha/Theming/Token/DtfmTokenModel.h"
MoveFile "Include/Matcha/Foundation/VariantNameRegistry.h" "Include/Matcha/Theming/Token/VariantNameRegistry.h"

# UiNodes/Core -> Event
$uiNodesToEvent = @(
    "BaseObject.h", "MetaClass.h", "EventNode.h", "CommandNode.h",
    "Notification.h", "NotificationQueue.h"
)
foreach ($f in $uiNodesToEvent) {
    MoveFile "Include/Matcha/UiNodes/Core/$f" "Include/Matcha/Event/$f"
}

# UiNodes/Core -> Tree
$uiNodesToTree = @(
    "UiNode.h", "UiNodeQuery.h", "ContainerNode.h", "WidgetNode.h",
    "WidgetWrapper.h", "GridConstants.h", "A11yRole.h",
    "UiNodeNotification.h", "WidgetNotification.h", "TooltipSpec.h"
)
foreach ($f in $uiNodesToTree) {
    MoveFile "Include/Matcha/UiNodes/Core/$f" "Include/Matcha/Tree/$f"
}

# UiNodes/Core -> Theming/Token
MoveFile "Include/Matcha/UiNodes/Core/TokenEnums.h" "Include/Matcha/Theming/Token/TokenEnums.h"
MoveFile "Include/Matcha/UiNodes/Core/ITokenRegistry.h" "Include/Matcha/Theming/Token/ITokenRegistry.h"
MoveFile "Include/Matcha/UiNodes/Core/TokenRegistryGlobal.h" "Include/Matcha/Theming/Token/TokenRegistryGlobal.h"

# UiNodes/Core -> Interaction
MoveFile "Include/Matcha/UiNodes/Core/FocusChain.h" "Include/Matcha/Interaction/Focus/FocusChain.h"
MoveFile "Include/Matcha/UiNodes/Core/FocusManager.h" "Include/Matcha/Interaction/Focus/FocusManager.h"
MoveFile "Include/Matcha/UiNodes/Core/MnemonicManager.h" "Include/Matcha/Interaction/Focus/MnemonicManager.h"
MoveFile "Include/Matcha/UiNodes/Core/SelectionModel.h" "Include/Matcha/Interaction/Selection/SelectionModel.h"
MoveFile "Include/Matcha/UiNodes/Core/PopupPositioner.h" "Include/Matcha/Interaction/PopupPositioner.h"

# UiNodes/Controls -> Tree/Controls (whole directory)
$controlFiles = Get-ChildItem -File (Join-Path $root "Include/Matcha/UiNodes/Controls") -ErrorAction SilentlyContinue
foreach ($f in $controlFiles) {
    MoveFile "Include/Matcha/UiNodes/Controls/$($f.Name)" "Include/Matcha/Tree/Controls/$($f.Name)"
}

# UiNodes/Shell -> Tree/Composition/Shell
$shellFiles = Get-ChildItem -File (Join-Path $root "Include/Matcha/UiNodes/Shell") -ErrorAction SilentlyContinue
foreach ($f in $shellFiles) {
    MoveFile "Include/Matcha/UiNodes/Shell/$($f.Name)" "Include/Matcha/Tree/Composition/Shell/$($f.Name)"
}

# UiNodes/ActionBar -> Tree/Composition/ActionBar
$abFiles = Get-ChildItem -File (Join-Path $root "Include/Matcha/UiNodes/ActionBar") -ErrorAction SilentlyContinue
foreach ($f in $abFiles) {
    MoveFile "Include/Matcha/UiNodes/ActionBar/$($f.Name)" "Include/Matcha/Tree/Composition/ActionBar/$($f.Name)"
}

# UiNodes/Document -> Tree/Composition/Document
$docFiles = Get-ChildItem -File (Join-Path $root "Include/Matcha/UiNodes/Document") -ErrorAction SilentlyContinue
foreach ($f in $docFiles) {
    MoveFile "Include/Matcha/UiNodes/Document/$($f.Name)" "Include/Matcha/Tree/Composition/Document/$($f.Name)"
}

# UiNodes/Menu -> Tree/Composition/Menu
$menuFiles = Get-ChildItem -File (Join-Path $root "Include/Matcha/UiNodes/Menu") -ErrorAction SilentlyContinue
foreach ($f in $menuFiles) {
    MoveFile "Include/Matcha/UiNodes/Menu/$($f.Name)" "Include/Matcha/Tree/Composition/Menu/$($f.Name)"
}

# UiNodes/Workbench -> Tree/Composition/Workbench
$wbFiles = Get-ChildItem -File (Join-Path $root "Include/Matcha/UiNodes/Workbench") -ErrorAction SilentlyContinue
foreach ($f in $wbFiles) {
    MoveFile "Include/Matcha/UiNodes/Workbench/$($f.Name)" "Include/Matcha/Tree/Composition/Workbench/$($f.Name)"
}

# Widgets/Core -> Theming / Animation / Interaction / Tree
MoveFile "Include/Matcha/Widgets/Core/DesignTokens.h" "Include/Matcha/Theming/DesignTokens.h"
MoveFile "Include/Matcha/Widgets/Core/NyanTheme.h" "Include/Matcha/Theming/NyanTheme.h"
MoveFile "Include/Matcha/Widgets/Core/ResolvedStyle.h" "Include/Matcha/Theming/ResolvedStyle.h"
MoveFile "Include/Matcha/Widgets/Core/ThemeAware.h" "Include/Matcha/Theming/ThemeAware.h"
MoveFile "Include/Matcha/Widgets/Core/IThemeService.h" "Include/Matcha/Theming/IThemeService.h"
MoveFile "Include/Matcha/Widgets/Core/IAnimationService.h" "Include/Matcha/Animation/IAnimationService.h"
MoveFile "Include/Matcha/Widgets/Core/DefaultPalette.h" "Include/Matcha/Theming/Palette/DefaultPalette.h"
MoveFile "Include/Matcha/Widgets/Core/TonalPaletteGenerator.h" "Include/Matcha/Theming/Palette/TonalPaletteGenerator.h"
MoveFile "Include/Matcha/Widgets/Core/ContrastChecker.h" "Include/Matcha/Theming/Palette/ContrastChecker.h"
MoveFile "Include/Matcha/Widgets/Core/A11yAudit.h" "Include/Matcha/Theming/A11yAudit.h"
MoveFile "Include/Matcha/Widgets/Core/MnemonicState.h" "Include/Matcha/Interaction/Focus/MnemonicState.h"
MoveFile "Include/Matcha/Widgets/Core/UpdateGuard.h" "Include/Matcha/Tree/UpdateGuard.h"
MoveFile "Include/Matcha/Widgets/Core/DropZoneOverlay.h" "Include/Matcha/Interaction/DropZoneOverlay.h"
MoveFile "Include/Matcha/Widgets/WidgetStyleSheet.h" "Include/Matcha/Theming/WidgetStyleSheet.h"

# ============================================================================
# Source/ moves — mirror Include/ structure
# ============================================================================
Write-Host "`n--- Moving Source/ files ---" -ForegroundColor Yellow

# Source/Foundation -> Source/Core
MoveFile "Source/Foundation/Placeholder.cpp" "Source/Core/Placeholder.cpp"

# Source/UiNodes/Core -> Source/Event
$srcToEvent = @(
    "BaseObject.cpp", "Notification.cpp", "EventNode.cpp",
    "CommandNode.cpp", "NotificationQueue.cpp"
)
foreach ($f in $srcToEvent) {
    MoveFile "Source/UiNodes/Core/$f" "Source/Event/$f"
}

# Source/UiNodes/Core -> Source/Tree
$srcToTree = @(
    "UiNode.cpp", "ContainerNode.cpp", "WidgetNode.cpp",
    "WidgetWrapper.cpp", "UiNodeQuery.cpp"
)
foreach ($f in $srcToTree) {
    MoveFile "Source/UiNodes/Core/$f" "Source/Tree/$f"
}

# Source/UiNodes/Core -> Source/Theming/Token
MoveFile "Source/UiNodes/Core/TokenRegistryGlobal.cpp" "Source/Theming/Token/TokenRegistryGlobal.cpp"
MoveFile "Source/UiNodes/Core/DtfmTokenModel.cpp" "Source/Theming/Token/DtfmTokenModel.cpp"
MoveFile "Source/UiNodes/Core/VariantNameRegistry.cpp" "Source/Theming/Token/VariantNameRegistry.cpp"

# Source/UiNodes/Core -> Source/Interaction
MoveFile "Source/UiNodes/Core/FocusChain.cpp" "Source/Interaction/Focus/FocusChain.cpp"
MoveFile "Source/UiNodes/Core/FocusManager.cpp" "Source/Interaction/Focus/FocusManager.cpp"
MoveFile "Source/UiNodes/Core/MnemonicManager.cpp" "Source/Interaction/Focus/MnemonicManager.cpp"
MoveFile "Source/UiNodes/Core/MnemonicManagerGlobal.cpp" "Source/Interaction/Focus/MnemonicManagerGlobal.cpp"
MoveFile "Source/UiNodes/Core/SelectionModel.cpp" "Source/Interaction/Selection/SelectionModel.cpp"
MoveFile "Source/UiNodes/Core/PopupPositioner.cpp" "Source/Interaction/PopupPositioner.cpp"
MoveFile "Source/UiNodes/Core/ContextMenuComposer.cpp" "Source/Interaction/ContextMenuComposer.cpp"
MoveFile "Source/UiNodes/Core/ContextMenuRequest.cpp" "Source/Interaction/ContextMenuRequest.cpp"
MoveFile "Source/UiNodes/Core/ContextualHelpService.cpp" "Source/Interaction/ContextualHelpService.cpp"
MoveFile "Source/UiNodes/Core/FormValidator.cpp" "Source/Interaction/FormValidator.cpp"
MoveFile "Source/UiNodes/Core/DragDropVisual.cpp" "Source/Interaction/DragDropVisual.cpp"
MoveFile "Source/UiNodes/Core/ShortcutManager.cpp" "Source/Interaction/Input/ShortcutManager.cpp"
MoveFile "Source/UiNodes/Core/KeyboardContract.cpp" "Source/Interaction/Input/KeyboardContract.cpp"
MoveFile "Source/UiNodes/Core/GestureMotionTracker.cpp" "Source/Interaction/Input/GestureMotionTracker.cpp"
MoveFile "Source/UiNodes/Core/ScrollPhysics.cpp" "Source/Interaction/Input/ScrollPhysics.cpp"

# Source/UiNodes/Core -> Source/Animation
MoveFile "Source/UiNodes/Core/ChoreographyEngine.cpp" "Source/Animation/ChoreographyEngine.cpp"
MoveFile "Source/UiNodes/Core/SharedElementTransition.cpp" "Source/Animation/SharedElementTransition.cpp"
MoveFile "Source/UiNodes/Core/AnimationBlendLayer.cpp" "Source/Animation/AnimationBlendLayer.cpp"
MoveFile "Source/UiNodes/Core/CompositionTemplate.cpp" "Source/Animation/CompositionTemplate.cpp"
MoveFile "Source/UiNodes/Core/InteractionTimingRegistry.cpp" "Source/Animation/InteractionTimingRegistry.cpp"

# Source/UiNodes/Core -> Source/Feedback
MoveFile "Source/UiNodes/Core/ErrorBoundary.cpp" "Source/Feedback/ErrorBoundary.cpp"
MoveFile "Source/UiNodes/Core/FeedbackPolicy.cpp" "Source/Feedback/FeedbackPolicy.cpp"
MoveFile "Source/UiNodes/Core/ContentStateModel.cpp" "Source/Feedback/ContentStateModel.cpp"
MoveFile "Source/UiNodes/Core/DestructiveActionPolicy.cpp" "Source/Feedback/DestructiveActionPolicy.cpp"
MoveFile "Source/UiNodes/Core/NotificationStackManager.cpp" "Source/Feedback/NotificationStackManager.cpp"
MoveFile "Source/UiNodes/Core/EdgeCaseGuard.cpp" "Source/Feedback/EdgeCaseGuard.cpp"

# Source/UiNodes/Core -> Source/Tree/Layout
MoveFile "Source/UiNodes/Core/LayoutEngine.cpp" "Source/Tree/Layout/LayoutEngine.cpp"
MoveFile "Source/UiNodes/Core/BreakpointObserver.cpp" "Source/Tree/Layout/BreakpointObserver.cpp"

# Source/UiNodes/Core -> Source/DSL
MoveFile "Source/UiNodes/Core/Materializer.cpp" "Source/DSL/Materializer.cpp"

# Source/UiNodes/Controls -> Source/Tree/Controls
$srcControlDir = Join-Path $root "Source/UiNodes/Controls"
if (Test-Path $srcControlDir) {
    Get-ChildItem -File $srcControlDir | ForEach-Object {
        MoveFile "Source/UiNodes/Controls/$($_.Name)" "Source/Tree/Controls/$($_.Name)"
    }
}

# Source/UiNodes/Shell -> Source/Tree/Composition/Shell
$srcShellDir = Join-Path $root "Source/UiNodes/Shell"
if (Test-Path $srcShellDir) {
    Get-ChildItem -File $srcShellDir | ForEach-Object {
        MoveFile "Source/UiNodes/Shell/$($_.Name)" "Source/Tree/Composition/Shell/$($_.Name)"
    }
}

# Source/UiNodes/ActionBar -> Source/Tree/Composition/ActionBar
$srcAbDir = Join-Path $root "Source/UiNodes/ActionBar"
if (Test-Path $srcAbDir) {
    Get-ChildItem -File $srcAbDir | ForEach-Object {
        MoveFile "Source/UiNodes/ActionBar/$($_.Name)" "Source/Tree/Composition/ActionBar/$($_.Name)"
    }
}

# Source/UiNodes/Document -> Source/Tree/Composition/Document
$srcDocDir = Join-Path $root "Source/UiNodes/Document"
if (Test-Path $srcDocDir) {
    Get-ChildItem -File $srcDocDir | ForEach-Object {
        MoveFile "Source/UiNodes/Document/$($_.Name)" "Source/Tree/Composition/Document/$($_.Name)"
    }
}

# Source/UiNodes/Menu -> Source/Tree/Composition/Menu
$srcMenuDir = Join-Path $root "Source/UiNodes/Menu"
if (Test-Path $srcMenuDir) {
    Get-ChildItem -File $srcMenuDir | ForEach-Object {
        MoveFile "Source/UiNodes/Menu/$($_.Name)" "Source/Tree/Composition/Menu/$($_.Name)"
    }
}

# Source/UiNodes/Workbench -> Source/Tree/Composition/Workbench
$srcWbDir = Join-Path $root "Source/UiNodes/Workbench"
if (Test-Path $srcWbDir) {
    Get-ChildItem -File $srcWbDir | ForEach-Object {
        MoveFile "Source/UiNodes/Workbench/$($_.Name)" "Source/Tree/Composition/Workbench/$($_.Name)"
    }
}

# Source/Widgets/Core -> various Theming/Animation/Interaction Source dirs
MoveFile "Source/Widgets/Core/ThemeAware.cpp" "Source/Theming/ThemeAware.cpp"
MoveFile "Source/Widgets/Core/ThemeServiceGlobal.cpp" "Source/Theming/ThemeServiceGlobal.cpp"
MoveFile "Source/Widgets/Core/NyanTheme.cpp" "Source/Theming/NyanTheme.cpp"
MoveFile "Source/Widgets/Core/TonalPaletteGenerator.cpp" "Source/Theming/Palette/TonalPaletteGenerator.cpp"
MoveFile "Source/Widgets/Core/ContrastChecker.cpp" "Source/Theming/Palette/ContrastChecker.cpp"
MoveFile "Source/Widgets/Core/A11yAudit.cpp" "Source/Theming/A11yAudit.cpp"
MoveFile "Source/Widgets/Core/AnimationService.cpp" "Source/Animation/AnimationService.cpp"
MoveFile "Source/Widgets/Core/AnimationService.h" "Source/Animation/AnimationService.h"
MoveFile "Source/Widgets/Core/AnimationServiceGlobal.cpp" "Source/Animation/AnimationServiceGlobal.cpp"
MoveFile "Source/Widgets/Core/SpringAnimation.cpp" "Source/Animation/SpringAnimation.cpp"
MoveFile "Source/Widgets/Core/SpringAnimation.h" "Source/Animation/SpringAnimation.h"
MoveFile "Source/Widgets/Core/StateTransition.cpp" "Source/Animation/StateTransition.cpp"
MoveFile "Source/Widgets/Core/StateTransition.h" "Source/Animation/StateTransition.h"
MoveFile "Source/Widgets/Core/MnemonicState.cpp" "Source/Interaction/Focus/MnemonicState.cpp"
MoveFile "Source/Widgets/Core/MnemonicStateGlobal.cpp" "Source/Interaction/Focus/MnemonicStateGlobal.cpp"
MoveFile "Source/Widgets/Core/UpdateGuard.cpp" "Source/Tree/UpdateGuard.cpp"
MoveFile "Source/Widgets/Core/DropZoneOverlay.cpp" "Source/Interaction/DropZoneOverlay.cpp"
MoveFile "Source/Widgets/Core/SvgIconProvider.cpp" "Source/Theming/SvgIconProvider.cpp"
MoveFile "Source/Widgets/Core/SvgIconProvider.h" "Source/Theming/SvgIconProvider.h"

# Remaining Source/Widgets/Core/ files (EventFilters, FlowLayout) stay as widget-private
# Move them to Source/Widgets/_Private/
$widgetCoreDir = Join-Path $root "Source/Widgets/Core"
if (Test-Path $widgetCoreDir) {
    Get-ChildItem -File $widgetCoreDir | ForEach-Object {
        MoveFile "Source/Widgets/Core/$($_.Name)" "Source/Widgets/_Private/$($_.Name)"
    }
}

# ============================================================================
# Move Tests/ to mirror new structure
# ============================================================================
Write-Host "`n--- Moving Tests/ files ---" -ForegroundColor Yellow

# Tests/Unit/Foundation -> various
$testRenames = @{
    "Tests/Unit/Foundation/StrongIdTest.cpp"           = "Tests/Unit/Core/StrongIdTest.cpp"
    "Tests/Unit/Foundation/ErrorCodeTest.cpp"           = "Tests/Unit/Core/ErrorCodeTest.cpp"
    "Tests/Unit/Foundation/ObserverPtrTest.cpp"         = "Tests/Unit/Core/ObserverPtrTest.cpp"
    "Tests/Unit/Foundation/StateMachineTest.cpp"        = "Tests/Unit/Core/StateMachineTest.cpp"
    "Tests/Unit/Foundation/ObservableTest.cpp"          = "Tests/Unit/Core/ObservableTest.cpp"
    "Tests/Unit/Foundation/InputEventTest.cpp"          = "Tests/Unit/Core/InputEventTest.cpp"
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
    "Tests/Unit/Foundation/BlueprintTest.cpp"           = "Tests/Unit/DSL/BlueprintTest.cpp"
    # Tests/Unit/Core -> various
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
    "Tests/Unit/Document/DocumentAreaTest.cpp"         = "Tests/Unit/Tree/DocumentAreaTest.cpp"
    "Tests/Unit/Document/ViewportGroupTest.cpp"        = "Tests/Unit/Tree/ViewportGroupTest.cpp"
    "Tests/Unit/Document/ViewportBindTest.cpp"         = "Tests/Unit/Tree/ViewportBindTest.cpp"
    "Tests/Unit/Document/ViewportGroupLayoutTest.cpp"  = "Tests/Unit/Tree/ViewportGroupLayoutTest.cpp"
    "Tests/Unit/Document/SplitTreeNodeTest.cpp"        = "Tests/Unit/Tree/SplitTreeNodeTest.cpp"
    "Tests/Unit/Shell/WorkspaceFrameTest.cpp"          = "Tests/Unit/Tree/WorkspaceFrameTest.cpp"
    "Tests/Unit/Query/UiNodeQueryTest.cpp"             = "Tests/Unit/Tree/UiNodeQueryTest.cpp"
    "Tests/Unit/Workbench/WorkshopRegistryTest.cpp"    = "Tests/Unit/Tree/WorkshopRegistryTest.cpp"
}

foreach ($kv in $testRenames.GetEnumerator()) {
    MoveFile $kv.Key $kv.Value
}

# ============================================================================
# STEP 2: Update all #include paths in .h and .cpp files
# ============================================================================
Write-Host "`n--- Updating #include paths ---" -ForegroundColor Yellow

# Collect all source files that might contain #include directives
$allFiles = @()
$allFiles += Get-ChildItem -Recurse -File -Include "*.h","*.cpp" (Join-Path $root "Include") -ErrorAction SilentlyContinue
$allFiles += Get-ChildItem -Recurse -File -Include "*.h","*.cpp" (Join-Path $root "Source") -ErrorAction SilentlyContinue
$allFiles += Get-ChildItem -Recurse -File -Include "*.h","*.cpp" (Join-Path $root "Tests") -ErrorAction SilentlyContinue
$allFiles += Get-ChildItem -Recurse -File -Include "*.h","*.cpp" (Join-Path $root "Demos") -ErrorAction SilentlyContinue

$totalReplacements = 0

foreach ($file in $allFiles) {
    $content = Get-Content $file.FullName -Raw -ErrorAction SilentlyContinue
    if (-not $content) { continue }

    $modified = $false
    $newContent = $content

    foreach ($kv in $includeMap.GetEnumerator()) {
        $old = $kv.Key
        $new = $kv.Value

        # Match both #include "..." and #include <...> forms
        # Also handle forward-slash and backslash
        $oldEscaped = [regex]::Escape($old)

        if ($newContent -match $oldEscaped) {
            $newContent = $newContent -replace $oldEscaped, $new
            $modified = $true
            $totalReplacements++
        }
    }

    if ($modified) {
        Set-Content -Path $file.FullName -Value $newContent -NoNewline
        Write-Host "  Updated: $($file.FullName.Replace($root, ''))" -ForegroundColor DarkGray
    }
}

Write-Host "  Total replacements: $totalReplacements" -ForegroundColor Green

# ============================================================================
# STEP 3: Clean up empty old directories
# ============================================================================
Write-Host "`n--- Cleaning up empty directories ---" -ForegroundColor Yellow

$oldDirs = @(
    "Include/Matcha/Foundation",
    "Include/Matcha/UiNodes/Core",
    "Include/Matcha/UiNodes/Controls",
    "Include/Matcha/UiNodes/Shell",
    "Include/Matcha/UiNodes/ActionBar",
    "Include/Matcha/UiNodes/Document",
    "Include/Matcha/UiNodes/Menu",
    "Include/Matcha/UiNodes/Workbench",
    "Include/Matcha/UiNodes",
    "Include/Matcha/Widgets/Core",
    "Source/Foundation",
    "Source/UiNodes/Core",
    "Source/UiNodes/Controls",
    "Source/UiNodes/Shell",
    "Source/UiNodes/ActionBar",
    "Source/UiNodes/Document",
    "Source/UiNodes/Menu",
    "Source/UiNodes/Workbench",
    "Source/UiNodes",
    "Source/Widgets/Core",
    "Tests/Unit/Foundation",
    "Tests/Unit/Document",
    "Tests/Unit/Shell",
    "Tests/Unit/Query",
    "Tests/Unit/Workbench"
)

foreach ($d in $oldDirs) {
    $fullPath = Join-Path $root $d
    if ((Test-Path $fullPath) -and ((Get-ChildItem $fullPath -Recurse -File).Count -eq 0)) {
        Remove-Item $fullPath -Recurse -Force
        Write-Host "  Removed empty: $d" -ForegroundColor DarkGray
    }
}

Write-Host "`n=== Reorganization complete ===" -ForegroundColor Cyan
Write-Host "Next step: Update CMakeLists.txt and rebuild."
