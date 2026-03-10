#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

/**
 * @file UiNodeQueryTest.cpp
 * @brief Tests for the compile-time UiNode path query system.
 *
 * Covers P0 (core query), P1 (RunAll/Exists/Matches/NodePath),
 * and P2 (sibling/negation).
 */

#include "doctest.h"

#include "Matcha/Foundation/FixedString.h"
#include "Matcha/UiNodes/Core/UiNodeQuery.h"
#include "Matcha/UiNodes/Core/UiNode.h"

#include <string>

using namespace matcha::fw;
using matcha::FixedString;

// =========================================================================== //
// Test tree builder helper
// =========================================================================== //

static auto MakeNode(std::string id, NodeType type, std::string name = "")
    -> std::unique_ptr<UiNode>
{
    return std::make_unique<UiNode>(std::move(id), type,
                                    name.empty() ? std::string{} : std::move(name));
}

/**
 * Build a realistic test tree:
 *
 *   Shell
 *     WindowNode  (id="win0")
 *       TitleBar  (id="titlebar0")
 *         MenuBar (id="menubar0")
 *           Menu  (id="file", name="File")
 *           Menu  (id="edit", name="Edit")
 *       WorkspaceFrame (id="wsf0")
 *         DocumentArea (id="docarea0")
 *           DocumentPage (id="page-1", name="Doc1")
 *             ViewportGroup (id="vg1")
 *               Viewport (id="vp1", name="Perspective")
 *               Viewport (id="vp2", name="Front")
 *           DocumentPage (id="page-2", name="Doc2")
 *             ViewportGroup (id="vg2")
 *               Viewport (id="vp3", name="Top")
 *         StatusBar (id="status0")
 *     WindowNode  (id="win1")
 *       TitleBar  (id="titlebar1")
 */
struct TestTree {
    UiNode shell{"shell", NodeType::Shell, "Shell"};
    UiNode* win0    = nullptr;
    UiNode* win1    = nullptr;
    UiNode* titlebar0 = nullptr;
    UiNode* menubar0  = nullptr;
    UiNode* fileMenu  = nullptr;
    UiNode* editMenu  = nullptr;
    UiNode* wsf0      = nullptr;
    UiNode* docarea0  = nullptr;
    UiNode* page1     = nullptr;
    UiNode* page2     = nullptr;
    UiNode* vg1       = nullptr;
    UiNode* vg2       = nullptr;
    UiNode* vp1       = nullptr;
    UiNode* vp2       = nullptr;
    UiNode* vp3       = nullptr;
    UiNode* status0   = nullptr;
    UiNode* titlebar1 = nullptr;

    TestTree() {
        // WindowNode 0
        auto w0 = MakeNode("win0", NodeType::WindowNode, "MainWindow");
        auto tb0 = MakeNode("titlebar0", NodeType::TitleBar);
        auto mb0 = MakeNode("menubar0", NodeType::MenuBar);
        auto fm = MakeNode("file", NodeType::Menu, "File");
        auto em = MakeNode("edit", NodeType::Menu, "Edit");
        fileMenu = mb0->AddNode(std::move(fm));
        editMenu = mb0->AddNode(std::move(em));
        menubar0 = tb0->AddNode(std::move(mb0));
        titlebar0 = w0->AddNode(std::move(tb0));

        auto wf = MakeNode("wsf0", NodeType::WorkspaceFrame);
        auto da = MakeNode("docarea0", NodeType::DocumentArea);

        auto p1 = MakeNode("page-1", NodeType::DocumentPage, "Doc1");
        auto v1 = MakeNode("vg1", NodeType::ViewportGroup);
        auto vp_1 = MakeNode("vp1", NodeType::Viewport, "Perspective");
        auto vp_2 = MakeNode("vp2", NodeType::Viewport, "Front");
        vp1 = v1->AddNode(std::move(vp_1));
        vp2 = v1->AddNode(std::move(vp_2));
        vg1 = p1->AddNode(std::move(v1));
        page1 = da->AddNode(std::move(p1));

        auto p2 = MakeNode("page-2", NodeType::DocumentPage, "Doc2");
        auto v2 = MakeNode("vg2", NodeType::ViewportGroup);
        auto vp_3 = MakeNode("vp3", NodeType::Viewport, "Top");
        vp3 = v2->AddNode(std::move(vp_3));
        vg2 = p2->AddNode(std::move(v2));
        page2 = da->AddNode(std::move(p2));

        docarea0 = wf->AddNode(std::move(da));

        auto sb = MakeNode("status0", NodeType::StatusBar);
        status0 = wf->AddNode(std::move(sb));
        wsf0 = w0->AddNode(std::move(wf));

        win0 = shell.AddNode(std::move(w0));

        // WindowNode 1
        auto w1 = MakeNode("win1", NodeType::WindowNode, "FloatingWindow");
        auto tb1 = MakeNode("titlebar1", NodeType::TitleBar);
        titlebar1 = w1->AddNode(std::move(tb1));
        win1 = shell.AddNode(std::move(w1));
    }
};

