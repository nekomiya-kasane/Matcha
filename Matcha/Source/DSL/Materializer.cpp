#include "Matcha/DSL/Blueprint.h"

#include "Matcha/Tree/ContainerNode.h"
#include "Matcha/Tree/Controls/BadgeNode.h"
#include "Matcha/Tree/Controls/CheckBoxNode.h"
#include "Matcha/Tree/Controls/CollapsibleSectionNode.h"
#include "Matcha/Tree/Controls/ColorPickerNode.h"
#include "Matcha/Tree/Controls/ColorSwatchNode.h"
#include "Matcha/Tree/Controls/ComboBoxNode.h"
#include "Matcha/Tree/Controls/DataTableNode.h"
#include "Matcha/Tree/Controls/DateTimePickerNode.h"
#include "Matcha/Tree/Controls/DoubleSpinBoxNode.h"
#include "Matcha/Tree/Controls/LabelNode.h"
#include "Matcha/Tree/Controls/LineEditNode.h"
#include "Matcha/Tree/Controls/LineNode.h"
#include "Matcha/Tree/Controls/ListWidgetNode.h"
#include "Matcha/Tree/Controls/MessageNode.h"
#include "Matcha/Tree/Controls/NotificationNode.h"
#include "Matcha/Tree/Controls/PaginatorNode.h"
#include "Matcha/Tree/Controls/PlainTextEditNode.h"
#include "Matcha/Tree/Controls/ProgressBarNode.h"
#include "Matcha/Tree/Controls/ProgressRingNode.h"
#include "Matcha/Tree/Controls/PropertyGridNode.h"
#include "Matcha/Tree/Controls/PushButtonNode.h"
#include "Matcha/Tree/Controls/RadioButtonNode.h"
#include "Matcha/Tree/Controls/RangeSliderNode.h"
#include "Matcha/Tree/Controls/SearchBoxNode.h"
#include "Matcha/Tree/Controls/SliderNode.h"
#include "Matcha/Tree/Controls/SpinBoxNode.h"
#include "Matcha/Tree/Controls/ToggleSwitchNode.h"
#include "Matcha/Tree/Controls/ToolButtonNode.h"
#include "Matcha/Tree/Controls/TreeWidgetNode.h"

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_map>

