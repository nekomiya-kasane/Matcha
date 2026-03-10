#pragma once

/**
 * @file WidgetWrapper.h
 * @brief UiNode that wraps a user-defined QWidget into the UiNode tree.
 *
 * Business layer uses WidgetWrapper as an escape hatch to embed custom
 * QWidgets that don't have a dedicated typed WidgetNode (Scheme D).
 * The wrapper does NOT own the QWidget -- Qt parent-child tree manages lifetime.
 */

#include "Matcha/UiNodes/Core/UiNode.h"

class QWidget;

namespace matcha::fw {

/**
 * @brief Wraps an arbitrary QWidget as a UiNode child.
 */
class MATCHA_EXPORT WidgetWrapper : public UiNode {
    MATCHA_DECLARE_CLASS

public:
    /// @brief Construct a WidgetWrapper.
    /// @param id Unique node identifier.
    /// @param widget Non-owning pointer to the wrapped QWidget.
    explicit WidgetWrapper(std::string id, QWidget* widget);

    ~WidgetWrapper() override;

    /// @brief Return the wrapped QWidget.
    [[nodiscard]] auto Widget() -> QWidget* override { return _widget; }

private:
    QWidget* _widget;
};

} // namespace matcha::fw