// =========================================================================== //
// Part 0: FixedString basics
// =========================================================================== //

TEST_SUITE("FixedString") {
    TEST_CASE("basic construction and view") {
        constexpr FixedString fs("hello");
        static_assert(fs.size() == 5);
        static_assert(fs.view() == "hello");
        static_assert(fs[0] == 'h');
    }

    TEST_CASE("equality") {
        constexpr FixedString a("abc");
        constexpr FixedString b("abc");
        constexpr FixedString c("xyz");
        static_assert(a == b);
        static_assert(!(a == c));
    }
}

// =========================================================================== //
// Part 1: NodeType constexpr mapping
// =========================================================================== //

TEST_SUITE("NodeType mapping") {
    TEST_CASE("ParseNodeType round-trip") {
        static_assert(ParseNodeType("Shell") == NodeType::Shell);
        static_assert(ParseNodeType("WindowNode") == NodeType::WindowNode);
        static_assert(ParseNodeType("TitleBar") == NodeType::TitleBar);
        static_assert(ParseNodeType("MenuBar") == NodeType::MenuBar);
        static_assert(ParseNodeType("Viewport") == NodeType::Viewport);
        static_assert(ParseNodeType("Custom") == NodeType::Custom);
        CHECK(true); // static_asserts are the real tests
    }

    TEST_CASE("NodeTypeName round-trip") {
        static_assert(NodeTypeName(NodeType::Shell) == "Shell");
        static_assert(NodeTypeName(NodeType::WindowNode) == "WindowNode");
        static_assert(NodeTypeName(NodeType::Viewport) == "Viewport");
        CHECK(true);
    }
}

// =========================================================================== //
// P0: Basic Query by NodeType chain
// =========================================================================== //

TEST_SUITE("P0: Query by NodeType") {
    TEST_CASE("single-level type query") {
        TestTree t;
        auto r = Q<"WindowNode">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("multi-level type query") {
        TestTree t;
        auto r = Q<"WindowNode/TitleBar/MenuBar">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.menubar0);
    }

    TEST_CASE("deep path to Viewport") {
        TestTree t;
        auto r = Q<"WindowNode/WorkspaceFrame/DocumentArea/DocumentPage/ViewportGroup/Viewport">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.vp1);
    }

    TEST_CASE("query failure returns error") {
        TestTree t;
        auto r = Q<"WindowNode/ActionBar">.Run(&t.shell);
        REQUIRE(!r.has_value());
        CHECK(r.error().segmentIndex == 1);
        CHECK(r.error().message.find("no matching child") != std::string::npos);
    }

    TEST_CASE("null root returns error") {
        auto r = Q<"WindowNode">.Run(nullptr);
        REQUIRE(!r.has_value());
        CHECK(r.error().message == "root node is null");
    }
}

// =========================================================================== //
// P0: Index selector [N]
// =========================================================================== //

TEST_SUITE("P0: Index selector") {
    TEST_CASE("[0] selects first match") {
        TestTree t;
        auto r = Q<"WindowNode[0]">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("[1] selects second match") {
        TestTree t;
        auto r = Q<"WindowNode[1]">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win1);
    }

    TEST_CASE("[N] out of range returns error") {
        TestTree t;
        auto r = Q<"WindowNode[5]">.Run(&t.shell);
        CHECK(!r.has_value());
    }

    TEST_CASE("nested index: second Menu") {
        TestTree t;
        auto r = Q<"WindowNode/TitleBar/MenuBar/Menu[1]">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.editMenu);
    }
}

