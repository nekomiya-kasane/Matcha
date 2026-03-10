#pragma once

/**
 * @file PropertyGridNode.h
 * @brief Typed WidgetNode wrapping NyanPropertyGrid for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <span>
#include <string>
#include <string_view>

#include "Matcha/Foundation/WidgetEnums.h"

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanPropertyGrid (Scheme D typed node).
 *
 * Dispatches: WidgetPropertyChanged (on property edit).
 */
class MATCHA_EXPORT PropertyGridNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    struct Notification : WidgetNode::Notification {
        using PropertyChanged = matcha::fw::PropertyChanged;
    };

    explicit PropertyGridNode(std::string id);
    ~PropertyGridNode() override;

    PropertyGridNode(const PropertyGridNode&) = delete;
    auto operator=(const PropertyGridNode&) -> PropertyGridNode& = delete;
    PropertyGridNode(PropertyGridNode&&) = delete;
    auto operator=(PropertyGridNode&&) -> PropertyGridNode& = delete;

    void AddProperty(std::string_view name, gui::PropertyType type,
                     std::string_view defaultValue = {});

    /// @brief Add a numeric (Double/Integer) property.
    void AddProperty(std::string_view name, gui::PropertyType type, double defaultValue);

    /// @brief Add a boolean property.
    void AddProperty(std::string_view name, gui::PropertyType type, bool defaultValue);

    /// @brief Add a Choice property with a default selection and options.
    void AddProperty(std::string_view name, gui::PropertyType type,
                     std::string_view defaultValue, std::span<const std::string> choices);

    void SetPropertyValue(std::string_view name, std::string_view value);
    [[nodiscard]] auto PropertyValue(std::string_view name) const -> std::string;

    void AddGroup(std::string_view name);

    void Clear();

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