namespace matcha::dsl {

using namespace matcha::fw;

// ============================================================================
// Node factory registry
// ============================================================================

using FactoryFn = std::function<std::unique_ptr<UiNode>(const std::string& id)>;

static auto GetFactoryRegistry() -> std::unordered_map<NodeType, FactoryFn>& {
    static std::unordered_map<NodeType, FactoryFn> registry = {
        {NodeType::Container,      [](const std::string& id) { return std::make_unique<ContainerNode>(id, LayoutKind::Vertical); }},
        {NodeType::Label,          [](const std::string& id) { return std::make_unique<LabelNode>(id); }},
        {NodeType::PushButton,     [](const std::string& id) { return std::make_unique<PushButtonNode>(id); }},
        {NodeType::LineEdit,       [](const std::string& id) { return std::make_unique<LineEditNode>(id); }},
        {NodeType::CheckBox,       [](const std::string& id) { return std::make_unique<CheckBoxNode>(id); }},
        {NodeType::ComboBox,       [](const std::string& id) { return std::make_unique<ComboBoxNode>(id); }},
        {NodeType::SpinBox,        [](const std::string& id) { return std::make_unique<SpinBoxNode>(id); }},
        {NodeType::DoubleSpinBox,  [](const std::string& id) { return std::make_unique<DoubleSpinBoxNode>(id); }},
        {NodeType::RadioButton,    [](const std::string& id) { return std::make_unique<RadioButtonNode>(id); }},
        {NodeType::ToolButton,     [](const std::string& id) { return std::make_unique<ToolButtonNode>(id); }},
        {NodeType::ToggleSwitch,   [](const std::string& id) { return std::make_unique<ToggleSwitchNode>(id); }},
        {NodeType::Slider,         [](const std::string& id) { return std::make_unique<SliderNode>(id); }},
        {NodeType::ProgressBar,    [](const std::string& id) { return std::make_unique<ProgressBarNode>(id); }},
        {NodeType::ProgressRing,   [](const std::string& id) { return std::make_unique<ProgressRingNode>(id); }},
        {NodeType::Badge,          [](const std::string& id) { return std::make_unique<BadgeNode>(id); }},
        {NodeType::ColorPicker,    [](const std::string& id) { return std::make_unique<ColorPickerNode>(id); }},
        {NodeType::ColorSwatch,    [](const std::string& id) { return std::make_unique<ColorSwatchNode>(id); }},
        {NodeType::DataTable,      [](const std::string& id) { return std::make_unique<DataTableNode>(id); }},
        {NodeType::PropertyGrid,   [](const std::string& id) { return std::make_unique<PropertyGridNode>(id); }},
        {NodeType::ListWidget,     [](const std::string& id) { return std::make_unique<ListWidgetNode>(id); }},
        {NodeType::TreeWidget,     [](const std::string& id) { return std::make_unique<TreeWidgetNode>(id); }},
        {NodeType::SearchBox,      [](const std::string& id) { return std::make_unique<SearchBoxNode>(id); }},
        {NodeType::PlainTextEdit,  [](const std::string& id) { return std::make_unique<PlainTextEditNode>(id); }},
        {NodeType::RangeSlider,    [](const std::string& id) { return std::make_unique<RangeSliderNode>(id); }},
        {NodeType::DateTimePicker, [](const std::string& id) { return std::make_unique<DateTimePickerNode>(id); }},
        {NodeType::Paginator,      [](const std::string& id) { return std::make_unique<PaginatorNode>(id); }},
        {NodeType::Message,        [](const std::string& id) { return std::make_unique<MessageNode>(id); }},
        {NodeType::Notification,   [](const std::string& id) { return std::make_unique<NotificationNode>(id); }},
        {NodeType::Line,           [](const std::string& id) { return std::make_unique<LineNode>(id); }},
        {NodeType::CollapsibleSection, [](const std::string& id) { return std::make_unique<CollapsibleSectionNode>(id); }},
    };
    return registry;
}

// ============================================================================
// Property applicator
// ============================================================================

static void ApplyProperty(UiNode* node, const std::string& key, const std::string& value) {
    auto* wn = dynamic_cast<WidgetNode*>(node);
    if (wn == nullptr) { return; }

    if (key == "text") {
        if (auto* le = dynamic_cast<LineEditNode*>(wn)) {
            le->SetText(value);
        } else if (auto* lb = dynamic_cast<LabelNode*>(wn)) {
            lb->SetText(value);
        } else if (auto* pb = dynamic_cast<PushButtonNode*>(wn)) {
            pb->SetText(value);
        } else if (auto* cb = dynamic_cast<CheckBoxNode*>(wn)) {
            cb->SetText(value);
        } else if (auto* tb = dynamic_cast<ToolButtonNode*>(wn)) {
            tb->SetText(value);
        } else if (auto* rb = dynamic_cast<RadioButtonNode*>(wn)) {
            rb->SetText(value);
        }
    } else if (key == "placeholder") {
        if (auto* le = dynamic_cast<LineEditNode*>(wn)) {
            le->SetPlaceholder(value);
        }
    } else if (key == "enabled") {
        wn->SetEnabled(value == "true" || value == "1");
    } else if (key == "visible") {
        wn->SetVisible(value == "true" || value == "1");
    } else if (key == "tooltip") {
        wn->SetToolTip(value);
    } else if (key == "readOnly") {
        if (auto* le = dynamic_cast<LineEditNode*>(wn)) {
            le->SetReadOnly(value == "true" || value == "1");
        }
    } else if (key == "checked") {
        if (auto* cb = dynamic_cast<CheckBoxNode*>(wn)) {
            cb->SetChecked(value == "true" || value == "1");
        } else if (auto* ts = dynamic_cast<ToggleSwitchNode*>(wn)) {
            ts->SetChecked(value == "true" || value == "1");
        }
    }
}

// ============================================================================
// Materialize
// ============================================================================

static auto MaterializeImpl(const Blueprint& bp, UiNode* parent) -> UiNode* {
    auto& registry = GetFactoryRegistry();
    auto it = registry.find(bp.type);
    if (it == registry.end()) {
        assert(false && "Materialize: no factory for NodeType");
        return nullptr;
    }

    auto node = it->second(bp.id);
    auto* raw = node.get();

    for (const auto& [key, value] : bp.properties) {
        ApplyProperty(raw, key, value);
    }

    for (const auto& child : bp.children) {
        MaterializeImpl(child, raw);
    }

    parent->AddNode(std::move(node));
    return raw;
}

auto Materialize(const Blueprint& bp, UiNode* parent) -> UiNode* {
    assert(parent != nullptr && "Materialize: parent must not be null");
    return MaterializeImpl(bp, parent);
}

} // namespace matcha::dsl