// =========================================================================== //
// P0: #id matching
// =========================================================================== //

TEST_SUITE("P0: Id matching") {
    TEST_CASE("#id direct child") {
        TestTree t;
        auto r = Q<"#win0">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("#id in path") {
        TestTree t;
        auto r = Q<"#win0/TitleBar/MenuBar">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.menubar0);
    }

    TEST_CASE("#id not found") {
        TestTree t;
        auto r = Q<"#nonexistent">.Run(&t.shell);
        CHECK(!r.has_value());
    }
}

// =========================================================================== //
// P0: @name matching
// =========================================================================== //

TEST_SUITE("P0: Name matching") {
    TEST_CASE("@name direct child") {
        TestTree t;
        auto r = Q<"WindowNode/TitleBar/MenuBar/@File">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.fileMenu);
    }

    TEST_CASE("@name second match") {
        TestTree t;
        auto r = Q<"WindowNode/TitleBar/MenuBar/@Edit">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.editMenu);
    }
}

// =========================================================================== //
// P0: Combined type+id, type+name
// =========================================================================== //

TEST_SUITE("P0: Combined matching") {
    TEST_CASE("Type#id") {
        TestTree t;
        auto r = Q<"WindowNode#win0">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("Type#id mismatch") {
        TestTree t;
        auto r = Q<"WindowNode#win99">.Run(&t.shell);
        CHECK(!r.has_value());
    }

    TEST_CASE("Type@name") {
        TestTree t;
        auto r = Q<"WindowNode@MainWindow">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("Type@name second window") {
        TestTree t;
        auto r = Q<"WindowNode@FloatingWindow">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win1);
    }
}

// =========================================================================== //
// P0: Wildcard *
// =========================================================================== //

TEST_SUITE("P0: Wildcard") {
    TEST_CASE("* matches first child") {
        TestTree t;
        auto r = Q<"*">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("*[1] matches second child") {
        TestTree t;
        auto r = Q<"*[1]">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win1);
    }

    TEST_CASE("*/TitleBar matches first window's titlebar") {
        TestTree t;
        auto r = Q<"*/TitleBar">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.titlebar0);
    }
}

// =========================================================================== //
// P0: ** recursive descend
// =========================================================================== //

TEST_SUITE("P0: Recursive descend") {
    TEST_CASE("**/Viewport finds first viewport in tree") {
        TestTree t;
        auto r = Q<"**/Viewport">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.vp1);
    }

    TEST_CASE("**/Viewport[2] finds third viewport") {
        TestTree t;
        auto r = Q<"**/Viewport[2]">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.vp3);
    }

    TEST_CASE("**/MenuBar finds deeply nested menubar") {
        TestTree t;
        auto r = Q<"**/MenuBar">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.menubar0);
    }

    TEST_CASE("**/Viewport@Front") {
        TestTree t;
        auto r = Q<"**/Viewport@Front">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.vp2);
    }

    TEST_CASE("** at middle of path") {
        TestTree t;
        auto r = Q<"WindowNode/**/Viewport">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.vp1);
    }

    TEST_CASE("** no match") {
        TestTree t;
        auto r = Q<"**/ActionBar">.Run(&t.shell);
        CHECK(!r.has_value());
    }
}

// =========================================================================== //
// P0: ^^ recursive ascend
// =========================================================================== //

TEST_SUITE("P0: Recursive ascend") {
    TEST_CASE("^^/WindowNode from viewport") {
        TestTree t;
        auto r = Q<"^^/WindowNode">.Run(t.vp1);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("^^/Shell from deep node") {
        TestTree t;
        auto r = Q<"^^/Shell">.Run(t.vp3);
        REQUIRE(r.has_value());
        CHECK(r.value() == &t.shell);
    }

    TEST_CASE("^^/ActionBar from viewport fails") {
        TestTree t;
        auto r = Q<"^^/ActionBar">.Run(t.vp1);
        CHECK(!r.has_value());
    }
}

// =========================================================================== //
// P0: .. parent navigation
// =========================================================================== //

TEST_SUITE("P0: Parent navigation") {
    TEST_CASE(".. goes to parent") {
        TestTree t;
        auto r = Q<"..">.Run(t.menubar0);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.titlebar0);
    }

    TEST_CASE("../../.. multiple levels") {
        TestTree t;
        auto r = Q<"../../..">.Run(t.menubar0);
        REQUIRE(r.has_value());
        CHECK(r.value() == &t.shell);
    }

    TEST_CASE(".. then sibling: ../WorkspaceFrame") {
        TestTree t;
        auto r = Q<"../WorkspaceFrame">.Run(t.titlebar0);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.wsf0);
    }

    TEST_CASE(".. on root fails") {
        TestTree t;
        auto r = Q<"..">.Run(&t.shell);
        CHECK(!r.has_value());
    }
}

