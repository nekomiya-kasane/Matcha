/**
 * @file LayerDemo.cpp
 * @brief Implementation of the Layer Demo page — showcases all 8 UI layer types.
 */

#include "LayerDemo.h"

#include "Matcha/Tree/ContainerNode.h"
#include "Matcha/Tree/PopupNode.h"
#include "Matcha/Tree/Controls/LabelNode.h"
#include "Matcha/Tree/Controls/PushButtonNode.h"
#include "Matcha/Tree/Composition/Menu/DialogNode.h"
#include "Matcha/Tree/Composition/Menu/MenuNode.h"
#include "Matcha/Tree/Composition/Menu/PopConfirmNode.h"
#include "Matcha/Tree/Composition/Shell/Application.h"
#include "Matcha/Event/EventNode.h"
#include "Matcha/Event/Notification.h"
#include "Matcha/Feedback/NotificationStackManager.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

using namespace matcha::fw;
using matcha::gui::LabelRole;
using matcha::gui::ButtonVariant;
using matcha::EventNode;
using matcha::Notification;

// ============================================================================
// Concrete PopupNode subclasses for the demo
// ============================================================================

namespace {

// -- Popover demo: a simple color picker placeholder --
class DemoPopoverNode : public PopupNode {
public:
    explicit DemoPopoverNode(std::string id)
        : PopupNode(std::move(id), NodeType::Popup, PopupBehavior::Dropdown) {}
protected:
    auto CreatePopupContent(QWidget* parent) -> QWidget* override {
        auto* w = new QWidget(parent);
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(12, 12, 12, 12);
        auto* lbl = new QLabel("Popover Content\n\nClick away to close.\nEscape also works.", w);
        lbl->setStyleSheet("color: #2d2640; background: #eaf8ff; padding: 16px; border-radius: 8px;");
        lay->addWidget(lbl);
        return w;
    }
    auto PreferredSize() -> Size override { return {260, 140}; }
};

// -- Tooltip demo: a simple rich tooltip --
class DemoTooltipNode : public PopupNode {
public:
    explicit DemoTooltipNode(std::string id)
        : PopupNode(std::move(id), NodeType::Popup, PopupBehavior::Tooltip) {}
protected:
    auto CreatePopupContent(QWidget* parent) -> QWidget* override {
        auto* w = new QWidget(parent);
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(8, 6, 8, 6);
        auto* lbl = new QLabel("Tooltip: Qt::ToolTip flag\nNo focus, no click-away handler.\nAuto-positioned via PopupPositioner.", w);
        lbl->setStyleSheet("color: #2d2640; background: #fff0d6; padding: 8px; border-radius: 4px; font-size: 12px;");
        lay->addWidget(lbl);
        return w;
    }
    auto PreferredSize() -> Size override { return {300, 80}; }
};

// -- FloatingPanel demo: a persistent tool palette --
class DemoFloatingPanelNode : public PopupNode {
public:
    explicit DemoFloatingPanelNode(std::string id)
        : PopupNode(std::move(id), NodeType::Popup, PopupBehavior::Floating) {}
protected:
    auto CreatePopupContent(QWidget* parent) -> QWidget* override {
        auto* w = new QWidget(parent);
        auto* lay = new QVBoxLayout(w);
        lay->setContentsMargins(12, 12, 12, 12);
        auto* title = new QLabel("Floating Tool Palette", w);
        title->setStyleSheet("font-weight: bold; font-size: 14px; color: #2d2640;");
        lay->addWidget(title);
        auto* body = new QLabel(
            "Qt::Tool | FramelessWindowHint\n\n"
            "- Non-modal\n"
            "- No click-away close\n"
            "- No Escape close (default)\n"
            "- Receives focus\n"
            "- Stays on top of parent window", w);
        body->setStyleSheet("color: #3b2d5e; background: #d8cfe8; padding: 12px; border-radius: 6px;");
        body->setWordWrap(true);
        lay->addWidget(body);
        return w;
    }
    auto PreferredSize() -> Size override { return {280, 220}; }
};

} // namespace

// ============================================================================
// Helper: build a labeled section with title + button grid
// ============================================================================

