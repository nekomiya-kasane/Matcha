/**
 * @file FloatingWindowNode.cpp
 * @brief Implementation of FloatingWindowNode -- lightweight floating window base.
 */

#include "Matcha/Tree/Composition/Shell/FloatingWindowNode.h"

#include "Matcha/Tree/Composition/Shell/FloatingTitleBarNode.h"
#include "Matcha/Widgets/Shell/NyanFloatingTitleBar.h"

#include <QEvent>
#include <QVBoxLayout>
#include <QWidget>

namespace matcha::fw {

namespace {

class FloatingCloseEventFilter : public QObject {
public:
    explicit FloatingCloseEventFilter(WindowNode& node, QObject* parent = nullptr)
        : QObject(parent), _node(node) {}

protected:
    auto eventFilter(QObject* obj, QEvent* event) -> bool override
    {
        if (event->type() == QEvent::Close) {
            _node.MarkCloseRequested();
        }
        return QObject::eventFilter(obj, event);
    }

private:
    WindowNode& _node;
};

} // anonymous namespace

MATCHA_IMPLEMENT_CLASS(FloatingWindowNode, WindowNode)

FloatingWindowNode::FloatingWindowNode(std::string id, WindowId windowId,
                                       WindowKind kind)
    : WindowNode(std::move(id), windowId, kind)
{
}

FloatingWindowNode::~FloatingWindowNode() = default;

auto FloatingWindowNode::GetFloatingTitleBar() -> observer_ptr<FloatingTitleBarNode>
{
    for (auto* node : ChildrenOfType(NodeType::TitleBar)) {
        if (auto* ftb = dynamic_cast<FloatingTitleBarNode*>(node)) {
            return make_observer(ftb);
        }
    }
    return observer_ptr<FloatingTitleBarNode>{};
}

void FloatingWindowNode::BuildWindow(QWidget* parent)
{
    if (_mainWindow != nullptr) {
        return;
    }

    // Use a plain QWidget (not QMainWindow) for lighter weight
    auto* window = new QWidget(parent);
    window->setMinimumSize(640, 400);
    window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    if (parent != nullptr) {
        window->setWindowFlags(window->windowFlags() | Qt::Window);
    }

    // Top-level vertical layout: FloatingTitleBar + [content from subclass]
    auto* layout = new QVBoxLayout(window);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // -- FloatingTitleBarNode as UiNode child --
    auto titleBarNode = std::make_unique<FloatingTitleBarNode>("floating-titlebar");
    auto* floatTitleBar = titleBarNode->FloatingTitleBar();
    layout->addWidget(floatTitleBar);

    QObject::connect(floatTitleBar, &gui::NyanFloatingTitleBar::MinimizeRequested,
                     window, [window]() { window->showMinimized(); });
    QObject::connect(floatTitleBar, &gui::NyanFloatingTitleBar::MaximizeRequested,
                     window, [window]() {
        if (window->isMaximized()) {
            window->showNormal();
        } else {
            window->showMaximized();
        }
    });
    QObject::connect(floatTitleBar, &gui::NyanFloatingTitleBar::CloseRequested,
                     window, [window]() { window->close(); });
    AddNode(std::move(titleBarNode));

    // Intercept QCloseEvent to set _closeRequested
    window->installEventFilter(new FloatingCloseEventFilter(*this, window));

    // -- Central area (owns VBoxLayout for subclass content like DocumentArea) --
    _centralArea = new QWidget(window);
    _centralArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* centralLayout = new QVBoxLayout(_centralArea);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    layout->addWidget(_centralArea, /*stretch=*/1);

    // -- Let subclass populate the content area --
    BuildContent(_centralArea, layout);

    _mainWindow = window;
}

void FloatingWindowNode::BuildContent(QWidget* /*contentParent*/, QVBoxLayout* /*layout*/)
{
    // Default: no content. Subclasses override to populate.
}

} // namespace matcha::fw
