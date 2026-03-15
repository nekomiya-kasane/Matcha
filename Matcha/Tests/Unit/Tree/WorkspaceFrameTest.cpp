#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc2y-extensions"
#endif

#include <ostream>

#include "doctest.h"

#include "Matcha/Tree/Composition/Shell/WorkspaceFrame.h"

#include "Matcha/Tree/Composition/Shell/ControlBar.h"
#include "Matcha/Tree/Composition/Document/DocumentArea.h"
#include "Matcha/Tree/UiNode.h"

using matcha::fw::ControlBar;
using matcha::fw::DocumentArea;
using matcha::fw::NodeType;
using matcha::fw::WorkspaceFrame;

TEST_CASE("WorkspaceFrame: constructor sets correct type") {
    WorkspaceFrame ws("test-ws", nullptr);
    CHECK(ws.Type() == NodeType::WorkspaceFrame);
    CHECK(ws.Name() == "test-ws");
}

TEST_CASE("WorkspaceFrame: GetDocumentArea returns nullptr when empty") {
    WorkspaceFrame ws("ws", nullptr);
    auto da = ws.GetDocumentArea();
    CHECK(da.get() == nullptr);
}

TEST_CASE("WorkspaceFrame: GetDocumentArea finds child") {
    WorkspaceFrame ws("ws", nullptr);
    ws.AddNode(std::make_unique<DocumentArea>("doc-area"));
    auto da = ws.GetDocumentArea();
    CHECK(da.get() != nullptr);
    CHECK(da->Type() == NodeType::DocumentArea);
}

TEST_CASE("WorkspaceFrame: GetControlBar finds child") {
    WorkspaceFrame ws("ws", nullptr);
    ws.AddNode(std::make_unique<ControlBar>("ctrl-bar"));
    auto cb = ws.GetControlBar();
    CHECK(cb.get() != nullptr);
    CHECK(cb->Type() == NodeType::ControlBar);
}

TEST_CASE("ControlBar: Clear removes all children") {
    ControlBar cb("cb");

    // Use a simple UiNode subclass for children
    class TestChild : public matcha::fw::UiNode {
    public:
        explicit TestChild(std::string id) : UiNode(std::move(id), NodeType::Custom) {}
    };

    cb.AddNode(std::make_unique<TestChild>("c1"));
    cb.AddNode(std::make_unique<TestChild>("c2"));
    CHECK(cb.NodeCount() == 2);

    cb.Clear();
    CHECK(cb.NodeCount() == 0);
}
