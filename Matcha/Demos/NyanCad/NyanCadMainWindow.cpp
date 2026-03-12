/**
 * @file NyanCadMainWindow.cpp
 * @brief NyanCadMainWindow implementation.
 *
 * Configures the main WindowNode using the UiNode tree API:
 *   TitleBar: Row 1 (File menu, window buttons) + Row 2 (ModuleCombo, DocumentBar)
 *   ActionBar: tabs + toolbars + buttons
 *   StatusBar: item-based left/right labels
 *   DocumentArea: tab-based MDI (tab bar hidden -- DocumentBar in TitleBar)
 */

#include "NyanCadMainWindow.h"

#include "DocumentView.h"

#include "Matcha/UiNodes/ActionBar/ActionBarNode.h"
#include "Matcha/UiNodes/ActionBar/ActionButtonNode.h"
#include "Matcha/UiNodes/ActionBar/ActionTabNode.h"
#include "Matcha/UiNodes/ActionBar/ActionToolbarNode.h"
#include "Matcha/UiNodes/Controls/BadgeNode.h"
#include "Matcha/UiNodes/Controls/CheckBoxNode.h"
#include "Matcha/UiNodes/Controls/CollapsibleSectionNode.h"
#include "Matcha/UiNodes/Controls/ColorPickerNode.h"
#include "Matcha/UiNodes/Controls/ColorSwatchNode.h"
#include "Matcha/UiNodes/Controls/ComboBoxNode.h"
#include "Matcha/UiNodes/Controls/DataTableNode.h"
#include "Matcha/UiNodes/Controls/DateTimePickerNode.h"
#include "Matcha/UiNodes/Controls/DoubleSpinBoxNode.h"
#include "Matcha/UiNodes/Controls/LabelNode.h"
#include "Matcha/UiNodes/Controls/LineEditNode.h"
#include "Matcha/UiNodes/Controls/LineNode.h"
#include "Matcha/UiNodes/Controls/ListWidgetNode.h"
#include "Matcha/UiNodes/Controls/MessageNode.h"
#include "Matcha/UiNodes/Controls/NotificationNode.h"
#include "Matcha/UiNodes/Controls/PaginatorNode.h"
#include "Matcha/UiNodes/Controls/PlainTextEditNode.h"
#include "Matcha/UiNodes/Controls/ProgressBarNode.h"
#include "Matcha/UiNodes/Controls/ProgressRingNode.h"
#include "Matcha/UiNodes/Controls/PropertyGridNode.h"
#include "Matcha/UiNodes/Controls/PushButtonNode.h"
#include "Matcha/UiNodes/Controls/RadioButtonNode.h"
#include "Matcha/UiNodes/Controls/RangeSliderNode.h"
#include "Matcha/UiNodes/Controls/SearchBoxNode.h"
#include "Matcha/UiNodes/Controls/SliderNode.h"
#include "Matcha/UiNodes/Controls/SpinBoxNode.h"
#include "Matcha/UiNodes/Controls/ToggleSwitchNode.h"
#include "Matcha/UiNodes/Controls/ToolButtonNode.h"
#include "Matcha/UiNodes/Controls/TreeWidgetNode.h"
#include "Matcha/UiNodes/Core/ContainerNode.h"
#include "Matcha/UiNodes/Core/UiNodeNotification.h"
#include "Matcha/UiNodes/Document/DocumentArea.h"
#include "Matcha/Services/DocumentManager.h"
#include "Matcha/UiNodes/Document/TabBarNode.h"
#include "Matcha/UiNodes/Menu/DialogNode.h"
#include "Matcha/UiNodes/Menu/MenuBarNode.h"
#include "Matcha/UiNodes/Menu/MenuItemNode.h"
#include "Matcha/UiNodes/Menu/MenuNode.h"
#include "Matcha/Widgets/Core/IAnimationService.h"
#include "Matcha/Widgets/Core/MnemonicState.h"

#include "Matcha/UiNodes/Controls/TreeItemNode.h"
#include "Matcha/UiNodes/Shell/Application.h"
#include "Matcha/UiNodes/Shell/FloatingTabWindowNode.h"
#include "Matcha/UiNodes/Shell/DocumentToolBarNode.h"
#include "Matcha/UiNodes/Shell/MainTitleBarNode.h"
#include "Matcha/UiNodes/Shell/Shell.h"
#include "Matcha/UiNodes/Shell/StatusBarNode.h"
#include "Matcha/UiNodes/Shell/WindowNode.h"
#include "Matcha/UiNodes/Shell/WorkspaceFrame.h"