// =========================================================================== //
// P0: Pipe operators
// =========================================================================== //

TEST_SUITE("P0: Pipe operators") {
    TEST_CASE("node | Q gives result") {
        TestTree t;
        auto r = t.shell | Q<"WindowNode/TitleBar">;
        REQUIRE(r.has_value());
        CHECK(r.value() == t.titlebar0);
    }

    TEST_CASE("pointer | Q") {
        TestTree t;
        UiNode* root = &t.shell;
        auto r = root | Q<"WindowNode">;
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("chained pipe: result | Q") {
        TestTree t;
        auto r = t.shell | Q<"WindowNode"> | Q<"TitleBar/MenuBar">;
        REQUIRE(r.has_value());
        CHECK(r.value() == t.menubar0);
    }

    TEST_CASE("chained pipe propagates error") {
        TestTree t;
        auto r = t.shell | Q<"WindowNode/ActionBar"> | Q<"TitleBar">;
        CHECK(!r.has_value());
    }
}

// =========================================================================== //
// P1: RunAll
// =========================================================================== //

TEST_SUITE("P1: RunAll") {
    TEST_CASE("RunAll **/Viewport collects all viewports") {
        TestTree t;
        auto vps = Q<"**/Viewport">.RunAll(&t.shell);
        CHECK(vps.size() == 3);
        CHECK(vps[0] == t.vp1);
        CHECK(vps[1] == t.vp2);
        CHECK(vps[2] == t.vp3);
    }

    TEST_CASE("RunAll WindowNode collects both windows") {
        TestTree t;
        auto wins = Q<"WindowNode">.RunAll(&t.shell);
        CHECK(wins.size() == 2);
        CHECK(wins[0] == t.win0);
        CHECK(wins[1] == t.win1);
    }

    TEST_CASE("RunAll **/Menu collects both menus") {
        TestTree t;
        auto menus = Q<"**/Menu">.RunAll(&t.shell);
        CHECK(menus.size() == 2);
        CHECK(menus[0] == t.fileMenu);
        CHECK(menus[1] == t.editMenu);
    }

    TEST_CASE("RunAll with no matches returns empty") {
        TestTree t;
        auto r = Q<"**/ActionBar">.RunAll(&t.shell);
        CHECK(r.empty());
    }

    TEST_CASE("pipe: node | Q | All") {
        TestTree t;
        auto vps = t.shell | (Q<"**/Viewport"> | All);
        CHECK(vps.size() == 3);
    }
}

// =========================================================================== //
// P1: Exists
// =========================================================================== //

TEST_SUITE("P1: Exists") {
    TEST_CASE("Exists true") {
        TestTree t;
        CHECK(Q<"WindowNode/TitleBar/MenuBar">.Exists(&t.shell));
    }

    TEST_CASE("Exists false") {
        TestTree t;
        CHECK(!Q<"WindowNode/ActionBar">.Exists(&t.shell));
    }

    TEST_CASE("pipe: node | Q | Exist") {
        TestTree t;
        bool e = t.shell | (Q<"**/Viewport"> | Exist);
        CHECK(e);
    }
}

// =========================================================================== //
// P1: RunOr
// =========================================================================== //

TEST_SUITE("P1: RunOr") {
    TEST_CASE("RunOr returns found node") {
        TestTree t;
        auto* r = Q<"WindowNode">.RunOr(&t.shell, nullptr);
        CHECK(r == t.win0);
    }

    TEST_CASE("RunOr returns fallback on failure") {
        TestTree t;
        auto* r = Q<"ActionBar">.RunOr(&t.shell, &t.shell);
        CHECK(r == &t.shell);
    }
}

// =========================================================================== //
// P1: Matches<Pattern>
// =========================================================================== //

TEST_SUITE("P1: Matches") {
    TEST_CASE("exact path matches") {
        TestTree t;
        CHECK(Matches<"Shell/WindowNode/TitleBar/MenuBar">(t.menubar0));
    }

    TEST_CASE("** wildcard in pattern") {
        TestTree t;
        CHECK(Matches<"Shell/**/MenuBar">(t.menubar0));
    }

    TEST_CASE("** at start") {
        TestTree t;
        CHECK(Matches<"**/Viewport">(t.vp1));
        CHECK(Matches<"**/Viewport">(t.vp2));
        CHECK(Matches<"**/Viewport">(t.vp3));
    }

    TEST_CASE("** at start and end") {
        TestTree t;
        CHECK(Matches<"**/MenuBar/**">(t.fileMenu));
        CHECK(Matches<"**/MenuBar/**">(t.editMenu));
    }

    TEST_CASE("non-matching pattern") {
        TestTree t;
        CHECK(!Matches<"Shell/ActionBar">(t.menubar0));
    }

    TEST_CASE("null node") {
        CHECK(!Matches<"Shell">(static_cast<UiNode*>(nullptr)));
    }
}

// =========================================================================== //
// P1: NodePath (debug path)
// =========================================================================== //

TEST_SUITE("P1: NodePath") {
    TEST_CASE("root node path") {
        TestTree t;
        CHECK(NodePath(&t.shell) == "Shell");
    }

    TEST_CASE("deep node path") {
        TestTree t;
        auto path = NodePath(t.menubar0);
        CHECK(path == "Shell/WindowNode[0]/TitleBar/MenuBar");
    }

    TEST_CASE("viewport path with index") {
        TestTree t;
        auto path = NodePath(t.vp2);
        CHECK(path == "Shell/WindowNode[0]/WorkspaceFrame/DocumentArea/DocumentPage[0]/ViewportGroup/Viewport[1]");
    }

    TEST_CASE("second window path") {
        TestTree t;
        auto path = NodePath(t.win1);
        CHECK(path == "Shell/WindowNode[1]");
    }

    TEST_CASE("null returns empty") {
        CHECK(NodePath(nullptr).empty());
    }
}

// =========================================================================== //
// P2: Sibling navigation
// =========================================================================== //

TEST_SUITE("P2: Sibling navigation") {
    TEST_CASE(">WindowNode finds next window sibling") {
        TestTree t;
        auto r = Q<">WindowNode">.Run(t.win0);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win1);
    }

    TEST_CASE(">WindowNode on last sibling fails") {
        TestTree t;
        auto r = Q<">WindowNode">.Run(t.win1);
        CHECK(!r.has_value());
    }

    TEST_CASE("<WindowNode finds prev window sibling") {
        TestTree t;
        auto r = Q<"<WindowNode">.Run(t.win1);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("<WindowNode on first sibling fails") {
        TestTree t;
        auto r = Q<"<WindowNode">.Run(t.win0);
        CHECK(!r.has_value());
    }

    TEST_CASE(">Viewport finds next viewport") {
        TestTree t;
        auto r = Q<">Viewport">.Run(t.vp1);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.vp2);
    }
}