namespace nyancad {

static auto MakeSection(const std::string& id, const std::string& title)
    -> std::unique_ptr<ContainerNode>
{
    auto section = std::make_unique<ContainerNode>(id, LayoutKind::Vertical);
    section->SetSpacing(SpacingToken::Px4);

    auto lbl = std::make_unique<LabelNode>(id + "-title");
    lbl->SetText(title);
    lbl->SetRole(LabelRole::Title);
    section->AddNode(std::move(lbl));

    return section;
}

static auto MakeButton(const std::string& id, const std::string& text,
                        ButtonVariant variant = ButtonVariant::Ghost)
    -> std::unique_ptr<PushButtonNode>
{
    auto btn = std::make_unique<PushButtonNode>(id);
    btn->SetText(text);
    btn->SetVariant(variant);
    return btn;
}

// ============================================================================
// BuildLayerDemoPage
// ============================================================================

auto BuildLayerDemoPage(Application& /*app*/) -> std::unique_ptr<ContainerNode>
{
    auto root = std::make_unique<ContainerNode>("layer-demo-root", LayoutKind::Vertical);
    root->SetMargins(SpacingToken::Px8);
    root->SetSpacing(SpacingToken::Px8);

    // Page title
    auto pageTitle = std::make_unique<LabelNode>("layer-demo-title");
    pageTitle->SetText("Layer Demo — Semantic UI Layering System");
    pageTitle->SetRole(LabelRole::Title);
    root->AddNode(std::move(pageTitle));

    auto desc = std::make_unique<LabelNode>("layer-demo-desc");
    desc->SetText(
        "Demonstrates all 8 UI layer types defined by the OverlayPolicy specification. "
        "Each button triggers a different layer behavior. Observe Z-order, focus, "
        "click-away, and Escape key handling.");
    root->AddNode(std::move(desc));

    // Grid of buttons (2 columns via nested horizontal containers)
    auto grid = std::make_unique<ContainerNode>("layer-grid", LayoutKind::Vertical);
    grid->SetSpacing(SpacingToken::Px6);

    // ================================================================
    // Row 1: FloatingPanel + Dialog
    // ================================================================
    {
        auto row = std::make_unique<ContainerNode>("row-1", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px6);

        // -- FloatingPanel --
        auto sec1 = MakeSection("sec-floating", "1. FloatingPanel (LayerToken::Elevated)");
        auto descLbl = std::make_unique<LabelNode>("desc-floating");
        descLbl->SetText("Non-modal, no Escape close, no click-away. Qt::Tool window.");
        sec1->AddNode(std::move(descLbl));

        auto btn1 = MakeButton("btn-floating", "Open Floating Panel");
        auto* btn1Raw = btn1.get();
        sec1->AddNode(std::move(btn1));
        row->AddNode(std::move(sec1));

        // -- Dialog --
        auto sec2 = MakeSection("sec-dialog", "3. Dialog (LayerToken::Modal)");
        auto descLbl2 = std::make_unique<LabelNode>("desc-dialog");
        descLbl2->SetText("Modal, Escape close, focus trap, scrim, prevent scroll.");
        sec2->AddNode(std::move(descLbl2));

        auto btn2 = MakeButton("btn-dialog", "Open Modal Dialog", ButtonVariant::Primary);
        auto* btn2Raw = btn2.get();
        sec2->AddNode(std::move(btn2));
        row->AddNode(std::move(sec2));

        grid->AddNode(std::move(row));

        // Wire: FloatingPanel button
        static std::unique_ptr<DemoFloatingPanelNode> sFloatingPanel;
        btn1Raw->Subscribe(btn1Raw, "Clicked",
            [](EventNode& /*s*/, Notification& /*n*/) {
                if (sFloatingPanel && sFloatingPanel->IsOpen()) {
                    sFloatingPanel->Close();
                    return;
                }
                sFloatingPanel = std::make_unique<DemoFloatingPanelNode>("demo-floating");
                sFloatingPanel->OpenAtPoint({200, 200});
            });

        // Wire: Dialog button
        btn2Raw->Subscribe(btn2Raw, "Clicked",
            [](EventNode& /*s*/, Notification& /*n*/) {
                auto dlg = std::make_unique<DialogNode>("demo-dialog");
                dlg->SetTitle("Example Modal Dialog");
                dlg->SetWidth(DialogWidth::Medium);

                auto* content = new QWidget;
                auto* lay = new QVBoxLayout(content);
                auto* lbl = new QLabel(
                    "This is a Modal Dialog.\n\n"
                    "- Qt::WindowModal blocks the parent window\n"
                    "- Focus is trapped inside (Tab cycles within)\n"
                    "- Semi-transparent scrim covers the parent\n"
                    "- Escape key closes the dialog\n"
                    "- Click outside does NOT close (modal policy)");
                lbl->setWordWrap(true);
                lay->addWidget(lbl);
                dlg->SetContent(content);

                (void)dlg->ShowModal();
            });
    }

    // ================================================================
    // Row 2: Popover + Menu
    // ================================================================
    {
        auto row = std::make_unique<ContainerNode>("row-2", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px6);

        // -- Popover --
        auto sec1 = MakeSection("sec-popover", "4. Popover (LayerToken::Popover)");
        auto descLbl = std::make_unique<LabelNode>("desc-popover");
        descLbl->SetText("Non-modal, Escape close, click-away close, auto-positioned.");
        sec1->AddNode(std::move(descLbl));

        auto btn1 = MakeButton("btn-popover", "Open Popover");
        auto* btn1Raw = btn1.get();
        sec1->AddNode(std::move(btn1));
        row->AddNode(std::move(sec1));

        // -- Menu --
        auto sec2 = MakeSection("sec-menu", "5. Menu (LayerToken::Dropdown)");
        auto descLbl2 = std::make_unique<LabelNode>("desc-menu");
        descLbl2->SetText("Non-modal, Escape + click-away close, keyboard nav.");
        sec2->AddNode(std::move(descLbl2));

        auto btn2 = MakeButton("btn-menu", "Open Context Menu");
        auto* btn2Raw = btn2.get();
        sec2->AddNode(std::move(btn2));
        row->AddNode(std::move(sec2));

        grid->AddNode(std::move(row));

        // Wire: Popover button
        static std::unique_ptr<DemoPopoverNode> sPopover;
        btn1Raw->Subscribe(btn1Raw, "Clicked",
            [btn1Raw](EventNode& /*s*/, Notification& /*n*/) {
                if (sPopover && sPopover->IsOpen()) {
                    sPopover->Close();
                    return;
                }
                sPopover = std::make_unique<DemoPopoverNode>("demo-popover");
                sPopover->Open(btn1Raw, PopupPlacement::BottomStart);
            });

        // Wire: Menu button
        static std::unique_ptr<MenuNode> sMenu;
        btn2Raw->Subscribe(btn2Raw, "Clicked",
            [btn2Raw](EventNode& /*s*/, Notification& /*n*/) {
                sMenu = std::make_unique<MenuNode>("demo-menu");
                sMenu->AddItem("Cut");
                sMenu->AddItem("Copy");
                sMenu->AddItem("Paste");
                sMenu->AddSeparator();
                auto* sub = sMenu->AddSubmenu("More");
                sub->AddItem("Select All");
                sub->AddItem("Find...");

                if (btn2Raw->Widget() != nullptr) {
                    auto pos = btn2Raw->Widget()->mapToGlobal(
                        QPoint(0, btn2Raw->Widget()->height()));
                    sMenu->Popup(pos);
                }
            });
    }

    // ================================================================
    // Row 3: Tooltip + PopConfirm
    // ================================================================
    {
        auto row = std::make_unique<ContainerNode>("row-3", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px6);

        // -- Tooltip --
        auto sec1 = MakeSection("sec-tooltip", "6. Tooltip (LayerToken::Popover)");
        auto descLbl = std::make_unique<LabelNode>("desc-tooltip");
        descLbl->SetText("Non-modal, no focus, auto-positioned. ShowWithoutActivating.");
        sec1->AddNode(std::move(descLbl));

        auto btn1 = MakeButton("btn-tooltip", "Show Tooltip (3s)");
        auto* btn1Raw = btn1.get();
        sec1->AddNode(std::move(btn1));
        row->AddNode(std::move(sec1));

        // -- PopConfirm (Popover variant) --
        auto sec2 = MakeSection("sec-popconfirm", "4b. PopConfirm (Popover + confirm)");
        auto descLbl2 = std::make_unique<LabelNode>("desc-popconfirm");
        descLbl2->SetText("PopupNode::Dropdown wrapping NyanPopConfirm. Click-away close.");
        sec2->AddNode(std::move(descLbl2));

        auto btn2 = MakeButton("btn-popconfirm", "Delete Item?", ButtonVariant::Danger);
        auto* btn2Raw = btn2.get();
        sec2->AddNode(std::move(btn2));
        row->AddNode(std::move(sec2));

        grid->AddNode(std::move(row));

        // Wire: Tooltip button
        static std::unique_ptr<DemoTooltipNode> sTooltip;
        btn1Raw->Subscribe(btn1Raw, "Clicked",
            [btn1Raw](EventNode& /*s*/, Notification& /*n*/) {
                if (sTooltip && sTooltip->IsOpen()) {
                    sTooltip->Close();
                    return;
                }
                sTooltip = std::make_unique<DemoTooltipNode>("demo-tooltip");
                sTooltip->Open(btn1Raw, PopupPlacement::BottomCenter);
            });

        // Wire: PopConfirm button
        static std::unique_ptr<PopConfirmNode> sPopConfirm;
        btn2Raw->Subscribe(btn2Raw, "Clicked",
            [btn2Raw](EventNode& /*s*/, Notification& /*n*/) {
                if (sPopConfirm && sPopConfirm->IsOpen()) {
                    sPopConfirm->Close();
                    return;
                }
                sPopConfirm = std::make_unique<PopConfirmNode>("demo-popconfirm");
                sPopConfirm->Open(btn2Raw, PopupPlacement::BottomStart);
                sPopConfirm->SetTitle("Confirm Deletion");
                sPopConfirm->SetMessage("Are you sure you want to delete this item? This action cannot be undone.");
            });
    }

    // ================================================================
    // Row 4: Toast + DragGhost info
    // ================================================================
    {
        auto row = std::make_unique<ContainerNode>("row-4", LayoutKind::Horizontal);
        row->SetSpacing(SpacingToken::Px6);

        // -- Toast --
        auto sec1 = MakeSection("sec-toast", "7. Toast (LayerToken::Notification)");
        auto descLbl = std::make_unique<LabelNode>("desc-toast");
        descLbl->SetText("Non-modal, auto-dismiss. Managed by NotificationStackManager.");
        sec1->AddNode(std::move(descLbl));

        auto btn1 = MakeButton("btn-toast", "Push Toast Notification");
        auto* btn1Raw = btn1.get();
        sec1->AddNode(std::move(btn1));

        // Show current toast count
        auto toastCount = std::make_unique<LabelNode>("lbl-toast-count");
        toastCount->SetText("(Toast count: 0)");
        auto* toastCountRaw = toastCount.get();
        sec1->AddNode(std::move(toastCount));
        row->AddNode(std::move(sec1));

        // -- DragGhost --
        auto sec2 = MakeSection("sec-drag", "8. DragGhost (LayerToken::Maximum)");
        auto descLbl2 = std::make_unique<LabelNode>("desc-drag");
        descLbl2->SetText(
            "Highest Z-layer. No events, follows cursor.\n"
            "Configured via DragDropVisualManager.\n"
            "Drag any tab in the DocumentBar to see the ghost.");
        sec2->AddNode(std::move(descLbl2));

        auto infoLbl = std::make_unique<LabelNode>("lbl-drag-info");
        infoLbl->SetText(
            "DragPreviewStyle: Ghost | Icon | Compact | Custom\n"
            "DropZoneHighlight: None | Idle | Hover | Accepted | Rejected");
        sec2->AddNode(std::move(infoLbl));
        row->AddNode(std::move(sec2));

        grid->AddNode(std::move(row));

        // Wire: Toast button
        static NotificationStackManager sToastMgr;
        static int sToastSeq = 0;
        btn1Raw->Subscribe(btn1Raw, "Clicked",
            [toastCountRaw](EventNode& /*s*/, Notification& /*n*/) {
                ++sToastSeq;
                StackNotification toast;
                toast.priority = NotificationPriority::Normal;
                toast.title    = "Toast #" + std::to_string(sToastSeq);
                toast.message  = "This notification auto-dismisses after 5 seconds.";
                toast.duration = std::chrono::milliseconds{5000};
                sToastMgr.Push(std::move(toast));
                toastCountRaw->SetText(
                    "(Visible: " + std::to_string(sToastMgr.VisibleCount()) +
                    ", Queued: " + std::to_string(sToastMgr.TotalCount() - sToastMgr.VisibleCount()) + ")");
            });
    }

    root->AddNode(std::move(grid));

    // ================================================================
    // Layer Token Reference Table (as a label with monospace formatting)
    // ================================================================
    auto refSection = MakeSection("sec-ref", "LayerToken Reference");
    auto refLbl = std::make_unique<LabelNode>("lbl-ref");
    refLbl->SetText(
        "Base=0  Elevated=100  Sticky=200  Dropdown=300\n"
        "Modal=400  Popover=500  Notification=600  Overlay=700  Maximum=9999");
    root->AddNode(std::move(refSection));
    root->AddNode(std::move(refLbl));

    return root;
}

} // namespace nyancad
