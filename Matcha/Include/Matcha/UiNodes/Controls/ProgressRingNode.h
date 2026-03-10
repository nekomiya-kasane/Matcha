#pragma once

/**
 * @file ProgressRingNode.h
 * @brief Typed WidgetNode wrapping NyanProgressRing for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanProgressRing (Scheme D typed node).
 *
 * Circular progress indicator supporting determinate (0-100%) and
 * indeterminate (spinning) modes. Ring thickness and text overlay
 * are configurable.
 *
 * Dispatches: (none — display-only widget).
 */
class MATCHA_EXPORT ProgressRingNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    explicit ProgressRingNode(std::string id);
    ~ProgressRingNode() override;

    ProgressRingNode(const ProgressRingNode&) = delete;
    auto operator=(const ProgressRingNode&) -> ProgressRingNode& = delete;
    ProgressRingNode(ProgressRingNode&&) = delete;
    auto operator=(ProgressRingNode&&) -> ProgressRingNode& = delete;

    void SetValue(int value);
    [[nodiscard]] auto Value() const -> int;

    void SetRange(int minimum, int maximum);

    void SetIndeterminate(bool indeterminate);
    [[nodiscard]] auto IsIndeterminate() const -> bool;

    void SetThickness(int thickness);
    [[nodiscard]] auto Thickness() const -> int;

    void SetTextVisible(bool visible);
    [[nodiscard]] auto IsTextVisible() const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
