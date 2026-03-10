# Matcha

C++ GUI framework built on Qt 6, designed for CAD/CAE-class desktop applications.
Matcha provides a **UiNode abstraction layer** over Qt widgets, a **design-token theming engine**,
and a declarative **Workshop/Workbench architecture** inspired by the CATIA V5/3DEXPERIENCE platform.

---

## Key Features

| Category | Details |
|----------|---------|
| **Two-layer architecture** | UiNode tree (logic) + Nyan\* widget layer (rendering). Business code never touches Qt directly. |
| **Design-token theming** | Material 3 tonal palette, dynamic light/dark, WCAG 2.1 contrast audit, spring-physics animations. |
| **Workshop / Workbench** | Declarative descriptors, `WorkshopRegistry`, `WorkbenchManager` state machine, lazy command loading, push/pop stack, RAII `WorkbenchGuard`. |
| **Tabbed MDI documents** | `DocumentManager` + `DocumentArea` + `TabBarNode` with cross-window drag & drop. |
| **Multi-viewport** | `ViewportGroup` binary split tree, `IViewportRenderer` interface, viewport header bar overlay. |
| **50+ widgets** | CheckBox, ComboBox, Slider, RangeSlider, SpinBox, DataTable, PropertyGrid, ColorPicker, TreeWidget, Breadcrumb, Badge, Tag, ProgressRing, DateTimePicker, SearchBox, Paginator, CollapsibleSection, etc. |
| **Notification system** | Type-safe `Notification` classes, upward propagation through command tree, sync + async dispatch, 3-layer lifetime safety. |
| **Plugin system** | `IExpansionPlugin` + `PluginHost` for DLL-based addins; `IWorkshopContributor` for toolbar/command injection. |
| **C ABI** | `NyanCApi.h` stable FFI boundary for Python / Rust / C# bindings. |
| **Accessibility** | `A11yAudit`, contrast checking, focus management (`FocusManager`), keyboard navigation. |

---

## Architecture Overview

```
Layer 3  Application     NyanCad demo, business logic, plugins
Layer 2  Framework       WorkbenchManager, DocumentManager, Shell, Application
Layer 1  UiNode          WidgetNode, CommandNode, EventNode, Notification tree
Layer 0  Widget / Qt     Nyan* widgets (NyanPushButton, NyanActionBar, ...), QMainWindow
```

All public headers are in `Include/Matcha/`. The framework compiles to a single shared library (`Matcha.dll` / `libMatcha.so`).

**Design principle:** Business code programs against the UiNode API (Layer 1-2). The widget layer (Layer 0) is an implementation detail. This decoupling allows future rendering backends without breaking application code.

---

## Requirements

| Dependency | Version |
|------------|---------|
| C++ standard | C++23 |
| Compiler | Clang 21+ or MSVC 2022 |
| CMake | >= 3.28 |
| Qt | >= 6.7 (Widgets, Svg, Test modules) |
| Generator | Ninja (recommended) |

---

## Build

```bash
# Configure + build (debug)
cmake --preset debug
cmake --build --preset debug
```

Available CMake presets:

| Preset | Description |
|--------|-------------|
| `debug` | Debug build, `-Werror` enabled |
| `release` | Optimized release build |
| `relwithdebinfo` | Release with debug symbols |
| `asan` | Debug + AddressSanitizer |
| `tsan` | Debug + ThreadSanitizer |

All presets use Ninja as the generator and set `QT_QPA_PLATFORM=offscreen` for headless test execution.

---

## Test

```bash
ctest --preset debug
```