namespace nyancad {

NyanCadMainWindow::~NyanCadMainWindow()
{
    Teardown();
}

void NyanCadMainWindow::Teardown()
{
    // Destroy all business-layer objects that hold ScopedSubscriptions to
    // UiNode tree nodes. This MUST happen while the UiNode tree is still
    // alive, i.e. before Application::Shutdown().
    _floatingDocStates.clear();
    delete _docView;
    _docView = nullptr;
}

// ============================================================================
// Helper: build left sidebar panel with PropertyGrid
// ============================================================================

static auto BuildLeftSidebar() -> std::unique_ptr<matcha::fw::ContainerNode>
{
    using namespace matcha::fw;
    using matcha::gui::PropertyType;
    using matcha::gui::LabelRole;

    auto panel = std::make_unique<ContainerNode>("left-sidebar", LayoutKind::Vertical);
    panel->SetMargins(SpacingToken::Px4);
    panel->SetSpacing(SpacingToken::Px4);
    panel->SetMinimumSize(220, 0);
    panel->SetMaximumSize(360, 16777215);

    // Title
    auto titleNode = std::make_unique<LabelNode>("lbl-props");
    titleNode->SetText("Properties");
    titleNode->SetRole(LabelRole::Title);
    panel->AddNode(std::move(titleNode));

    // Property grid: Mesh Parameters
    auto propNode = std::make_unique<PropertyGridNode>("prop-grid");
    propNode->AddGroup("Mesh Parameters");
    propNode->AddProperty("Element Size", PropertyType::Double, 2.5);
    propNode->AddProperty("Min Size",     PropertyType::Double, 0.5);
    propNode->AddProperty("Max Size",     PropertyType::Double, 10.0);
    propNode->AddProperty("Growth Rate",  PropertyType::Double, 1.2);
    const std::array<std::string, 5> elemChoices = {"Tri", "Quad", "Hex", "Tet", "Mixed"};
    propNode->AddProperty("Element Type", PropertyType::Choice, "Quad",
                          std::span<const std::string>(elemChoices));

    propNode->AddGroup("Quality");
    propNode->AddProperty("Min Angle",     PropertyType::Double, 15.0);
    propNode->AddProperty("Max Aspect",    PropertyType::Double, 5.0);
    propNode->AddProperty("Check Quality", PropertyType::Bool, true);

    propNode->AddGroup("Display");
    propNode->AddProperty("Show Mesh",    PropertyType::Bool, true);
    propNode->AddProperty("Show Normals", PropertyType::Bool, false);
    const std::array<std::string, 3> colorChoices = {"By Part", "By Quality", "Uniform"};
    propNode->AddProperty("Color Mode", PropertyType::Choice, "By Part",
                          std::span<const std::string>(colorChoices));

    panel->AddNode(std::move(propNode));

    return panel;
}

// ============================================================================
// Helper: build right properties/form panel
// ============================================================================

static auto BuildRightPanel() -> std::unique_ptr<matcha::fw::ContainerNode>
{
    using namespace matcha::fw;
    using matcha::gui::LabelRole;
    using matcha::gui::ButtonVariant;
    using matcha::gui::HAlign;

    auto panel = std::make_unique<ContainerNode>("right-panel", LayoutKind::Vertical);
    panel->SetMargins(SpacingToken::Px6);
    panel->SetSpacing(SpacingToken::Px6);
    panel->SetMinimumSize(200, 0);
    panel->SetMaximumSize(320, 16777215);

    // Title
    auto titleNode = std::make_unique<LabelNode>("lbl-inspector");
    titleNode->SetText("Inspector");
    titleNode->SetRole(LabelRole::Title);
    panel->AddNode(std::move(titleNode));

    // Name field
    auto nameLabel = std::make_unique<LabelNode>("lbl-name");
    nameLabel->SetText("Name");
    nameLabel->SetRole(LabelRole::Name);
    panel->AddNode(std::move(nameLabel));
    auto nameEdit = std::make_unique<LineEditNode>("edit-name");
    nameEdit->SetText("Part_Body_001");
    panel->AddNode(std::move(nameEdit));

    // Material combo
    auto matLabel = std::make_unique<LabelNode>("lbl-material");
    matLabel->SetText("Material");
    matLabel->SetRole(LabelRole::Name);
    panel->AddNode(std::move(matLabel));
    auto matCombo = std::make_unique<ComboBoxNode>("combo-material");
    matCombo->AddItem("Steel");
    matCombo->AddItem("Aluminum");
    matCombo->AddItem("Titanium");
    matCombo->AddItem("CFRP");
    matCombo->AddItem("Concrete");
    panel->AddNode(std::move(matCombo));

    // Thickness (as a line edit — simplified since LineEditNode doesn't expose InputMode yet)
    auto thickLabel = std::make_unique<LabelNode>("lbl-thickness");
    thickLabel->SetText("Thickness");
    thickLabel->SetRole(LabelRole::Name);
    panel->AddNode(std::move(thickLabel));
    auto thickEdit = std::make_unique<LineEditNode>("edit-thickness");
    thickEdit->SetText("1.500");
    thickEdit->SetPlaceholder("mm");
    panel->AddNode(std::move(thickEdit));

    // Checkboxes
    auto chk1 = std::make_unique<CheckBoxNode>("chk-autorefine");
    chk1->SetText("Auto-refine");
    chk1->SetChecked(true);
    panel->AddNode(std::move(chk1));
    auto chk2 = std::make_unique<CheckBoxNode>("chk-preserve");
    chk2->SetText("Preserve features");
    panel->AddNode(std::move(chk2));

    // Progress bar
    auto progressLabel = std::make_unique<LabelNode>("lbl-progress");
    progressLabel->SetText("Mesh Progress");
    progressLabel->SetRole(LabelRole::Caption);
    panel->AddNode(std::move(progressLabel));
    auto progress = std::make_unique<ProgressBarNode>("progress-mesh");
    progress->SetRange(0, 100);
    progress->SetValue(67);
    panel->AddNode(std::move(progress));

    // Buttons row
    auto btnRow = std::make_unique<ContainerNode>("btn-row", LayoutKind::Horizontal);
    btnRow->SetMargins(SpacingToken::None, SpacingToken::Px4,
                        SpacingToken::None, SpacingToken::None);
    btnRow->SetSpacing(SpacingToken::Px4);
    auto applyBtn = std::make_unique<PushButtonNode>("btn-apply");
    applyBtn->SetText("Apply");
    applyBtn->SetIcon(matcha::fw::icons::Check);
    applyBtn->SetIconSize(matcha::fw::IconSize::Sm);
    applyBtn->SetVariant(ButtonVariant::Primary);
    btnRow->AddNode(std::move(applyBtn));
    auto resetBtn = std::make_unique<PushButtonNode>("btn-reset");
    resetBtn->SetText("Reset");
    resetBtn->SetIcon(matcha::fw::icons::Refresh);
    resetBtn->SetIconSize(matcha::fw::IconSize::Sm);
    resetBtn->SetVariant(ButtonVariant::Ghost);
    btnRow->AddNode(std::move(resetBtn));
    panel->AddNode(std::move(btnRow));

    return panel;
}

// ============================================================================
// Setup
// ============================================================================

void NyanCadMainWindow::Setup(matcha::fw::Application& app)
{
    _app = &app;
    auto* _docMgr = app.GetDocumentManagerImpl();
    auto& mainWin = app.MainWindow();
    mainWin.SetTitle("NyanCad -- Multi-Window Multi-Viewport Demo");
    mainWin.Resize(1400, 900);

    // -- TitleBar (Row 1 + Row 2) -------------------------------------------
    auto titleBarObs = mainWin.GetTitleBar();
    auto* titleBarNode = dynamic_cast<matcha::fw::MainTitleBarNode*>(titleBarObs.get());
    if (titleBarNode != nullptr) {
        titleBarNode->SetTitle("NyanCad");

        // Row 1: Menu bar via MenuBarNode (auto-created by MainTitleBarNode)
        auto menuBarObs = titleBarNode->GetMenuBar();
        if (menuBarObs.get() != nullptr) {
            // -- File menu with 3-level submenu chain --
            auto* fileMenu = menuBarObs->AddMenu("&File");
            fileMenu->AddItem("New");
            fileMenu->AddItem("Open");
            auto* recentSub = fileMenu->AddSubmenu("Open Recent");
            recentSub->AddItem("Part1.stp");
            recentSub->AddItem("Mesh1.fem");
            recentSub->AddSeparator();
            auto* templateSub = recentSub->AddSubmenu("From Template");
            templateSub->AddItem("Blank Part");
            templateSub->AddItem("Sheet Metal");
            templateSub->AddItem("Assembly");
            auto* importSub = fileMenu->AddSubmenu("Import");
            importSub->AddItem("STEP (.stp)");
            importSub->AddItem("IGES (.igs)");
            importSub->AddItem("STL (.stl)");
            importSub->AddItem("Nastran (.bdf)");
            auto* exportSub = fileMenu->AddSubmenu("Export");
            exportSub->AddItem("STEP (.stp)");
            exportSub->AddItem("IGES (.igs)");
            exportSub->AddItem("STL (.stl)");
            exportSub->AddItem("PDF Report");
            fileMenu->AddSeparator();
            fileMenu->AddItem("Save");
            fileMenu->AddItem("Save As...");
            fileMenu->AddItem("Save All");
            fileMenu->AddSeparator();
            fileMenu->AddItem("Close");

            // -- Edit menu with multiple submenus for hover-switch demo --
            auto* editMenu = menuBarObs->AddMenu("&Edit");
            editMenu->AddItem("Undo");
            editMenu->AddItem("Redo");
            editMenu->AddSeparator();
            editMenu->AddItem("Cut");
            editMenu->AddItem("Copy");
            editMenu->AddItem("Paste");
            editMenu->AddSeparator();
            auto* findSub = editMenu->AddSubmenu("Find");
            findSub->AddItem("Find...");
            findSub->AddItem("Find and Replace...");
            findSub->AddItem("Find in Files...");
            findSub->AddItem("Go to Line...");
            auto* refactorSub = editMenu->AddSubmenu("Refactor");
            refactorSub->AddItem("Rename Symbol...");
            refactorSub->AddItem("Extract Method...");
            refactorSub->AddItem("Inline Variable");
            editMenu->AddSeparator();
            editMenu->AddItem("Select All");
            editMenu->AddItem("Preferences...");

            // -- View menu with submenus --
            auto* viewMenu = menuBarObs->AddMenu("&View");
            viewMenu->AddItem("Zoom In");
            viewMenu->AddItem("Zoom Out");
            viewMenu->AddItem("Fit All");
            viewMenu->AddSeparator();
            auto* panelsSub = viewMenu->AddSubmenu("Panels");
            panelsSub->AddCheckItem("Properties", true);
            panelsSub->AddCheckItem("Inspector", true);
            panelsSub->AddCheckItem("Output Log", false);
            panelsSub->AddCheckItem("Model Tree", true);
            auto* layoutSub = viewMenu->AddSubmenu("Layout");
            layoutSub->AddItem("Single Viewport");
            layoutSub->AddItem("2x2 Grid");
            layoutSub->AddItem("Top + Bottom");
            layoutSub->AddItem("Left + Right");
            layoutSub->AddItem("Reset Layout");
            viewMenu->AddSeparator();
            viewMenu->AddCheckItem("Show Grid", true);
            viewMenu->AddCheckItem("Show Wireframe", false);
            viewMenu->AddCheckItem("Show Normals", false);

            // -- Mesh menu --
            auto* meshMenu = menuBarObs->AddMenu("&Mesh");
            meshMenu->AddItem("Generate Surface Mesh");
            meshMenu->AddItem("Generate Volume Mesh");
            meshMenu->AddSeparator();
            auto* qualitySub = meshMenu->AddSubmenu("Quality Check");
            qualitySub->AddItem("Aspect Ratio");
            qualitySub->AddItem("Skewness");
            qualitySub->AddItem("Jacobian");
            qualitySub->AddItem("Full Report");
            meshMenu->AddItem("Refine...");
            meshMenu->AddItem("Coarsen...");
            meshMenu->AddSeparator();
            meshMenu->AddItem("Clear Mesh");

            // -- Help menu --
            auto* helpMenu = menuBarObs->AddMenu("&Help");
            helpMenu->AddItem("User Guide");
            helpMenu->AddItem("API Reference");
            helpMenu->AddSeparator();
            helpMenu->AddItem("About NyanCad...");
        }

        // Row 1: Quick Access Toolbar (after menu bar)
        auto qatSlot = titleBarNode->GetQuickCommandSlot();
        if (qatSlot.get() != nullptr) {
            using matcha::fw::ToolButtonNode;
            using matcha::fw::LineNode;
            qatSlot->SetSpacing(matcha::fw::SpacingToken::Px2);

            auto btnSave = std::make_unique<ToolButtonNode>("qat-save");
            btnSave->SetText("Save");
            btnSave->SetToolTip("Save (Ctrl+S)");
            qatSlot->AddNode(std::move(btnSave));

            auto btnUndo = std::make_unique<ToolButtonNode>("qat-undo");
            btnUndo->SetText("Undo");
            btnUndo->SetToolTip("Undo (Ctrl+Z)");
            qatSlot->AddNode(std::move(btnUndo));

            auto btnRedo = std::make_unique<ToolButtonNode>("qat-redo");
            btnRedo->SetText("Redo");
            btnRedo->SetToolTip("Redo (Ctrl+Y)");
            qatSlot->AddNode(std::move(btnRedo));

            auto sep = std::make_unique<LineNode>("qat-sep");
            qatSlot->AddNode(std::move(sep));

            auto btnFit = std::make_unique<ToolButtonNode>("qat-fit");
            btnFit->SetText("Fit");
            btnFit->SetToolTip("Fit All (F)");
            qatSlot->AddNode(std::move(btnFit));

            auto btnWire = std::make_unique<ToolButtonNode>("qat-wireframe");
            btnWire->SetText("Wire");
            btnWire->SetToolTip("Toggle Wireframe");
            btnWire->SetCheckable(true);
            qatSlot->AddNode(std::move(btnWire));
        }
    }

    // -- DocumentToolBar (Row 2: ModuleCombo + TabBar + GlobalButtons) ----------
    auto docToolBarObs = mainWin.GetDocumentToolBar();
    auto* docToolBarNode = docToolBarObs.get();
    if (docToolBarNode != nullptr) {
        const std::array<std::string, 3> modules = {"Mesh", "Part Design", "Sketch"};
        docToolBarNode->SetModuleItems(modules);
        docToolBarNode->SetCurrentModule("Mesh");

        // Global buttons (right side)
        auto globalSlot = docToolBarNode->GetGlobalButtonSlot();
        if (globalSlot.get() != nullptr) {
            globalSlot->SetSpacing(matcha::fw::SpacingToken::Px4);
            auto searchBtn = std::make_unique<matcha::fw::PushButtonNode>("btn-search");
            searchBtn->SetIcon(matcha::fw::icons::Search);
            searchBtn->SetIconSize(matcha::fw::IconSize::Sm);
            searchBtn->SetFixedSize(28, 28);
            globalSlot->AddNode(std::move(searchBtn));
            auto settingsBtn = std::make_unique<matcha::fw::PushButtonNode>("btn-settings");
            settingsBtn->SetIcon(matcha::fw::icons::Settings);
            settingsBtn->SetIconSize(matcha::fw::IconSize::Sm);
            settingsBtn->SetFixedSize(28, 28);
            globalSlot->AddNode(std::move(settingsBtn));
            auto helpBtn = std::make_unique<matcha::fw::PushButtonNode>("btn-help");
            helpBtn->SetIcon(matcha::fw::icons::Help);
            helpBtn->SetIconSize(matcha::fw::IconSize::Sm);
            helpBtn->SetFixedSize(28, 28);
            globalSlot->AddNode(std::move(helpBtn));
        }
    }

    // -- ActionBar tabs are now materialized by WorkbenchManager ----------
    // (see NyanCadApp::Run -> ActivateWorkshop after Setup)
    auto& shell = app.GetShell();

    // -- StatusBar (via UiNode tree) ------------------------------------------
    auto statusBarPtr = shell.GetStatusBar();
    if (statusBarPtr.get() != nullptr) {
        statusBarPtr->AddLabel("msg", "Ready -- NyanCad",
                               matcha::gui::StatusBarSide::Left);
        statusBarPtr->AddLabel("theme-info", "Dark Theme",
                               matcha::gui::StatusBarSide::Right);
    }

    // -- Central layout: HSplitter(sidebar | docArea | inspector) --
    // Wrap ContentArea with a ContainerNode, then add an HSplitter child.
    auto* central = mainWin.ContentArea();
    if (central != nullptr) {
        using namespace matcha::fw;
        using matcha::gui::HAlign;

        auto centralContainer = ContainerNode::Wrap("central", central);
        auto splitter = std::make_unique<ContainerNode>("main-splitter", LayoutKind::HSplitter);

        // Left sidebar
        auto leftSidebar = BuildLeftSidebar();
        auto* leftWidget = leftSidebar->Widget();
        splitter->AddNode(std::move(leftSidebar));

        // Center: DocumentView (per-window document manager view)
        auto mainWinId = mainWin.Id();
        auto tabBarObs = docToolBarNode ? docToolBarNode->GetTabBar()
            : observer_ptr<TabBarNode>{};

        // Get DocumentArea from the framework UiNode tree (owned by WorkspaceFrame)
        auto mainWsFrame = mainWin.GetWorkspaceFrame();
        auto* docArea = mainWsFrame ? mainWsFrame->GetDocumentArea().get() : nullptr;

        _docView = new DocumentView(mainWinId, *_docMgr, tabBarObs.get(), docArea);

        // Subscribe on DocumentArea for tab drag/drop Notifications
        if (docArea != nullptr) {
            docArea->SetAcceptDrops(true);
            docArea->Subscribe(docArea, "DragEntered",
                [](matcha::EventNode&, matcha::Notification& n) {
                    if (auto* e = n.As<DragEntered>()) {
                        for (const auto& mt : e->MimeTypes()) {
                            if (mt == "application/x-matcha-tab") {
                                n.SetAccepted(true);
                                return;
                            }
                        }
                    }
                });
            docArea->Subscribe(docArea, "DragMoved",
                [](matcha::EventNode&, matcha::Notification& n) {
                    n.SetAccepted(true);
                });
            docArea->Subscribe(docArea, "Dropped",
                [this, mainWinId](matcha::EventNode&, matcha::Notification& n) {
                    if (auto* e = n.As<Dropped>()) {
                        if (e->MimeType() == "application/x-matcha-tab" && !e->Data().empty()) {
                            auto pageIdVal = std::stoull(std::string(e->Data().begin(), e->Data().end()));
                            auto pageId = PageId::From(pageIdVal);
                            if (_app->GetDocumentManagerImpl()) { (void)_app->GetDocumentManagerImpl()->MoveDocumentPage(pageId, mainWinId); }
                            n.SetAccepted(true);
                        }
                    }
                });
            docArea->Subscribe(docArea, "TabPageDraggedOut",
                [this](matcha::EventNode&, matcha::Notification& n) {
                    if (auto* e = n.As<TabPageDraggedOut>()) {
                        CreateFloatingWindowForPage(e->GetPageId(), e->GlobalX(), e->GlobalY());
                    }
                });
        }
        // -- Context Menu demo: right-click on DocumentArea shows context menu --
        if (docArea != nullptr) {
            docArea->SetContextMenuEnabled(true);

            // DocumentArea-level contributions (closest to click source)
            docArea->Subscribe(nullptr, "ContextMenuRequest",
                [](matcha::EventNode&, matcha::Notification& n) {
                    auto* req = n.As<ContextMenuRequest>();
                    if (!req) { return; }

                    // Contribute viewport-specific items
                    auto fitAll = std::make_unique<MenuItemNode>("ctx-fit-all");
                    fitAll->SetText("Fit All");
                    req->AddNode(std::move(fitAll));

                    auto zoomIn = std::make_unique<MenuItemNode>("ctx-zoom-in");
                    zoomIn->SetText("Zoom In");
                    req->AddNode(std::move(zoomIn));

                    auto zoomOut = std::make_unique<MenuItemNode>("ctx-zoom-out");
                    zoomOut->SetText("Zoom Out");
                    req->AddNode(std::move(zoomOut));
                });

            // Shell-level contributions (global items added at top of command tree)
            shell.Subscribe(nullptr, "ContextMenuRequest",
                [](matcha::EventNode&, matcha::Notification& n) {
                    auto* req = n.As<ContextMenuRequest>();
                    if (!req) { return; }

                    auto prefs = std::make_unique<MenuItemNode>("ctx-preferences");
                    prefs->SetText("Preferences...");
                    req->AddNode(std::move(prefs));

                    auto about = std::make_unique<MenuItemNode>("ctx-about");
                    about->SetText("About NyanCad");
                    req->AddNode(std::move(about));
                });

            // Wire the popup: Shell handles ContextMenuRequest after all
            // ancestors have contributed. Collect nodes and show the menu.
            shell.Subscribe(nullptr, "ContextMenuRequest",
                [&shell](matcha::EventNode&, matcha::Notification& n) {
                    auto* req = n.As<ContextMenuRequest>();
                    if (!req) { return; }

                    auto* ctxMenu = shell.GetContextMenu();
                    if (!ctxMenu) { return; }

                    // Clear previous contents
                    ctxMenu->ClearAll();

                    // Install all contributed nodes
                    auto nodes = req->TakeNodes();
                    for (auto& node : nodes) {
                        ctxMenu->InstallNode(std::move(node));
                    }

                    // Popup at click position
                    ctxMenu->Popup(QPoint(req->GlobalX(), req->GlobalY()));
                });
        }

        // Wrap DocumentArea's widget as a ContainerNode child in the splitter
        auto docViewNode = ContainerNode::Wrap("doc-view-wrap", docArea ? docArea->Widget() : nullptr);
        splitter->AddNode(std::move(docViewNode));

        // Right panel
        auto rightPanel = BuildRightPanel();
        auto* rightWidget = rightPanel->Widget();
        splitter->AddNode(std::move(rightPanel));

        // Sidebars hidden by default
        if (leftWidget != nullptr) { leftWidget->hide(); }
        if (rightWidget != nullptr) { rightWidget->hide(); }

        centralContainer->AddNode(std::move(splitter));

        // Transfer ownership to mainWin's UiNode tree
        mainWin.AddNode(std::move(centralContainer));
    }

}

void NyanCadMainWindow::OnOpenDialog()
{
    using namespace matcha::fw;
    using matcha::gui::HAlign;

    if (_app == nullptr) { return; }
    auto& mainWin = _app->MainWindow();

    // Create a modeless (non-modal) dialog -- embedded in WorkspaceFrame
    auto dialogNode = std::make_unique<DialogNode>("controls-showcase", nullptr);
    dialogNode->SetTitle("Matcha Widget Showcase");
    dialogNode->SetWidth(DialogWidth::Large);

    // Build content using pure UiNode API
    auto content = std::make_unique<ContainerNode>("dlg-content", LayoutKind::Vertical);
    content->SetMargins(SpacingToken::Px8);
    content->SetSpacing(SpacingToken::Px6);

    // =====================================================================
    // Section 1: Theme Settings (live)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-theme-title");
        secTitle->SetText("Theme Settings");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto themeRow = std::make_unique<ContainerNode>("theme-row", LayoutKind::Horizontal);
        themeRow->SetSpacing(SpacingToken::Px8);

        // Light/Dark toggle
        auto themeLabel = std::make_unique<LabelNode>("lbl-theme-mode");
        themeLabel->SetText("Dark Mode:");
        themeRow->AddNode(std::move(themeLabel));
        auto themeToggle = std::make_unique<ToggleSwitchNode>("tog-theme");
        themeToggle->SetChecked(false);
        themeToggle->Subscribe(themeToggle.get(), "Toggled",
            [](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<Toggled>()) {
                    auto& svc = matcha::gui::GetThemeService();
                    svc.SetTheme(e->IsChecked() ? matcha::gui::kThemeDark
                                                : matcha::gui::kThemeLight);
                }
            });
        themeRow->AddNode(std::move(themeToggle));

