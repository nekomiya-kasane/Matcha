#pragma once

/**
 * @file ProgressBarNode.h
 * @brief Typed WidgetNode wrapping NyanProgressBar for UiNode tree.
 */

#include "Matcha/UiNodes/Core/WidgetNode.h"

#include <string>

namespace matcha::fw {

/**
 * @brief UiNode wrapper for NyanProgressBar (Scheme D typed node).
 *
 * Pure display node -- no notification dispatch.
 */
class MATCHA_EXPORT ProgressBarNode : public WidgetNode {
    MATCHA_DECLARE_CLASS

public:
    explicit ProgressBarNode(std::string id);
    ~ProgressBarNode() override;

    ProgressBarNode(const ProgressBarNode&) = delete;
    auto operator=(const ProgressBarNode&) -> ProgressBarNode& = delete;
    ProgressBarNode(ProgressBarNode&&) = delete;
    auto operator=(ProgressBarNode&&) -> ProgressBarNode& = delete;

    void SetValue(int value);
    [[nodiscard]] auto Value() const -> int;

    void SetRange(int minimum, int maximum);
    [[nodiscard]] auto Minimum() const -> int;
    [[nodiscard]] auto Maximum() const -> int;

    void SetIndeterminate(bool indeterminate);
    [[nodiscard]] auto IsIndeterminate() const -> bool;

protected:
    auto CreateWidget(QWidget* parent) -> QWidget* override;
};

} // namespace matcha::fw
