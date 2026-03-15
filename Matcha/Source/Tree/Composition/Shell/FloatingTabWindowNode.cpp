/**
 * @file FloatingTabWindowNode.cpp
 * @brief Implementation of FloatingTabWindowNode -- detached-tab floating window.
 */

#include "Matcha/Tree/Composition/Shell/FloatingTabWindowNode.h"

#include "Matcha/Tree/Composition/ActionBar/ActionBarNode.h"
#include "Matcha/Tree/Composition/Document/DocumentArea.h"
#include "Matcha/Tree/Composition/Document/TabBarNode.h"
#include "Matcha/Tree/Composition/Shell/ControlBar.h"
#include "Matcha/Tree/Composition/Shell/StatusBarNode.h"
#include "Matcha/Tree/Composition/Shell/WorkspaceFrame.h"
#include "Matcha/Widgets/ActionBar/NyanActionBar.h"
#include "Matcha/Widgets/Shell/NyanStatusBar.h"
#include "Matcha/Widgets/Shell/NyanTabBar.h"

#include <QVBoxLayout>
#include <QWidget>


namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(FloatingTabWindowNode, FloatingWindowNode)

FloatingTabWindowNode::FloatingTabWindowNode(std::string id, WindowId windowId)
    : FloatingWindowNode(std::move(id), windowId, WindowKind::Floating)
{
}

FloatingTabWindowNode::~FloatingTabWindowNode() = default;

auto FloatingTabWindowNode::GetTabBarNode() -> observer_ptr<TabBarNode>
{
    for (auto* node : ChildrenOfType(NodeType::TabBar)) {
        if (auto* tb = dynamic_cast<TabBarNode*>(node)) {
            return make_observer(tb);
        }
    }
    return observer_ptr<TabBarNode>{};
}

void FloatingTabWindowNode::BuildContent(QWidget* contentParent, QVBoxLayout* layout)
{
    // -- TabBarNode: floating-style tab bar below title bar --
    auto tabBarNode = std::make_unique<TabBarNode>(
        "floating-tabbar", gui::TabStyle::Floating, contentParent);
    auto* tabBarWidget = tabBarNode->TabBar();
    layout->insertWidget(layout->count() - 1, tabBarWidget);
    AddNode(std::move(tabBarNode));

    // -- WorkspaceFrame: ActionBar + DocumentArea + ControlBar --
    auto wsFrame = std::make_unique<WorkspaceFrame>("floating-workspace");
    wsFrame->SetContainerWidget(contentParent);

    // ActionBar overlays on top of contentParent (floating, not in layout)
    auto actionBarNode = std::make_unique<ActionBarNode>();
    auto* actionBarWidget = actionBarNode->ActionBar();
    actionBarWidget->setParent(contentParent);
    actionBarWidget->raise();
    wsFrame->AddNode(std::move(actionBarNode));

    // DocumentArea: added to wsFrame first so ParentNode chain is correct,
    // then its widget is placed into contentParent's VBoxLayout.
    auto docArea = std::make_unique<DocumentArea>("floating-document-area");
    wsFrame->AddNode(std::move(docArea));

    auto controlBar = std::make_unique<ControlBar>("floating-control-bar");
    wsFrame->AddNode(std::move(controlBar));

    AddNode(std::move(wsFrame));

    // Now that DocumentArea is in the tree, get its widget (triggers EnsureWidget
    // with correct parent chain: DocumentArea -> WorkspaceFrame -> _centralArea)
    // and add it to contentParent's layout.
    auto wsFramePtr = GetWorkspaceFrame();
    if (wsFramePtr) {
        auto docAreaObs = wsFramePtr->GetDocumentArea();
        if (docAreaObs) {
            auto* docWidget = docAreaObs->Widget();
            if (docWidget) {
                auto* centralLayout = contentParent->layout();
                if (centralLayout) {
                    centralLayout->addWidget(docWidget);
                }
                docAreaObs->SetAcceptDrops(true);
            }
        }
    }

    // StatusBar (appended to the window's top-level layout, after central area)
    auto statusBarNode = std::make_unique<StatusBarNode>();
    layout->addWidget(statusBarNode->StatusBar());
    AddNode(std::move(statusBarNode));
}

} // namespace matcha::fw