The test suite uses [doctest](https://github.com/doctest/doctest) and Qt Test:

- **Unit tests** (`Tests/Unit/`) -- pure logic, no `QApplication` required.
- **Integration tests** (`Tests/Integration/`) -- headless `QApplication` via `WidgetTestFixture`, offscreen rendering.

Running a specific test binary directly:

```bash
# Unit tests only
./build/debug/MatchaUnitTests.exe --no-color

# Integration tests only (requires Qt plugins in PATH)
./build/debug/MatchaIntegrationTests.exe --no-color
```

---

## Run Demo

```bash
# After building, run the NyanCad demo (requires Qt DLLs in PATH)
./build/debug/NyanCad.exe
```

NyanCad is a minimal CAD application demonstrating Workshop/Workbench switching, multi-document tabs, viewport splitting, and the full widget catalog.

---

## Quick Start

### Creating UiNodes

Business code works exclusively with UiNode wrappers -- never with Qt widgets directly:

```cpp
#include <Matcha/UiNodes/Controls/LabelNode.h>
#include <Matcha/UiNodes/Controls/PushButtonNode.h>
#include <Matcha/UiNodes/Controls/LineEditNode.h>

// Create typed UiNode wrappers
auto label = std::make_unique<matcha::fw::LabelNode>("my-label");
label->SetText("Hello Matcha");

auto button = std::make_unique<matcha::fw::PushButtonNode>("btn-ok");
button->SetText("OK");
button->SetIcon(matcha::fw::icons::Check);

auto input = std::make_unique<matcha::fw::LineEditNode>("input-name");
input->SetPlaceholder("Enter name...");
input->SetEnabled(true);

// Add to a container
container->AddNode(std::move(label));
container->AddNode(std::move(button));
container->AddNode(std::move(input));
```

### Workshop / Workbench Registration

Define workshops and workbenches declaratively, then let `WorkbenchManager` materialize the UI:

```cpp
#include <Matcha/UiNodes/Workbench/WorkshopRegistry.h>
#include <Matcha/UiNodes/Workbench/WorkbenchTypes.h>

using namespace matcha::fw;

// 1. Define commands
CommandHeaderDescriptor saveCmd;
saveCmd.id      = CmdHeaderId::From("cmd.save");
saveCmd.label   = "Save";
saveCmd.iconId  = "asset://matcha/icons/save";
saveCmd.tooltip = "Save Document";

// 2. Build tab blueprint
TabBlueprint fileTab;
fileTab.tabId = "file";
fileTab.label = "File";
ToolbarBlueprint tb;
tb.toolbarId = "file_ops";
tb.label     = "File Operations";
tb.commands  = { CmdHeaderId::From("cmd.save") };
fileTab.toolbars.push_back(std::move(tb));

// 3. Assemble workshop descriptor
WorkshopDescriptor ws;
ws.id    = WorkshopId::From("mesh");
ws.label = "Mesh";
ws.commands.push_back(std::move(saveCmd));
ws.baseTabs.push_back(std::move(fileTab));
ws.defaultWorkbenchId = WorkbenchId::From("surface_mesh");
ws.workbenchIds = { WorkbenchId::From("surface_mesh") };

// 4. Register and activate
registry.RegisterWorkshop(std::move(ws));
// ... register workbenches ...
wbMgr->ActivateWorkshop(WorkshopId::From("mesh"));
```

### Subscribing to Notifications

Notifications propagate upward through the command tree:

```cpp
#include <Matcha/UiNodes/Core/EventNode.h>

// Subscribe to a specific notification type from a sender
auto sub = subscriber->Subscribe<matcha::fw::WorkbenchActivated>(
    sender,
    [](matcha::fw::WorkbenchActivated& notif) {
        // Handle workbench activation
    });

// ScopedSubscription auto-unsubscribes on destruction
// Store it in a member variable for lifetime management
```

---

## Widget Catalog

### Input Controls

| UiNode | Widget | Description |
|--------|--------|-------------|
| `LineEditNode` | `NyanLineEdit` | Single-line text input with validation |
| `PlainTextEditNode` | -- | Multi-line plain text editor |
| `SpinBoxNode` | `NyanSpinBox` | Integer spinner |
| `DoubleSpinBoxNode` | `NyanDoubleSpinBox` | Floating-point spinner |
| `SliderNode` | `NyanSlider` | Single-value slider |
| `RangeSliderNode` | `NyanRangeSlider` | Dual-handle range slider |
| `ComboBoxNode` | `NyanComboBox` | Dropdown selection |
| `CheckBoxNode` | `NyanCheckBox` | Boolean checkbox |
| `RadioButtonNode` | `NyanRadioButton` | Exclusive radio button |
| `ToggleSwitchNode` | `NyanToggleSwitch` | On/off toggle |
| `SearchBoxNode` | `NyanSearchBox` | Search input with filtering |
| `ColorPickerNode` | `NyanColorPicker` | Color selection dialog |
| `DateTimePickerNode` | `NyanDateTimePicker` | Date/time input |

### Display Controls

| UiNode | Widget | Description |
|--------|--------|-------------|
| `LabelNode` | `NyanLabel` | Static text / rich text display |
| `BadgeNode` | `NyanBadge` | Numeric or dot badge |
| `ProgressBarNode` | `NyanProgressBar` | Linear progress indicator |
| `ProgressRingNode` | `NyanProgressRing` | Circular progress indicator |
| `ColorSwatchNode` | `NyanColorSwatch` | Color preview tile |
| `LineNode` | `NyanLine` | 1px themed separator |

### Action Controls

| UiNode | Widget | Description |
|--------|--------|-------------|
| `PushButtonNode` | `NyanPushButton` | Standard button (primary / secondary / ghost) |
| `ToolButtonNode` | `NyanToolButton` | Compact toolbar button |

### Data Controls

| UiNode | Widget | Description |
|--------|--------|-------------|
| `DataTableNode` | `NyanDataTable` | Sortable/filterable data grid |
| `ListWidgetNode` | `NyanListWidget` | Vertical list |
| `TreeWidgetNode` | `NyanStructureTree` | Hierarchical tree with pin button |
| `PropertyGridNode` | `NyanPropertyGrid` | Key-value property editor |
| `PaginatorNode` | `NyanPaginator` | Page navigation |

### Layout & Navigation

| UiNode | Widget | Description |
|--------|--------|-------------|
| `CollapsibleSectionNode` | `NyanCollapsibleSection` | Expandable section with header |
| -- | `NyanBreadcrumb` | Path-style navigation |
| -- | `NyanTag` | Removable tag / chip |
| -- | `NyanGroupBox` | Titled container box |

### Feedback

| Widget | Description |
|--------|-------------|
| `NyanRichTooltip` | Two-tier rich tooltip (brief + detailed) with icon, shortcut, preview |
| `NyanMessage` | Inline message bar (info / success / warning / error) |
| `NyanNotification` | Toast notification with auto-dismiss |
| `NyanPopConfirm` | Confirmation popover |
| `NyanInputDialog` | Modal input dialog |

---

## Design Token Theming

Matcha uses a Material 3 inspired design token system:

- **Color tokens**: `Primary`, `OnPrimary`, `Surface`, `OnSurface`, `Error`, etc.
- **Tonal palette**: Auto-generated from a seed color via `TonalPaletteGenerator`.
- **Typography tokens**: `DisplayLarge` .. `LabelSmall` (15 levels).
- **Spacing tokens**: `Px2`, `Px4`, `Px8`, `Px12`, `Px16`, `Px24`, `Px32`.
- **Elevation tokens**: `Level0` .. `Level5` with shadow parameters.
- **Animation tokens**: Spring-physics based (`SpringAnimation`), state transitions (`StateTransition`).
- **Icon system**: Open URI-based `IconId` (`asset://matcha/icons/save`), SVG colorization, runtime directory registration.

Theme changes propagate automatically to all `ThemeAware` widgets via the `OnThemeChanged()` virtual.

---

## Notification System

Matcha uses a type-safe notification architecture for decoupled communication:

```
Sender --> Parent --> Grandparent --> ... --> Shell (root)
```

- **Direction**: Notifications propagate **upward** through the `CommandNode` tree.
- **Dispatch modes**: Synchronous (`SendNotification`) and asynchronous (`SendNotificationQueued`).
- **Lifetime safety**: 3-layer defense (subscriber token, publisher token, generation counter).
- **Subscription**: `ScopedSubscription` RAII for automatic cleanup.

Notification categories:

| Category | Examples |
|----------|----------|
| Widget | `VisibilityChanged`, `EnabledChanged`, `FocusChanged` |
| Drag & Drop | `DragEntered`, `DragMoved`, `DragLeft`, `Dropped` |
| Document | `DocumentCreated`, `DocumentSwitched`, `DocumentClosing`, `DocumentClosed` |
| Tab | `TabDroppedIn`, `TabReordered`, `TabPageDraggedOut` |
| Viewport | `ActiveVpChanged`, `VpCreated`, `VpRemoved`, `VpSplitRatioChanged` |
| Workbench | `WorkshopActivated`, `WorkbenchActivated`, `CommandInvoked` |

---

## Project Structure

```
Matcha/
  Include/Matcha/
    Foundation/        Core types: StrongId, StringId, observer_ptr, WidgetEnums
    UiNodes/
      Core/            WidgetNode, CommandNode, EventNode, FocusManager, MetaClass
      Shell/           Shell, WindowNode, MainTitleBarNode, StatusBarNode, WorkspaceFrame
      ActionBar/       ActionBarNode, ActionTabNode, ActionToolbarNode, ActionButtonNode
      Controls/        30 typed UiNode wrappers (LabelNode .. TreeWidgetNode)
      Document/        DocumentArea, DocumentPage, TabBarNode, Viewport, ViewportGroup
      Menu/            MenuBarNode, MenuNode, DialogNode
      Workbench/       WorkbenchManager, WorkshopRegistry, WorkbenchTypes
    Widgets/
      Core/            NyanTheme, AnimationService, SvgIconProvider, UpdateGuard
      Controls/        41 themed Qt widgets (NyanPushButton .. NyanStructureTree)
      ActionBar/       NyanActionBar, NyanPanel, ActionBarFloatingFrame
      Shell/           NyanMainTitleBar, ViewportFrame, NyanSplitter, NyanScrollArea
      Menu/            NyanMenu, NyanDialog, NyanPopConfirm, NyanInputDialog
    Services/          DocumentManager, PluginHost, IViewportRenderer
    CApi/              C ABI header (NyanCApi.h)
  Source/              Implementation (.cpp)
  Tests/
    Unit/              doctest unit tests (logic, no Qt event loop)
    Integration/       doctest + QApplication integration tests
    TestUtils/         WidgetTestFixture, test helpers
  Demos/NyanCad/       Demo CAD application
  Resources/           Icons, palettes, stylesheets
  docs/                Design System Specification
```

---

## Documentation

The full design specification is at `docs/Matcha_Design_System_Specification.md` (~9000 lines), covering:

- Design tokens (color, typography, spacing, elevation, animation)
- Widget specifications (28 chapters, each with states, anatomy, theming, a11y)
- Workshop/Workbench architecture (Appendix C)
- Business-layer ABI boundary (Appendix D)
- Application architecture glossary (Appendix E)

---

## Coding Standards

| Rule | Detail |
|------|--------|
| Language standard | C++23, no modules (header-only public API) |
| Naming: classes | `PascalCase` (`WidgetNode`, `WorkshopRegistry`) |
| Naming: methods | `PascalCase` (`SetEnabled`, `ActivateWorkshop`) |
| Naming: members | `_camelCase` prefix (`_activeWorkshopId`) |
| Naming: constants | `kPascalCase` (`kDefaultTier1`, `kMaxWidth`) |
| Documentation | Doxygen-style (`@brief`, `@param`, `@return`) |
| Error handling | Return `bool` or `ErrorCode`; no exceptions across DLL boundaries |
| Thread safety | All UI mutations assert Qt main thread |
| Memory | `std::unique_ptr` ownership, `observer_ptr` for non-owning refs |

---

## License

Proprietary.
