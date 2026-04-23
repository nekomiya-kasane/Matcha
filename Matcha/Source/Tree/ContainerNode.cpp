#include "Matcha/Tree/ContainerNode.h"
#include "Matcha/Theming/Token/ITokenRegistry.h"
#include "Matcha/Theming/Token/TokenRegistryGlobal.h"

#include "Matcha/Widgets/Shell/NyanSplitter.h"
#include "Widgets/_Private/FlowLayout.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QSplitter>
#include <QWidget>

namespace matcha::fw {

MATCHA_IMPLEMENT_CLASS(ContainerNode, UiNode)

namespace {

auto CreateLayout(LayoutKind kind, QWidget* container) -> QLayout*
{
    switch (kind) {
    case LayoutKind::Vertical:
        return new QVBoxLayout(container);
    case LayoutKind::Horizontal:
        return new QHBoxLayout(container);
    case LayoutKind::Grid:
        return new QGridLayout(container);
    case LayoutKind::Form:
        return new QFormLayout(container);
    case LayoutKind::Stack:
        return new QStackedLayout(container);
    case LayoutKind::Flow:
        return new matcha::gui::detail::FlowLayout(container);
    case LayoutKind::HSplitter:
    case LayoutKind::VSplitter:
        return nullptr; // splitter manages children directly, no layout
    }
    return new QVBoxLayout(container); // fallback
}

} // namespace

ContainerNode::ContainerNode(std::string id, LayoutKind kind, UiNode* parentHint)
    : UiNode(std::move(id), NodeType::Container)
    , _container(nullptr)
    , _layout(nullptr)
    , _kind(kind)
{
    auto* parentWidget = parentHint ? parentHint->Widget() : nullptr;
    if (kind == LayoutKind::HSplitter || kind == LayoutKind::VSplitter) {
        auto orientation = (kind == LayoutKind::HSplitter) ? Qt::Horizontal : Qt::Vertical;
        _container = new matcha::gui::NyanSplitter(orientation, parentWidget);
        // _layout stays nullptr — splitter manages children directly
    } else {
        _container = new QWidget(parentWidget);
        _layout = CreateLayout(kind, _container);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(0);
    }
}

auto ContainerNode::Wrap(std::string id, QWidget* widget) -> std::unique_ptr<ContainerNode>
{
    // Create a normal ContainerNode then swap its widget to the existing one.
    auto node = std::unique_ptr<ContainerNode>(new ContainerNode(
        std::move(id), LayoutKind::Horizontal, /*sentinel*/ nullptr));
    delete node->_container;
    node->_container = widget;
    // Use existing layout if any; otherwise create a default HBoxLayout.
    node->_layout = widget->layout();
    if (node->_layout == nullptr) {
        node->_layout = new QHBoxLayout(widget);
        node->_layout->setContentsMargins(0, 0, 0, 0);
        node->_layout->setSpacing(0);
    }
    node->_ownsWidget = false;
    return node;
}

ContainerNode::~ContainerNode()
{
    if (_ownsWidget && _container != nullptr && _container->parent() == nullptr) {
        delete _container;
    }
}

auto ContainerNode::Kind() const -> LayoutKind
{
    return _kind;
}

namespace {

auto ResolveToken(SpaceToken token) -> int
{
    auto* reg = GetGlobalTokenRegistry();
    if (reg != nullptr) {
        return reg->SpacingPx(token);
    }
    return ToPixels(token); // fallback: base px without density scaling
}

} // namespace

void ContainerNode::SetSpacing(SpaceToken token)
{
    _spacingToken = token;
    ApplySpacing();
}

void ContainerNode::SetMargins(SpaceToken token)
{
    _marginLeft = _marginTop = _marginRight = _marginBottom = token;
    ApplyMargins();
}

void ContainerNode::SetMargins(SpaceToken left, SpaceToken top,
                               SpaceToken right, SpaceToken bottom)
{
    _marginLeft   = left;
    _marginTop    = top;
    _marginRight  = right;
    _marginBottom = bottom;
    ApplyMargins();
}

void ContainerNode::ApplySpacing()
{
    if (_layout != nullptr) {
        _layout->setSpacing(ResolveToken(_spacingToken));
    }
}

void ContainerNode::SetDirection(TextDirection dir)
{
    _direction = dir;
    if (_container != nullptr) {
        _container->setLayoutDirection(
            dir == TextDirection::RTL ? Qt::RightToLeft : Qt::LeftToRight);
    }
    ApplyMargins(); // RTL swaps left/right margins
}

auto ContainerNode::Direction() const -> TextDirection
{
    return _direction;
}

void ContainerNode::ApplyMargins()
{
    if (_layout != nullptr) {
        int left  = ResolveToken(_marginLeft);
        int top   = ResolveToken(_marginTop);
        int right = ResolveToken(_marginRight);
        int bot   = ResolveToken(_marginBottom);

        // RTL: swap left and right margins
        if (_direction == TextDirection::RTL) {
            std::swap(left, right);
        }

        _layout->setContentsMargins(left, top, right, bot);
    }
}

void ContainerNode::SetStretch(int childIndex, int factor)
{
    if (_kind == LayoutKind::Vertical || _kind == LayoutKind::Horizontal) {
        auto* box = static_cast<QBoxLayout*>(_layout);
        if (childIndex >= 0 && childIndex < box->count()) {
            box->setStretch(childIndex, factor);
        }
    }
}

void ContainerNode::PlaceChild(UiNode* child, int row, int col, int rowSpan, int colSpan)
{
    if (_kind != LayoutKind::Grid || child == nullptr) {
        return;
    }

    auto* grid = static_cast<QGridLayout*>(_layout);
    auto* widget = child->Widget();
    if (widget != nullptr) {
        grid->addWidget(widget, row, col, rowSpan, colSpan);
    }
}

void ContainerNode::SetCurrentIndex(int index)
{
    if (_kind == LayoutKind::Stack) {
        auto* stack = static_cast<QStackedLayout*>(_layout);
        if (index >= 0 && index < stack->count()) {
            stack->setCurrentIndex(index);
        }
    }
}

auto ContainerNode::CurrentIndex() const -> int
{
    if (_kind == LayoutKind::Stack) {
        return static_cast<const QStackedLayout*>(_layout)->currentIndex();
    }
    return -1;
}

void ContainerNode::SetMinimumSize(int w, int h)
{
    if (_container != nullptr) { _container->setMinimumSize(w, h); }
}

void ContainerNode::SetMaximumSize(int w, int h)
{
    if (_container != nullptr) { _container->setMaximumSize(w, h); }
}

void ContainerNode::SetFixedSize(int w, int h)
{
    if (_container != nullptr) { _container->setFixedSize(w, h); }
}

auto ContainerNode::Widget() -> QWidget*
{
    return _container;
}

auto ContainerNode::AddNode(std::unique_ptr<UiNode> child) -> UiNode*
{
    auto* widget = child ? child->Widget() : nullptr;
    auto* raw = UiNode::AddNode(std::move(child));

    if (widget != nullptr) {
        switch (_kind) {
        case LayoutKind::Vertical:
        case LayoutKind::Horizontal:
            static_cast<QBoxLayout*>(_layout)->addWidget(widget);
            break;
        case LayoutKind::Grid:
            // Grid: default to next row, column 0. Use PlaceChild() for explicit placement.
            {
                auto* grid = static_cast<QGridLayout*>(_layout);
                grid->addWidget(widget, grid->rowCount(), 0);
            }
            break;
        case LayoutKind::Form:
            // Form: add as a spanning row. Use AddFormRow() for label-input pairs.
            static_cast<QFormLayout*>(_layout)->addRow(widget);
            break;
        case LayoutKind::Stack:
            static_cast<QStackedLayout*>(_layout)->addWidget(widget);
            break;
        case LayoutKind::Flow:
            _layout->addWidget(widget);
            break;
        case LayoutKind::HSplitter:
        case LayoutKind::VSplitter:
            static_cast<QSplitter*>(_container)->addWidget(widget);
            break;
        }
    }
    return raw;
}

auto ContainerNode::RemoveNode(UiNode* child) -> std::unique_ptr<UiNode>
{
    if (child != nullptr) {
        auto* widget = child->Widget();
        if (widget != nullptr) {
            if (_layout != nullptr) {
                _layout->removeWidget(widget);
            }
            widget->setParent(nullptr);
        }
    }
    return UiNode::RemoveNode(child);
}

} // namespace matcha::fw