// =========================================================================== //
// P2: Negation
// =========================================================================== //

TEST_SUITE("P2: Negation") {
    TEST_CASE("!StatusBar matches non-StatusBar child") {
        TestTree t;
        // wsf0 has children: DocumentArea, StatusBar
        // !StatusBar should match DocumentArea (first non-StatusBar child)
        auto r = Q<"!StatusBar">.Run(t.wsf0);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.docarea0);
    }

    TEST_CASE("!DocumentArea matches StatusBar") {
        TestTree t;
        auto r = Q<"!DocumentArea">.Run(t.wsf0);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.status0);
    }
}

// =========================================================================== //
// P0: Error messages
// =========================================================================== //

TEST_SUITE("Error messages") {
    TEST_CASE("QueryError::What() format") {
        TestTree t;
        auto r = Q<"WindowNode/ActionBar">.Run(&t.shell);
        REQUIRE(!r.has_value());
        auto msg = r.error().What();
        CHECK(msg.find("segment 1") != std::string::npos);
        CHECK(msg.find("ActionBar") != std::string::npos);
        CHECK(msg.find("no matching child") != std::string::npos);
    }
}

// =========================================================================== //
// Complex / integration scenarios
// =========================================================================== //

TEST_SUITE("Complex queries") {
    TEST_CASE("^^/WindowNode then down to StatusBar") {
        TestTree t;
        auto r = t.vp1
            | Q<"^^/WindowNode">
            | Q<"WorkspaceFrame/StatusBar">;
        REQUIRE(r.has_value());
        CHECK(r.value() == t.status0);
    }

    TEST_CASE(".. then sibling index") {
        TestTree t;
        // From fileMenu, go up to MenuBar, then get second Menu
        auto r = Q<"../Menu[1]">.Run(t.fileMenu);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.editMenu);
    }

    TEST_CASE("** + combined type@name") {
        TestTree t;
        auto r = Q<"**/Viewport@Top">.Run(&t.shell);
        REQUIRE(r.has_value());
        CHECK(r.value() == t.vp3);
    }

    TEST_CASE("RunAll ^^ from deep node") {
        TestTree t;
        // From vp1, collect all ancestors
        auto ancestors = Q<"^^">.RunAll(t.vp1);
        // Should be: ViewportGroup, DocumentPage, DocumentArea, WorkspaceFrame, WindowNode, Shell
        CHECK(ancestors.size() == 6);
        CHECK(ancestors[0] == t.vg1);
        CHECK(ancestors[5] == &t.shell);
    }

    TEST_CASE("RunAll ^^/WindowNode from viewport") {
        TestTree t;
        auto wins = Q<"^^/WindowNode">.RunAll(t.vp1);
        CHECK(wins.size() == 1);
        CHECK(wins[0] == t.win0);
    }
}