        // Font size preset
        auto fontLabel = std::make_unique<LabelNode>("lbl-font-preset");
        fontLabel->SetText("Font Size:");
        themeRow->AddNode(std::move(fontLabel));
        auto fontCombo = std::make_unique<ComboBoxNode>("cb-font-preset");
        const std::array<std::string, 3> fontPresets = {"Small", "Medium", "Large"};
        fontCombo->AddItems(fontPresets);
        fontCombo->SetCurrentIndex(1);
        fontCombo->Subscribe(fontCombo.get(), "IndexChanged",
            [](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<IndexChanged>()) {
                    auto& svc = matcha::gui::GetThemeService();
                    svc.SetFontSizePreset(static_cast<FontSizePreset>(e->Index()));
                }
            });
        themeRow->AddNode(std::move(fontCombo));

        // Density
        auto densLabel = std::make_unique<LabelNode>("lbl-density");
        densLabel->SetText("Density:");
        themeRow->AddNode(std::move(densLabel));
        auto densCombo = std::make_unique<ComboBoxNode>("cb-density");
        const std::array<std::string, 3> densities = {"Compact", "Default", "Comfortable"};
        densCombo->AddItems(densities);
        densCombo->SetCurrentIndex(1);
        densCombo->Subscribe(densCombo.get(), "IndexChanged",
            [](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<IndexChanged>()) {
                    auto& svc = matcha::gui::GetThemeService();
                    svc.SetDensity(static_cast<DensityLevel>(e->Index()));
                }
            });
        themeRow->AddNode(std::move(densCombo));

        content->AddNode(std::move(themeRow));
    }

    // =====================================================================
    // Section 2: Text Input Controls
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-input-title");
        secTitle->SetText("Text Input Controls");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto row = std::make_unique<ContainerNode>("input-row", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px8);

        auto lineEdit = std::make_unique<LineEditNode>("le-demo");
        lineEdit->SetPlaceholder("LineEditNode...");
        lineEdit->SetText("Part_Body_001");
        row->AddNode(std::move(lineEdit));

        auto searchBox = std::make_unique<SearchBoxNode>("sb-demo");
        searchBox->SetPlaceholder("SearchBoxNode...");
        row->AddNode(std::move(searchBox));

        auto combo = std::make_unique<ComboBoxNode>("cb-demo");
        const std::array<std::string, 5> items = {"Steel", "Aluminum", "Titanium", "CFRP", "Concrete"};
        combo->AddItems(items);
        combo->SetCurrentIndex(0);
        row->AddNode(std::move(combo));

        content->AddNode(std::move(row));
    }

    // =====================================================================
    // Section 3: Numeric Input Controls
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-numeric-title");
        secTitle->SetText("Numeric Input Controls");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto row = std::make_unique<ContainerNode>("numeric-row", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px8);

        auto spin = std::make_unique<SpinBoxNode>("spin-demo");
        spin->SetRange(0, 100);
        spin->SetValue(42);
        spin->SetSuffix(" items");
        row->AddNode(std::move(spin));

        auto dblSpin = std::make_unique<DoubleSpinBoxNode>("dspin-demo");
        dblSpin->SetRange(0.0, 100.0);
        dblSpin->SetValue(3.14159);
        dblSpin->SetPrecision(4);
        dblSpin->SetStep(0.1);
        dblSpin->SetSuffix(" mm");
        row->AddNode(std::move(dblSpin));

        auto slider = std::make_unique<SliderNode>("slider-demo");
        slider->SetRange(0, 100);
        slider->SetValue(75);
        row->AddNode(std::move(slider));

        content->AddNode(std::move(row));
    }

    // =====================================================================
    // Section 4: Boolean / Selection Controls
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-bool-title");
        secTitle->SetText("Boolean & Selection Controls");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto row = std::make_unique<ContainerNode>("bool-row", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px8);

        auto chk1 = std::make_unique<CheckBoxNode>("chk-demo-1");
        chk1->SetText("CheckBox (on)");
        chk1->SetChecked(true);
        row->AddNode(std::move(chk1));

        auto chk2 = std::make_unique<CheckBoxNode>("chk-demo-2");
        chk2->SetText("CheckBox (off)");
        chk2->SetChecked(false);
        row->AddNode(std::move(chk2));

        auto tog = std::make_unique<ToggleSwitchNode>("tog-demo");
        tog->SetChecked(true);
        row->AddNode(std::move(tog));

        content->AddNode(std::move(row));

        // RadioButtons
        auto radioRow = std::make_unique<ContainerNode>("radio-row", LayoutKind::Horizontal);
        radioRow->SetSpacing(SpacingToken::Px8);

        auto radioLabel = std::make_unique<LabelNode>("lbl-radio");
        radioLabel->SetText("RadioButton:");
        radioRow->AddNode(std::move(radioLabel));

        auto rb1 = std::make_unique<RadioButtonNode>("rb-tri");
        rb1->SetText("Triangle");
        rb1->SetChecked(true);
        radioRow->AddNode(std::move(rb1));

        auto rb2 = std::make_unique<RadioButtonNode>("rb-quad");
        rb2->SetText("Quadrilateral");
        radioRow->AddNode(std::move(rb2));

        auto rb3 = std::make_unique<RadioButtonNode>("rb-hex");
        rb3->SetText("Hexahedral");
        radioRow->AddNode(std::move(rb3));

        content->AddNode(std::move(radioRow));
    }

    // =====================================================================
    // Section 5: Color Controls
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-color-title");
        secTitle->SetText("Color Controls");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto row = std::make_unique<ContainerNode>("color-row", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px8);

        auto picker = std::make_unique<ColorPickerNode>("cpick-demo");
        picker->SetColor(0x4488CCFF);
        picker->SetAlphaEnabled(true);
        row->AddNode(std::move(picker));

        auto sw1 = std::make_unique<ColorSwatchNode>("csw-red");
        sw1->SetColor(0xE53935FF);
        sw1->SetTitle("Error");
        sw1->SetSwatchSize(32, 32);
        row->AddNode(std::move(sw1));

        auto sw2 = std::make_unique<ColorSwatchNode>("csw-green");
        sw2->SetColor(0x43A047FF);
        sw2->SetTitle("Success");
        sw2->SetSwatchSize(32, 32);
        row->AddNode(std::move(sw2));

        auto sw3 = std::make_unique<ColorSwatchNode>("csw-blue");
        sw3->SetColor(0x1E88E5FF);
        sw3->SetTitle("Primary");
        sw3->SetSwatchSize(32, 32);
        row->AddNode(std::move(sw3));

        auto sw4 = std::make_unique<ColorSwatchNode>("csw-amber");
        sw4->SetColor(0xFFB300FF);
        sw4->SetTitle("Warning");
        sw4->SetSwatchSize(32, 32);
        row->AddNode(std::move(sw4));

        content->AddNode(std::move(row));
    }

    // =====================================================================
    // Section 6: Buttons & ToolButtons
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-btn-title");
        secTitle->SetText("Buttons & ToolButtons");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto row = std::make_unique<ContainerNode>("btn-row-dlg", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px8);

        auto btnPrimary = std::make_unique<PushButtonNode>("btn-primary");
        btnPrimary->SetText("Primary");
        btnPrimary->SetIcon(icons::Check);
        btnPrimary->SetIconSize(IconSize::Sm);
        btnPrimary->SetVariant(matcha::gui::ButtonVariant::Primary);
        row->AddNode(std::move(btnPrimary));

        auto btnGhost = std::make_unique<PushButtonNode>("btn-ghost");
        btnGhost->SetText("Ghost");
        btnGhost->SetIcon(icons::Refresh);
        btnGhost->SetIconSize(IconSize::Sm);
        btnGhost->SetVariant(matcha::gui::ButtonVariant::Ghost);
        row->AddNode(std::move(btnGhost));

        auto btnDefault = std::make_unique<PushButtonNode>("btn-default");
        btnDefault->SetText("Default");
        btnDefault->SetIcon(icons::Settings);
        btnDefault->SetIconSize(IconSize::Sm);
        row->AddNode(std::move(btnDefault));

        auto tb1 = std::make_unique<ToolButtonNode>("tb-wire-dlg");
        tb1->SetText("Wireframe");
        tb1->SetCheckable(true);
        tb1->SetChecked(false);
        row->AddNode(std::move(tb1));

        auto tb2 = std::make_unique<ToolButtonNode>("tb-shade-dlg");
        tb2->SetText("Shaded");
        tb2->SetCheckable(true);
        tb2->SetChecked(true);
        row->AddNode(std::move(tb2));

        content->AddNode(std::move(row));
    }

    // =====================================================================
    // Section 7: Progress Indicators
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-progress-title");
        secTitle->SetText("Progress Indicators");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto progressNode = std::make_unique<ProgressBarNode>("prog-demo");
        progressNode->SetRange(0, 100);
        progressNode->SetValue(67);
        content->AddNode(std::move(progressNode));
    }

    // =====================================================================
    // Section 8: Data Views (DataTable + ListWidget)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-data-title");
        secTitle->SetText("Data Views");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        // --- DataTable: comprehensive showcase ---
        auto tableContainer = std::make_unique<ContainerNode>("dt-container", LayoutKind::Vertical);
        tableContainer->SetSpacing(SpacingToken::Px4);

        auto tableNode = std::make_unique<DataTableNode>("dt-demo");

        // Rich column definitions via DataColumnDef
        const std::array<DataColumnDef, 5> cols = {{
            {.title = "Part",     .width = 130, .minWidth = 80, .alignment = HAlign::Left,
             .sortable = true, .editable = false, .visible = true, .resizable = true},
            {.title = "Elements", .width = 90,  .minWidth = 50, .alignment = HAlign::Center,
             .sortable = true, .editable = false, .visible = true, .resizable = true},
            {.title = "Quality",  .width = 80,  .minWidth = 50, .alignment = HAlign::Center,
             .sortable = true, .editable = true,  .visible = true, .resizable = true},
            {.title = "Status",   .width = 90,  .minWidth = 60, .alignment = HAlign::Center,
             .sortable = true, .editable = false, .visible = true, .resizable = true},
            {.title = "Notes",    .width = 160, .minWidth = 80, .alignment = HAlign::Left,
             .sortable = false, .editable = true, .visible = true, .resizable = true},
        }};
        tableNode->SetColumns(cols);

        // Sample data (8 rows for scrolling demo)
        const std::array<std::string, 5> r0 = {"Body_001",    "1024", "0.95", "OK",      "Main body"};
        const std::array<std::string, 5> r1 = {"Body_002",    "2048", "0.87", "OK",      "Secondary"};
        const std::array<std::string, 5> r2 = {"Shell_001",   "512",  "0.72", "Warning", "Thin shell"};
        const std::array<std::string, 5> r3 = {"Beam_003",    "256",  "0.99", "OK",      "I-beam"};
        const std::array<std::string, 5> r4 = {"Plate_004",   "4096", "0.91", "OK",      "Base plate"};
        const std::array<std::string, 5> r5 = {"Bolt_Asm",    "128",  "0.65", "Error",   "Check thread"};
        const std::array<std::string, 5> r6 = {"Flange_005",  "768",  "0.88", "OK",      "Welded flange"};
        const std::array<std::string, 5> r7 = {"Gasket_006",  "64",   "0.78", "Warning", "Rubber seal"};
        tableNode->AddRow(r0);
        tableNode->AddRow(r1);
        tableNode->AddRow(r2);
        tableNode->AddRow(r3);
        tableNode->AddRow(r4);
        tableNode->AddRow(r5);
        tableNode->AddRow(r6);
        tableNode->AddRow(r7);

        // Per-row icons
        tableNode->SetRowIcon(0, matcha::fw::icons::Check);
        tableNode->SetRowIcon(1, matcha::fw::icons::Check);
        tableNode->SetRowIcon(2, matcha::fw::icons::Warning);
        tableNode->SetRowIcon(3, matcha::fw::icons::Check);
        tableNode->SetRowIcon(4, matcha::fw::icons::Check);
        tableNode->SetRowIcon(5, matcha::fw::icons::Error);
        tableNode->SetRowIcon(6, matcha::fw::icons::Check);
        tableNode->SetRowIcon(7, matcha::fw::icons::Warning);

        // Appearance
        tableNode->SetAlternatingRowColors(true);
        tableNode->SetEditable(true);
        tableNode->SetSelectionMode(matcha::gui::SelectionMode::MultiRow);

        // Freeze first column ("Part" stays visible during h-scroll)
        tableNode->SetFrozenColumnCount(1);

        // Sort by Quality descending by default
        tableNode->SortByColumn(2, matcha::gui::SortOrder::Descending);

        tableNode->SetFixedSize(0, 200);
        auto* dtPtr = tableNode.get();
        tableContainer->AddNode(std::move(tableNode));

        // Controls row: filter + diagnostic label
        auto ctrlRow = std::make_unique<ContainerNode>("dt-ctrl-row", LayoutKind::Horizontal);
        ctrlRow->SetSpacing(SpacingToken::Px4);

        auto filterEdit = std::make_unique<LineEditNode>("dt-filter");
        filterEdit->SetPlaceholder("Filter rows...");
        auto* filterPtr = filterEdit.get();
        ctrlRow->AddNode(std::move(filterEdit));

        auto dtDiagLabel = std::make_unique<LabelNode>("dt-diag");
        dtDiagLabel->SetText("[diag] 8 rows, frozen=1, sort=Quality desc");
        dtDiagLabel->SetRole(matcha::gui::LabelRole::Caption);
        auto* dtDiag = dtDiagLabel.get();
        ctrlRow->AddNode(std::move(dtDiagLabel));

        tableContainer->AddNode(std::move(ctrlRow));

        // Wire filter: LineEditNode TextChanged -> DataTableNode::SetFilterText
        filterPtr->Subscribe(filterPtr, "TextChanged",
            [dtPtr, dtDiag](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<TextChanged>()) {
                    dtPtr->SetFilterText(e->Text());
                    dtDiag->SetText("[diag] filter=\"" + std::string(e->Text()) +
                        "\" showing " + std::to_string(dtPtr->DisplayRowCount()) + " rows");
                }
            });

        // Wire selection notification
        dtPtr->Subscribe(dtPtr, "SelectionChanged",
            [dtPtr, dtDiag](matcha::EventNode&, matcha::Notification&) {
                auto sel = dtPtr->SelectedRows();
                dtDiag->SetText("[diag] selected " + std::to_string(sel.size()) + " row(s)");
            });

        content->AddNode(std::move(tableContainer));

        // --- ListWidget: rich UiNode items showcase ---
        auto listLabel = std::make_unique<LabelNode>("lbl-lw-rich");
        listLabel->SetText("ListWidget (Rich UiNode Items)");
        listLabel->SetRole(matcha::gui::LabelRole::Name);
        content->AddNode(std::move(listLabel));

        auto listNode = std::make_unique<ListWidgetNode>("lw-demo");
        {
            // Item 1: HBox[ Label "Body_001" | Badge "OK" | ProgressBar 92% ]
            auto row1 = std::make_unique<ContainerNode>("lw-row1", LayoutKind::Horizontal);
            row1->SetSpacing(matcha::gui::SpacingToken::Px8);
            auto lbl1 = std::make_unique<LabelNode>("lw-lbl1");
            lbl1->SetText("Body_001");
            row1->AddNode(std::move(lbl1));
            auto badge1 = std::make_unique<BadgeNode>("lw-badge1");
            badge1->SetText("OK");
            badge1->SetVariant(0);
            row1->AddNode(std::move(badge1));
            auto prog1 = std::make_unique<ProgressBarNode>("lw-prog1");
            prog1->SetRange(0, 100);
            prog1->SetValue(92);
            row1->AddNode(std::move(prog1));
            listNode->AddItemNode(std::move(row1));

            // Item 2: HBox[ Label "Shell_001" | Badge "WARN" | ProgressBar 72% ]
            auto row2 = std::make_unique<ContainerNode>("lw-row2", LayoutKind::Horizontal);
            row2->SetSpacing(matcha::gui::SpacingToken::Px8);
            auto lbl2 = std::make_unique<LabelNode>("lw-lbl2");
            lbl2->SetText("Shell_001");
            row2->AddNode(std::move(lbl2));
            auto badge2 = std::make_unique<BadgeNode>("lw-badge2");
            badge2->SetText("WARN");
            badge2->SetVariant(1);
            row2->AddNode(std::move(badge2));
            auto prog2 = std::make_unique<ProgressBarNode>("lw-prog2");
            prog2->SetRange(0, 100);
            prog2->SetValue(72);
            row2->AddNode(std::move(prog2));
            listNode->AddItemNode(std::move(row2));

            // Item 3: HBox[ CheckBox "Bolt_Assembly" | Label "128 elems" ]
            auto row3 = std::make_unique<ContainerNode>("lw-row3", LayoutKind::Horizontal);
            row3->SetSpacing(matcha::gui::SpacingToken::Px8);
            auto chk3 = std::make_unique<CheckBoxNode>("lw-chk3");
            chk3->SetText("Bolt_Assembly");
            chk3->SetChecked(true);
            row3->AddNode(std::move(chk3));
            auto desc3 = std::make_unique<LabelNode>("lw-desc3");
            desc3->SetText("128 elements");
            desc3->SetRole(matcha::gui::LabelRole::Caption);
            row3->AddNode(std::move(desc3));
            listNode->AddItemNode(std::move(row3));

            // Item 4: plain text item
            listNode->AddItem("Beam_003 (text only)");

            // Item 5: HBox[ ColorSwatch | Label "Plate_004" | Badge "LOCKED" ]
            auto row5 = std::make_unique<ContainerNode>("lw-row5", LayoutKind::Horizontal);
            row5->SetSpacing(matcha::gui::SpacingToken::Px8);
            auto swatch5 = std::make_unique<ColorSwatchNode>("lw-swatch5");
            swatch5->SetColor(0xFF4488CC);
            row5->AddNode(std::move(swatch5));
            auto lbl5 = std::make_unique<LabelNode>("lw-lbl5");
            lbl5->SetText("Plate_004");
            row5->AddNode(std::move(lbl5));
            auto badge5 = std::make_unique<BadgeNode>("lw-badge5");
            badge5->SetText("LOCKED");
            badge5->SetVariant(2);
            row5->AddNode(std::move(badge5));
            listNode->AddItemNode(std::move(row5));
        }
        listNode->SetCurrentIndex(0);
        content->AddNode(std::move(listNode));

        // --- ComboBox: rich UiNode items showcase ---
        auto comboLabel = std::make_unique<LabelNode>("lbl-cb-rich");
        comboLabel->SetText("ComboBox (Rich UiNode Items)");
        comboLabel->SetRole(matcha::gui::LabelRole::Name);
        content->AddNode(std::move(comboLabel));

        auto richCombo = std::make_unique<ComboBoxNode>("cb-rich-demo");
        richCombo->SetPlaceholder("Select analysis type...");
        {
            // Item 1: HBox[ Label "Linear Static" | Badge "Ready" ]
            auto cr1 = std::make_unique<ContainerNode>("cb-row1", LayoutKind::Horizontal);
            cr1->SetSpacing(matcha::gui::SpacingToken::Px8);
            auto cl1 = std::make_unique<LabelNode>("cb-lbl1");
            cl1->SetText("Linear Static");
            cr1->AddNode(std::move(cl1));
            auto cb1 = std::make_unique<BadgeNode>("cb-badge1");
            cb1->SetText("Ready");
            cb1->SetVariant(0);
            cr1->AddNode(std::move(cb1));
            richCombo->AddItemNode(std::move(cr1));

            // Item 2: HBox[ Label "Modal Analysis" | Badge "Running" | ProgressRing ]
            auto cr2 = std::make_unique<ContainerNode>("cb-row2", LayoutKind::Horizontal);
            cr2->SetSpacing(matcha::gui::SpacingToken::Px8);
            auto cl2 = std::make_unique<LabelNode>("cb-lbl2");
            cl2->SetText("Modal Analysis");
            cr2->AddNode(std::move(cl2));
            auto cb2 = std::make_unique<BadgeNode>("cb-badge2");
            cb2->SetText("Running");
            cb2->SetVariant(1);
            cr2->AddNode(std::move(cb2));
            auto ring2 = std::make_unique<ProgressRingNode>("cb-ring2");
            ring2->SetValue(65);
            cr2->AddNode(std::move(ring2));
            richCombo->AddItemNode(std::move(cr2));

            // Item 3: plain text
            richCombo->AddItem("Thermal");

            // Item 4: HBox[ Label "Fatigue" | Badge "Locked" ]
            auto cr4 = std::make_unique<ContainerNode>("cb-row4", LayoutKind::Horizontal);
            cr4->SetSpacing(matcha::gui::SpacingToken::Px8);
            auto cl4 = std::make_unique<LabelNode>("cb-lbl4");
            cl4->SetText("Fatigue");
            cr4->AddNode(std::move(cl4));
            auto cb4 = std::make_unique<BadgeNode>("cb-badge4");
            cb4->SetText("Locked");
            cb4->SetVariant(2);
            cr4->AddNode(std::move(cb4));
            richCombo->AddItemNode(std::move(cr4));
        }
        richCombo->SetCurrentIndex(0);
        content->AddNode(std::move(richCombo));
    }

    // =====================================================================
    // Section 9: Log Output (PlainTextEditNode)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-log-title");
        secTitle->SetText("Log Output (PlainTextEditNode)");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto logNode = std::make_unique<PlainTextEditNode>("log-demo");
        logNode->SetReadOnly(true);
        logNode->SetMaximumBlockCount(200);
        logNode->SetPlainText(
            "[INFO]  Application initialized\n"
            "[INFO]  Theme set to Light\n"
            "[INFO]  Icon directory registered: 48 icons\n"
            "[WARN]  Shell_001 quality below threshold (0.72)\n"
            "[INFO]  Mesh generation complete: 3840 elements\n");
        logNode->SetFixedSize(0, 100);
        content->AddNode(std::move(logNode));
    }

    // =====================================================================
    // Section 10: New UiNode Controls (LineNode, BadgeNode, MessageNode,
    //             ProgressRingNode, RangeSliderNode, DateTimePickerNode,
    //             PaginatorNode, CollapsibleSectionNode)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-new-nodes-title");
        secTitle->SetText("New UiNode Controls");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        // --- LineNode (separator) ---
        auto line1 = std::make_unique<LineNode>("line-sep-1");
        content->AddNode(std::move(line1));

        // --- BadgeNode row ---
        auto badgeRow = std::make_unique<ContainerNode>("badge-row", LayoutKind::Horizontal);
        badgeRow->SetSpacing(SpacingToken::Px4);
        auto badgeLabel = std::make_unique<LabelNode>("lbl-badge");
        badgeLabel->SetText("Badges:");
        badgeRow->AddNode(std::move(badgeLabel));

        auto badgeInfo = std::make_unique<BadgeNode>("badge-info");
        badgeInfo->SetText("Info");
        badgeInfo->SetVariant(0); // Neutral
        badgeRow->AddNode(std::move(badgeInfo));

        auto badgeOk = std::make_unique<BadgeNode>("badge-ok");
        badgeOk->SetText("Success");
        badgeOk->SetVariant(1); // Success
        badgeRow->AddNode(std::move(badgeOk));

        auto badgeWarn = std::make_unique<BadgeNode>("badge-warn");
        badgeWarn->SetText("Warning");
        badgeWarn->SetVariant(3); // Warning
        badgeRow->AddNode(std::move(badgeWarn));

        auto badgeErr = std::make_unique<BadgeNode>("badge-err");
        badgeErr->SetText("Error");
        badgeErr->SetVariant(4); // Error
        badgeErr->SetClosable(true);
        badgeRow->AddNode(std::move(badgeErr));

        content->AddNode(std::move(badgeRow));

        // --- MessageNode ---
        auto msgInfo = std::make_unique<MessageNode>("msg-info");
        msgInfo->SetType(0); // Info
        msgInfo->SetText("This is an informational message bar.");
        msgInfo->SetClosable(true);
        content->AddNode(std::move(msgInfo));

        auto msgWarn = std::make_unique<MessageNode>("msg-warn");
        msgWarn->SetType(2); // Warning
        msgWarn->SetText("Low memory warning: 85% used.");
        msgWarn->SetAction("Free Memory");
        content->AddNode(std::move(msgWarn));

        // --- LineNode (vertical in horizontal row) ---
        auto line2 = std::make_unique<LineNode>("line-sep-2");
        content->AddNode(std::move(line2));

        // --- Progress row: ProgressBar + ProgressRing ---
        auto progressRow = std::make_unique<ContainerNode>("progress-row", LayoutKind::Horizontal);
        progressRow->SetSpacing(SpacingToken::Px8);

        auto progLabel = std::make_unique<LabelNode>("lbl-progress-ring");
        progLabel->SetText("Progress Ring:");
        progressRow->AddNode(std::move(progLabel));

        auto ring1 = std::make_unique<ProgressRingNode>("ring-det");
        ring1->SetValue(65);
        progressRow->AddNode(std::move(ring1));

        auto ring2 = std::make_unique<ProgressRingNode>("ring-indet");
        ring2->SetIndeterminate(true);
        ring2->SetTextVisible(false);
        progressRow->AddNode(std::move(ring2));

        content->AddNode(std::move(progressRow));

        // --- RangeSliderNode ---
        auto rangeRow = std::make_unique<ContainerNode>("range-row", LayoutKind::Horizontal);
        rangeRow->SetSpacing(SpacingToken::Px8);

        auto rangeLabel = std::make_unique<LabelNode>("lbl-range");
        rangeLabel->SetText("Range Slider [20, 80]:");
        rangeRow->AddNode(std::move(rangeLabel));

        auto rangeSlider = std::make_unique<RangeSliderNode>("range-demo");
        rangeSlider->SetRange(0, 100);
        rangeSlider->SetLow(20);
        rangeSlider->SetHigh(80);
        rangeSlider->SetStep(5);
        rangeRow->AddNode(std::move(rangeSlider));

        content->AddNode(std::move(rangeRow));

        // --- DateTimePickerNode ---
        auto dtRow = std::make_unique<ContainerNode>("dt-row", LayoutKind::Horizontal);
        dtRow->SetSpacing(SpacingToken::Px8);

        auto dtLabel = std::make_unique<LabelNode>("lbl-datetime");
        dtLabel->SetText("Date/Time:");
        dtRow->AddNode(std::move(dtLabel));

        auto dtPicker = std::make_unique<DateTimePickerNode>("dt-demo");
        dtPicker->SetMode(2); // DateTime mode
        dtRow->AddNode(std::move(dtPicker));

        content->AddNode(std::move(dtRow));

        // --- PaginatorNode ---
        auto pagRow = std::make_unique<ContainerNode>("pag-row", LayoutKind::Horizontal);
        pagRow->SetSpacing(SpacingToken::Px8);

        auto pagLabel = std::make_unique<LabelNode>("lbl-paginator");
        pagLabel->SetText("Paginator:");
        pagRow->AddNode(std::move(pagLabel));

        auto paginator = std::make_unique<PaginatorNode>("pag-demo");
        paginator->SetCount(10);
        paginator->SetCurrent(3);
        paginator->SetResetButtonVisible(true);
        pagRow->AddNode(std::move(paginator));

        content->AddNode(std::move(pagRow));

        // --- CollapsibleSectionNode ---
        auto collSec = std::make_unique<CollapsibleSectionNode>("coll-demo");
        collSec->SetTitle("Collapsible Section (click to toggle)");
        collSec->SetExpanded(true);
        content->AddNode(std::move(collSec));
    }

    // =====================================================================
    // Section 11: Mnemonic (Access Key) Showcase
    //
    // Demonstrates Alt+key mnemonic activation for labels and sections.
    // Hold Alt to see underlines on mnemonic characters.
    // =====================================================================
    {
        auto sep = std::make_unique<LineNode>("mnem-sep-top");
        content->AddNode(std::move(sep));

        auto secTitle = std::make_unique<LabelNode>("lbl-mnem-title");
        secTitle->SetText("Mnemonic (Access Key) Showcase");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto desc = std::make_unique<LabelNode>("lbl-mnem-desc");
        desc->SetText(
            "Hold Alt to reveal mnemonic underlines. "
            "Alt+N focuses Name, Alt+T focuses Thickness, Alt+M focuses Material. "
            "Menu bar mnemonics (Alt+F/E/V/etc.) also work.");
        content->AddNode(std::move(desc));

        // --- Always-Show toggle ---
        auto alwaysRow = std::make_unique<ContainerNode>("mnem-always-row", LayoutKind::Horizontal);
        alwaysRow->SetSpacing(SpacingToken::Px8);

        auto alwaysLabel = std::make_unique<LabelNode>("lbl-mnem-always");
        alwaysLabel->SetText("Always Show Underlines:");
        alwaysRow->AddNode(std::move(alwaysLabel));

        auto alwaysToggle = std::make_unique<ToggleSwitchNode>("tog-mnem-always");
        alwaysToggle->SetChecked(false);
        alwaysToggle->Subscribe(alwaysToggle.get(), "Toggled",
            [](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<Toggled>()) {
                    if (auto* ms = matcha::gui::GetMnemonicState()) {
                        ms->SetAlwaysShow(e->IsChecked());
                    }
                }
            });
        alwaysRow->AddNode(std::move(alwaysToggle));
        content->AddNode(std::move(alwaysRow));

        // --- Label + Buddy: "&Name" -> LineEdit ---
        auto nameRow = std::make_unique<ContainerNode>("mnem-name-row", LayoutKind::Horizontal);
        nameRow->SetSpacing(SpacingToken::Px8);

        auto nameLabel = std::make_unique<LabelNode>("lbl-mnem-name");
        nameLabel->SetText("&Name:");
        nameLabel->SetRole(matcha::gui::LabelRole::Name);
        auto* nameLabelPtr = nameLabel.get();
        nameRow->AddNode(std::move(nameLabel));

        auto nameEdit = std::make_unique<LineEditNode>("le-mnem-name");
        nameEdit->SetText("Part_Body_001");
        auto* nameEditPtr = nameEdit.get();
        nameRow->AddNode(std::move(nameEdit));
        content->AddNode(std::move(nameRow));

        // Wire buddy: Alt+N focuses the name LineEdit
        nameLabelPtr->SetBuddy(nameEditPtr);

        // --- Label + Buddy: "&Thickness" -> LineEdit ---
        auto thickRow = std::make_unique<ContainerNode>("mnem-thick-row", LayoutKind::Horizontal);
        thickRow->SetSpacing(SpacingToken::Px8);

        auto thickLabel = std::make_unique<LabelNode>("lbl-mnem-thick");
        thickLabel->SetText("&Thickness:");
        thickLabel->SetRole(matcha::gui::LabelRole::Name);
        auto* thickLabelPtr = thickLabel.get();
        thickRow->AddNode(std::move(thickLabel));

        auto thickEdit = std::make_unique<LineEditNode>("le-mnem-thick");
        thickEdit->SetText("1.500");
        thickEdit->SetPlaceholder("mm");
        auto* thickEditPtr = thickEdit.get();
        thickRow->AddNode(std::move(thickEdit));
        content->AddNode(std::move(thickRow));

        thickLabelPtr->SetBuddy(thickEditPtr);

        // --- Label + Buddy: "&Material" -> ComboBox ---
        auto matRow = std::make_unique<ContainerNode>("mnem-mat-row", LayoutKind::Horizontal);
        matRow->SetSpacing(SpacingToken::Px8);

        auto matLabel = std::make_unique<LabelNode>("lbl-mnem-mat");
        matLabel->SetText("&Material:");
        matLabel->SetRole(matcha::gui::LabelRole::Name);
        auto* matLabelPtr = matLabel.get();
        matRow->AddNode(std::move(matLabel));

        auto matCombo = std::make_unique<ComboBoxNode>("cb-mnem-mat");
        const std::array<std::string, 4> mats = {"Steel", "Aluminum", "Titanium", "CFRP"};
        matCombo->AddItems(mats);
        matCombo->SetCurrentIndex(0);
        auto* matComboPtr = matCombo.get();
        matRow->AddNode(std::move(matCombo));
        content->AddNode(std::move(matRow));

        matLabelPtr->SetBuddy(matComboPtr);

        // --- CollapsibleSection with mnemonic title ---
        auto collSec = std::make_unique<CollapsibleSectionNode>("coll-mnem-demo");
        collSec->SetTitle("&Parameters (collapsible with mnemonic)");
        collSec->SetExpanded(true);
        content->AddNode(std::move(collSec));

        // --- Diagnostic label ---
        auto diag = std::make_unique<LabelNode>("lbl-mnem-diag");
        auto* ms = matcha::gui::GetMnemonicState();
        std::string info = "[diag] MnemonicState=";
        info += ms ? "OK" : "NULL";
        if (ms) {
            info += " altHeld=" + std::string(ms->IsAltHeld() ? "true" : "false");
            info += " alwaysShow=" + std::string(ms->IsAlwaysShow() ? "true" : "false");
        }
        diag->SetText(info);
        diag->SetRole(matcha::gui::LabelRole::Caption);
        content->AddNode(std::move(diag));
    }

    // =====================================================================
    // Section 12: Animation Showcase (RFC-08)
    //
    // Structure per case: separator + title + description + target/buttons row + diag label
    // =====================================================================
    {
        auto animSep = std::make_unique<LineNode>("anim-sep-top");
        content->AddNode(std::move(animSep));

        auto animHeader = std::make_unique<LabelNode>("lbl-anim-header");
        animHeader->SetText("Animation Showcase (RFC-08)");
        animHeader->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(animHeader));

        auto animDesc = std::make_unique<LabelNode>("lbl-anim-desc");
        animDesc->SetText(
            "Tests AnimateProperty / AnimateSpring / CancelAnimation via the UiNode API. "
            "Each case: colored target, action buttons, [diag] label for runtime state.");
        content->AddNode(std::move(animDesc));

        // ==== Case 0: AnimationService diagnostics & controls =============
        {
            auto sep = std::make_unique<LineNode>("anim-case0-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case0-title");
            title->SetText("[Case 0] AnimationService Diagnostics & Controls");
            content->AddNode(std::move(title));

            auto diag = std::make_unique<LabelNode>("lbl-case0-diag");
            auto* animSvc = matcha::gui::GetAnimationService();
            std::string info = "[diag] AnimationService=";
            info += animSvc ? "OK" : "NULL";
            if (animSvc) {
                info += " reducedMotion=" + std::string(animSvc->IsReducedMotion() ? "TRUE" : "false");
                info += " speedMul=" + std::to_string(animSvc->SpeedMultiplier());
            }
            diag->SetText(info);
            auto* diagPtr = diag.get();
            content->AddNode(std::move(diag));

            auto row = std::make_unique<ContainerNode>("anim-case0-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto btnDisableRM = std::make_unique<PushButtonNode>("btn-disable-rm");
            btnDisableRM->SetText("Disable ReducedMotion");
            auto* btnDisableRMPtr = btnDisableRM.get();
            row->AddNode(std::move(btnDisableRM));

            auto btnEnableRM = std::make_unique<PushButtonNode>("btn-enable-rm");
            btnEnableRM->SetText("Enable ReducedMotion");
            auto* btnEnableRMPtr = btnEnableRM.get();
            row->AddNode(std::move(btnEnableRM));

            auto btnSpeed05 = std::make_unique<PushButtonNode>("btn-speed-05");
            btnSpeed05->SetText("Speed 0.3x");
            auto* btnSpeed05Ptr = btnSpeed05.get();
            row->AddNode(std::move(btnSpeed05));

            auto btnSpeed1 = std::make_unique<PushButtonNode>("btn-speed-1");
            btnSpeed1->SetText("Speed 1.0x");
            auto* btnSpeed1Ptr = btnSpeed1.get();
            row->AddNode(std::move(btnSpeed1));

            content->AddNode(std::move(row));

            btnDisableRMPtr->Subscribe(btnDisableRMPtr, "Activated",
                [diagPtr](matcha::EventNode&, matcha::Notification&) {
                    if (auto* svc = matcha::gui::GetAnimationService()) {
                        svc->SetReducedMotion(false);
                        diagPtr->SetText("[diag] ReducedMotion=false, speedMul=" +
                            std::to_string(svc->SpeedMultiplier()));
                    }
                });
            btnEnableRMPtr->Subscribe(btnEnableRMPtr, "Activated",
                [diagPtr](matcha::EventNode&, matcha::Notification&) {
                    if (auto* svc = matcha::gui::GetAnimationService()) {
                        svc->SetReducedMotion(true);
                        diagPtr->SetText("[diag] ReducedMotion=true (animations snap)");
                    }
                });
            btnSpeed05Ptr->Subscribe(btnSpeed05Ptr, "Activated",
                [diagPtr](matcha::EventNode&, matcha::Notification&) {
                    if (auto* svc = matcha::gui::GetAnimationService()) {
                        svc->SetSpeedMultiplier(0.3F);
                        diagPtr->SetText("[diag] speedMul=0.3 (slow motion)");
                    }
                });
            btnSpeed1Ptr->Subscribe(btnSpeed1Ptr, "Activated",
                [diagPtr](matcha::EventNode&, matcha::Notification&) {
                    if (auto* svc = matcha::gui::GetAnimationService()) {
                        svc->SetSpeedMultiplier(1.0F);
                        diagPtr->SetText("[diag] speedMul=1.0 (normal)");
                    }
                });
        }

        // ==== Case 1: Opacity (Fade Out / Fade In) ========================
        {
            auto sep = std::make_unique<LineNode>("anim-case1-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case1-title");
            title->SetText("[Case 1] Opacity: Fade Out (1.0->0.1, Normal+OutCubic) / Fade In (0.1->1.0, Quick+InOutCubic)");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case1-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto target = std::make_unique<PushButtonNode>("anim-case1-target");
            target->SetText("FADE TARGET");
            target->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgt = target.get();
            row->AddNode(std::move(target));

            auto btnOut = std::make_unique<PushButtonNode>("btn-case1-fadeout");
            btnOut->SetText("Fade Out");
            auto* bOut = btnOut.get();
            row->AddNode(std::move(btnOut));

            auto btnIn = std::make_unique<PushButtonNode>("btn-case1-fadein");
            btnIn->SetText("Fade In");
            auto* bIn = btnIn.get();
            row->AddNode(std::move(btnIn));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case1-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            bOut->Subscribe(bOut, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(
                        AnimationPropertyId::Opacity,
                        AnimatableValue::FromDouble(1.0),
                        AnimatableValue::FromDouble(0.1),
                        AnimationToken::Normal, EasingToken::OutCubic);
                    d->SetText("[diag] FadeOut h=" + std::to_string(static_cast<uint64_t>(h)));
                });
            bIn->Subscribe(bIn, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(
                        AnimationPropertyId::Opacity,
                        AnimatableValue::FromDouble(0.1),
                        AnimatableValue::FromDouble(1.0),
                        AnimationToken::Quick, EasingToken::InOutCubic);
                    d->SetText("[diag] FadeIn h=" + std::to_string(static_cast<uint64_t>(h)));
                });
        }

        // ==== Case 2: MinimumHeight (Grow / Shrink) =======================
        {
            auto sep = std::make_unique<LineNode>("anim-case2-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case2-title");
            title->SetText("[Case 2] MinimumHeight: Grow 24->120 (Slow+OutCubic) / Shrink 120->24 (Quick+OutCubic)");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case2-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto target = std::make_unique<PushButtonNode>("anim-case2-target");
            target->SetText("HEIGHT TARGET");
            target->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgt = target.get();
            row->AddNode(std::move(target));

            auto btnG = std::make_unique<PushButtonNode>("btn-case2-grow");
            btnG->SetText("Grow");
            auto* bG = btnG.get();
            row->AddNode(std::move(btnG));

            auto btnS = std::make_unique<PushButtonNode>("btn-case2-shrink");
            btnS->SetText("Shrink");
            auto* bS = btnS.get();
            row->AddNode(std::move(btnS));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case2-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            bG->Subscribe(bG, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(
                        AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(24), AnimatableValue::FromInt(120),
                        AnimationToken::Slow, EasingToken::OutCubic);
                    d->SetText("[diag] Grow h=" + std::to_string(static_cast<uint64_t>(h)) +
                        " minH=" + std::to_string(tgt->Widget() ? tgt->Widget()->minimumHeight() : -1));
                });
            bS->Subscribe(bS, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(
                        AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(120), AnimatableValue::FromInt(24),
                        AnimationToken::Quick, EasingToken::OutCubic);
                    d->SetText("[diag] Shrink h=" + std::to_string(static_cast<uint64_t>(h)) +
                        " minH=" + std::to_string(tgt->Widget() ? tgt->Widget()->minimumHeight() : -1));
                });
        }

        // ==== Case 3: MaximumHeight with Spring easing ====================
        {
            auto sep = std::make_unique<LineNode>("anim-case3-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case3-title");
            title->SetText("[Case 3] MaximumHeight: Spring bounce 30->100 (EasingToken::Spring)");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case3-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto target = std::make_unique<PushButtonNode>("anim-case3-target");
            target->SetText("SPRING HEIGHT TARGET");
            target->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgt = target.get();
            row->AddNode(std::move(target));

            auto btn = std::make_unique<PushButtonNode>("btn-case3-spring");
            btn->SetText("Spring Bounce");
            auto* b = btn.get();
            row->AddNode(std::move(btn));

            auto btnReset = std::make_unique<PushButtonNode>("btn-case3-reset");
            btnReset->SetText("Reset (snap)");
            auto* bR = btnReset.get();
            row->AddNode(std::move(btnReset));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case3-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            b->Subscribe(b, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(
                        AnimationPropertyId::MaximumHeight,
                        AnimatableValue::FromInt(30), AnimatableValue::FromInt(100),
                        AnimationToken::Normal, EasingToken::Spring);
                    d->SetText("[diag] Spring h=" + std::to_string(static_cast<uint64_t>(h)) +
                        " maxH=" + std::to_string(tgt->Widget() ? tgt->Widget()->maximumHeight() : -1));
                });
            bR->Subscribe(bR, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    if (auto* w = tgt->Widget()) { w->setMaximumHeight(16777215); }
                    d->SetText("[diag] Reset maxH to QWIDGETSIZE_MAX");
                });
        }

        // ==== Case 4: AnimateSpring with custom SpringSpec ================
        {
            auto sep = std::make_unique<LineNode>("anim-case4-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case4-title");
            title->SetText("[Case 4] AnimateSpring: MinimumHeight with custom SpringSpec (stiff=300, damp=8, mass=1)");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case4-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto target = std::make_unique<PushButtonNode>("anim-case4-target");
            target->SetText("CUSTOM SPRING TARGET");
            target->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgt = target.get();
            row->AddNode(std::move(target));

            auto btnBounce = std::make_unique<PushButtonNode>("btn-case4-bounce");
            btnBounce->SetText("Bouncy Spring (expand)");
            auto* bB = btnBounce.get();
            row->AddNode(std::move(btnBounce));

            auto btnStiff = std::make_unique<PushButtonNode>("btn-case4-stiff");
            btnStiff->SetText("Stiff Spring (contract)");
            auto* bSt = btnStiff.get();
            row->AddNode(std::move(btnStiff));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case4-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            bB->Subscribe(bB, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    SpringSpec spec;
                    spec.stiffness = 300.0F;
                    spec.damping   = 8.0F;
                    spec.mass      = 1.0F;
                    auto h = tgt->AnimateSpring(
                        AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(30), AnimatableValue::FromInt(100), spec);
                    d->SetText("[diag] BouncySpring h=" + std::to_string(static_cast<uint64_t>(h)));
                });
            bSt->Subscribe(bSt, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    SpringSpec spec;
                    spec.stiffness = 800.0F;
                    spec.damping   = 30.0F;
                    spec.mass      = 1.0F;
                    auto h = tgt->AnimateSpring(
                        AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(100), AnimatableValue::FromInt(30), spec);
                    d->SetText("[diag] StiffSpring h=" + std::to_string(static_cast<uint64_t>(h)));
                });
        }

        // ==== Case 5: Easing comparison (3 buttons, same property) ========
        {
            auto sep = std::make_unique<LineNode>("anim-case5-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case5-title");
            title->SetText("[Case 5] Easing comparison: Linear vs OutCubic vs InOutCubic on MinimumHeight 30->80");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case5-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto target = std::make_unique<PushButtonNode>("anim-case5-target");
            target->SetText("EASING TARGET");
            target->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgt = target.get();
            row->AddNode(std::move(target));

            auto btnLin = std::make_unique<PushButtonNode>("btn-case5-linear");
            btnLin->SetText("Linear");
            auto* bLin = btnLin.get();
            row->AddNode(std::move(btnLin));

            auto btnCub = std::make_unique<PushButtonNode>("btn-case5-outcubic");
            btnCub->SetText("OutCubic");
            auto* bCub = btnCub.get();
            row->AddNode(std::move(btnCub));

            auto btnIO = std::make_unique<PushButtonNode>("btn-case5-inoutcubic");
            btnIO->SetText("InOutCubic");
            auto* bIO = btnIO.get();
            row->AddNode(std::move(btnIO));

            auto btnReset = std::make_unique<PushButtonNode>("btn-case5-reset");
            btnReset->SetText("Reset");
            auto* bR = btnReset.get();
            row->AddNode(std::move(btnReset));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case5-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            bLin->Subscribe(bLin, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(30), AnimatableValue::FromInt(80),
                        AnimationToken::Slow, EasingToken::Linear);
                    d->SetText("[diag] Linear h=" + std::to_string(static_cast<uint64_t>(h)));
                });
            bCub->Subscribe(bCub, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(30), AnimatableValue::FromInt(80),
                        AnimationToken::Slow, EasingToken::OutCubic);
                    d->SetText("[diag] OutCubic h=" + std::to_string(static_cast<uint64_t>(h)));
                });
            bIO->Subscribe(bIO, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(30), AnimatableValue::FromInt(80),
                        AnimationToken::Slow, EasingToken::InOutCubic);
                    d->SetText("[diag] InOutCubic h=" + std::to_string(static_cast<uint64_t>(h)));
                });
            bR->Subscribe(bR, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    if (auto* w = tgt->Widget()) { w->setMinimumHeight(0); }
                    d->SetText("[diag] Reset minH=0");
                });
        }

        // ==== Case 6: Interruption (re-target mid-flight) =================
        {
            auto sep = std::make_unique<LineNode>("anim-case6-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case6-title");
            title->SetText("[Case 6] Interruption: start MinHeight 30->100, then re-target to 60 mid-flight");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case6-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto target = std::make_unique<PushButtonNode>("anim-case6-target");
            target->SetText("INTERRUPT TARGET");
            target->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgt = target.get();
            row->AddNode(std::move(target));

            auto btnStart = std::make_unique<PushButtonNode>("btn-case6-start");
            btnStart->SetText("Start 30->100 (Slow)");
            auto* bStart = btnStart.get();
            row->AddNode(std::move(btnStart));

            auto btnRetarget = std::make_unique<PushButtonNode>("btn-case6-retarget");
            btnRetarget->SetText("Re-target to 60 (Quick)");
            auto* bRe = btnRetarget.get();
            row->AddNode(std::move(btnRetarget));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case6-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            bStart->Subscribe(bStart, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(30), AnimatableValue::FromInt(100),
                        AnimationToken::Slow, EasingToken::OutCubic);
                    d->SetText("[diag] Started 30->100, h=" + std::to_string(static_cast<uint64_t>(h)));
                });
            bRe->Subscribe(bRe, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    int curMinH = tgt->Widget() ? tgt->Widget()->minimumHeight() : 30;
                    auto h = tgt->AnimateProperty(AnimationPropertyId::MinimumHeight,
                        AnimatableValue::FromInt(curMinH), AnimatableValue::FromInt(60),
                        AnimationToken::Quick, EasingToken::InOutCubic);
                    d->SetText("[diag] Re-targeted from " + std::to_string(curMinH) +
                        "->60, h=" + std::to_string(static_cast<uint64_t>(h)));
                });
        }

        // ==== Case 7: Cancel + IsAnimating + Notifications ================
        {
            auto sep = std::make_unique<LineNode>("anim-case7-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case7-title");
            title->SetText("[Case 7] Cancel / IsAnimating / Notification lifecycle (Opacity, Slow+Linear)");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case7-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto target = std::make_unique<PushButtonNode>("anim-case7-target");
            target->SetText("LIFECYCLE TARGET");
            target->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgt = target.get();
            row->AddNode(std::move(target));

            auto btnStart = std::make_unique<PushButtonNode>("btn-case7-start");
            btnStart->SetText("Start Slow Fade");
            auto* bStart = btnStart.get();
            row->AddNode(std::move(btnStart));

            auto btnCancel = std::make_unique<PushButtonNode>("btn-case7-cancel");
            btnCancel->SetText("Cancel");
            auto* bCancel = btnCancel.get();
            row->AddNode(std::move(btnCancel));

            auto btnQuery = std::make_unique<PushButtonNode>("btn-case7-query");
            btnQuery->SetText("Is Animating?");
            auto* bQuery = btnQuery.get();
            row->AddNode(std::move(btnQuery));

            auto btnRestore = std::make_unique<PushButtonNode>("btn-case7-restore");
            btnRestore->SetText("Fade In");
            auto* bRestore = btnRestore.get();
            row->AddNode(std::move(btnRestore));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case7-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            auto notifLog = std::make_unique<LabelNode>("lbl-case7-notif");
            notifLog->SetText("[notif] waiting...");
            auto* nLog = notifLog.get();
            content->AddNode(std::move(notifLog));

            static TransitionHandle s_case7Handle = TransitionHandle::Invalid;

            bStart->Subscribe(bStart, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    s_case7Handle = tgt->AnimateProperty(
                        AnimationPropertyId::Opacity,
                        AnimatableValue::FromDouble(1.0), AnimatableValue::FromDouble(0.0),
                        AnimationToken::Slow, EasingToken::Linear);
                    d->SetText("[diag] SlowFade h=" + std::to_string(static_cast<uint64_t>(s_case7Handle)));
                });
            bCancel->Subscribe(bCancel, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    d->SetText("[diag] Cancel h=" + std::to_string(static_cast<uint64_t>(s_case7Handle)));
                    tgt->CancelAnimation(s_case7Handle);
                    s_case7Handle = TransitionHandle::Invalid;
                });
            bQuery->Subscribe(bQuery, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    bool running = tgt->IsAnimating(AnimationPropertyId::Opacity);
                    d->SetText(std::string("[diag] IsAnimating(Opacity)=") +
                        (running ? "true" : "false") +
                        " h=" + std::to_string(static_cast<uint64_t>(s_case7Handle)));
                });
            bRestore->Subscribe(bRestore, "Activated",
                [tgt, d](matcha::EventNode&, matcha::Notification&) {
                    auto h = tgt->AnimateProperty(
                        AnimationPropertyId::Opacity,
                        AnimatableValue::FromDouble(0.0), AnimatableValue::FromDouble(1.0),
                        AnimationToken::Quick, EasingToken::InOutCubic);
                    d->SetText("[diag] FadeIn h=" + std::to_string(static_cast<uint64_t>(h)));
                });

            tgt->Subscribe(nullptr, "AnimationStarted",
                [nLog](matcha::EventNode&, matcha::Notification&) {
                    nLog->SetText("[notif] AnimationStarted");
                });
            tgt->Subscribe(nullptr, "AnimationCompleted",
                [nLog](matcha::EventNode&, matcha::Notification&) {
                    nLog->SetText("[notif] AnimationCompleted");
                });
            tgt->Subscribe(nullptr, "AnimationCancelled",
                [nLog](matcha::EventNode&, matcha::Notification&) {
                    nLog->SetText("[notif] AnimationCancelled");
                });
        }

        // ==== Case 8: Parallel Group Animation =============================
        {
            auto sep = std::make_unique<LineNode>("anim-case8-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case8-title");
            title->SetText("[Case 8] Parallel Group: Fade + Height on 2 targets simultaneously");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case8-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto targetA = std::make_unique<PushButtonNode>("anim-case8-tgtA");
            targetA->SetText("GROUP-A (opacity)");
            targetA->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgtA = targetA.get();
            row->AddNode(std::move(targetA));

            auto targetB = std::make_unique<PushButtonNode>("anim-case8-tgtB");
            targetB->SetText("GROUP-B (height)");
            targetB->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgtB = targetB.get();
            row->AddNode(std::move(targetB));

            auto btnRun = std::make_unique<PushButtonNode>("btn-case8-run");
            btnRun->SetText("Run Parallel Group");
            auto* bRun = btnRun.get();
            row->AddNode(std::move(btnRun));

            auto btnReverse = std::make_unique<PushButtonNode>("btn-case8-reverse");
            btnReverse->SetText("Reverse (re-target)");
            auto* bRev = btnReverse.get();
            row->AddNode(std::move(btnReverse));

            auto btnReset = std::make_unique<PushButtonNode>("btn-case8-reset");
            btnReset->SetText("Reset");
            auto* bRst = btnReset.get();
            row->AddNode(std::move(btnReset));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case8-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            auto notifLogA = std::make_unique<LabelNode>("lbl-case8-notifA");
            notifLogA->SetText("[notif-A] waiting...");
            auto* nLogA = notifLogA.get();
            content->AddNode(std::move(notifLogA));

            auto notifLogB = std::make_unique<LabelNode>("lbl-case8-notifB");
            notifLogB->SetText("[notif-B] waiting...");
            auto* nLogB = notifLogB.get();
            content->AddNode(std::move(notifLogB));

            bRun->Subscribe(bRun, "Activated",
                [tgtA, tgtB, d](matcha::EventNode&, matcha::Notification&) {
                    auto* svc = matcha::gui::GetAnimationService();
                    if (svc == nullptr) { d->SetText("[diag] no AnimationService"); return; }
                    using matcha::gui::GroupAnimationSpec;
                    using matcha::gui::GroupMode;
                    std::array<GroupAnimationSpec, 2> specs = {{
                        { .target=tgtA, .property=AnimationPropertyId::Opacity,
                          .from=AnimatableValue::FromDouble(1.0), .to=AnimatableValue::FromDouble(0.15),
                          .duration=AnimationToken::Slow, .easing=EasingToken::OutCubic },
                        { .target=tgtB, .property=AnimationPropertyId::MinimumHeight,
                          .from=AnimatableValue::FromInt(24), .to=AnimatableValue::FromInt(100),
                          .duration=AnimationToken::Slow, .easing=EasingToken::OutCubic },
                    }};
                    auto gid = svc->AnimateGroup(specs, GroupMode::Parallel);
                    d->SetText("[diag] Parallel gid=" +
                        std::to_string(static_cast<uint64_t>(gid)));
                });
            bRev->Subscribe(bRev, "Activated",
                [tgtA, tgtB, d](matcha::EventNode&, matcha::Notification&) {
                    auto* svc = matcha::gui::GetAnimationService();
                    if (svc == nullptr) { d->SetText("[diag] no AnimationService"); return; }
                    using matcha::gui::GroupAnimationSpec;
                    using matcha::gui::GroupMode;
                    std::array<GroupAnimationSpec, 2> specs = {{
                        { .target=tgtA, .property=AnimationPropertyId::Opacity,
                          .from=AnimatableValue::FromDouble(0.15), .to=AnimatableValue::FromDouble(1.0),
                          .duration=AnimationToken::Slow, .easing=EasingToken::InOutCubic },
                        { .target=tgtB, .property=AnimationPropertyId::MinimumHeight,
                          .from=AnimatableValue::FromInt(100), .to=AnimatableValue::FromInt(24),
                          .duration=AnimationToken::Slow, .easing=EasingToken::InOutCubic },
                    }};
                    auto gid = svc->AnimateGroup(specs, GroupMode::Parallel);
                    d->SetText("[diag] Reverse gid=" +
                        std::to_string(static_cast<uint64_t>(gid)) +
                        " (re-targets from current if mid-flight)");
                });
            bRst->Subscribe(bRst, "Activated",
                [tgtA, tgtB, d](matcha::EventNode&, matcha::Notification&) {
                    auto* svc = matcha::gui::GetAnimationService();
                    if (svc != nullptr) { svc->CancelAll(tgtA); svc->CancelAll(tgtB); }
                    tgtA->SetOpacity(1.0);
                    if (auto* w = tgtB->Widget()) { w->setMinimumHeight(0); }
                    d->SetText("[diag] Reset both targets");
                });

            tgtA->Subscribe(nullptr, "AnimationStarted",
                [nLogA](matcha::EventNode&, matcha::Notification&) {
                    nLogA->SetText("[notif-A] AnimationStarted");
                });
            tgtA->Subscribe(nullptr, "AnimationCompleted",
                [nLogA](matcha::EventNode&, matcha::Notification&) {
                    nLogA->SetText("[notif-A] AnimationCompleted");
                });
            tgtA->Subscribe(nullptr, "AnimationCancelled",
                [nLogA](matcha::EventNode&, matcha::Notification&) {
                    nLogA->SetText("[notif-A] AnimationCancelled");
                });
            tgtB->Subscribe(nullptr, "AnimationStarted",
                [nLogB](matcha::EventNode&, matcha::Notification&) {
                    nLogB->SetText("[notif-B] AnimationStarted");
                });
            tgtB->Subscribe(nullptr, "AnimationCompleted",
                [nLogB](matcha::EventNode&, matcha::Notification&) {
                    nLogB->SetText("[notif-B] AnimationCompleted");
                });
            tgtB->Subscribe(nullptr, "AnimationCancelled",
                [nLogB](matcha::EventNode&, matcha::Notification&) {
                    nLogB->SetText("[notif-B] AnimationCancelled");
                });
        }

        // ==== Case 9: Sequential Group + CancelGroup ========================
        {
            auto sep = std::make_unique<LineNode>("anim-case9-sep");
            content->AddNode(std::move(sep));

            auto title = std::make_unique<LabelNode>("lbl-case9-title");
            title->SetText("[Case 9] Sequential Group: A fades, then B grows. CancelGroup stops all.");
            content->AddNode(std::move(title));

            auto row = std::make_unique<ContainerNode>("anim-case9-row", LayoutKind::Horizontal);
            row->SetSpacing(SpacingToken::Px8);

            auto targetA = std::make_unique<PushButtonNode>("anim-case9-tgtA");
            targetA->SetText("SEQ-A (opacity)");
            targetA->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgtA = targetA.get();
            row->AddNode(std::move(targetA));

            auto targetB = std::make_unique<PushButtonNode>("anim-case9-tgtB");
            targetB->SetText("SEQ-B (height)");
            targetB->SetVariant(matcha::gui::ButtonVariant::Primary);
            auto* tgtB = targetB.get();
            row->AddNode(std::move(targetB));

            auto btnRun = std::make_unique<PushButtonNode>("btn-case9-run");
            btnRun->SetText("Run Sequential Group");
            auto* bRun = btnRun.get();
            row->AddNode(std::move(btnRun));

            auto btnCancel = std::make_unique<PushButtonNode>("btn-case9-cancel");
            btnCancel->SetText("CancelGroup");
            auto* bCancel = btnCancel.get();
            row->AddNode(std::move(btnCancel));

            auto btnReset = std::make_unique<PushButtonNode>("btn-case9-reset");
            btnReset->SetText("Reset");
            auto* bRst = btnReset.get();
            row->AddNode(std::move(btnReset));

            content->AddNode(std::move(row));

            auto diag = std::make_unique<LabelNode>("lbl-case9-diag");
            diag->SetText("[diag] idle");
            auto* d = diag.get();
            content->AddNode(std::move(diag));

            auto notifLogA = std::make_unique<LabelNode>("lbl-case9-notifA");
            notifLogA->SetText("[notif-A] waiting...");
            auto* nLogA = notifLogA.get();
            content->AddNode(std::move(notifLogA));

            auto notifLogB = std::make_unique<LabelNode>("lbl-case9-notifB");
            notifLogB->SetText("[notif-B] waiting...");
            auto* nLogB = notifLogB.get();
            content->AddNode(std::move(notifLogB));

            static matcha::gui::GroupId s_case9Gid = matcha::gui::GroupId::Invalid;

            bRun->Subscribe(bRun, "Activated",
                [tgtA, tgtB, d](matcha::EventNode&, matcha::Notification&) {
                    auto* svc = matcha::gui::GetAnimationService();
                    if (svc == nullptr) { d->SetText("[diag] no AnimationService"); return; }
                    using matcha::gui::GroupAnimationSpec;
                    using matcha::gui::GroupMode;
                    std::array<GroupAnimationSpec, 2> specs = {{
                        { .target=tgtA, .property=AnimationPropertyId::Opacity,
                          .from=AnimatableValue::FromDouble(1.0), .to=AnimatableValue::FromDouble(0.1),
                          .duration=AnimationToken::Slow, .easing=EasingToken::OutCubic },
                        { .target=tgtB, .property=AnimationPropertyId::MinimumHeight,
                          .from=AnimatableValue::FromInt(24), .to=AnimatableValue::FromInt(100),
                          .duration=AnimationToken::Slow, .easing=EasingToken::OutCubic },
                    }};
                    s_case9Gid = svc->AnimateGroup(specs, GroupMode::Sequential);
                    d->SetText("[diag] Sequential gid=" +
                        std::to_string(static_cast<uint64_t>(s_case9Gid)) +
                        " (A fades first, then B grows)");
                });
            bCancel->Subscribe(bCancel, "Activated",
                [d](matcha::EventNode&, matcha::Notification&) {
                    auto* svc = matcha::gui::GetAnimationService();
                    if (svc == nullptr) { d->SetText("[diag] no AnimationService"); return; }
                    d->SetText("[diag] CancelGroup gid=" +
                        std::to_string(static_cast<uint64_t>(s_case9Gid)));
                    svc->CancelGroup(s_case9Gid);
                    s_case9Gid = matcha::gui::GroupId::Invalid;
                });
            bRst->Subscribe(bRst, "Activated",
                [tgtA, tgtB, d](matcha::EventNode&, matcha::Notification&) {
                    auto* svc = matcha::gui::GetAnimationService();
                    if (svc != nullptr) { svc->CancelAll(tgtA); svc->CancelAll(tgtB); }
                    tgtA->SetOpacity(1.0);
                    if (auto* w = tgtB->Widget()) { w->setMinimumHeight(0); }
                    s_case9Gid = matcha::gui::GroupId::Invalid;
                    d->SetText("[diag] Reset both targets + cleared gid");
                });

            tgtA->Subscribe(nullptr, "AnimationStarted",
                [nLogA](matcha::EventNode&, matcha::Notification&) {
                    nLogA->SetText("[notif-A] AnimationStarted");
                });
            tgtA->Subscribe(nullptr, "AnimationCompleted",
                [nLogA](matcha::EventNode&, matcha::Notification&) {
                    nLogA->SetText("[notif-A] AnimationCompleted");
                });
            tgtA->Subscribe(nullptr, "AnimationCancelled",
                [nLogA](matcha::EventNode&, matcha::Notification&) {
                    nLogA->SetText("[notif-A] AnimationCancelled");
                });
            tgtB->Subscribe(nullptr, "AnimationStarted",
                [nLogB](matcha::EventNode&, matcha::Notification&) {
                    nLogB->SetText("[notif-B] AnimationStarted");
                });
            tgtB->Subscribe(nullptr, "AnimationCompleted",
                [nLogB](matcha::EventNode&, matcha::Notification&) {
                    nLogB->SetText("[notif-B] AnimationCompleted");
                });
            tgtB->Subscribe(nullptr, "AnimationCancelled",
                [nLogB](matcha::EventNode&, matcha::Notification&) {
                    nLogB->SetText("[notif-B] AnimationCancelled");
                });
        }

        auto animSepBot = std::make_unique<LineNode>("anim-sep-bottom");
        content->AddNode(std::move(animSepBot));
    }

    // =====================================================================
    // Section 12: Tooltip Showcase (S9 — Rich Tooltips)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-tooltip-title");
        secTitle->SetText("Tooltip Showcase (S9)");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto row = std::make_unique<ContainerNode>("tooltip-row", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px8);

        // Simple tooltip (tier 1 only)
        auto btnSimple = std::make_unique<PushButtonNode>("btn-tip-simple");
        btnSimple->SetText("Simple Tooltip");
        {
            TooltipSpec tip;
            tip.title = "Save document";
            btnSimple->SetTooltip(std::move(tip));
        }
        row->AddNode(std::move(btnSimple));

        // Tooltip with shortcut
        auto btnShortcut = std::make_unique<PushButtonNode>("btn-tip-shortcut");
        btnShortcut->SetText("With Shortcut");
        btnShortcut->SetIcon(icons::Save);
        btnShortcut->SetIconSize(IconSize::Sm);
        {
            TooltipSpec tip;
            tip.title = "Save";
            tip.shortcut = "Ctrl+S";
            btnShortcut->SetTooltip(std::move(tip));
        }
        row->AddNode(std::move(btnShortcut));

        // Full rich tooltip (tier 1 + tier 2 description + icon)
        auto btnRich = std::make_unique<PushButtonNode>("btn-tip-rich");
        btnRich->SetText("Rich Tooltip");
        btnRich->SetIcon(icons::Settings);
        btnRich->SetIconSize(IconSize::Sm);
        btnRich->SetVariant(matcha::gui::ButtonVariant::Primary);
        {
            TooltipSpec tip;
            tip.title = "Application Settings";
            tip.shortcut = "Ctrl+,";
            tip.description = "Opens the settings dialog where you can configure theme, "
                              "font size, density, keyboard shortcuts, and plugin preferences.";
            tip.iconId = std::string(icons::Settings);
            tip.tier1DelayMs = 150;
            tip.tier2DelayMs = 800;
            tip.position = TooltipPosition::Below;
            btnRich->SetTooltip(std::move(tip));
        }
        row->AddNode(std::move(btnRich));

        // Tooltip on a non-button (label)
        auto tipLabel = std::make_unique<LabelNode>("lbl-tip-hover");
        tipLabel->SetText("[Hover me for tooltip]");
        {
            TooltipSpec tip;
            tip.title = "Label Tooltip";
            tip.description = "Tooltips work on any WidgetNode, not just buttons.";
            tipLabel->SetTooltip(std::move(tip));
        }
        row->AddNode(std::move(tipLabel));

        // Tooltip with position hint
        auto btnAbove = std::make_unique<PushButtonNode>("btn-tip-above");
        btnAbove->SetText("Tooltip Above");
        {
            TooltipSpec tip;
            tip.title = "Positioned Above";
            tip.description = "This tooltip prefers to appear above the widget.";
            tip.position = TooltipPosition::Above;
            btnAbove->SetTooltip(std::move(tip));
        }
        row->AddNode(std::move(btnAbove));

        content->AddNode(std::move(row));
    }

    // =====================================================================
    // Section 13: PropertyGridNode (6 property types)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-propgrid-title");
        secTitle->SetText("PropertyGrid");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto pgRow = std::make_unique<ContainerNode>("propgrid-row", LayoutKind::Horizontal);
        pgRow->SetSpacing(SpacingToken::Px8);

        auto grid = std::make_unique<PropertyGridNode>("propgrid-demo");
        grid->AddGroup("Geometry");
        grid->AddProperty("Name", matcha::gui::PropertyType::Text, "Body_001");
        grid->AddProperty("Length", matcha::gui::PropertyType::Double, 150.0);
        grid->AddProperty("Width", matcha::gui::PropertyType::Double, 75.5);
        grid->AddProperty("Element Count", matcha::gui::PropertyType::Integer, 1024.0);
        grid->AddGroup("Material");
        const std::array<std::string, 4> matChoices = {"Steel", "Aluminum", "Titanium", "CFRP"};
        grid->AddProperty("Material", matcha::gui::PropertyType::Choice, "Steel", matChoices);
        grid->AddProperty("Is Rigid", matcha::gui::PropertyType::Bool, true);
        grid->AddProperty("Color", matcha::gui::PropertyType::Color, "#4488CC");
        grid->SetFixedSize(0, 200);
        auto* gridPtr = grid.get();
        pgRow->AddNode(std::move(grid));

        auto diagCol = std::make_unique<ContainerNode>("propgrid-diag", LayoutKind::Vertical);
        diagCol->SetSpacing(SpacingToken::Px4);
        auto diagLabel = std::make_unique<LabelNode>("lbl-propgrid-diag");
        diagLabel->SetText("[diag] Edit a property to see notifications");
        auto* dPg = diagLabel.get();
        diagCol->AddNode(std::move(diagLabel));
        pgRow->AddNode(std::move(diagCol));

        gridPtr->Subscribe(gridPtr, "PropertyChanged",
            [dPg](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<PropertyChanged>()) {
                    dPg->SetText("[diag] PropertyChanged: \"" +
                        std::string(e->Key()) + "\" = \"" +
                        std::string(e->Value()) + "\"");
                }
            });

        content->AddNode(std::move(pgRow));
    }

    // =====================================================================
    // Section 14: TreeWidgetNode
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-tree-title");
        secTitle->SetText("TreeWidget (Structure Tree)");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto treeRow = std::make_unique<ContainerNode>("tree-row", LayoutKind::Horizontal);
        treeRow->SetSpacing(SpacingToken::Px8);

        auto tree = std::make_unique<TreeWidgetNode>("tree-demo");
        tree->SetTitle("Assembly");

        // Build tree data via TreeItemNode (no Qt types)
        tree->SetHeaderLabel("Component");
        {
            auto body = std::make_unique<TreeItemNode>("Body_001");
            body->AddChild(std::make_unique<TreeItemNode>("Face_1"));
            body->AddChild(std::make_unique<TreeItemNode>("Face_2"));
            body->AddChild(std::make_unique<TreeItemNode>("Edge_Loop_1"));
            tree->AddRootItem(std::move(body));

            auto shell = std::make_unique<TreeItemNode>("Shell_001");
            shell->AddChild(std::make_unique<TreeItemNode>("Face_3"));
            shell->AddChild(std::make_unique<TreeItemNode>("Face_4"));
            tree->AddRootItem(std::move(shell));

            tree->AddRootItem(std::make_unique<TreeItemNode>("Bolt_Assembly"));
        }
        tree->ExpandAll();
        tree->SetFixedSize(0, 180);
        auto* treePtr = tree.get();
        treeRow->AddNode(std::move(tree));

        auto treeDiag = std::make_unique<ContainerNode>("tree-diag", LayoutKind::Vertical);
        treeDiag->SetSpacing(SpacingToken::Px4);

        auto btnExpand = std::make_unique<PushButtonNode>("btn-tree-expand");
        btnExpand->SetText("Expand All");
        auto* bExp = btnExpand.get();
        treeDiag->AddNode(std::move(btnExpand));

        auto btnCollapse = std::make_unique<PushButtonNode>("btn-tree-collapse");
        btnCollapse->SetText("Collapse All");
        auto* bCol = btnCollapse.get();
        treeDiag->AddNode(std::move(btnCollapse));

        auto treeDiagLabel = std::make_unique<LabelNode>("lbl-tree-diag");
        treeDiagLabel->SetText("[diag] Select a tree item");
        auto* dTree = treeDiagLabel.get();
        treeDiag->AddNode(std::move(treeDiagLabel));

        treeRow->AddNode(std::move(treeDiag));

        bExp->Subscribe(bExp, "Activated",
            [treePtr](matcha::EventNode&, matcha::Notification&) {
                treePtr->ExpandAll();
            });
        bCol->Subscribe(bCol, "Activated",
            [treePtr](matcha::EventNode&, matcha::Notification&) {
                treePtr->CollapseAll();
            });
        treePtr->Subscribe(treePtr, "SelectionChanged",
            [dTree, treePtr](matcha::EventNode&, matcha::Notification&) {
                auto path = treePtr->SelectedPath();
                std::string pathStr;
                for (size_t i = 0; i < path.size(); ++i) {
                    if (i > 0) { pathStr += '/'; }
                    pathStr += std::to_string(path[i]);
                }
                dTree->SetText("[diag] SelectionChanged path=" + pathStr);
            });

        content->AddNode(std::move(treeRow));
    }

    // =====================================================================
    // Section 15: NotificationNode (Toast Popups)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-notif-title");
        secTitle->SetText("Notification Toasts");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto row = std::make_unique<ContainerNode>("notif-row", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px8);

        auto diagLabel = std::make_unique<LabelNode>("lbl-notif-diag");
        diagLabel->SetText("[diag] Click a button to show a toast");
        auto* dNotif = diagLabel.get();

        auto btnInfo = std::make_unique<PushButtonNode>("btn-notif-info");
        btnInfo->SetText("Info Toast");
        btnInfo->SetIcon(icons::Info);
        btnInfo->SetIconSize(IconSize::Sm);
        auto* bInfo = btnInfo.get();
        row->AddNode(std::move(btnInfo));

        auto btnSuccess = std::make_unique<PushButtonNode>("btn-notif-success");
        btnSuccess->SetText("Success Toast");
        btnSuccess->SetIcon(icons::Success);
        btnSuccess->SetIconSize(IconSize::Sm);
        auto* bSuccess = btnSuccess.get();
        row->AddNode(std::move(btnSuccess));

        auto btnWarn = std::make_unique<PushButtonNode>("btn-notif-warn");
        btnWarn->SetText("Warning Toast");
        btnWarn->SetIcon(icons::Warning);
        btnWarn->SetIconSize(IconSize::Sm);
        auto* bWarn = btnWarn.get();
        row->AddNode(std::move(btnWarn));

        auto btnErr = std::make_unique<PushButtonNode>("btn-notif-error");
        btnErr->SetText("Error Toast");
        btnErr->SetIcon(icons::Error);
        btnErr->SetIconSize(IconSize::Sm);
        btnErr->SetVariant(matcha::gui::ButtonVariant::Danger);
        auto* bErr = btnErr.get();
        row->AddNode(std::move(btnErr));

        auto btnAction = std::make_unique<PushButtonNode>("btn-notif-action");
        btnAction->SetText("Toast with Action");
        auto* bAct = btnAction.get();
        row->AddNode(std::move(btnAction));

        content->AddNode(std::move(row));
        content->AddNode(std::move(diagLabel));

        static int sToastCounter = 0;
        bInfo->Subscribe(bInfo, "Activated",
            [dNotif, bInfo](matcha::EventNode&, matcha::Notification&) {
                auto toast = std::make_unique<NotificationNode>("toast-info-" + std::to_string(sToastCounter++));
                toast->SetMessage("Document saved successfully.");
                toast->SetType(0);
                toast->SetDurationMs(3000);
                dNotif->SetText("[diag] Showing Info toast (3s)");
                auto* w = bInfo->Widget();
                if (w) {
                    auto pos = w->mapToGlobal(QPoint(0, w->height()));
                    toast->ShowAt(pos.x(), pos.y());
                }
                bInfo->AddNode(std::move(toast));
            });
        bSuccess->Subscribe(bSuccess, "Activated",
            [dNotif, bSuccess](matcha::EventNode&, matcha::Notification&) {
                auto toast = std::make_unique<NotificationNode>("toast-success-" + std::to_string(sToastCounter++));
                toast->SetMessage("Mesh generation complete: 3840 elements.");
                toast->SetType(1);
                toast->SetDurationMs(4000);
                dNotif->SetText("[diag] Showing Success toast (4s)");
                auto* w = bSuccess->Widget();
                if (w) {
                    auto pos = w->mapToGlobal(QPoint(0, w->height()));
                    toast->ShowAt(pos.x(), pos.y());
                }
                bSuccess->AddNode(std::move(toast));
            });
        bWarn->Subscribe(bWarn, "Activated",
            [dNotif, bWarn](matcha::EventNode&, matcha::Notification&) {
                auto toast = std::make_unique<NotificationNode>("toast-warn-" + std::to_string(sToastCounter++));
                toast->SetMessage("Low memory warning: 85% used.");
                toast->SetType(2);
                toast->SetDurationMs(5000);
                dNotif->SetText("[diag] Showing Warning toast (5s)");
                auto* w = bWarn->Widget();
                if (w) {
                    auto pos = w->mapToGlobal(QPoint(0, w->height()));
                    toast->ShowAt(pos.x(), pos.y());
                }
                bWarn->AddNode(std::move(toast));
            });
        bErr->Subscribe(bErr, "Activated",
            [dNotif, bErr](matcha::EventNode&, matcha::Notification&) {
                auto toast = std::make_unique<NotificationNode>("toast-error-" + std::to_string(sToastCounter++));
                toast->SetMessage("Failed to write output file.");
                toast->SetType(3);
                toast->SetDurationMs(0); // persistent until dismissed
                dNotif->SetText("[diag] Showing Error toast (persistent)");
                auto* w = bErr->Widget();
                if (w) {
                    auto pos = w->mapToGlobal(QPoint(0, w->height()));
                    toast->ShowAt(pos.x(), pos.y());
                }
                bErr->AddNode(std::move(toast));
            });
        bAct->Subscribe(bAct, "Activated",
            [dNotif, bAct](matcha::EventNode&, matcha::Notification&) {
                auto toast = std::make_unique<NotificationNode>("toast-action-" + std::to_string(sToastCounter++));
                toast->SetMessage("Build completed with 2 warnings.");
                toast->SetType(2);
                toast->SetAction("View Log");
                toast->SetDurationMs(6000);
                auto* tPtr = toast.get();
                tPtr->Subscribe(tPtr, "ActionClicked",
                    [dNotif](matcha::EventNode&, matcha::Notification&) {
                        dNotif->SetText("[diag] Toast action clicked: View Log");
                    });
                dNotif->SetText("[diag] Showing toast with action button (6s)");
                auto* w = bAct->Widget();
                if (w) {
                    auto pos = w->mapToGlobal(QPoint(0, w->height()));
                    toast->ShowAt(pos.x(), pos.y());
                }
                bAct->AddNode(std::move(toast));
            });
    }

    // =====================================================================
    // Section 16: Icon Gallery (fw::icons::*)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-icon-gallery-title");
        secTitle->SetText("Icon Gallery (asset:// URI)");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        struct IconEntry { std::string_view id; std::string_view label; };
        const std::array<IconEntry, 30> iconList = {{
            {icons::Save, "Save"}, {icons::Open, "Open"}, {icons::NewFile, "NewFile"},
            {icons::Copy, "Copy"}, {icons::Paste, "Paste"}, {icons::Cut, "Cut"},
            {icons::Undo, "Undo"}, {icons::Redo, "Redo"}, {icons::Delete, "Delete"},
            {icons::Search, "Search"}, {icons::Filter, "Filter"}, {icons::Settings, "Settings"},
            {icons::Add, "Add"}, {icons::Remove, "Remove"}, {icons::Edit, "Edit"},
            {icons::Check, "Check"}, {icons::Cross, "Cross"}, {icons::Refresh, "Refresh"},
            {icons::Info, "Info"}, {icons::Warning, "Warning"}, {icons::Error, "Error"},
            {icons::Success, "Success"}, {icons::Help, "Help"}, {icons::Question, "Question"},
            {icons::ZoomIn, "ZoomIn"}, {icons::ZoomOut, "ZoomOut"}, {icons::ZoomFit, "ZoomFit"},
            {icons::Eye, "Eye"}, {icons::Lock, "Lock"}, {icons::Download, "Download"},
        }};

        // Render as rows of 10 icon buttons each
        for (size_t rowStart = 0; rowStart < iconList.size(); rowStart += 10) {
            auto iconRow = std::make_unique<ContainerNode>(
                "icon-row-" + std::to_string(rowStart), LayoutKind::Horizontal);
            iconRow->SetSpacing(SpacingToken::Px4);
            for (size_t i = rowStart; i < rowStart + 10 && i < iconList.size(); ++i) {
                auto btn = std::make_unique<PushButtonNode>(
                    "btn-icon-" + std::string(iconList[i].label));
                btn->SetText(std::string(iconList[i].label));
                btn->SetIcon(iconList[i].id);
                btn->SetIconSize(IconSize::Sm);
                {
                    TooltipSpec tip;
                    tip.title = std::string(iconList[i].label);
                    tip.description = std::string(iconList[i].id);
                    btn->SetTooltip(std::move(tip));
                }
                iconRow->AddNode(std::move(btn));
            }
            content->AddNode(std::move(iconRow));
        }
    }

    // =====================================================================
    // Section 17: Context Menu
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-ctx-title");
        secTitle->SetText("Context Menu (right-click)");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto ctxRow = std::make_unique<ContainerNode>("ctx-row", LayoutKind::Horizontal);
        ctxRow->SetSpacing(SpacingToken::Px8);

        auto ctxTarget = std::make_unique<PushButtonNode>("btn-ctx-target");
        ctxTarget->SetText("Right-click me for Context Menu");
        ctxTarget->SetVariant(matcha::gui::ButtonVariant::Primary);
        ctxTarget->SetContextMenuEnabled(true);
        auto* ctxTgt = ctxTarget.get();
        ctxRow->AddNode(std::move(ctxTarget));

        auto ctxDiag = std::make_unique<LabelNode>("lbl-ctx-diag");
        ctxDiag->SetText("[diag] Right-click the button");
        auto* dCtx = ctxDiag.get();
        ctxRow->AddNode(std::move(ctxDiag));

        content->AddNode(std::move(ctxRow));

        ctxTgt->Subscribe(ctxTgt, "ContextMenuRequest",
            [](matcha::EventNode&, matcha::Notification& n) {
                if (auto* req = n.As<ContextMenuRequest>()) {
                    auto item1 = std::make_unique<MenuItemNode>("ctx-cut");
                    item1->SetText("Cut");
                    req->AddNode(std::move(item1));

                    auto item2 = std::make_unique<MenuItemNode>("ctx-copy");
                    item2->SetText("Copy");
                    req->AddNode(std::move(item2));

                    auto item3 = std::make_unique<MenuItemNode>("ctx-paste");
                    item3->SetText("Paste");
                    req->AddNode(std::move(item3));

                    auto item4 = std::make_unique<MenuItemNode>("ctx-delete");
                    item4->SetText("Delete");
                    req->AddNode(std::move(item4));
                }
            });
        ctxTgt->Subscribe(ctxTgt, "ContextMenuItemActivated",
            [dCtx](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<ContextMenuItemActivated>()) {
                    dCtx->SetText("[diag] ContextMenu action: " + std::string(e->ActionId()));
                }
            });
    }

    // =====================================================================
    // Section 18: Help System (StatusHint, WhatsThis, HelpId)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-help-title");
        secTitle->SetText("Help System (S13)");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        auto helpRow = std::make_unique<ContainerNode>("help-row", LayoutKind::Horizontal);
        helpRow->SetSpacing(SpacingToken::Px8);

        auto btnStatus = std::make_unique<PushButtonNode>("btn-help-status");
        btnStatus->SetText("StatusHint (hover)");
        btnStatus->SetStatusHint("Shows brief help text in the status bar when hovered.");
        helpRow->AddNode(std::move(btnStatus));

        auto btnWhatsThis = std::make_unique<PushButtonNode>("btn-help-whats");
        btnWhatsThis->SetText("WhatsThis");
        btnWhatsThis->SetWhatsThis(
            "This button demonstrates the WhatsThis help feature.\n"
            "In a real application, pressing Shift+F1 then clicking a widget\n"
            "would display this extended help text.");
        helpRow->AddNode(std::move(btnWhatsThis));

        auto btnHelpId = std::make_unique<PushButtonNode>("btn-help-id");
        btnHelpId->SetText("HelpId = 'mesh.settings'");
        btnHelpId->SetHelpId("mesh.settings");
        {
            TooltipSpec tip;
            tip.title = "Help ID";
            tip.description = "HelpId='mesh.settings' — links to external help system.";
            btnHelpId->SetTooltip(std::move(tip));
        }
        helpRow->AddNode(std::move(btnHelpId));

        content->AddNode(std::move(helpRow));
    }

    // =====================================================================
    // Section 19: Button Variants + LabelRole Comparison
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-variants-title");
        secTitle->SetText("Button Variants & Label Roles");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        // All 4 button variants
        auto btnRow = std::make_unique<ContainerNode>("variant-row", LayoutKind::Horizontal);
        btnRow->SetSpacing(SpacingToken::Px8);

        auto bPri = std::make_unique<PushButtonNode>("btn-var-primary");
        bPri->SetText("Primary");
        bPri->SetIcon(icons::Check);
        bPri->SetIconSize(IconSize::Sm);
        bPri->SetVariant(matcha::gui::ButtonVariant::Primary);
        btnRow->AddNode(std::move(bPri));

        auto bSec = std::make_unique<PushButtonNode>("btn-var-secondary");
        bSec->SetText("Secondary");
        bSec->SetIcon(icons::Settings);
        bSec->SetIconSize(IconSize::Sm);
        bSec->SetVariant(matcha::gui::ButtonVariant::Secondary);
        btnRow->AddNode(std::move(bSec));

        auto bGhost = std::make_unique<PushButtonNode>("btn-var-ghost");
        bGhost->SetText("Ghost");
        bGhost->SetIcon(icons::Refresh);
        bGhost->SetIconSize(IconSize::Sm);
        bGhost->SetVariant(matcha::gui::ButtonVariant::Ghost);
        btnRow->AddNode(std::move(bGhost));

        auto bDanger = std::make_unique<PushButtonNode>("btn-var-danger");
        bDanger->SetText("Danger");
        bDanger->SetIcon(icons::Delete);
        bDanger->SetIconSize(IconSize::Sm);
        bDanger->SetVariant(matcha::gui::ButtonVariant::Danger);
        btnRow->AddNode(std::move(bDanger));

        content->AddNode(std::move(btnRow));

        // LabelRole comparison
        auto labelRow = std::make_unique<ContainerNode>("label-role-row", LayoutKind::Vertical);
        labelRow->SetSpacing(SpacingToken::Px4);

        auto lblTitle = std::make_unique<LabelNode>("lbl-role-title");
        lblTitle->SetText("LabelRole::Title — Section headers");
        lblTitle->SetRole(matcha::gui::LabelRole::Title);
        labelRow->AddNode(std::move(lblTitle));

        auto lblName = std::make_unique<LabelNode>("lbl-role-name");
        lblName->SetText("LabelRole::Name — Property names / field labels");
        lblName->SetRole(matcha::gui::LabelRole::Name);
        labelRow->AddNode(std::move(lblName));

        auto lblBody = std::make_unique<LabelNode>("lbl-role-body");
        lblBody->SetText("LabelRole::Body — Default body text for descriptions and content");
        lblBody->SetRole(matcha::gui::LabelRole::Body);
        labelRow->AddNode(std::move(lblBody));

        auto lblCaption = std::make_unique<LabelNode>("lbl-role-caption");
        lblCaption->SetText("LabelRole::Caption — Small annotation text for hints and metadata");
        lblCaption->SetRole(matcha::gui::LabelRole::Caption);
        labelRow->AddNode(std::move(lblCaption));

        content->AddNode(std::move(labelRow));
    }

    // =====================================================================
    // Section 20: Widget States (Enabled/Disabled, Visible, A11y, DnD)
    // =====================================================================
    {
        auto secTitle = std::make_unique<LabelNode>("lbl-states-title");
        secTitle->SetText("Widget States");
        secTitle->SetRole(matcha::gui::LabelRole::Title);
        content->AddNode(std::move(secTitle));

        // Enabled / Disabled comparison
        auto stateRow1 = std::make_unique<ContainerNode>("state-row1", LayoutKind::Horizontal);
        stateRow1->SetSpacing(SpacingToken::Px8);

        auto lblEnabled = std::make_unique<LabelNode>("lbl-state-enabled");
        lblEnabled->SetText("Enabled/Disabled:");
        stateRow1->AddNode(std::move(lblEnabled));

        auto btnEnabled = std::make_unique<PushButtonNode>("btn-state-enabled");
        btnEnabled->SetText("Enabled Button");
        btnEnabled->SetEnabled(true);
        stateRow1->AddNode(std::move(btnEnabled));

        auto btnDisabled = std::make_unique<PushButtonNode>("btn-state-disabled");
        btnDisabled->SetText("Disabled Button");
        btnDisabled->SetEnabled(false);
        stateRow1->AddNode(std::move(btnDisabled));

        auto chkDisabled = std::make_unique<CheckBoxNode>("chk-state-disabled");
        chkDisabled->SetText("Disabled Checkbox");
        chkDisabled->SetChecked(true);
        chkDisabled->SetEnabled(false);
        stateRow1->AddNode(std::move(chkDisabled));

        auto leDisabled = std::make_unique<LineEditNode>("le-state-disabled");
        leDisabled->SetText("Disabled LineEdit");
        leDisabled->SetEnabled(false);
        stateRow1->AddNode(std::move(leDisabled));

        auto slDisabled = std::make_unique<SliderNode>("sl-state-disabled");
        slDisabled->SetRange(0, 100);
        slDisabled->SetValue(50);
        slDisabled->SetEnabled(false);
        stateRow1->AddNode(std::move(slDisabled));

        content->AddNode(std::move(stateRow1));

        // Visible/Hidden toggle
        auto stateRow2 = std::make_unique<ContainerNode>("state-row2", LayoutKind::Horizontal);
        stateRow2->SetSpacing(SpacingToken::Px8);

        auto visTarget = std::make_unique<PushButtonNode>("btn-vis-target");
        visTarget->SetText("I am visible");
        visTarget->SetVariant(matcha::gui::ButtonVariant::Primary);
        auto* visPtr = visTarget.get();
        stateRow2->AddNode(std::move(visTarget));

        auto btnShow = std::make_unique<PushButtonNode>("btn-vis-show");
        btnShow->SetText("Show");
        auto* bShow = btnShow.get();
        stateRow2->AddNode(std::move(btnShow));

        auto btnHide = std::make_unique<PushButtonNode>("btn-vis-hide");
        btnHide->SetText("Hide");
        auto* bHide = btnHide.get();
        stateRow2->AddNode(std::move(btnHide));

        auto visDiag = std::make_unique<LabelNode>("lbl-vis-diag");
        visDiag->SetText("[diag] visible=true");
        auto* dVis = visDiag.get();
        stateRow2->AddNode(std::move(visDiag));

        content->AddNode(std::move(stateRow2));

        bShow->Subscribe(bShow, "Activated",
            [visPtr, dVis](matcha::EventNode&, matcha::Notification&) {
                visPtr->SetVisible(true);
                dVis->SetText("[diag] visible=true");
            });
        bHide->Subscribe(bHide, "Activated",
            [visPtr, dVis](matcha::EventNode&, matcha::Notification&) {
                visPtr->SetVisible(false);
                dVis->SetText("[diag] visible=false");
            });

        // Accessibility
        auto stateRow3 = std::make_unique<ContainerNode>("state-row3", LayoutKind::Horizontal);
        stateRow3->SetSpacing(SpacingToken::Px8);

        auto a11yLabel = std::make_unique<LabelNode>("lbl-a11y-demo");
        a11yLabel->SetText("A11y:");
        stateRow3->AddNode(std::move(a11yLabel));

        auto a11yBtn = std::make_unique<PushButtonNode>("btn-a11y-demo");
        a11yBtn->SetText("Accessible Button");
        a11yBtn->SetAccessibleName("mesh-quality-check-button");
        a11yBtn->SetA11yRole(A11yRole::Button);
        {
            TooltipSpec tip;
            tip.title = "A11y Demo";
            tip.description = "AccessibleName='mesh-quality-check-button', A11yRole=Button";
            a11yBtn->SetTooltip(std::move(tip));
        }
        stateRow3->AddNode(std::move(a11yBtn));

        auto a11ySlider = std::make_unique<SliderNode>("sl-a11y-demo");
        a11ySlider->SetRange(0, 100);
        a11ySlider->SetValue(60);
        a11ySlider->SetAccessibleName("mesh-density-slider");
        a11ySlider->SetA11yRole(A11yRole::Slider);
        stateRow3->AddNode(std::move(a11ySlider));

        content->AddNode(std::move(stateRow3));

        // DnD demo (drop target)
        auto stateRow4 = std::make_unique<ContainerNode>("state-row4", LayoutKind::Horizontal);
        stateRow4->SetSpacing(SpacingToken::Px8);

        auto dndLabel = std::make_unique<LabelNode>("lbl-dnd-demo");
        dndLabel->SetText("Drag & Drop target (accepts text/plain):");
        stateRow4->AddNode(std::move(dndLabel));

        auto dndTarget = std::make_unique<LineEditNode>("le-dnd-target");
        dndTarget->SetPlaceholder("Drop text here...");
        dndTarget->SetAcceptDrops(true);
        auto* dndPtr = dndTarget.get();
        stateRow4->AddNode(std::move(dndTarget));

        auto dndDiag = std::make_unique<LabelNode>("lbl-dnd-diag");
        dndDiag->SetText("[diag] waiting for drop...");
        auto* dDnd = dndDiag.get();
        stateRow4->AddNode(std::move(dndDiag));

        dndPtr->Subscribe(dndPtr, "DragEntered",
            [dDnd](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<DragEntered>()) {
                    std::string mimes;
                    for (auto& m : e->MimeTypes()) {
                        if (!mimes.empty()) mimes += ", ";
                        mimes += m;
                    }
                    dDnd->SetText("[diag] DragEntered mimes=[" + mimes + "]");
                    n.SetAccepted(true);
                }
            });
        dndPtr->Subscribe(dndPtr, "Dropped",
            [dDnd, dndPtr](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<Dropped>()) {
                    std::string data(e->Data().begin(), e->Data().end());
                    dDnd->SetText("[diag] Dropped: mime=" + std::string(e->MimeType()) +
                        " size=" + std::to_string(e->Data().size()));
                    dndPtr->SetText(data);
                    n.SetAccepted(true);
                }
            });

        content->AddNode(std::move(stateRow4));
    }

    dialogNode->SetContentNode(std::move(content));

    // Show via WorkspaceFrame -- dialog is embedded and constrained
    auto wsFrame = mainWin.GetWorkspaceFrame();
    if (wsFrame.get() != nullptr) {
        wsFrame->ShowDialog(std::move(dialogNode));
    }
}
void NyanCadMainWindow::OnOpenFloatingWindow()
{
    if (_app == nullptr) { return; }
    auto* _docMgr = _app->GetDocumentManagerImpl();
    if (_docMgr == nullptr) { return; }
    auto& mainWin = _app->MainWindow();

    // Create a FloatingWindowNode with a unique WindowId
    static uint32_t sNextFloatId = 100;
    auto winId = matcha::fw::WindowId::From(sNextFloatId++);
    auto floatNode = std::make_unique<matcha::fw::FloatingTabWindowNode>(
        "floating-" + std::to_string(winId.value), winId);
    floatNode->BuildWindow(mainWin.Widget());
    floatNode->SetTitle("Floating Tab Window");

    // Get framework-layer nodes created by FloatingTabWindowNode::BuildContent
    auto tabBarObs = floatNode->GetTabBarNode();
    auto wsFrame = floatNode->GetWorkspaceFrame();
    auto* floatDocArea = wsFrame ? wsFrame->GetDocumentArea().get() : nullptr;

    // Create DocumentView bound to this window's ID + shared DocumentManager
    // DocumentView observes (non-owning) the DocumentArea from the UiNode tree
    auto docView = std::make_unique<DocumentView>(
        winId, *_docMgr, tabBarObs.get(), floatDocArea);

    // Subscribe DnD on DocumentArea (already in UiNode tree with correct parenting)
    if (floatDocArea != nullptr) {
        floatDocArea->Subscribe(floatDocArea, "DragEntered",
            [](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<matcha::fw::DragEntered>()) {
                    for (const auto& mt : e->MimeTypes()) {
                        if (mt == "application/x-matcha-tab") { n.SetAccepted(true); return; }
                    }
                }
            });
        floatDocArea->Subscribe(floatDocArea, "DragMoved",
            [](matcha::EventNode&, matcha::Notification& n) { n.SetAccepted(true); });
        floatDocArea->Subscribe(floatDocArea, "Dropped",
            [this, winId](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<matcha::fw::Dropped>()) {
                    if (e->MimeType() == "application/x-matcha-tab" && !e->Data().empty()) {
                        auto pid = matcha::fw::PageId::From(
                            std::stoull(std::string(e->Data().begin(), e->Data().end())));
                        auto* dm = _app->GetDocumentManagerImpl();
                        if (dm) { (void)dm->MoveDocumentPage(pid, winId); }
                        n.SetAccepted(true);
                    }
                }
            });
        floatDocArea->Subscribe(floatDocArea, "TabPageDraggedOut",
            [this](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<matcha::fw::TabPageDraggedOut>()) {
                    CreateFloatingWindowForPage(e->GetPageId(), e->GlobalX(), e->GlobalY());
                }
            });
    }

    // Create a demo document in this floating window
    (void)_docMgr->CreateDocument("Float-Doc", winId);

    floatNode->Show();
    floatNode->Resize(800, 500);

    // Transfer ownership
    mainWin.AddNode(std::move(floatNode));
    auto state = std::make_unique<FloatingDocState>();
    state->docView = std::move(docView);
    _floatingDocStates.push_back(std::move(state));
}

