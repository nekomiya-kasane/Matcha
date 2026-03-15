#pragma once

/**
 * @file TitleBarNode.h
 * @brief Abstract base UiNode for window title bars.
 *
 * Provides a common interface for all title bar types (Main, Floating).
 * Subclasses wrap the concrete NyanWidget and expose type-specific features.
 *
 * @see MainTitleBarNode, FloatingTitleBarNode
 */

#include "Matcha/Tree/UiNode.h"

#include <string>
#include <string_view>

class QWidget;

namespace matcha::fw {

/**
 * @brief Abstract base UiNode for title bars.
 *
 * Common interface: SetTitle / Title / Widget.
 * Subclass for Main (NyanMainTitleBar) and Floating (NyanFloatingTitleBar).
 */
class MATCHA_EXPORT TitleBarNode : public UiNode {
public:
    explicit TitleBarNode(std::string id);
    ~TitleBarNode() override;

    TitleBarNode(const TitleBarNode&)            = delete;
    TitleBarNode& operator=(const TitleBarNode&) = delete;
    TitleBarNode(TitleBarNode&&)                 = delete;
    TitleBarNode& operator=(TitleBarNode&&)      = delete;

    /// @brief Set the title bar display text.
    virtual void SetTitle(std::string_view title) = 0;

    /// @brief Get the title bar display text.
    [[nodiscard]] virtual auto Title() const -> std::string = 0;
};

} // namespace matcha::fw