// =========================================================================== //
// ResolveQueryTarget -- proxy/redirect
// =========================================================================== //

namespace {

/// A proxy node that redirects query results to another node.
class ProxyNode : public UiNode {
public:
    ProxyNode(std::string id, UiNode* target)
        : UiNode(std::move(id), NodeType::Custom, "proxy")
        , _target(target) {}

    auto ResolveQueryTarget() -> UiNode* override { return _target; }

private:
    UiNode* _target;
};

} // namespace

TEST_SUITE("ResolveQueryTarget") {
    TEST_CASE("Run resolves proxy to target") {
        // Build: Shell -> ProxyNode(proxy0) -> (resolves to vp1)
        //        Shell -> WindowNode -> ... -> vp1
        TestTree t;
        auto proxy = std::make_unique<ProxyNode>("proxy0", t.vp1);
        auto* proxyPtr = t.shell.AddNode(std::move(proxy));
        (void)proxyPtr;

        auto r = Q<"Custom#proxy0">.Run(&t.shell);
        REQUIRE(r.has_value());
        // Should resolve to vp1, not the proxy itself
        CHECK(r.value() == t.vp1);
    }

    TEST_CASE("RunAll resolves each result through ResolveQueryTarget") {
        TestTree t;
        // Add two proxy nodes pointing to different targets
        auto proxy1 = std::make_unique<ProxyNode>("pa", t.menubar0);
        auto proxy2 = std::make_unique<ProxyNode>("pb", t.status0);
        t.shell.AddNode(std::move(proxy1));
        t.shell.AddNode(std::move(proxy2));

        // Query all Custom nodes under shell
        auto results = Q<"Custom">.RunAll(&t.shell);
        // Both should be resolved
        CHECK(results.size() == 2);
        CHECK(results[0] == t.menubar0);
        CHECK(results[1] == t.status0);
    }

    TEST_CASE("Resolve returning nullptr is filtered out in RunAll") {
        TestTree t;
        auto proxy = std::make_unique<ProxyNode>("null_proxy", nullptr);
        t.shell.AddNode(std::move(proxy));

        auto results = Q<"Custom">.RunAll(&t.shell);
        // null-resolved proxy should be filtered out
        CHECK(results.empty());
    }

    TEST_CASE("default ResolveQueryTarget returns this") {
        TestTree t;
        CHECK(t.win0->ResolveQueryTarget() == t.win0);
        CHECK(t.menubar0->ResolveQueryTarget() == t.menubar0);
    }

    TEST_CASE("pipe resolves proxy") {
        TestTree t;
        auto proxy = std::make_unique<ProxyNode>("pxy", t.editMenu);
        t.shell.AddNode(std::move(proxy));

        auto r = t.shell | Q<"Custom#pxy">;
        REQUIRE(r.has_value());
        CHECK(r.value() == t.editMenu);
    }
}