void NyanCadMainWindow::CreateFloatingWindowForPage(
    matcha::fw::PageId pageId, int globalX, int globalY)
{
    if (_app == nullptr) { return; }
    auto* _docMgr = _app->GetDocumentManagerImpl();
    if (_docMgr == nullptr) { return; }
    auto& mainWin = _app->MainWindow();

    // Create a new Floating Tab Window
    static uint32_t sDetachFloatId = 200;
    auto winId = matcha::fw::WindowId::From(sDetachFloatId++);
    auto floatNode = std::make_unique<matcha::fw::FloatingTabWindowNode>(
        "detach-" + std::to_string(winId.value), winId);
    floatNode->BuildWindow(mainWin.Widget());
    floatNode->SetTitle("Floating Tab Window");

    // Get framework-layer nodes created by FloatingTabWindowNode::BuildContent
    auto tabBarObs = floatNode->GetTabBarNode();
    auto detachWsFrame = floatNode->GetWorkspaceFrame();
    auto* detachDocArea = detachWsFrame ? detachWsFrame->GetDocumentArea().get() : nullptr;

    auto docView = std::make_unique<DocumentView>(
        winId, *_docMgr, tabBarObs.get(), detachDocArea);

    if (detachDocArea != nullptr) {
        detachDocArea->Subscribe(detachDocArea, "DragEntered",
            [](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<matcha::fw::DragEntered>()) {
                    for (const auto& mt : e->MimeTypes()) {
                        if (mt == "application/x-matcha-tab") { n.SetAccepted(true); return; }
                    }
                }
            });
        detachDocArea->Subscribe(detachDocArea, "DragMoved",
            [](matcha::EventNode&, matcha::Notification& n) { n.SetAccepted(true); });
        detachDocArea->Subscribe(detachDocArea, "Dropped",
            [this, winId](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<matcha::fw::Dropped>()) {
                    if (e->MimeType() == "application/x-matcha-tab" && !e->Data().empty()) {
                        auto pid = matcha::fw::PageId::From(
                            std::stoull(std::string(e->Data().begin(), e->Data().end())));
                        auto* dm = _app->GetDocumentManagerImpl();
                        if (dm) { (void)dm->MoveDocumentPage(pid, winId); }
                        n.SetAccepted(true);
                    }
                }
            });
        detachDocArea->Subscribe(detachDocArea, "TabPageDraggedOut",
            [this](matcha::EventNode&, matcha::Notification& n) {
                if (auto* e = n.As<matcha::fw::TabPageDraggedOut>()) {
                    CreateFloatingWindowForPage(e->GetPageId(), e->GlobalX(), e->GlobalY());
                }
            });
    }

    // Position the window near the drop point
    floatNode->Resize(800, 500);
    floatNode->Move(globalX - 400, globalY - 30);
    floatNode->Show();

    // Move the page to this new window
    (void)_docMgr->MoveDocumentPage(pageId, winId);

    mainWin.AddNode(std::move(floatNode));
    auto state = std::make_unique<FloatingDocState>();
    state->docView = std::move(docView);
    _floatingDocStates.push_back(std::move(state));
}

auto NyanCadMainWindow::GetDocumentView() -> DocumentView*
{
    return _docView;
}

void NyanCadMainWindow::CloseFloatingWindows()
{
    // Release demo-layer state (DocumentViews, TabAdapters) first
    _floatingDocStates.clear();

    // Close all floating WindowNodes via the Shell
    if (_app == nullptr) { return; }
    auto& mainWin = _app->MainWindow();
    std::vector<matcha::fw::UiNode*> toRemove;
    for (size_t i = 0; i < mainWin.NodeCount(); ++i) {
        auto* child = mainWin.NodeAt(i);
        if (child != nullptr && child->Type() == matcha::fw::NodeType::WindowNode) {
            toRemove.push_back(child);
        }
    }
    for (auto* node : toRemove) {
        auto* win = dynamic_cast<matcha::fw::WindowNode*>(node);
        if (win == nullptr) { continue; }
        win->Close();
        win->Hide();
        mainWin.RemoveNode(node);
    }
}

} // namespace nyancad