// =========================================================================== //
// Predicate lambda filtering
// =========================================================================== //

TEST_SUITE("Predicate filtering") {
    TEST_CASE("Run with predicate accepts") {
        TestTree t;
        auto r = Q<"WindowNode">.Run(&t.shell, [](UiNode* n) {
            return n->Name() == "MainWindow";
        });
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("Run with predicate rejects") {
        TestTree t;
        auto r = Q<"WindowNode">.Run(&t.shell, [](UiNode* n) {
            return n->Name() == "NonExistent";
        });
        REQUIRE(!r.has_value());
        CHECK(r.error().message.find("predicate rejected") != std::string::npos);
    }

    TEST_CASE("RunAll with predicate filters results") {
        TestTree t;
        auto vps = Q<"**/Viewport">.RunAll(&t.shell, [](UiNode* n) {
            return n->Name() != "Front";
        });
        // Should exclude vp2 (name="Front"), keep vp1 and vp3
        CHECK(vps.size() == 2);
        CHECK(vps[0] == t.vp1);
        CHECK(vps[1] == t.vp3);
    }

    TEST_CASE("Exists with predicate true") {
        TestTree t;
        CHECK(Q<"**/Viewport">.Exists(&t.shell, [](UiNode* n) {
            return n->Name() == "Perspective";
        }));
    }

    TEST_CASE("Exists with predicate false") {
        TestTree t;
        CHECK(!Q<"**/Viewport">.Exists(&t.shell, [](UiNode* n) {
            return n->Name() == "NoSuchName";
        }));
    }

    TEST_CASE("RunOr with predicate falls back") {
        TestTree t;
        auto* r = Q<"WindowNode">.RunOr(&t.shell, &t.shell, [](UiNode* n) {
            return n->Name() == "NoMatch";
        });
        CHECK(r == &t.shell); // fallback
    }

    TEST_CASE("RunOr with predicate succeeds") {
        TestTree t;
        auto* r = Q<"WindowNode">.RunOr(&t.shell, nullptr, [](UiNode* n) {
            return n->Name() == "MainWindow";
        });
        CHECK(r == t.win0);
    }
}

// =========================================================================== //
// Filter pipe terminal
// =========================================================================== //

TEST_SUITE("Filter pipe") {
    TEST_CASE("QueryResult | Filter accepts") {
        TestTree t;
        auto r = (t.shell | Q<"WindowNode">) | Filter([](UiNode* n) {
            return n->Name() == "MainWindow";
        });
        REQUIRE(r.has_value());
        CHECK(r.value() == t.win0);
    }

    TEST_CASE("QueryResult | Filter rejects") {
        TestTree t;
        auto r = (t.shell | Q<"WindowNode">) | Filter([](UiNode* n) {
            return n->Name() == "Nope";
        });
        CHECK(!r.has_value());
    }

    TEST_CASE("vector | Filter filters results") {
        TestTree t;
        auto vps = (t.shell | (Q<"**/Viewport"> | All))
            | Filter([](UiNode* n) { return n->Name() == "Top"; });
        CHECK(vps.size() == 1);
        CHECK(vps[0] == t.vp3);
    }

    TEST_CASE("chained query then filter") {
        TestTree t;
        auto r = t.shell
            | Q<"WindowNode">
            | Q<"TitleBar/MenuBar/Menu">
            | Filter([](UiNode* n) { return n->Name() == "File"; });
        REQUIRE(r.has_value());
        CHECK(r.value() == t.fileMenu);
    }

    TEST_CASE("filter on failed query propagates error") {
        TestTree t;
        auto r = (t.shell | Q<"WindowNode/ActionBar">)
            | Filter([](UiNode*) { return true; });
        CHECK(!r.has_value());
    }
}
